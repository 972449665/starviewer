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
    static const QString DatabaseAbsoluteFilePath;
    /// Path del directori de la cache
    static const QString CachePath;
    /// Pol�tiques d'autogesti� de cache
    static const QString DeleteLeastRecentlyUsedStudiesInDaysCriteria;
    static const QString DeleteLeastRecentlyUsedStudiesNoFreeSpaceCriteria;
    static const QString MinimumGigaBytesToFreeIfCacheIsFull;
    static const QString MinimumFreeGigaBytesForCache;
    static const QString MinimumDaysUnusedToDeleteStudy;
    /// Controlar quin estudi est� baixant-se
    static const QString RetrievingStudy;

    /// Conjunt de PACS que tenim seleccionats per defecte per consultar. 
    /// La llista consistir� en un string on cada PACS s'identificar� pel seu camp clau, els quals estaran delimitats per "//"
    static const QString DefaultPACSListToQuery;

    /// Par�metres del RIS
    static const QString RisRequestsPort;
    static const QString ListenToRISRequests;
    static const QString RisRequestViewOnceRetrieved;

    /// �ltim path des del que hem obert un dicomdir
    static const QString LastOpenedDICOMDIRPath;
    /// Geometria de les columnes del llistat d'estudis d'un DICOMDIR/llista d'operacions/PACS
    static const QString DicomdirStudyListColumnsWidth;
    static const QString CreateDicomdirStudyListColumnsWidth;
    static const QString OperationStateColumnsWidth;
    static const QString PacsStudyListColumnsWidth;
    static const QString ConfigurationScreenPACSList;
    static const QString LocalDatabaseStudyList;
    static const QString LocalDatabaseSplitterState;
    static const QString QueryScreenGeometry;

    /// Par�metres de connexi� amb un PACS
    static const QString LocalAETitle;
    static const QString QueryRetrieveLocalPort;
    static const QString PacsConnectionTimeout;
    static const QString MaximumPACSConnections;

    /// Llista de PACS
    static const QString PacsListConfigurationSectionName;

    /// Par�metres de la configuraci� DICOMDIR
    /// Path del programa de gravaci�
    static const QString DICOMDIRBurningApplicationPathKey;
    /// Parameters del programa de gravaci�
    static const QString DICOMDIRBurningApplicationParametersKey;
    /// Par�metres de gravaci� d'un CD quan s�n diferents amb els de DVD
    static const QString DICOMDIRBurningApplicationCDParametersKey;
    /// Par�metres de gravaci� d'un DVD quan s�n diferents amb els de CD
    static const QString DICOMDIRBurningApplicationDVDParametersKey;
    /// CheckBox per saber si cal tenir o no en compte els par�metres de gravaci� d'un CD o DVD
    static const QString DICOMDIRBurningApplicationHasDifferentCDDVDParametersKey;
    /// CheckBox per saber si cal convertir les imatges del DICOMDIR a little endian
    static const QString ConvertDICOMDIRImagesToLittleEndianKey;

};

} // end namespace udg 

#endif
