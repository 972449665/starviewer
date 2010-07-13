#include "volumepixeldatareaderitkgdcm.h"

#include "logging.h"
#include "dicomtagreader.h"

// ITK
#include <itkTileImageFilter.h>
// Qt
#include <QFileInfo>
#include <QDir>

namespace udg {

VolumePixelDataReaderITKGDCM::VolumePixelDataReaderITKGDCM(QObject *parent)
: VolumePixelDataReader(parent)
{
    m_reader = ReaderType::New();
    m_seriesReader = SeriesReaderType::New();

    m_gdcmIO = ImageIOType::New();

    m_itkToVtkFilter = ItkToVtkFilterType::New();
    
    m_progressSignalAdaptor = new itk::QtSignalAdaptor();
    // Connect the adaptor as an observer of a Filter's event
    m_seriesReader->AddObserver(itk::ProgressEvent(), m_progressSignalAdaptor->GetCommand());

    // Connect the adaptor's Signal to the Qt Widget Slot
    connect(m_progressSignalAdaptor, SIGNAL(Signal()), SLOT(slotProgress()));
}

VolumePixelDataReaderITKGDCM::~VolumePixelDataReaderITKGDCM()
{
}

int VolumePixelDataReaderITKGDCM::read(const QStringList &filenames)
{
    int errorCode = NoError;
    if (filenames.isEmpty())
    {
        WARN_LOG("La llista de noms de fitxer per carregar �s buida");
        errorCode = InvalidFileName;
        return errorCode;
    }

    if (filenames.size() > 1)
    {
        // Aix� �s necessari per despr�s poder demanar-li el diccionari de meta-dades i obtenir els tags del DICOM
        // TODO Aquesta crida potser �s innecess�ria, caldria comprovar-ho
        //m_seriesReader->SetImageIO(m_gdcmIO);

        // Convertim la QStringList al format std::vector<std::string> que s'esperen les itk
        std::vector<std::string> stlFilenames;
        for (int i = 0; i < filenames.size(); i++)
        {
            stlFilenames.push_back(filenames.at(i).toStdString());
        }

        m_seriesReader->SetFileNames(stlFilenames);

        try
        {
            m_seriesReader->Update();
        }
        catch (itk::ExceptionObject & e)
        {
            WARN_LOG(QString("Excepci� llegint els arxius del directori [%1] Descripci�: [%2]").arg(QFileInfo(filenames.at(0)).dir().path()).arg(e.GetDescription()));
            // Llegim el missatge d'error per esbrinar de quin error es tracta
            errorCode = identifyErrorMessage(QString(e.GetDescription()) );
        }
        
        switch (errorCode)
        {
            case NoError:
                setData(m_seriesReader->GetOutput());
                break;

            case SizeMismatch:
                // TODO Aix� es podria fer a ::getVtkData o ja est� b� aqu�?
                errorCode = NoError;
                readDifferentSizeImagesIntoOneVolume(filenames);
                emit progress(100);
                break;
        }
    }
    else
    {
        errorCode = readSingleFile(filenames.first());
    }
    
    return errorCode;
}

int VolumePixelDataReaderITKGDCM::readSingleFile(const QString &fileName)
{
    int errorCode = NoError;

    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(qPrintable(fileName));
    emit progress(0);
    try
    {
        reader->Update();
    }
    catch (itk::ExceptionObject & e)
    {
        WARN_LOG(QString("Excepci� llegint l'arxiu [%1] Descripci�: [%2]").arg(fileName).arg(e.GetDescription()));
        // Llegim el missatge d'error per esbrinar de quin error es tracta
        errorCode = identifyErrorMessage(QString(e.GetDescription()));
    }
    catch (std::bad_alloc)
    {
        errorCode = OutOfMemory;
    }

    if (errorCode == NoError)
    {
        setData(reader->GetOutput());
    }
    emit progress(100);
    /*
    if (errorCode == NoError)
    {
        // HACK En els casos que les imatges siguin enhanced, les gdcm no omplen correctament
        // ni l'origen ni l'sapcing x,y i �s per aix� que li assignem l'origen i l'spacing 
        // que hem llegit correctament a Image
        // TODO Quan solucionem correctament el ticket #1166 (actualitzaci� a gdcm 2.0.x) aquesta assignaci� desapareixer�
        if (!m_imageSet.isEmpty())
        {
            reader->GetOutput()->SetOrigin(m_imageSet.first()->getImagePositionPatient());
            // Cal tenir en compte si la imatge original cont� informaci� d'spacing v�lida per fer l'assignaci�
            const double *imageSpacing = m_imageSet.first()->getPixelSpacing();
            if (imageSpacing[0] > 0.0)
            {
                double spacing[3];
                spacing[0] = imageSpacing[0];
                spacing[1] = imageSpacing[1];
                // HACK ticket #1204 - Degut a bugs en les gdcm integrades amb itk, l'spacing between slices no es calcula
                // correctament i se li assigna l'slice thickness al z-spacing. Una soluci� temporal i r�pida �s llegir el tag
                // Spacing Between Slices i actualitzar el z-spacing, si aquest tag existeix
                // El cost de llegir aquest tag per un fitxer de 320 imatges �s d'uns 470 milisegons aproximadament 
                // TODO un cop actualitzats a gdcm 2.0.x, aquest HACK ser� innecessari
                DICOMTagReader *dicomReader = new DICOMTagReader(m_imageSet.first()->getPath());
                double zSpacing = dicomReader->getValueAttributeAsQString(DICOMSpacingBetweenSlices).toDouble();
                if ( zSpacing == 0.0 )
                    zSpacing = reader->GetOutput()->GetSpacing()[2];
                
                spacing[2] = zSpacing;                
                reader->GetOutput()->SetSpacing(spacing);
            }
        }
        
        setData(reader->GetOutput());
        // Emetem progress 100, perqu� el corresponent di�leg de progr�s es tanqui
        emit progress(100);
    }
    */
    // TODO Falta tractament d'errors!?
    return errorCode;
}

void VolumePixelDataReaderITKGDCM::readDifferentSizeImagesIntoOneVolume(const QStringList &filenames)
{
    int errorCode = NoError;
    // Declarem el filtre de tiling
    typedef itk::TileImageFilter<Volume::ItkImageType, Volume::ItkImageType> TileFilterType;
    TileFilterType::Pointer tileFilter = TileFilterType::New();
    // Inicialitzem les seves variables
    // El layout ens serveix per indicar cap on creix la cua. En aquest cas volem fer creixer la coordenada Z
    TileFilterType::LayoutArrayType layout;
    layout[0] = 1;
    layout[1] = 1;
    layout[2] = 0;
    tileFilter->SetLayout(layout);

    int progressCount = 0;
    int progressIncrement = static_cast<int>((1.0 / (double)filenames.count()) * 100);

    m_reader->SetImageIO(m_gdcmIO);
    foreach (const QString &file, filenames)
    {
        emit progress(progressCount);
        // Declarem la imatge que volem carregar
        Volume::ItkImageTypePointer itkImage;
        m_reader->SetFileName(qPrintable(file));
        try
        {
            m_reader->UpdateLargestPossibleRegion();
        }
        catch (itk::ExceptionObject & e)
        {
            WARN_LOG(QString("Excepci� llegint els arxius del directori [%1] Descripci�: [%2]").arg(QFileInfo(filenames.at(0)).dir().path()).arg(e.GetDescription()));

            // Llegim el missatge d'error per esbrinar de quin error es tracta
            errorCode = identifyErrorMessage(QString(e.GetDescription()));
        }
        if (errorCode == NoError)
        {
            itkImage = m_reader->GetOutput();
            m_reader->GetOutput()->DisconnectPipeline();
        }
        // TODO No es fa tractament d'errors!

        // Un cop llegit el block, fem el tiling
        tileFilter->PushBackInput(itkImage);
        progressCount += progressIncrement;
    }
    tileFilter->Update();
    setData(tileFilter->GetOutput());
    emit progress(100);
}

int VolumePixelDataReaderITKGDCM::identifyErrorMessage(const QString &errorMessage)
{
    if (errorMessage.contains("Size mismatch") || errorMessage.contains("ImageIO returns IO region that does not fully contain the requested regionRequested"))
    {
        return SizeMismatch;
    }
    else if (errorMessage.contains("Failed to allocate memory for image"))
    {
        return OutOfMemory;
    }
    else if (errorMessage.contains("The file doesn't exists"))
    {
        return MissingFile;
    }
    else if (errorMessage.contains("A spacing of 0 is not allowed"))
    {
        /*
        DEBUG_LOG("spacing of 0 not allowed!!!!");
        foreach (Image *image, m_imageSet)
        {
            DICOMTagReader *dicomReader = new DICOMTagReader(image->getPath());
            DEBUG_LOG("Spacing between slices is: " + dicomReader->getValueAttributeAsQString(DICOMSpacingBetweenSlices));
            DEBUG_LOG(QString("Slice thickness: %1").arg(image->getSliceThickness()));
        }
        */
        return ZeroSpacingNotAllowed;
    }
    else
    {
        return UnknownError;
    }
}

void VolumePixelDataReaderITKGDCM::setData(Volume::ItkImageTypePointer itkImage)
{
    // Li donem l'input al filtre i fem la conversi�
    m_itkToVtkFilter->SetInput(itkImage);
    try
    {
        m_itkToVtkFilter->Update();
    }
    catch (itk::ExceptionObject & exception)
    {
        WARN_LOG(QString("Excepci� en el filtre itkToVtk :: VolumePixelDataReaderITKGDCM::setData(ItkImageTypePointer itkImage) -> ") + exception.GetDescription() );
    }
    // Assignem l'output
    m_vtkImageData = m_itkToVtkFilter->GetOutput();
    //m_itkToVtkFilter->Delete();
}

void VolumePixelDataReaderITKGDCM::slotProgress()
{
    emit progress((int)(m_seriesReader->GetProgress()*100));
}

} // End namespace udg
