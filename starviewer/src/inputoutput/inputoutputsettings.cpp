#include "inputoutputsettings.h"
#include "settingsregistry.h"

#include "starviewerapplication.h" // pel UserDataRootPath
#include <QHostInfo> //pel "localhostName"

namespace udg {

// Definici� de les claus
const QString CacheBase("PACS/cache/");
const QString InputOutputSettings::DatabaseAbsoluteFilePath( CacheBase + "sdatabasePath");
const QString InputOutputSettings::CachePath( CacheBase + "imagePath");
const QString InputOutputSettings::DeleteLeastRecentlyUsedStudiesInDaysCriteria( CacheBase + "deleteOldStudiesHasNotViewedInDays");
const QString InputOutputSettings::DeleteLeastRecentlyUsedStudiesNoFreeSpaceCriteria( CacheBase + "deleteOldStudiesIfNotEnoughSpaceAvailable");
const QString InputOutputSettings::MinimumDaysUnusedToDeleteStudy( CacheBase + "MaximumDaysNotViewedStudy");
const QString InputOutputSettings::MinimumFreeGigaBytesForCache( CacheBase + "minimumSpaceRequiredToRetrieveInGbytes");
const QString InputOutputSettings::MinimumGigaBytesToFreeIfCacheIsFull( CacheBase + "GbytesOfOldStudiesToDeleteIfNotEnoughSapaceAvailable");

const QString InputOutputSettings::RetrievingStudy("/PACS/RetrievingStudy");

const QString RisBase("PACS/risRequests/");
const QString InputOutputSettings::ListenToRISRequests( RisBase + "listen");
const QString InputOutputSettings::RisRequestsPort( RisBase + "listenPort");
const QString InputOutputSettings::RisRequestViewOnceRetrieved( RisBase + "viewAutomaticallyAStudyRetrievedFromRisRequest");

const QString InterfaceBase("PACS/interface/");
const QString InputOutputSettings::LastOpenedDICOMDIRPath( InterfaceBase + "lastOpenedDICOMDIRPath");  
const QString InputOutputSettings::DicomdirStudyListColumnsWidth( InterfaceBase + "studyDicomdirList/");
const QString InputOutputSettings::CreateDicomdirStudyListColumnsWidth( InterfaceBase + "qCreateDicomdir/");
const QString InputOutputSettings::OperationStateColumnsWidth( InterfaceBase + "qOperationState/");
const QString InputOutputSettings::PacsStudyListColumnsWidth( InterfaceBase + "studyPacsList/");
const QString InputOutputSettings::ConfigurationScreenPACSList( InterfaceBase + "qConfigurationPacsDevice/");
const QString InputOutputSettings::LocalDatabaseStudyList( InterfaceBase + "studyCacheList/");
const QString InputOutputSettings::LocalDatabaseSplitterState( InterfaceBase + "queryscreen/StudyTreeSeriesListQSplitterState");
const QString InputOutputSettings::QueryScreenGeometry( InterfaceBase + "queryscreen/geometry");

const QString PacsParamBase("PACS/pacsparam/");
const QString InputOutputSettings::QueryRetrieveLocalPort( PacsParamBase + "localPort");
const QString InputOutputSettings::LocalAETitle( PacsParamBase + "AETitle");
const QString InputOutputSettings::PacsConnectionTimeout( PacsParamBase + "timeout");
const QString InputOutputSettings::MaximumPACSConnections( PacsParamBase + "MaxConnects");

const QString DICOMDIRBaseKey("DICOMDIR/");
const QString InputOutputSettings::DICOMDIRBurningApplicationPathKey( DICOMDIRBaseKey + "DICOMDIRBurningApplication");
const QString InputOutputSettings::DICOMDIRBurningApplicationParametersKey( DICOMDIRBaseKey + "DICOMDIRBurningApplicationParameters");

InputOutputSettings::InputOutputSettings()
{
}

InputOutputSettings::~InputOutputSettings()
{
}

void InputOutputSettings::init()
{
    SettingsRegistry *settingsRegistry = SettingsRegistry::instance();
    
    settingsRegistry->addSetting( DatabaseAbsoluteFilePath, UserDataRootPath + "pacs/database/dicom.sdb", Settings::Parseable );
    settingsRegistry->addSetting( CachePath, UserDataRootPath + "pacs/dicom/", Settings::Parseable );

    settingsRegistry->addSetting( DeleteLeastRecentlyUsedStudiesInDaysCriteria, true );
    settingsRegistry->addSetting( DeleteLeastRecentlyUsedStudiesNoFreeSpaceCriteria, true );
    settingsRegistry->addSetting( MinimumDaysUnusedToDeleteStudy, 7 );
    settingsRegistry->addSetting( MinimumFreeGigaBytesForCache, 5 );
    settingsRegistry->addSetting( MinimumGigaBytesToFreeIfCacheIsFull, 2 );

    settingsRegistry->addSetting( ListenToRISRequests, true );
    settingsRegistry->addSetting( RisRequestsPort, 11110 );
    settingsRegistry->addSetting( RisRequestViewOnceRetrieved, true );

    settingsRegistry->addSetting( LastOpenedDICOMDIRPath, QDir::homePath() );

    settingsRegistry->addSetting( QueryRetrieveLocalPort, 4006 );
    // TODO podr�em definir-lo directament amb %HOSTNAME%
    settingsRegistry->addSetting( LocalAETitle, QHostInfo::localHostName(), Settings::Parseable );
    settingsRegistry->addSetting( PacsConnectionTimeout, 20 );
    settingsRegistry->addSetting( MaximumPACSConnections, 3 );

#ifdef _WIN32
    settingsRegistry->addSetting( DICOMDIRBurningApplicationPathKey, "C:\\Archivos de programa\\ImgBurn\\ImgBurn.exe" );
    settingsRegistry->addSetting( DICOMDIRBurningApplicationParametersKey, "/MODE write /SRC %1 /EJECT YES /VERIFY NO /CLOSESUCCESS /START" );
#else
    settingsRegistry->addSetting( DICOMDIRBurningApplicationPathKey, "k3b" );
    settingsRegistry->addSetting( DICOMDIRBurningApplicationParametersKey, "--nosplash --cdimage %1" );
#endif
}

} // end namespace udg 

