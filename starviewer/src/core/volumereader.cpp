#include "volumereader.h"

#include "volumepixeldatareaderitkgdcm.h"
#include "volumepixeldatareadervtkgdcm.h"

#include "volume.h"
#include "image.h"
#include "series.h"
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
        DEBUG_LOG("El volum proporcionat és NUL! Retornem.");
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
                WARN_LOG("No podem carregar els arxius següents perquè no caben a memòria\n" + fileList.join("\n"));
                volume->createNeutralVolume();
                QMessageBox::warning(0, tr("Out of memory"), tr("There's not enough memory to load the Series you requested. Try to close all the opened %1 windows and restart the application and try again. If the problem persists, adding more RAM memory or switching to a 64 bit operating system may solve the problem.").arg(ApplicationNameString));
                break;

            case VolumePixelDataReader::MissingFile:
                // Fem el mateix que en el cas OutOfMemory, canviant el missatge d'error
                volume->createNeutralVolume();
                QMessageBox::warning(0, tr("Missing Files"), tr("%1 could not find the corresponding files for this Series. Maybe they had been removed or are corrupted.").arg(ApplicationNameString));
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

    // TODO De moment, per defecte llegirem amb ITK-GDCM
    // tret que es doni una condició que ho canvïi
    m_suitablePixelDataReader = ITKGDCMPixelDataReader;

    int imageSize[2];
    if (!imageSet.empty())
    {
        imageSize[0] = imageSet.first()->getRows();
        imageSize[1] = imageSet.first()->getColumns();
    }
    
    bool containsDifferentSizeImages = false;
    bool containsColorImages = false;
    bool avoidWrongPixelType = false;
    
    foreach (Image *image, imageSet)
    {
        if (!fileList.contains(image->getPath())) // Evitem afegir més vegades l'arxiu si aquest és multiframe
        {
            fileList << image->getPath();
        }

        // Comprovem que no tingui imatges de diferents mides
        // TODO Aquesta comprovació podria desaparèixer ja que ara fem que les 
        // imatges de diferents mides es guardin en volums diferents, per tant, hauria de
        // ser anòmal i inesperat trobar-nos amb aquest cas. Tot i així ens serveix per evitar que
        // si es donés el cas ens petés el programa
        if (imageSize[0] != image->getRows() || imageSize[1] != image->getColumns())
        {
            DEBUG_LOG("Tenim imatges de diferents mides!");
            containsDifferentSizeImages = true;
        }

        // Comprovem si es tracta d'una imatge a color
        // TODO Caldria diferenciar també els casos en que tenim més d'una imatge de diferents mides i que alguna és de color
        photometricInterpretation = image->getPhotometricInterpretation();
        if (!photometricInterpretation.contains("MONOCHROME"))
        {
            // Si photometric interpretation no és ni MONOCHROME1 ni MONOCHROME2, llavors és algun tipu d'imatge a color:
            // PALETTE COLOR, RGB YBR_FULL, YBR_FULL_422, YBR_PARTIAL_422, YBR_PARTIAL_420, YBR_ICT o YBR_RCT
            containsColorImages = true;
            DEBUG_LOG("Photometric Interpretation: " + photometricInterpretation);
        }
        else if (image->getBitsAllocated() == 16 && image->getBitsStored() == 16 && !image->getSOPInstanceUID().contains("MHDImage"))
        {
            // Aquesta comprovació es fa per evitar casos com el del ticket #1257
            // Com que itkImage sempre s'allotja amb el tipus de pixel signed short int, 
            // quan tenim 16 bits allocated i stored, podria ser que el rang de dades necessités
            // que el tipus de pixel fos unsigned short int (0..65536) perquè la imatge es visualitzi correctament
            
            // Només ho aplicarem si es tracta de modalitats d'imatge "no volumètriques" per acotar el problema
            // i evitar que es produeixi el problema remarcat al ticket #1313
            QStringList supportedModalities;
            supportedModalities << "CR" << "RF" << "DX" << "MG" << "OP" << "US" << "ES" << "NM" << "DT" << "PT" << "XA" << "XC";
            if (supportedModalities.contains(image->getParentSeries()->getModality()))
            {
                avoidWrongPixelType = true;
            }
        }
    }

    if (!containsDifferentSizeImages && containsColorImages)
    {
        // Si conté imatges de color i totes són de la mateixa mida les llegirem amb VTK-GDCM
        m_suitablePixelDataReader = VTKGDCMPixelDataReader;
    }
    else if (avoidWrongPixelType)
    {
        // Com que el reader de vtkGDCM decideix el tipus dinàmicament, allotjarem el tipus de pixel correcte
        m_suitablePixelDataReader = VTKGDCMPixelDataReader;
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

        case VTKGDCMPixelDataReader:
            m_volumePixelDataReader = new VolumePixelDataReaderVTKGDCM(this);
            DEBUG_LOG("Escollim VTK-GDCM per llegir la pixel data del volum");
            break;
    }

    // Connectem les senyals de notificació de progrés
    connect(m_volumePixelDataReader, SIGNAL(progress(int)), SIGNAL(progress(int)));
}

} // End namespace udg
