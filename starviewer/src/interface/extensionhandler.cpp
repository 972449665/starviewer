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
#include "interfacesettings.h"

// Espai reservat pels include de les mini-apps
#include "appimportfile.h"

// Fi de l'espai reservat pels include de les mini-apps

// PACS --------------------------------------------
#include "queryscreen.h"
#include "patientfiller.h"

namespace udg {

typedef SingletonPointer<QueryScreen> QueryScreenSingleton;

ExtensionHandler::ExtensionHandler(QApplicationMainWindow *mainApp, QObject *parent)
 : QObject(parent )
{
    m_mainApp = mainApp;

    // Aquí en principi només farem l'inicialització
    m_importFileApp = new AppImportFile;

    createConnections();

    //TODO:xapussa per a que l'starviewer escolti les peticions del RIS, com que tot el codi d'escoltar les peticions del ris està a la 
    //queryscreen l'hem d'instanciar ja a l'inici perquè ja escolti les peticions
    QueryScreenSingleton::instance();
    connect(QueryScreenSingleton::instance(), SIGNAL( selectedPatients(QList<Patient *>,bool) ), SLOT(processInput(QList<Patient*>,bool)));
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
    INFO_LOG("Request d'extensió amb ID: " + QString::number(who));
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
            disconnect(QueryScreenSingleton::instance(), SIGNAL( selectedPatients(QList<Patient *>,bool) ), 0, 0 );
            QueryScreenSingleton::instance()->showPACSTab();
            connect(QueryScreenSingleton::instance(), SIGNAL( selectedPatients(QList<Patient *>,bool) ), SLOT(processInput(QList<Patient*>,bool)));
            break;

        case 8:
            // HACK degut a que la QueryScreen és un singleton, això provoca efectes colaterals quan teníem
            // dues finestres ( mirar ticket #542 ). Fem aquest petit hack perquè això no passi.
            // Queda pendent resoldre-ho de la forma adequada
            disconnect(QueryScreenSingleton::instance(), SIGNAL( selectedPatients(QList<Patient *>,bool) ), 0, 0 );
            QueryScreenSingleton::instance()->openDicomdir();
            connect(QueryScreenSingleton::instance(), SIGNAL( selectedPatients(QList<Patient *>,bool) ), SLOT(processInput(QList<Patient*>,bool)));
            break;

        case 10: // Mostrar local
            // HACK degut a que la QueryScreen és un singleton, això provoca efectes colaterals quan teníem
            // dues finestres ( mirar ticket #542 ). Fem aquest petit hack perquè això no passi.
            // Queda pendent resoldre-ho de la forma adequada
            disconnect(QueryScreenSingleton::instance(), SIGNAL( selectedPatients(QList<Patient *>,bool) ), 0, 0 );
            QueryScreenSingleton::instance()->showLocalExams();
            connect(QueryScreenSingleton::instance(), SIGNAL( selectedPatients(QList<Patient *>,bool) ), SLOT(processInput(QList<Patient*>,bool)));
            break;
    }
}

bool ExtensionHandler::request( const QString &who )
{
    bool ok = true;
    ExtensionMediator *mediator = ExtensionMediatorFactory::instance()->create(who);
    if( !mediator )
    {
        WARN_LOG( "No s'ha pogut crear el mediator per: " + who );
        DEBUG_LOG( "No s'ha pogut crear el mediator per: " + who );
        ok = false;
        return ok;
    }

    bool createExtension = true;
    int extensionIndex = 0;
    QString requestedExtensionLabel = mediator->getExtensionID().getLabel();
    if( !Settings().getValue( InterfaceSettings::AllowMultipleInstancesPerExtension ).toBool() )
    {
        // Només volem una instància per extensió
        // Cal comprovar llavors que l'extensió que demanem no estigui ja creada
        int count = m_mainApp->getExtensionWorkspace()->count();
        bool found = false;
        while( extensionIndex < count && !found )
        {
            if( m_mainApp->getExtensionWorkspace()->tabText(extensionIndex) == requestedExtensionLabel )
                found = true;
            else
                extensionIndex++;
        }
        // Si la trobem, no caldrà crear-la de nou
        if( found )
            createExtension = false;
    }

    // Segons la configuració i les extensions existents, haurem de crear o no l'extensió demanada
    if( createExtension ) 
    {
        QWidget *extension = ExtensionFactory::instance()->create(who);
        if (extension)
        {
            INFO_LOG("Activem extensió: " + who);
            mediator->initializeExtension(extension, m_extensionContext );
            m_mainApp->getExtensionWorkspace()->addApplication(extension, requestedExtensionLabel, who);
        }
        else
        {
            ok = false;
            DEBUG_LOG( "Error carregant " + who );
        }
    }
    else // sinó mostrem l'extensió ja existent
    {
        m_mainApp->getExtensionWorkspace()->setCurrentIndex(extensionIndex);
    }

    delete mediator;

    return ok;
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

void ExtensionHandler::processInput(const QStringList &inputFiles)
{
    if( inputFiles.isEmpty() )
        return;

    QProgressDialog progressDialog( m_mainApp );
    progressDialog.setModal( true );
    progressDialog.setRange(0, 0);
    progressDialog.setMinimumDuration( 0 );
    progressDialog.setWindowTitle( tr("Patient loading") );
    progressDialog.setLabelText( tr("Loading, please wait...") );
    progressDialog.setCancelButton( 0 );

    qApp->processEvents();
    PatientFiller patientFiller;
    connect(&patientFiller, SIGNAL( progress(int) ), &progressDialog, SLOT( setValue(int) ));

    QList<Patient*> patientsList = patientFiller.processFiles(inputFiles);

    progressDialog.close();

    unsigned int numberOfPatients = patientsList.size();

    if (numberOfPatients == 0)
    {
        QMessageBox::critical(0, ApplicationNameString, tr("Sorry, it seems that there is no patient data we can load."));
        ERROR_LOG("Error fent el fill de patientFiller. Ha retornat 0 pacients.");
        return;
    }

    DEBUG_LOG( QString("NumberOfPatients: %1").arg( numberOfPatients ) );

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
    if( patientsWithError.isEmpty() )
    {
        // No hi ha cap error, els carreguem tots
        processInput( patientsList );
    }
    else
    {
        // Carreguem únicament els correctes
        QList<Patient *> rightPatients;
        foreach( int index, correctlyLoadedPatients )
        {
            rightPatients << patientsList.at(index);
        }
        processInput( rightPatients );
    }
}

void ExtensionHandler::processInput( QList<Patient *> patientsList, bool loadOnly )
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
        generatePatientVolumes(patient,QString());
        this->addPatientToWindow( patient, canReplaceActualPatient, loadOnly );
        canReplaceActualPatient = false; //Un cop carregat un pacient, ja no el podem reemplaçar
    }
}

