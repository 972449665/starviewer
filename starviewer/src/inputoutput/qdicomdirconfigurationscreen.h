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

private slots:
    /// Mostra un QDialog per especificar on es troba el programa de gravaci� en el disc dur
    void examinateDICOMDIRBurningApplicationPath();

    /// Mostra QDialog que permet especificar el directori on es troba el contingut que s'ha de copiar al crear un DICOMDIR
    void examinateDICOMDIRFolderPathToCopy();

private:
    /// Crea els connects dels signals i slots
    void createConnections();

    /// Carrega les dades de configuraci� del programa de gravaci�
    void loadDICOMDIRDefaults();

    /// Aplica els canvis fets a la configuraci� del programa de gravaci�
    void applyChangesDICOMDIR();

    /// Valida que els canvis de la configuraci� siguin correctes
    bool validateChanges();
};

};// End namespace udg

#endif
