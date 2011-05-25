#ifndef UDGQDICOMADDPRINTERWIGET_H
#define UDGQDICOMADDPRINTERWIGET_H

#include "ui_qdicomaddprinterwidgetbase.h"

namespace udg {

class DicomPrinter;

/**
    Classe que ens permet afegir una nova impressora Dicom al Sistema
  */
class QDicomAddPrinterWidget : public QWidget, private::Ui::QDicomAddPrinterWidgetBase {
Q_OBJECT

public:
    QDicomAddPrinterWidget();

    /// Esborra les dades entrades per l'usuari
    void clearInputs();

signals:
    /// Signal que es llan�a quan una impressora s'ha afegit al sistema.
    void newPrinterAddedSignal(int);

public slots:
    /// Posa operatiu el boto d'afegir impressora quan tots els par�metres estan entrats.
    void enableAddButton();

    /// Afegeix una impressora dicom a partir de les dades entrades per l'usuari.
    void addPrinter();

private:
    /// Es creen els connexions de la interf�cie
    void createConnections();

    /// Configurem InputValidator per alguns dels QLineEdit
    void configureInputValidator();

    /// Comprova la validesa dels par�metres entrats. Retorna false si hi ha algun par�metre erroni.
    bool validateInputSettings();

    /// Agafa les dades introduides per l'usuari i les afegeix a un DicomPrinter.
    void getPrinterSettingsFromControls(DicomPrinter &printer);
};
};
#endif