void ExtensionHandler::generatePatientVolumes(Patient *patient, const QString &defaultSeriesUID)
{
    Q_UNUSED( defaultSeriesUID );
    foreach(Study *study, patient->getStudies() )
    {
        // Per cada sèrie, si les seves imatges són multiframe o de mides diferents entre sí aniran en volums separats
        foreach(Series *series, study->getViewableSeries() )
        {
            int currentVolumeNumber;
            QMap<int,QList<Image *> > volumesImages;
            foreach( Image *image, series->getImages() )
            {
                currentVolumeNumber = image->getVolumeNumberInSeries();
                if( volumesImages.contains(currentVolumeNumber) )
                {
                    volumesImages[currentVolumeNumber] << image;
                }
                else
                {
                    QList<Image *> newImageList;
                    newImageList << image;
                    volumesImages.insert( currentVolumeNumber, newImageList );
                }
            }
            typedef QList<Image *> ImageListType;
            foreach( ImageListType imageList, volumesImages )
            {
                int numberOfPhases = 1;
                bool found = false;
                int i = 0;
                while( !found && i<imageList.count()-1 )
                {
                    if( imageList.at(i+1)->getPhaseNumber() > imageList.at(i)->getPhaseNumber() )
                        numberOfPhases++;
                    else
                        found = true;
                    i++;
                }
                int numberOfSlicesPerPhase = imageList.count() / numberOfPhases;

                Volume *volume = new Volume;
                volume->setImages( imageList );
                volume->setNumberOfPhases( numberOfPhases );
                volume->setNumberOfSlicesPerPhase( numberOfSlicesPerPhase );
                volume->setThumbnail( imageList.at(imageList.count()/2)->getThumbnail(true) );
                series->addVolume(volume);
            }
        }
    }
    DEBUG_LOG( QString("Patient:\n%1").arg( patient->toString() ));
}

void ExtensionHandler::addPatientToWindow(Patient *patient, bool canReplaceActualPatient, bool loadOnly )
{
    if( !m_mainApp->getCurrentPatient() )
    {
        m_mainApp->setPatient(patient);
        DEBUG_LOG("No tenim dades de cap pacient. Obrim en la finestra actual");
    }
    else if( ( m_mainApp->getCurrentPatient()->compareTo( patient ) == Patient::SamePatients ))
    {
        m_mainApp->connectPatientVolumesToNotifier(patient);
        *(m_mainApp->getCurrentPatient()) += *patient;
        DEBUG_LOG("Ja teníem dades d'aquest pacient. Fusionem informació");

        //mirem si hi ha alguna extensió oberta, sinó obrim la de per defecte
        if ( m_mainApp->getExtensionWorkspace()->count() == 0 )
            openDefaultExtension();

        if( !loadOnly )
        {
            // Hem fet un "view", per tant cal reinicialitzar les extensions que ho requereixin
            QMap<QWidget *,QString> extensions = m_mainApp->getExtensionWorkspace()->getActiveExtensions();
            QMapIterator<QWidget *,QString> iterator(extensions);
            while( iterator.hasNext() )
            {
                iterator.next();
                ExtensionMediator *mediator = ExtensionMediatorFactory::instance()->create( iterator.value() );
                mediator->reinitializeExtension( iterator.key() );
            }
        }
    }
    else //Són diferents o no sabem diferenciar
    {
        if ( !loadOnly )
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
        else
        {
            //Alliberem la memoria dels volums generats
            // \TODO Cal fer una discussió de si és el lloc de fer-ho i de què es fa amb el Patient que no s'assigna
            foreach( Study *study, patient->getStudies() )
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
}

void ExtensionHandler::openDefaultExtension()
{
    if( m_mainApp->getCurrentPatient() )
    {
        Settings settings;
        QString defaultExtension = settings.getValue( InterfaceSettings::DefaultExtension ).toString();
        if( !request( defaultExtension ) )
        {
            WARN_LOG( "Ha fallat la petició per la default extension anomenada: " + defaultExtension + ". Engeguem extensió 2D per defecte(hardcoded)" );
            DEBUG_LOG( "Ha fallat la petició per la default extension anomenada: " + defaultExtension + ". Engeguem extensió 2D per defecte(hardcoded)" );
            request( "Q2DViewerExtension" );
        }
    }
    else
    {
        DEBUG_LOG("No hi ha dades de pacient!");
    }
}

};  // end namespace udg

