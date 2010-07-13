#include "volumereader.h"

#include "volumepixeldatareaderitkgdcm.h"
#ifdef VTK_GDCM_SUPPORT
#include "volumepixeldatareadervtkgdcm.h"
#endif

#include "volume.h"
#include "image.h"
#include "logging.h"
#include "starviewerapplication.h"

#include <QMessageBox>

namespace udg {

VolumeReader::VolumeReader(QObject *parent)
: QObject(parent), m_volumePixelDataReader(0), m_suitablePixelDataReader(ITKGDCMPixelDataReader)
{
}

VolumeReader::~VolumeReader()
{
    if (m_volumePixelDataReader)
    {
        delete m_volumePixelDataReader;
    }
}

void VolumeReader::read(Volume *volume)
{
    if (!volume)
    {
        DEBUG_LOG("El volum proporcionat �s NUL! Retornem.");
        return;
    }    
    
    // Obtenim els arxius que hem de llegir
    QStringList fileList = chooseFilesAndSuitableReader(volume);
    if (!fileList.isEmpty())
    {
        // Posem a punt el reader i llegim les dades
        setUpReader();
        switch (m_volumePixelDataReader->read(fileList))
        {
            case VolumePixelDataReader::OutOfMemory: 
                WARN_LOG("No podem carregar els arxius seg�ents perqu� no caben a mem�ria\n" + fileList.join("\n"));
                volume->createNeutralVolume();
                QMessageBox::warning(0, tr("Out of memory"), tr("There's not enough memory to load the Series you requested. Try to close all the opened %1 windows and restart the application and try again. If the problem persists, adding more RAM memory or switching to a 64 bit operating system may solve the problem.").arg(ApplicationNameString));
                break;

            case VolumePixelDataReader::MissingFile:
                // Fem el mateix que en el cas OutOfMemory, canviant el missatge d'error
                volume->createNeutralVolume();
                QMessageBox::warning(0, tr("Missing Files"), tr("%1 could not find the corresponding files for this Series. Maybe they had been removed or are corrupted.").arg(ApplicationNameString));
                break;

            case VolumePixelDataReader::ZeroSpacingNotAllowed:
                // S'ha trobat que el z-spacing �s de 0 TODO De moment carreguem un volum neutre per� cal oferir una millor soluci�: Ticket #1245
                volume->createNeutralVolume();
                QMessageBox::warning(0, tr("Zero-Spacing Error"), tr("%1 could not load the images because there is some image information missing. No series data has been loaded.").arg(ApplicationNameString));
                break;
            
            case VolumePixelDataReader::UnknownError:
                // Hi ha hagut un error no controlat, creem el volum neutral per evitar desastres majors
                volume->createNeutralVolume();
                QMessageBox::warning(0, tr("Unkwown Error"), tr("%1 found an unexpected error reading this Series. No Series data has been loaded.").arg(ApplicationNameString));
                break;

            case VolumePixelDataReader::NoError:
                // Tot ha anat ok, assignem les dades al volum
                volume->setData(m_volumePixelDataReader->getVTKData());
                break;
        }
    }
}

const QStringList VolumeReader::chooseFilesAndSuitableReader(Volume *volume)
{
    QString photometricInterpretation;
    QStringList fileList;
    QList<Image *> imageSet = volume->getImages();

    // TODO De moment sempre llegirem amb ITKGDCM
    m_suitablePixelDataReader = ITKGDCMPixelDataReader;

    foreach (Image *image, imageSet)
    {
        if (!fileList.contains(image->getPath())) // Evitem afegir m�s vegades l'arxiu si aquest �s multiframe
            fileList << image->getPath();

#ifdef VTK_GDCM_SUPPORT
        // Comprovem si es tracta d'una imatge a color
        // TODO Caldria diferenciar tamb� els casos en que tenim m�s d'una imatge de diferents mides i que alguna �s de color
        photometricInterpretation = image->getPhotometricInterpretation();
        if (photometricInterpretation == "PALETTE COLOR" || photometricInterpretation == "RGB" || photometricInterpretation == "YBR_FULL" || photometricInterpretation == "YBR_FULL_422" || photometricInterpretation == "YBR_PARTIAL_422" || photometricInterpretation == "YBR_PARTIAL_420" || photometricInterpretation == "YBR_ICT" || photometricInterpretation == "YBR_RCT")
        {
            m_suitablePixelDataReader = VTKGDCMPixelDataReader;
            DEBUG_LOG("Photometric Interpretation: " + photometricInterpretation);
        }
#endif
    }

    return fileList;
}

void VolumeReader::setUpReader()
{
    // Eliminem un lector anterior si l'havia
    if (m_volumePixelDataReader)
    {
        delete m_volumePixelDataReader;
    }

    // Segons quin pixel data reader estigui seleccionat, crearem l'objecte que toqui
    switch (m_suitablePixelDataReader)
    {
        case ITKGDCMPixelDataReader:
            m_volumePixelDataReader = new VolumePixelDataReaderITKGDCM(this);
            DEBUG_LOG("Escollim ITK-GDCM per llegir la pixel data del volum");
            break;
#ifdef VTK_GDCM_SUPPORT
        case VTKGDCMPixelDataReader:
            m_volumePixelDataReader = new VolumePixelDataReaderVTKGDCM(this);
            DEBUG_LOG("Escollim VTK-GDCM per llegir la pixel data del volum");
            break;
#endif
    }

    // Connectem les senyals de notificaci� de progr�s
    connect(m_volumePixelDataReader, SIGNAL(progress(int)), SIGNAL(progress(int)));
}

} // End namespace udg
