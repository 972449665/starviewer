/***************************************************************************
 *   Copyright (C) 2005 by Grup de Gràfics de Girona                       *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#include "extensionhandler.h"

// qt
#include <QFileInfo>
#include <QDir>
#include <QProgressDialog>
#include <QMessageBox>
// recursos
#include "logging.h"
#include "extensionworkspace.h"
#include "qapplicationmainwindow.h"
#include "volumerepository.h" // per esborrar els volums
#include "extensionmediatorfactory.h"
#include "extensionfactory.h"
#include "extensioncontext.h"
#include "singleton.h"
#include "starviewerapplication.h"

// Espai reservat pels include de les mini-apps
#include "appimportfile.h"

// Fi de l'espai reservat pels include de les mini-apps

// PACS --------------------------------------------
#include "queryscreen.h"
#include "patientfiller.h"
#include "patientfillerinput.h"
#include "mhdfileclassifierstep.h"

namespace udg {

typedef SingletonPointer<QueryScreen> QueryScreenSingleton;

ExtensionHandler::ExtensionHandler( QApplicationMainWindow *mainApp , QObject *parent, QString name)
 : QObject(parent )
{
    this->setObjectName( name );
    m_mainApp = mainApp;

    // Aquí en principi només farem l'inicialització
    m_importFileApp = new AppImportFile;

    createConnections();

    //TODO:xapussa per a que l'starviewer escolti les peticions del RIS, com que tot el codi d'escoltar les peticions del ris està a la 
    //queryscreen l'hem d'instanciar ja a l'inici perquè ja escolti les peticions
    QueryScreenSingleton::instance();
    connect(QueryScreenSingleton::instance(), SIGNAL( selectedPatients(QList<Patient *>) ), SLOT(processInput(QList<Patient*>)));
}

ExtensionHandler::~ExtensionHandler()
{
    // Si és la última finestra oberta, hem de tancar la queryscreen
    if (m_mainApp->getCountQApplicationMainWindow() == 1)
    {
        delete QueryScreenSingleton::instance();
    }
}

void ExtensionHandler::request( int who )
{
    // \TODO: crear l'extensió amb el factory ::createExtension, no com està ara
    // \TODO la numeració és completament temporal!!! s'haurà de canviar aquest sistema
    STAT_LOG("Activated open " + QString::number(who));
    switch( who )
    {
        case 1:
            m_importFileApp->open();
            break;

        case 6:
            m_importFileApp->openDirectory();
            break;

        case 7:
            // HACK degut a que la QueryScreen és un singleton, això provoca efectes colaterals quan teníem
            // dues finestres ( mirar ticket #542 ). Fem aquest petit hack perquè això no passi.
            // Queda pendent resoldre-ho de la forma adequada
            disconnect(QueryScreenSingleton::instance(), SIGNAL( selectedPatients(QList<Patient *>) ), 0, 0 );
            QueryScreenSingleton::instance()->bringToFront();
            connect(QueryScreenSingleton::instance(), SIGNAL( selectedPatients(QList<Patient *>) ), SLOT(processInput(QList<Patient*>)));
            break;

        case 8:
            // HACK degut a que la QueryScreen és un singleton, això provoca efectes colaterals quan teníem
            // dues finestres ( mirar ticket #542 ). Fem aquest petit hack perquè això no passi.
            // Queda pendent resoldre-ho de la forma adequada
            disconnect(QueryScreenSingleton::instance(), SIGNAL( selectedPatients(QList<Patient *>) ), 0, 0 );
            QueryScreenSingleton::instance()->openDicomdir();
            connect(QueryScreenSingleton::instance(), SIGNAL( selectedPatients(QList<Patient *>) ), SLOT(processInput(QList<Patient*>)));
            break;
    }
}

void ExtensionHandler::request( const QString &who )
{
    QWidget *extension = ExtensionFactory::instance()->create(who);
    ExtensionMediator* mediator = ExtensionMediatorFactory::instance()->create(who);

    if (mediator && extension)
    {
        STAT_LOG("Activated extension " + who);
        mediator->initializeExtension(extension, m_extensionContext );
        m_mainApp->getExtensionWorkspace()->addApplication(extension, mediator->getExtensionID().getLabel() );
		delete mediator;
    }
    else
    {
        DEBUG_LOG( "Error carregant " + who );
    }
}

void ExtensionHandler::killBill()
{
    // eliminem totes les extensions
    m_mainApp->getExtensionWorkspace()->killThemAll();
    // TODO descarregar tots els volums que tingui el pacient en aquesta finestra
    // quan ens destruim alliberem tots els volums que hi hagi a memòria
    if (m_mainApp->getCurrentPatient() != NULL)
    {
        foreach( Study *study, m_mainApp->getCurrentPatient()->getStudies() )
        {
            foreach( Series *series, study->getSeries() )
            {
                foreach( Identifier id, series->getVolumesIDList()  )
                {
                    VolumeRepository::getRepository()->removeVolume( id );
                }
            }
        }
    }
}

void ExtensionHandler::setContext( const ExtensionContext &context )
{
    m_extensionContext = context;
}

ExtensionContext &ExtensionHandler::getContext()
{
    return m_extensionContext;
}

void ExtensionHandler::updateConfiguration(const QString &configuration)
{
    QueryScreenSingleton::instance()->updateConfiguration(configuration);
}

void ExtensionHandler::createConnections()
{
    connect( m_importFileApp,SIGNAL( selectedFiles(QStringList) ), SLOT(processInput(QStringList) ) );
}

void ExtensionHandler::processInput(QStringList inputFiles)
{
    QProgressDialog progressDialog( m_mainApp );
    progressDialog.setModal( true );
    progressDialog.setRange(0, 0);
    progressDialog.setMinimumDuration( 0 );
    progressDialog.setWindowTitle( tr("Patient loading") );
    progressDialog.setLabelText( tr("Loading, please wait...") );
    progressDialog.setCancelButton( 0 );


    qApp->processEvents();
    QList<Patient*> patientsList;
    if(inputFiles.first().contains(".mhd"))
    {
        PatientFillerInput patientFillerInput;
        patientFillerInput.setFilesList(inputFiles);
        MHDFileClassifierStep mhdFileClassiferStep;
        mhdFileClassiferStep.setInput(&patientFillerInput);
        mhdFileClassiferStep.fill();

        patientsList = patientFillerInput.getPatientsList();
    }
    else
    {
        PatientFiller patientFiller;
        connect(&patientFiller, SIGNAL( progress(int) ), &progressDialog, SLOT( setValue(int) ));

        patientsList = patientFiller.processDICOMFileList(inputFiles);
    }
    progressDialog.close();

    unsigned int numberOfPatients = patientsList.size();

    if (numberOfPatients == 0)
    {
        QMessageBox::critical(0, ApplicationNameString, tr("Sorry, it seems that there is no patient data we can load."));
        ERROR_LOG("Error fent el fill de patientFiller. Ha retornat 0 pacients.");
        return;
    }

    DEBUG_LOG( QString("getNumberOfPatients: %1").arg( numberOfPatients ) );

    QList<int> correctlyLoadedPatients;

    for(unsigned int i = 0; i < numberOfPatients; i++ )
    {
        DEBUG_LOG( QString("Patient #%1\n %2").arg(i).arg( patientsList.at(i)->toString() ) );

        bool error = true;

        // marquem les series seleccionades
        QList<Study *> studyList = patientsList.at(i)->getStudies();
        if ( !studyList.isEmpty() )
        {
            Study *study = studyList.first();

            QList<Series *> seriesList = study->getSeries();
            if( !seriesList.isEmpty() )
            {
                Series *series = seriesList.first();
                series->select();
                error = false;
            }
            else
            {
                ERROR_LOG(patientsList.at(i)->toString());
                ERROR_LOG("Error carregant aquest pacient. La serie retornada és null.");
            }
        }
        else
        {
            ERROR_LOG(patientsList.at(i)->toString());
            ERROR_LOG("Error carregant aquest pacient. L'study retornat és null.");
        }

        if (!error)
        {
            /// Hem de fer el que feia l'step del volume perquè ja no es fa.
            foreach(Study *study, patientsList.at(i)->getStudies() )
            {
                foreach(Series *series, study->getSeries() )
                {
                    // TODO ara el que fem és que 1 Series equival a 1 Volume, més endavant es podrien fer un tracte més elaborat
                    Volume *volume = new Volume;
                    volume->setImages( series->getImages() );
                    volume->setNumberOfPhases( series->getNumberOfPhases() );
                    series->addVolume(volume);
                }
            }

            correctlyLoadedPatients << i;
        }
    }

    QString patientsWithError;
    if (correctlyLoadedPatients.count() != numberOfPatients)
    {
        for (unsigned int i = 0; i < numberOfPatients; i++ )
        {
            if (!correctlyLoadedPatients.contains(i))
            {
                patientsWithError += "- " + patientsList.at(i)->getFullName() + "; ID: " + patientsList.at(i)->getID() + "<br>";
            }
        }
        QMessageBox::critical(0, ApplicationNameString, tr("Sorry, an error ocurred while loading the data of patients:<br> %1").arg(patientsWithError) );
    }

    processInput( patientsList );
}

void ExtensionHandler::processInput( QList<Patient *> patientsList )
{
    // Si de tots els pacients que es carreguen intentem carregar-ne un d'igual al que ja tenim carregat, el mantenim
    bool canReplaceActualPatient = true;
    if (m_mainApp->getCurrentPatient())
    {
        foreach (Patient *patient, patientsList)
        {
            if (m_mainApp->getCurrentPatient()->compareTo( patient ) == Patient::SamePatients )
            {
                canReplaceActualPatient = false;
                break;
            }
        }
    }

    // Afegim els pacients carregats correctament
    foreach (Patient *patient, patientsList)
    {
        processInput(patient,QString());
        this->addPatientToWindow( patient, canReplaceActualPatient );
        canReplaceActualPatient = false; //Un cop carregat un pacient, ja no el podem reemplaçar
    }
}

void ExtensionHandler::processInput(Patient *patient, const QString &defaultSeriesUID)
{
    Q_UNUSED( defaultSeriesUID );
    foreach(Study *study, patient->getStudies() )
    {
        // Per cada sèrie, si les seves imatges són multiframe o de mides diferents entre sí aniran en volums separats
        foreach(Series *series, study->getViewableSeries() )
        {
            QList<Image *> volumeImages;
            foreach( Image *image, series->getImages() )
            {
                // TODO amb aquest sistema podria ser que tinguéssim una sèrie amb una o més imatges multiframe 
                // barrejades amb una o més imatges single-frame que en els passos dels fillers steps s'haguéssin
                // comptabilitzat amb fases, per tant, els hi assignaríem un rang erroni de fases i llesqes per fase
                // És un cas extrem i extrany, però que ningú ens diu que no pugui passar.
                // Tenir-ho en compte per en un futur fer la implementació adequada
                if( image->isMultiFrame() )
                {
                    Volume *volume = new Volume;
                    volume->addImage(image);
                    volume->setNumberOfPhases(1);
                    volume->setNumberOfSlicesPerPhase( image->getNumberOfFrames() );
                    volume->setThumbnail( image->getThumbnail() );
                    series->addVolume( volume );
                }
                else
                {
                    volumeImages << image;
                }
            }

            if( !volumeImages.isEmpty() )
            {
                Volume *volume = new Volume;
                volume->setImages( volumeImages );
                volume->setNumberOfPhases( series->getNumberOfPhases() );
                volume->setNumberOfSlicesPerPhase( series->getNumberOfSlicesPerPhase() );
                volume->setThumbnail( volumeImages.at(volumeImages.count()/2)->getThumbnail() );
                series->addVolume(volume);
            }
        }
    }
    DEBUG_LOG( QString("Patient:\n%1").arg( patient->toString() ));
}

void ExtensionHandler::addPatientToWindow(Patient *patient, bool canReplaceActualPatient)
{
    if( !m_mainApp->getCurrentPatient() )
    {
        m_mainApp->setPatient(patient);
        DEBUG_LOG("No tenim dades de cap pacient. Obrim en la finestra actual");
    }
    else if( ( m_mainApp->getCurrentPatient()->compareTo( patient ) == Patient::SamePatients ))
    {
        *(m_mainApp->getCurrentPatient()) += *patient;
        m_mainApp->connectPatientVolumesToNotifier(patient);
        DEBUG_LOG("Ja teníem dades d'aquest pacient. Fusionem informació");

        //mirem si hi ha alguna extensió oberta, sinó obrim la de per defecte
        if ( m_mainApp->getExtensionWorkspace()->count() == 0 )
            openDefaultExtension();
    }
    else //Són diferents o no sabem diferenciar
    {
        if (canReplaceActualPatient)
        {
            m_mainApp->setPatient(patient);
            DEBUG_LOG("Tenim pacient i el substituim");
        }
        else
        {
            m_mainApp->setPatientInNewWindow(patient);
            DEBUG_LOG("Tenim pacient i no ens deixen substituir-lo. L'obrim en una finestra nova.");
        }
    }
}

void ExtensionHandler::openDefaultExtension()
{
    if( m_mainApp->getCurrentPatient() )
    {
        // TODO de moment simplement cridem el Q2DViewerExtension
        request("Q2DViewerExtension");
    }
    else
    {
        DEBUG_LOG("No hi ha dades de pacient!");
    }
}

};  // end namespace udg

