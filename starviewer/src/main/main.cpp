/***************************************************************************
 *   Copyright (C) 2005 by Grup de Gràfics de Girona                       *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/

#include "qapplicationmainwindow.h"
#include <QApplication>
#include <QSplashScreen>
#include <QTranslator>
#include <QSettings>
#include <QLocale>
#include <QDir>
#include <QTextCodec>
#include "logging.h"
#include "extensions.h"
#include "extensionmediatorfactory.h"
// definicions globals d'aplicació
#include "starviewerapplication.h"
// decodificacio jpeg
#include "dcmtk/dcmjpeg/djdecode.h"

#ifndef NO_CRASH_REPORTER
#include "crashhandler.h"
#endif

void configureLogging()
{
    // primer comprovem que existeixi el direcotori ~/.starviewer/log/ on guradarem els logs
    QDir logDir = udg::UserLogsPath;
    if( !logDir.exists() )
    {
        // creem el directori
        logDir.mkpath( udg::UserLogsPath );
    }
    // TODO donem per fet que l'arxiu es diu així i es troba a la localització que indiquem. S'hauria de fer una mica més flexible o genèric;
    // està així perquè de moment volem anar per feina i no entretenir-nos però s'ha de fer bé.
    QString configurationFile = "/etc/starviewer/log.conf";
    if( ! QFile::exists(configurationFile) )
    {
        configurationFile = qApp->applicationDirPath() + "/log.conf";
    }

    LOGGER_INIT( configurationFile.toStdString() );
    DEBUG_LOG("Arxiu de configuració del log: " + configurationFile );
}

void loadTranslator(QApplication &app, QString pathTranslator)
{
    QTranslator *translator = new QTranslator(&app);
    if (translator->load( pathTranslator ))
    {
        app.installTranslator( translator );
    }
    else
    {
        ERROR_LOG("No s'ha pogut carregar el translator " + pathTranslator);
    }
}

void initializeTranslations(QApplication &app)
{
    QSettings settings;
    settings.beginGroup("Starviewer-Language");
    QString m_defaultLocale = settings.value( "languageLocale", QLocale::system().name() ).toString();
    settings.endGroup();

	// configurem les locales de l'aplicació
	// TODO ara està simplificat només als idiomes que nosaltres
	// suportem. Mirar si es pot millorar i fer més genèric
	QLocale::Language language;
	QLocale::Country country;

	if( m_defaultLocale.startsWith("en_") )
		language = QLocale::English;
	else if( m_defaultLocale.startsWith("es_") )
		language = QLocale::Spanish;
	else if( m_defaultLocale.startsWith("ca_") )
		language = QLocale::Catalan;
	else
		language = QLocale::C; // TODO no hauria de ser anglès per defecte?

	if( m_defaultLocale.endsWith("_GB") )
		country = QLocale::UnitedKingdom;
	else if( m_defaultLocale.endsWith("_ES") )
		country = QLocale::Spain;
	else
		country = QLocale::AnyCountry; // // TODO no hauria de ser EEUU/UK per defecte?

	// li indiquem la locale corresponent
	QLocale::setDefault( QLocale( language, country ) );

    loadTranslator(app, ":/core/core_" + m_defaultLocale);
    loadTranslator(app, ":/interface/interface_" + m_defaultLocale);
    loadTranslator(app, ":/inputoutput/inputoutput_" + m_defaultLocale);

    initExtensionsResources();
    INFO_LOG("Locales = " + m_defaultLocale);

    QList<QString> extensionsMediatorNames = udg::ExtensionMediatorFactory::instance()->getFactoryNamesList();
    foreach(QString mediatorName, extensionsMediatorNames)
    {
        udg::ExtensionMediator* mediator = udg::ExtensionMediatorFactory::instance()->create(mediatorName);

        if (mediator)
        {
            loadTranslator(app, ":/extensions/" + mediator->getExtensionID().getID() + "/translations_" + m_defaultLocale);
            delete mediator;
        }
        else
        {
            ERROR_LOG( "Error carregant el mediator de " + mediatorName );
        }
    }
}

/// Afegeix els directoris on s'han de buscar els plugins de Qt. Útil a windows.
void initQtPluginsDirectory()
{
#ifdef Q_OS_WIN32
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath() + "/plugins");
#endif
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setOrganizationName( udg::OrganizationNameString );
    app.setOrganizationDomain( udg::OrganizationDomainString );
    app.setApplicationName( udg::ApplicationNameString );

	// Inicialitzem el crash handler en el cas que ho suportem.
#ifndef NO_CRASH_REPORTER
    CrashHandler *crashHandler = new CrashHandler();
#endif
    
	//TODO tot aquest proces inicial de "setups" hauria d'anar encapsulat en
    // una classe dedicada a tal efecte

    // ajustem el codec per els strings pelats ( no QString,sinó "bla bla bla" ).
    // Amb aquesta crida escollirà el codec més apropiat segons el sistema. En aquest cas ens agafarà utf-8 (Mandriva 2007)
    QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );

    configureLogging();
    STAT_LOG("Inicialització de l'aplicació");

    initQtPluginsDirectory();
    initializeTranslations(app);

    // TODO aixo es necessari per, entre d'altres coses, poder crear thumbnails,
    // dicomdirs, etc de dicoms comprimits i tractar-los correctament amb dcmtk
    // aixo esta temporalment aqui, a la llarga anira a una classe de setup
    // registrem els codecs decompressors JPEG
    DJDecoderRegistration::registerCodecs();

    QPixmap splashPixmap;
#ifdef STARVIEWER_LITE
    splashPixmap.load(":/images/splashLite.png");
#else
    splashPixmap.load(":/images/splash.png");
#endif
    QSplashScreen *splash = new QSplashScreen( splashPixmap );
    splash->show();

    udg::QApplicationMainWindow *mainWin = new udg::QApplicationMainWindow;
    INFO_LOG("Creada finestra principal");
    mainWin->show();

    QObject::connect( &app, SIGNAL( lastWindowClosed() ),
                      &app, SLOT( quit() ));
    splash->finish( mainWin );
    delete splash;

    int returnValue = app.exec();

    STAT_LOG("Es tanca l'aplicació");

    return returnValue;
}
