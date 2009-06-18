#ifndef UDGINPUTOUTPUTSETTINGS_H
#define UDGINPUTOUTPUTSETTINGS_H

#include "defaultsettings.h"

namespace udg {

class InputOutputSettings : public DefaultSettings
{
public:
    InputOutputSettings();
    ~InputOutputSettings();

    void init();

    /// Declaraci� de claus
    /// Path absolut de l'arxiu de base dades
    static const QString databaseAbsoluteFilePathKey;
    /// Path del directori de la cache
    static const QString cachePathKey;
    /// Pol�tiques d'autogesti� de cache
    static const QString deleteLeastRecentlyUsedStudiesInDaysCriteriaKey;
    static const QString deleteLeastRecentlyUsedStudiesNoFreeSpaceCriteriaKey;
    static const QString minimumGigaBytesToFreeIfCacheIsFullKey;
    static const QString minimumFreeGigaBytesForCacheKey;
    static const QString minimumDaysUnusedToDeleteStudyKey;

    /// Par�metres del RIS
    static const QString risRequestsPortKey;
    static const QString listenToRISRequestsKey;
    static const QString risRequestViewOnceRetrievedKey;

    /// �ltim path des del que hem obert un dicomdir
    static const QString lastOpenedDICOMDIRPathKey;
    /// Geometria de les columnes del llistat d'estudis d'un DICOMDIR/llista d'operacions
    static const QString dicomdirStudyListColumnsWidthKey;
    static const QString createDicomdirStudyListColumnsWidthKey;
    static const QString operationStateColumnsWidthKey;
    static const QString pacsStudyListColumnsWidthKey;

    /// Par�metres de connexi� amb un PACS
    static const QString localAETitleKey;
    static const QString queryRetrieveLocalPortKey;
    static const QString pacsConnectionTimeoutKey;
    static const QString maximumPACSConnectionsKey;
};

} // end namespace udg 

#endif
