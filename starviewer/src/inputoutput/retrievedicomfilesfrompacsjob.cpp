#include "retrievedicomfilesfrompacsjob.h"

#include <QtGlobal>
#include <QThread>

#include "logging.h"
#include "patient.h"
#include "study.h"
#include "series.h"
#include "image.h"
#include "starviewerapplication.h"
#include "retrievedicomfilesfrompacs.h"
#include "dicommask.h"
#include "localdatabasemanager.h"
#include "patientfiller.h"
#include "deletedirectory.h"
#include "utils.h"
#include "harddiskinformation.h"
#include "inputoutputsettings.h"
#include "dicomtagreader.h"

namespace udg
{
RetrieveDICOMFilesFromPACSJob::RetrieveDICOMFilesFromPACSJob(PacsDevice pacsDevice, Study *studyToRetrieveDICOMFiles, DicomMask dicomMaskToRetrieve, RetrievePriorityJob retrievePriorityJob):PACSJob(pacsDevice)
{
    Q_ASSERT(studyToRetrieveDICOMFiles);
    Q_ASSERT(studyToRetrieveDICOMFiles->getParentPatient());

    m_retrieveDICOMFilesFromPACS = new RetrieveDICOMFilesFromPACS(getPacsDevice());
    m_studyToRetrieveDICOMFiles = studyToRetrieveDICOMFiles;
    m_dicomMaskToRetrieve = dicomMaskToRetrieve;
    m_retrievePriorityJob = retrievePriorityJob;
}

RetrieveDICOMFilesFromPACSJob::~RetrieveDICOMFilesFromPACSJob()
{
    delete m_retrieveDICOMFilesFromPACS;
}

PACSJob::PACSJobType RetrieveDICOMFilesFromPACSJob::getPACSJobType()
{
    return PACSJob::RetrieveDICOMFilesFromPACSJobType;
}

Study* RetrieveDICOMFilesFromPACSJob::getStudyToRetrieveDICOMFiles()
{
    return m_studyToRetrieveDICOMFiles;
}

void RetrieveDICOMFilesFromPACSJob::run()
{
    Settings settings;
    //TODO: �s aquest el lloc per aquest missatge ? no seria potser millor fer-ho a RetrieveDICOMFilesFromPACS
    INFO_LOG(QString("Iniciant descarrega del PACS %1, IP: %2, Port: %3, AETitle Local: %4 Port local: %5, l'estudi UID: %6, series UID: %7, SOP Instance UID:%8")
        .arg(getPacsDevice().getAETitle(), getPacsDevice().getAddress(), QString::number(getPacsDevice().getQueryRetrieveServicePort()))
        .arg(settings.getValue(InputOutputSettings::LocalAETitle).toString(), settings.getValue(InputOutputSettings::IncomingDICOMConnectionsPort).toString())
        .arg(m_dicomMaskToRetrieve.getStudyInstanceUID(), m_dicomMaskToRetrieve.getSeriesInstanceUID(), m_dicomMaskToRetrieve.getSOPInstanceUID()));

    m_numberOfSeriesRetrieved = 0;
    m_lastImageSeriesInstanceUID = "";
    m_retrievedSeriesInstanceUID.clear();

    m_retrieveRequestStatus = thereIsAvailableSpaceOnHardDisk();

    if (m_retrieveRequestStatus != PACSRequestStatus::RetrieveOk)
    {
        return;
    }

    int localPort = settings.getValue(InputOutputSettings::IncomingDICOMConnectionsPort).toInt();

    if (Utils::isPortInUse(localPort))
    {
        m_retrieveRequestStatus = PACSRequestStatus::RetrieveIncomingDICOMConnectionsPortInUse;
        ERROR_LOG("El port " + QString::number(localPort) + " per a connexions entrants del PACS, esta en us, no es pot descarregar l'estudi");
    }
    else
    {
        PatientFiller patientFiller;
        QThread fillersThread;
        patientFiller.moveToThread(&fillersThread);
        LocalDatabaseManager localDatabaseManager;

        /*S'ha d'especificar com a DirectConnection, perqu� sin� aquest signal l'aten qui ha creat el Job, que �s la interf�cie, per tant
          no s'atendria fins que la interf�cie estigui lliure, provocant comportaments incorrectes*/
        connect(m_retrieveDICOMFilesFromPACS, SIGNAL(DICOMFileRetrieved(DICOMTagReader*, int)), this, SLOT(DICOMFileRetrieved(DICOMTagReader*, int)), Qt::DirectConnection);
        //Connectem amb els signals del patientFiller per processar els fitxers descarregats
        connect(this, SIGNAL(DICOMTagReaderReadyForProcess(DICOMTagReader *)), &patientFiller, SLOT(processDICOMFile(DICOMTagReader *)));
        connect(this, SIGNAL(DICOMFilesRetrieveFinished()), &patientFiller, SLOT(finishDICOMFilesProcess()));
        /*Connexi� entre el processat dels fitxers DICOM i l'inserci� al a BD, �s important que aquest signal sigui un Qt:DirectConnection perqu� aix� el 
          el processa els thread dels fillers, d'aquesta manera el thread de descarrega que est� esperant a fillersThread.wait() quan surt 
          d'aqu� perqu� els fillers ja han acabat ja s'ha inserit el pacient a la base de dades.*/
        connect(&patientFiller, SIGNAL(patientProcessed(Patient *)), &localDatabaseManager, SLOT(save(Patient *)), Qt::DirectConnection);
        //Connexions per finalitzar els threads
        connect(&patientFiller, SIGNAL(patientProcessed(Patient *)), &fillersThread, SLOT(quit()), Qt::DirectConnection);

        localDatabaseManager.setStudyRetrieving(m_dicomMaskToRetrieve.getStudyInstanceUID());
        fillersThread.start();
        
        m_retrieveRequestStatus = m_retrieveDICOMFilesFromPACS->retrieve(m_dicomMaskToRetrieve);

        if ((m_retrieveRequestStatus == PACSRequestStatus::RetrieveOk || m_retrieveRequestStatus == PACSRequestStatus::RetrieveSomeDICOMFilesFailed) &&
            !this->isAbortRequested())
        {
            INFO_LOG(QString("Ha finalitzat la descarrega de l'estudi %1 del PACS %2, s'han descarregat %3 fitxers").
                arg(m_dicomMaskToRetrieve.getStudyInstanceUID(),getPacsDevice().getAETitle()).arg(m_retrieveDICOMFilesFromPACS->getNumberOfDICOMFilesRetrieved()));

            m_numberOfSeriesRetrieved++;
            emit DICOMSeriesRetrieved(this, m_numberOfSeriesRetrieved);//Indiquem que s'ha descarregat la �ltima s�rie
            emit DICOMFilesRetrieveFinished(); 

            //Esperem que el processat i l'insersi� a la base de dades acabin
            fillersThread.wait();

            if (localDatabaseManager.getLastError() != LocalDatabaseManager::Ok)
            {
                if (localDatabaseManager.getLastError() == LocalDatabaseManager::PatientInconsistent)
                {
                    //No s'ha pogut inserir el patient, perqu� patientfiller no ha pogut emplenar l'informaci� de patient correctament
                    m_retrieveRequestStatus = PACSRequestStatus::RetrievePatientInconsistent;
                }
                else 
                {
                    m_retrieveRequestStatus = PACSRequestStatus::RetrieveDatabaseError;
                }
            }
        }
        else
        {
            fillersThread.quit();
            /*Esperem que el thread acabi, ja que pel que s'interpreta de la documentaci�, sembla que el quit del thread no es fa fins que aquest retorna
              al eventLoop, aix� provoca per exemple en els casos que ens han cancel�lat la desc�rrega d'un estudi, si no esperem al thread que estigui mort 
              poguem esborrar imatges que els fillers estan processant en aquell moment mentre encara s'estan executant i peti l'Starviewer, perqu� no s'ha at�s el 
              Slot quit del thread, per aix� esperem que aquest estigui mort a esborrar les imatges descarregades.*/
            fillersThread.wait();
            deleteRetrievedDICOMFilesIfStudyNotExistInDatabase();
        }

        localDatabaseManager.setStudyRetrieveFinished();
    }
}

void RetrieveDICOMFilesFromPACSJob::requestCancelJob()
{
    INFO_LOG(QString("S'ha demanat la cancel.lacio del Job de descarrega d'imatges de l'estudi %1 del PACS %2").arg(getStudyToRetrieveDICOMFiles()->getInstanceUID(),
        getPacsDevice().getAETitle()));
    m_retrieveDICOMFilesFromPACS->requestCancel();
}

PACSRequestStatus::RetrieveRequestStatus RetrieveDICOMFilesFromPACSJob::getStatus()
{
    return m_retrieveRequestStatus;
}

void RetrieveDICOMFilesFromPACSJob::DICOMFileRetrieved(DICOMTagReader *dicomTagReader, int numberOfImagesRetrieved)
{
    emit DICOMFileRetrieved(this, numberOfImagesRetrieved);

    QString seriesInstancedUIDRetrievedImage = dicomTagReader->getValueAttributeAsQString(DICOMSeriesInstanceUID);

    /**Comprovem si hem descarregat una nova s�rie. Degut a que pot ser que el PACS ens envi� les imatges desornades (que no estiguin agrupades per s�rie-9
       hem de comprovar que quan detectem un canvi de s�rie respecte l'anterior imatge descarregada, aquesta no s'hagi ja comptabilitzat. Som conscients
       que podem donar informaci� falsejada a l'usuari, per� es fa per donar-lo m�s feedback amb la desc�rrega d'imatges.*/
    if (seriesInstancedUIDRetrievedImage != m_lastImageSeriesInstanceUID && !m_lastImageSeriesInstanceUID.isEmpty())
    {
        if (!m_retrievedSeriesInstanceUID.contains(seriesInstancedUIDRetrievedImage))
        {
            m_numberOfSeriesRetrieved++;
            emit DICOMSeriesRetrieved(this, m_numberOfSeriesRetrieved);

            m_retrievedSeriesInstanceUID.append(seriesInstancedUIDRetrievedImage);
        }
    }

    
    /*Fem un emit indicat que dicomTagReader est� a punt per ser processat per l'Slot processDICOMFile de PatientFiller, no podem fer un connect
      directament entre el signal de DICOMFileRetrieved de RetrieveDICOMFileFromPACS i processDICOMFile de PatientFiller, perqu� ens podr�em trobar
      que quan en aquest m�tode comprova si hem descarregat una nova s�rie que DICOMTagReader ja no tingui valor, per aix� primer capturem el signal de 
      RetrieveDICOMFileFromPACS comprovem si �s una s�rie nova la que es descarrega i llavors fem l'emit per que PatientFiller processi el DICOMTagReader*/

    emit DICOMTagReaderReadyForProcess(dicomTagReader);

    m_lastImageSeriesInstanceUID = seriesInstancedUIDRetrievedImage;
}

int RetrieveDICOMFilesFromPACSJob::priority() const
{
    return m_retrievePriorityJob;
}


PACSRequestStatus::RetrieveRequestStatus RetrieveDICOMFilesFromPACSJob::thereIsAvailableSpaceOnHardDisk()
{
    LocalDatabaseManager localDatabaseManager;
    /*TODO: Aquest signal no s'hauria de fer des d'aquesta classe sin� des d'una CacheManager, per� com de moment encara no est� implementada 
            temporalment emetem el signal des d'aqu�*/
    connect(&localDatabaseManager, SIGNAL(studyWillBeDeleted(QString)), SIGNAL(studyFromCacheWillBeDeleted(QString)));
   
    if (!localDatabaseManager.thereIsAvailableSpaceOnHardDisk())
    {
        if (localDatabaseManager.getLastError() == LocalDatabaseManager::Ok) //si no hi ha prou espai emitim aquest signal
        {
            return PACSRequestStatus::RetrieveNoEnoughSpace;
        }
        else
        {
            return PACSRequestStatus::RetrieveErrorFreeingSpace;
        }
    }

    return PACSRequestStatus::RetrieveOk;
}

void RetrieveDICOMFilesFromPACSJob::deleteRetrievedDICOMFilesIfStudyNotExistInDatabase()
{
    /*Comprovem si l'estudi est� inserit a la base de dades, si �s aix� vol dir que anteriorment s'havia descarregat un part o tot l'estudi,
     *com que ja tenim altres elements d'aquest estudi inserits a la base de dades no esborrem el directori de l'estudi*/
    if (!existStudyInLocalDatabase(m_dicomMaskToRetrieve.getStudyInstanceUID()))
    {
        /*Si l'estudi no existeix a la base de dades esborrem el contingut del directori, en principi segons la normativa DICO; si rebem un status de 
         * tipus error per part de MoveSCP indicaria s'ha pogut descarregar cap objecte dicom amb �xit */

        INFO_LOG("L'estudi " + m_dicomMaskToRetrieve.getStudyInstanceUID() + " no existeix a la base de de dades, esborrem el contingut del seu directori.");
        DeleteDirectory().deleteDirectory(LocalDatabaseManager().getStudyPath(m_dicomMaskToRetrieve.getStudyInstanceUID()), true);
    }
    else
    {
        INFO_LOG("L'estudi " + m_dicomMaskToRetrieve.getStudyInstanceUID() + " existeix a la base de dades, no esborrem el contingut del seu directori.");
    }
}

bool RetrieveDICOMFilesFromPACSJob::existStudyInLocalDatabase(QString studyInstanceUID)
{
    LocalDatabaseManager localDatabaseManager;
    DicomMask dicomMask;

    dicomMask.setStudyInstanceUID(studyInstanceUID);

    return localDatabaseManager.queryStudy(dicomMask).count() > 0;
}

/*TODO:Centralitzem la contrucci� dels missatges d'error perqu� a totes les interf�cies en puguin utilitzar un, i no calgui tenir el tractament d'errors duplicat
       ni traduccions, per� �s el millor lloc aqu� posar aquest codi? */
QString RetrieveDICOMFilesFromPACSJob::getStatusDescription()
{
    QString message;
    QString studyID = getStudyToRetrieveDICOMFiles()->getID();
    QString patientName = getStudyToRetrieveDICOMFiles()->getParentPatient()->getFullName();
    QString pacsAETitle = getPacsDevice().getAETitle();
    Settings settings;

    switch (getStatus())
    {
        case PACSRequestStatus::RetrieveOk:
            message = tr("Images from study %1 of patient %2 has been retrieved succesfully from PACS %3.").arg(studyID, patientName, pacsAETitle);
            break;
        case PACSRequestStatus::RetrieveCancelled:
            message = tr("Retrieve images from study %1 of patient %2 from PACS %3 has been cancelled.").arg(studyID, patientName, pacsAETitle);
            break;
        case PACSRequestStatus::RetrieveCanNotConnectToPACS :
            message = tr("%1 can't connect to PACS %2 trying to retrieve images from study %3 of patient %4.\n")
                .arg(ApplicationNameString, pacsAETitle, studyID,patientName);
            message += tr("\nBe sure that your computer is connected on network and the PACS parameters are correct.");
            message += tr("\nIf the problem persists contact with an administrator.");
            break;
        case PACSRequestStatus::RetrieveNoEnoughSpace:
            {
                Settings settings;
                HardDiskInformation hardDiskInformation;
                quint64 freeSpaceInHardDisk = hardDiskInformation.getNumberOfFreeMBytes(LocalDatabaseManager::getCachePath());
                quint64 minimumSpaceRequired = quint64(settings.getValue(InputOutputSettings::MinimumFreeGigaBytesForCache).toULongLong() * 1024);
                message = tr("There is not enough space to retrieve images of study %1 from patient %2, please free space or change your Local Database settings.").arg(
                    studyID, patientName);
                message += tr("\n\nAvailable space in Disk: %1 Mb").arg(freeSpaceInHardDisk);
                message += tr("\nMinimum space required in Disk to retrieve studies: %1 Mb").arg(minimumSpaceRequired);
            }
            break;
        case PACSRequestStatus::RetrieveErrorFreeingSpace :
            message = tr("An error ocurred freeing space on harddisk, the images from study %1 of patient %2 won't be retrieved.").arg(studyID, patientName);
            message += tr("\n\nClose all %1 windows and try again."
                         "\nIf the problem persists contact with an administrator.").arg(ApplicationNameString);
            break;
        case PACSRequestStatus::RetrieveDatabaseError :
            message = tr("Images from study %1 of patient %2 can't be retrieved because a database error ocurred.").arg(studyID, patientName);
            message += tr("\n\nClose all %1 windows and try again."
                         "\nIf the problem persists contact with an administrator.").arg(ApplicationNameString);
            break;
        case PACSRequestStatus::RetrievePatientInconsistent :
            message = tr("Images from study %1 of patient %2 can't be retrieved from PACS %3 because %4 has not be capable of read correctly data images.")
                .arg(studyID, patientName, pacsAETitle, ApplicationNameString);
            message += tr("\n\nThe study may be corrupted, if It is not corrupted please contact with %1 team.").arg(ApplicationNameString);
            break;
        case PACSRequestStatus::RetrieveDestinationAETileUnknown:
            message = tr("Images from study %1 of patient %2 can't be retrieved because PACS %3 doesn't recognize your computer's AETitle %4.")
                    .arg(studyID, patientName, pacsAETitle, settings.getValue(InputOutputSettings::LocalAETitle).toString());
            message += tr("\n\nContact with an administrador to register your computer to the PACS.");
            break;
        case PACSRequestStatus::RetrieveUnknowStatus:
        case PACSRequestStatus::RetrieveFailureOrRefused:
            message = tr("Images from study %1 of patient %2 can't be retrieved because PACS %3 doesn't respond as expected.\n\n")
                .arg(studyID, patientName, pacsAETitle);
            message += tr("The cause of the error can be that the requested images are corrupted or the incoming connections port in PACS configuration is not correct.");
            break;
        case PACSRequestStatus::RetrieveIncomingDICOMConnectionsPortInUse :
            message = tr("Images from study %1 of patient %2 can't be retrieved because port %3 for incoming connections from PACS is already in use by another application.")
                .arg(studyID, patientName, settings.getValue(InputOutputSettings::IncomingDICOMConnectionsPort).toString());
            break;
        case PACSRequestStatus::RetrieveSomeDICOMFilesFailed:
            message = tr("%1 can't retrieve all images from study %2 of patient %3 from PACS %4. May be those images are missing or corrupted in PACS.")
                .arg(ApplicationNameString, studyID, patientName, pacsAETitle);
            message += "\n";
            break;
        default:
            message = tr("An unknown error has ocurred and %1 can't retrieve images from study %2 of patient %3 from PACS %4.")
                .arg(ApplicationNameString, studyID, patientName, pacsAETitle);
            message += tr("\n\nClose all %1 windows and try again."
                         "\nIf the problem persists contact with an administrator.").arg(ApplicationNameString);
            break;
    }

    return message;
}

};
