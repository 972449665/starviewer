#ifndef UDGQDICOMPRINTERCONFIGURATIONWIDGET_H
#define UDGQDICOMPRINTERCONFIGURATIONWIDGET_H

#include "ui_qdicomprinterconfigurationwidgetbase.h"
#include "qprinterconfigurationwidget.h"

namespace udg {

class DicomPrinter;

/**
 * Implementaci� de la interf�cie QPrinterConfigurationWidget per impressi� d'imatge DICOM en DCMTK.
 */

class QDicomPrinterConfigurationWidget : public QPrinterConfigurationWidget , private::Ui::QDicomPrinterConfigurationWidgetBase
{
Q_OBJECT

public:
    QDicomPrinterConfigurationWidget();
    ~QDicomPrinterConfigurationWidget();
	
public slots:
    // Acualiza la informaci� del Printer cada vegada que es selecciona al QTree una impressora.
    void printerSelectionChanged();
    // Afegeix una nova impressora Dicom a partir dels par�metres introduits per l'usuari.
    void addPrinter();
    // Modifica una impressora Dicom ja existen amb els par�metres introduits per l'usuari.
    void updatePrinter();
    // Esborra una impressora.
    void deletePrinter();
    // Comprova la connexi� amb una impressora entrada al sistema.
    void testPrinter();
    // Mostra per pantalla els diferents valors que poden agafar cadascun dels par�metres d'una impressora.
    void getAvailableParameters();

private:
    void createConnections();
    // Actualitza la llista d'impressores entrades al sistema.
    void refreshPrinterList();
    // Comprova la validesa dels p�rametres d'entrada.
    bool validatePrinterSettings();
    // Neteja els valors de la inter�fice.
    void clearPrinterSettings();
    // Es mostra la informaci� b�sica de la impressora per pantalla (/p printer �s la impressora que cont� la informaci� a mostrar)
    void setPrinterSettingsToControls(DicomPrinter& printer);
    // Es guarda la informaci� b�sica de la impressora de l'interf�cie a un objecte DicomPrinter (/p printer �s la impressora on es guarden les dades).
    void getPrinterSettingsFromControls(DicomPrinter& printer);
    // Es mostra la informaci� avan�ada de la impressora per pantalla (/p printer �s la impressora que cont� la informaci� a mostrar)
    void setDefaultPrinterSettingsToControls(DicomPrinter& printer);
     // Es guarda la informaci� avan�ada de la impressora de l'interf�cie a un objecte DicomPrinter (/p printer �s la impressora on es guarden les dades).
    void getDefaultPrinterSettingsFromControls(DicomPrinter& printer);
    // Es mostra la informaci� del Job de la impressora per pantalla (/p printer �s la impressora que cont� la informaci� a mostrar)
    void setDefaultJobSettingsToControls(DicomPrinter& printer);
    // Es guarda la informaci� del Job de la impressora de l'interf�cie a un objecte DicomPrinter (/p printer �s la impressora on es guarden les dades).
    void getDefaultJobSettingsFromControls(DicomPrinter& printer);
    
    int m_selectedPrinterId;
    bool m_semafor;
};
};
#endif
