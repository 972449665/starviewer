#ifndef UDGINPUTOUTPUTSETTINGS_H
#define UDGINPUTOUTPUTSETTINGS_H

#include "defaultsettings.h"

namespace udg {

class InputOutputSettings : public DefaultSettings {
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
    static const QString RISRequestsPort;
    static const QString ListenToRISRequests;
    static const QString RISRequestViewOnceRetrieved;

    /// �ltim path des del que hem obert un dicomdir
    static const QString LastOpenedDICOMDIRPath;
    /// Geometria de les columnes del llistat d'estudis d'un DICOMDIR/llista d'operacions/PACS
    static const QString DICOMDIRStudyListColumnsWidth;
    static const QString CreateDICOMDIRStudyListColumnsWidth;
    static const QString OperationStateColumnsWidth;
    static const QString PACSStudyListColumnsWidth;
    static const QString ConfigurationScreenPACSList;
    static const QString LocalDatabaseStudyList;
    static const QString LocalDatabaseSplitterState;
    static const QString QueryScreenGeometry;
    static const QString PACSStudyListSortByColumn;
    static const QString PACSStudyListSortOrder;
    static const QString LocalDatabaseStudyListSortByColumn;
    static const QString LocalDatabaseStudyListSortOrder;
    static const QString DICOMDIRStudyListSortByColumn;
    static const QString DICOMDIRStudyListSortOrder;

    /// Par�metres de connexi� amb un PACS
    static const QString LocalAETitle;
    static const QString IncomingDICOMConnectionsPort;
    static const QString PACSConnectionTimeout;
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
    ///Path del directori del que s'ha de copiar el visor DICOMDIR
    static const QString DICOMDIRFolderPathToCopy;
    /// Checkbox per saber si cal copiar el contingut del directori que apunta el setting DICOMDIRFolderPathToCopy als CD/DVD DICOMDIR
    static const QString CopyFolderContentToDICOMDIROnCDOrDVD;
    /// Checkbox per saber si cal copiar el contingut del directori que apunta el setting DICOMDIRFolderPathToCopy als Usb/HardDisk DICOMDIR
    static const QString CopyFolderContentToDICOMDIROnUSBOrHardDisk;

    //Informaci� Instituci�
    static const QString InstitutionName;
    static const QString InstitutionTown;
    static const QString InstitutionAddress;
    static const QString InstitutionZipCode;
    static const QString InstitutionCountry;
    static const QString InstitutionPhoneNumber;
    static const QString InstitutionEmail;

    //Columna i ordre d'ordenaci� de la QOperationStateScreen
    static const QString OperationStateListSortByColumn;
    static const QString OperationStateListSortOrder;

    //Boolea per saber si s'ha de cercar previes a partir del nom del pacient.
    static const QString SearchRelatedStudiesByName;
};

} // end namespace udg

#endif
