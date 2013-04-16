#ifndef UDGQDICOMDIRCONFIGURATIONSCREEN_H
#define UDGQDICOMDIRCONFIGURATIONSCREEN_H

#include "ui_qdicomdirconfigurationscreenbase.h"

namespace udg {

class Status;

/**
    Widget en el que es configuren els par�metres del programa de gravaci�
  */
class QDICOMDIRConfigurationScreen : public QWidget, private Ui::QDICOMDIRConfigurationScreenBase {
Q_OBJECT

public:
    /// Constructor de la classe
    QDICOMDIRConfigurationScreen(QWidget *parent = 0);

    /// Destructor de classe
    ~QDICOMDIRConfigurationScreen();

public slots:
    /// Aplica els canvis de la configuraci�
    bool applyChanges();

private:
    /// Crea els connects dels signals i slots
    void createConnections();
    
    /// Creates the connections with widgets to edit settings properly
    void setupSettingsUpdatesConnections();

    /// Carrega les dades de configuraci� del programa de gravaci�
    void loadDICOMDIRDefaults();

    /// Enables or disables labels, text box and button related to folder to copy content from
    void enableCopyFolderContentPathWidgets(bool enable);
    
    /// Updates the state of the "copy folder contents" section widgets depending on wheter the checkboxes are checked or not
    void updateCopyFolderContentPathOptions();

    /// Validation methods for the required fields
    
    /// Validates the burning application path to be an existing executable file
    bool validateBurningApplicationPath(const QString &path);
    
    /// Validates copy folder path to be a valid path to a folder which does not contain any item named DICOM nor DICOMDIR
    bool validateCopyFolderContentPath(const QString &path);

private slots:
    /// Mostra un QDialog per especificar on es troba el programa de gravaci� en el disc dur
    void examinateDICOMDIRBurningApplicationPath();

    /// Mostra QDialog que permet especificar el directori on es troba el contingut que s'ha de copiar al crear un DICOMDIR
    void examinateDICOMDIRFolderPathToCopy();

    /// Methods to update and write the pointed settings when appropiate
    void updateExplicitLittleEndianSetting(bool enable);
    void checkAndUpdateBurningApplicationPathSetting(const QString &text);
    void updateBurningApplicationParametersSetting();
    void updateDifferentCDDVDParametersSetting(bool enable);
    void updateBurningApplicationCDParametersSetting();
    void updateBurningApplicationDVDParametersSetting();
    void checkAndUpdateCopyFolderContentPathSetting(const QString &text);
    void updateCopyContentsFromUSBOrHardDiskSetting(bool enable);
    void updateCopyContentsFromCDOrDVDSetting(bool enable);
};

};// End namespace udg

#endif
