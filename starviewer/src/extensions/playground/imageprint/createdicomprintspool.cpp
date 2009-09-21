#include "createdicomprintspool.h"

#include "dcmtk/dcmpstat/dviface.h"
#include "dcmtk/dcmpstat/dvpssp.h"      /* for class DVPSStoredPrint */
#include <dvpshlp.h>

#include <QDir>
#include <QDateTime>
#include "dicomprintjob.h"
#include "dicomprintpage.h"
#include "dicomprinter.h"
#include "image.h"
#include "logging.h"
#include "starviewerapplication.h"

namespace udg
{

QString CreateDicomPrintSpool::createPrintSpool(DicomPrinter dicomPrinter, DicomPrintJob dicomPrintJob, const QString &spoolDirectoryPath)
{
    QDir spoolDir;
    QString storedPrintDcmtkFilePath = spoolDirectoryPath + QDir::separator() + "storedPrint.dcm";

    //TODO: S'ha de fer aqu� ? Comprovem si existeix el directori on s'ha de generar l'spool
    if (!spoolDir.exists(spoolDirectoryPath))
    {
        spoolDir.mkdir(spoolDirectoryPath);
    }

    m_dicomPrintJob = dicomPrintJob;
    m_dicomPrinter = dicomPrinter;

    setBasicFilmBoxAttributes();

    foreach(Image *image, dicomPrintJob.getPrintPage().getImagesToPrint())
    {
        transformImageForPrinting(image, spoolDirectoryPath);
    }

    setImageBoxAttributes();

    createStoredPrintDcmtkFile(storedPrintDcmtkFilePath);

    return storedPrintDcmtkFilePath;
}

void CreateDicomPrintSpool::setBasicFilmBoxAttributes()
{
    /*El constructor del DVPStoredPrint se li ha de passar com a par�metres
        1r El tag (2010,015E) Illumination de la Basic Film Box
        2n El tag (2010,0160) Reflected Ambient Light de la Basic Film Box
        3r AETitle del Starviewer

        Els dos primer par�metres nom�s s'utilitzen si la impressora suporta el Presentation Lut, ara mateix no ho soportem (no est� implementat) per tant se 
        suposa que aquests valors s'ignoraran. De totes maneres se li ha donat aquests valors per defecte 2000 i 10 respectivament perqu� s�n els que utilitza dcmtk i tamb�
        s'ha consultat el dicom conformance de les impressores agfa i kodak i tamb� utiltizen aquests valors per defecte.
     */
    //TODO preguntar perqu� necessita el Illumination i Reflected Ambient Ligth, preguntar si realement s�n aquests tags
    m_storedPrint = new DVPSStoredPrint(2000 ,10 , qPrintable(m_dicomPrinter.getAETitle()));
    m_storedPrint->setDestination(qPrintable(m_dicomPrinter.getAETitle()));//S'ha d'indicar el AETitle de la impressora

    //Indiquem el layout de la placa
    m_storedPrint->setImageDisplayFormat(m_dicomPrintJob.getPrintPage().getFilmLayoutColumns(), m_dicomPrintJob.getPrintPage().getFilmLayoutRows());

    m_storedPrint->setFilmSizeID(qPrintable(m_dicomPrintJob.getPrintPage().getFilmSize()));

    //Interpolaci� que s'aplicar� si s'ha d'escalar o ampliar la imatge perqu� c�piga a la cel�la
    m_storedPrint->setMagnificationType(qPrintable(m_dicomPrintJob.getPrintPage().getMagnificationType()));

    if (m_dicomPrintJob.getPrintPage().getMagnificationType().compare("CUBIC"))
    {
        /*El Smoothing Type, tag 2010,0080 del Basic Film Box, nom�s se li pot donar valor sir el tag Magnification Type 2010,0060 t� com a valor 'CUBIC'
          Especifica el tipus de funci� d'interpollaci� a aplicar*/
        m_storedPrint->setSmoothingType(qPrintable(m_dicomPrintJob.getPrintPage().getSmoothingType()));
    }

    //Densitat del marc que separa les imatges
    m_storedPrint->setBorderDensity(qPrintable(m_dicomPrintJob.getPrintPage().getBorderDensity()));
    //Densitat de les cel�les buides
    m_storedPrint->setEmtpyImageDensity(qPrintable(m_dicomPrintJob.getPrintPage().getEmptyImageDensity()));
    m_storedPrint->setMaxDensity(qPrintable(QString().setNum(m_dicomPrintJob.getPrintPage().getMaxDensity())));
    m_storedPrint->setMinDensity(qPrintable(QString().setNum(m_dicomPrintJob.getPrintPage().getMinDensity())));
    
    if (m_dicomPrintJob.getPrintPage().getFilmOrientation() == "PORTRAIT")
    {
        m_storedPrint->setFilmOrientation(DVPSF_portrait);
    }
    else if (m_dicomPrintJob.getPrintPage().getFilmOrientation() == "LANDSCAPE")
    {
        m_storedPrint->setFilmOrientation(DVPSF_landscape);
    }
    
    m_storedPrint->setTrim(m_dicomPrintJob.getPrintPage().getTrim() ? DVPSH_trim_on : DVPSH_trim_off);
    
    /*Tag Configuration Information (2010,0150) de Basic Film Box no li donem valor ara mateix, aquest camp permet configurar les impressions 
      amb caracter�stiques que no s�n Dicom Conformance, sin� que s�n dependents de al impressora*/
    //m_storedPrint->setConfigurationInformation("");

     /*Tag Requested Resolution ID (2020,0050) de Basic Film Box serveix per especificar amb quina resoluci� s'han d'imprimir les imatges, 
      t� dos valors STANTARD i HIGH.
      No se li assigna valor, perqu� mirant el dicom conformance de varies impressores, la majoria no accepten aquest tag i les que l'accepten 
      nom�s l'accepten amb el valor STANDARD, per tant no s'especifica.
     */ 
    //m_storedPrint->setResolutionID(NULL);

    //TODO: No s� si es guarden al fitxer StoredPrint potser s'han d'especificar en el moment d'imprimir ?
    /* Tag Requested Decimate/Crop Behaviour (2020,0040) de Imagex Box Indica que s'ha de fer si la imatge excedeix el m�xim de p�xels que suporta la cel�la
       Hi ha 3 comportaments :
            - Decimate : Escala la imatge fins que hi c�piga
            - Crop : El centra de la imatge es posa al centra de la cel�la i tot lo que no hi c�piga no s'imprimeix.
            - Fail : La impressora ens ha d'indicar que no ha pogut imprimir

       Aquest tag no s'especifica, perqu� mirant el dicom conformance de les impressores, per defecte tenen ja establert el comportament m�s desitjat que �s el 
       del decimate, per aix� de moment no s'especifica, i la impressora es comportar� com estigui definida per defecte.
     */
    //m_storedPrint->setRequestedDecimateCropBehaviour(DVPSI_decimate); 
}

void CreateDicomPrintSpool::transformImageForPrinting(Image *imageToPrint, const QString &spoolDirectoryPath)
{
    DiDisplayFunction *l_displayFunction[DVPSD_max];
    unsigned long bitmapWidth, bitmapHeight, bitmapSize;
    double pixelAspectRatio;
    void *pixelData;
    DcmFileFormat *imageToPrintDcmFileFormat = NULL;
    DcmDataset *imageToPrintDataset = NULL;

    //TODO:m_presentationState cal que sigui global ? cada vegada li fem un new cal ?
    //TODO: Comprovar que els display function realment al dviface no se li assigni cap valor
    //TODO: DisplayFunction Se li pot donar valor null, no es necessita per la impressora ?

    /*El constructor del m�tode DVPresentationState necessita els seg�ents par�metres
        1r - Llista d'objectes que descriuen les caracter�stiques de la pantalla 

        2n,3r i 4t,5� Indiquen la resoluci� m�nima d'impressi� H/V i la m�xima H/V respectivament, se li donen els valors per defecte de les dcmtk, consultant
        el DICOM Conformance d'algunes impressores, s'ha vist que imprimint una sola imatge amb format STARDARD\1,1 per tamanys del film grans algunes 
        impressores poden imprimir en una resoluci� superior de fins 11000, per� com que dif�cilment tindrem casos en els que s'imprimeixin una sola imatge en films
        grans deixem els valors per defecte de les dcmtk.

        5�,6� - Resoluci� per la previsualitzaci� de la imatge, com que no en farem previsualitzaci� deixem els valors standards.*/
    m_presentationState = new DVPresentationState(OFstatic_cast(DiDisplayFunction **, l_displayFunction), 1024 , 1024 , 8192 , 8192, 256, 256);

    DVPSHelper::loadFileFormat(qPrintable(imageToPrint->getPath()), imageToPrintDcmFileFormat);//Carreguem la imatge que hem d'imprimor
    imageToPrintDataset = imageToPrintDcmFileFormat->getDataset();
    
    /*El 2n par�mete del attach image indica, si el presentation state �s l'amo de la imatge passada per par�metre, per poder destruir l'objecte,
      en aquest cas l'indiquem que no �s l'amo, per poder-lo destruir nosaltres.*/
    m_presentationState->attachImage(imageToPrintDcmFileFormat, false);

    bitmapSize = m_presentationState->getPrintBitmapSize();
    m_presentationState->getPrintBitmapWidthHeight(bitmapWidth, bitmapHeight); 
    pixelAspectRatio = m_presentationState->getPrintBitmapPixelAspectRatio();  	    
    pixelData = new char[bitmapSize];

    /*El 3r par�metre indica si la imatge s'ha de redenritzar amb el presentation LUT invers*/
    m_presentationState->getPrintBitmap(pixelData, bitmapSize, false); 

    //Guardem la imatge a disc
    createHardcopyGrayscaleImage(imageToPrint, pixelData, bitmapWidth, bitmapHeight, pixelAspectRatio, spoolDirectoryPath);
}

void CreateDicomPrintSpool::createHardcopyGrayscaleImage(Image *imageToPrint, const void *pixelData, unsigned long bitmapWidth, unsigned long bitmapHeight, double pixelAspectRatio, const QString &spoolDirectoryPath)
{
    char InstanceUIDOfTransformedImage[70];
    OFString requestedImageSizeAsOFString;
    DcmFileFormat *transformedImageToPrint = new DcmFileFormat();
    DcmDataset *transformedImageDatasetToPrint = transformedImageToPrint->getDataset();
    QString transformedImagePath;

    // write patient module
    m_presentationState->writeHardcopyImageAttributes(*transformedImageDatasetToPrint);
    // write general study and general series module
    m_storedPrint->writeHardcopyImageAttributes(*transformedImageDatasetToPrint);

    // Hardcopy Equipment Module
    transformedImageDatasetToPrint->putAndInsertString(DCM_HardcopyDeviceManufacturer, qPrintable(ApplicationNameString), true);
    transformedImageDatasetToPrint->putAndInsertString(DCM_HardcopyDeviceSoftwareVersion, qPrintable(StarviewerVersionString), true);	

    // General Image Module
    transformedImageDatasetToPrint->putAndInsertString(DCM_InstanceNumber, qPrintable(imageToPrint->getInstanceNumber()));
    transformedImageDatasetToPrint->putAndInsertString(DCM_PatientOrientation, qPrintable(imageToPrint->getPatientOrientation()));
    transformedImageDatasetToPrint->putAndInsertString(DCM_ImageType, "DERIVED\\SECONDARY", true);
    transformedImageDatasetToPrint->putAndInsertString(DCM_DerivationDescription, "Hardcopy rendered using Presentation State");
    	
    // SOP Common Module
    transformedImageDatasetToPrint->putAndInsertString(DCM_SOPClassUID, UID_HardcopyGrayscaleImageStorage);

    dcmGenerateUniqueIdentifier(InstanceUIDOfTransformedImage);    
    transformedImageDatasetToPrint->putAndInsertString(DCM_SOPInstanceUID, InstanceUIDOfTransformedImage);		

    //TODO:Prova de substiuir per m�tode qt ? 
    transformedImageDatasetToPrint->putAndInsertString(DCM_InstanceCreationDate, qPrintable(QDateTime::currentDateTime().toString("yyyyMMdd")));

    //TODO:Prova de substiuir per m�tode qt ? 
    transformedImageDatasetToPrint->putAndInsertString(DCM_InstanceCreationTime, qPrintable(QDateTime::currentDateTime().toString("hhmmss")));

    // Hardcopy Grayscale Image Module
    transformedImageDatasetToPrint->putAndInsertString(DCM_PhotometricInterpretation, "MONOCHROME2");
    transformedImageDatasetToPrint->putAndInsertUint16(DCM_SamplesPerPixel, 1);

    transformedImageDatasetToPrint->putAndInsertUint16(DCM_Rows, OFstatic_cast(Uint16, bitmapWidth));
    transformedImageDatasetToPrint->putAndInsertUint16(DCM_Columns, OFstatic_cast(Uint16, bitmapHeight));
    transformedImageDatasetToPrint->putAndInsertUint16(DCM_BitsAllocated, 16);
    transformedImageDatasetToPrint->putAndInsertUint16(DCM_BitsStored, 12);
    transformedImageDatasetToPrint->putAndInsertUint16(DCM_HighBit, 11);
    transformedImageDatasetToPrint->putAndInsertUint16(DCM_PixelRepresentation, 0);

    if(pixelAspectRatio!=1.0)
    {
        char pixelAspectRatioAsChar[70];

        sprintf(pixelAspectRatioAsChar, "%ld\\%ld", 1000L, OFstatic_cast(long, pixelAspectRatio*1000.0));
        transformedImageDatasetToPrint->putAndInsertString(DCM_PixelAspectRatio, pixelAspectRatioAsChar);
    }

    DcmPolymorphOBOW *pxData = new DcmPolymorphOBOW(DCM_PixelData);

    if(pxData)
    {
        pxData->putUint16Array(OFstatic_cast(Uint16 *, OFconst_cast(void *, pixelData)), OFstatic_cast(unsigned long, bitmapWidth*bitmapHeight));
        transformedImageDatasetToPrint->insert(pxData, OFTrue);
    }

    if(m_presentationState->getPresentationLUT() == DVPSP_table)
    {
        m_presentationState->writePresentationLUTforPrint(*transformedImageDatasetToPrint);	
    }

    //TODO:S'hauria de fer a un altre lloc aquest c�lcul perqu� tamb� s'utilitza a PrintDicomSpool
    transformedImagePath = spoolDirectoryPath + QDir::separator() + InstanceUIDOfTransformedImage + ".dcm";
    OFCondition saveImageCondition = DVPSHelper::saveFileFormat(qPrintable(transformedImagePath), transformedImageToPrint, true);

    m_presentationState->getPrintBitmapRequestedImageSize(requestedImageSizeAsOFString);
    m_storedPrint->addImageBox(qPrintable(m_dicomPrinter.getAETitle()), InstanceUIDOfTransformedImage, requestedImageSizeAsOFString.c_str(), NULL, 
                               m_presentationState->getPresentationLUTData(), m_presentationState->isMonochrome1Image());

    DEBUG_LOG(QString("Imatge Creada %1").arg(InstanceUIDOfTransformedImage));
    DEBUG_LOG(qPrintable(m_dicomPrinter.getAETitle()));

    //TODO faltaran delete dels punters?
}

void CreateDicomPrintSpool::setImageBoxAttributes()
{
    size_t numImages = m_storedPrint->getNumberOfImages();

    for (size_t i = 0; i < numImages; i++)
    {
        //TODO Assegurar que quan no s'han seleccionat aquests par�metres tenen valor empty, podria ser que tinguessin valor NONE, funciona igualment ?
        if (!m_dicomPrintJob.getPrintPage().getPolarity().isEmpty())
        {
            m_storedPrint->setImagePolarity(i, qPrintable(m_dicomPrintJob.getPrintPage().getPolarity()));
        }

        //TODO: El podem especificar a nivell de FilmBox, potser seria millor ja que no ho deixem triar a nivell d'imatge
        if (!m_dicomPrintJob.getPrintPage().getMagnificationType().isEmpty())
        {
            m_storedPrint->setImageMagnificationType(i, qPrintable(m_dicomPrintJob.getPrintPage().getMagnificationType()));
        }

        //TODO: El podem especificar a nivell de FilmBox, potser seria millor ja que no ho deixem triar a nivell d'imatge
        if (!m_dicomPrintJob.getPrintPage().getSmoothingType().isEmpty())
        {
            m_storedPrint->setImageSmoothingType(i, qPrintable(m_dicomPrintJob.getPrintPage().getSmoothingType()));
        }
     
        //TODO: Fa falta especificar-lo si t� valor null?
        m_storedPrint->setImageConfigurationInformation(i, NULL);
    }
}

void CreateDicomPrintSpool::createStoredPrintDcmtkFile(const QString &storedPrintDcmtkFilePath)
{
    DcmFileFormat *fileformat = new DcmFileFormat();
    DcmDataset *dataset	= fileformat->getDataset();	
    char storedPrintInstanceUID[70];

    dcmGenerateUniqueIdentifier(storedPrintInstanceUID);

    m_storedPrint->deleteAnnotations();	
    m_storedPrint->setInstanceUID(storedPrintInstanceUID);	
    OFCondition write = m_storedPrint->write(*dataset, false, OFTrue, OFFalse, OFTrue);

    write = DVPSHelper::saveFileFormat(qPrintable(storedPrintDcmtkFilePath), fileformat, true);
}
}
