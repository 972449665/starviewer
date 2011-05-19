
#ifndef UDGQDICOMPRINTEXTENSION_H
#define UDGQDICOMPRINTEXTENSION_H

#include "ui_qdicomprintextensionbase.h"
#include "dicomprint.h"

#include <QTimer>

namespace udg {

class QDicomPrinterConfigurationWidget;
class Study;
class Image;
class DicomPrintPage;
class DicomPrintJob;
class Volume;
class ToolManager;

class QDicomPrintExtension : public QWidget, private::Ui::QDicomPrintExtensionBase {
Q_OBJECT

public:
    QDicomPrintExtension(QWidget *parent = 0);

public slots:

    /// Li assigna el volum que s'ha d'imprimir
    void setInput(Volume *input);
    /// Actualitzem tots els elements que estan lligats al Volume que estem visualitzant.
    void updateInput();

private slots:

    // Actualitza la informaci� que mostrem de la impressora
    void selectedDicomPrinterChanged(int indexOfSelectedDicomPrinter);
    /// Omple el combobox amb les impressores i mostra com a seleccionada la que esta marca com a impressora per defecte
    void fillSelectedDicomPrinterComboBox();
    /// Activa/desactiva el fram que cont� poder escollir el interval i des de quina imatge fins quina imprimir en funci� del mode de selecci�
    /// d'imatges escollit
    void imageSelectionModeChanged();
    /// Slot que s'executa quan canviem de valor a trav�s del Slider l'interval d'imatges a imprimir
    void m_intervalImagesSliderValueChanged(int value);
    /// Slot que s'executa quan canviem de valor a trav�s del Slider a partir de quina imatge imprimir
    void m_fromImageSliderValueChanged(int value);
    /// Slot que s'executa quan canviem de valor a trav�s del Slider fins quina imatge imprimir
    void m_toImageSliderValueChanged(int value);
    /// Slot que s'exectua quan el LineEdit per especificar el valor del interval d'imatges a imprimir �s editat, per traspassar el seu valor al Slider
    void m_intervalImagesLineEditTextEdited(const QString &text);
    /// Slot que s'exectua quan el LineEdit per especificar a partir de quina imatges s'haa imprimir �s editat, per traspassar el seu valor al Slider
    void m_fromImageLineEditTextEdited(const QString &text);
    /// Slot que s'exectua quan el LineEdit per especificar fins quina imatge s'ha d'imprimir �s editat, per traspassar el seu valor al Slider
    void m_toImageLineEditTextEdited(const QString &text);
    /// Actualitza label indicant el n�mero de p�gines dicom print que s'imprimiran
    void updateNumberOfDicomPrintPagesToPrint();

    /// Envia a imprimir les imatges seleccionades a la impressora seleccionada
    void print();

    /// Amaga el frame que indica que s'han enviat a imprimir correctament les imatges
    void timeoutTimer();

private:

    /// Crea les connexions
    void createConnections();

    /// Crea inputValidators pels lineEdit de la selecci� d'imatges
    void configureInputValidator();

    /// Inicialitza les tools que volem tenir activades al viewer
    void initializeViewerTools();

    /// Configura els controls de selecci� d'imatges en funci� dels nombre d'imatges
    void updateSelectionImagesValue();

    /// Retorna el DicomPrintJob que s'ha d'enviar a imprimir en funci� de la impressora i imatges seleccionades
    DicomPrintJob getDicomPrintJobToPrint();

    /// Retorna una llista de p�gines per imprimir
    QList<DicomPrintPage> getDicomPrintPageListToPrint();

    /// Retorna les imatges s'han d'enviar a imprimir en funci� de lo definit a la selecci� d'imatges
    QList<Image*> getSelectedImagesToPrint();

    /// Retorna la impressora seleccionada
    DicomPrinter getSelectedDicomPrinter();

    /// Indica amb la selecci� actua el n�mero de p�gines a imprimir, sin� l'ha pogut calcu�lar correctament retorna 0
    int getNumberOfPagesToPrint();

    /// Retorna el n�mero d'imatges que caben en la p�gina en funci� del layout escollit
    int getNumberOfImagesPerFilm(QString filmLayout);

    /// Ens retorna un DicomPrintPage amb els par�metres d'impressi� omplerts a partir d'una impressora. No afegeix les imatges ni n�mero de p�gina
    DicomPrintPage fillDicomPrintPagePrintSettings(DicomPrinter dicomPrinter);

    /// Mostra per pantalla els errors que s'han produ�t alhora d'imprimir
    /// Degut a nom�s podem tenir una p�gina per FilmSession degut a limitacions de dcmtk fa que haguem d'imprimir p�gina per p�gina
    /// per aix� ens podem trobar que la primera p�gina s'imprimeixi b�, i les restants no, per aix� passem el par�metre printedSomePage per indica que nom�s
    /// algunes de les p�gines han fallat
    void showDicomPrintError(DicomPrint::DicomPrintError error, bool printedSomePage);

    /// Comprova si se suporta el format de la s�rie i actualitza la interf�cie segons convingui.
    void updateVolumeSupport();

    void setEnabledPrintControls(bool enabled);

    /// Ens afegeix anotacions a la p�gina a imprimir amb informaci� de la s�rie i de l'estudi, com nom de pacient,
    /// Data i hora estudi, Instituci�, ID Estudi i descripci�,...
    void addSeriesInformationAsAnnotationsToDicomPrintPage(DicomPrintPage *dicomPrintPage, Series *seriesToPrint);

private:
    /// Gestor de tools pel viewer
    ToolManager *m_toolManager;

    QTimer *m_qTimer;

    QDicomPrinterConfigurationWidget *m_qDicomPrinterConfigurationWidgetProof;
};

} // end namespace udg.

#endif
