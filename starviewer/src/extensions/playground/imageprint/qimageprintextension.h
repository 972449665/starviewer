
#ifndef UDGQIMAGEPRINTEXTENSION_H
#define UDGQIMAGEPRINTEXTENSION_H

#include "ui_qimageprintextensionbase.h"


namespace udg {

class ImagePrintFactory;
class QPrintJobCreatorWidget;
class QPrinterConfigurationWidget;
class Image;
class DicomPrintPage;
class DicomPrintJob;
class Volume;

class QImagePrintExtension : public QWidget , private::Ui::QImagePrintExtensionBase {
Q_OBJECT

public:
    QImagePrintExtension( QWidget *parent = 0 );
    ~QImagePrintExtension();

public slots:

    void configurationPrinter();

    ///Li assigna el volum que s'ha d'imprimir
    void setInput( Volume *input );

private slots:

    //Actualitza la informaci� que mostrem de la impressora 
    void selectedDicomPrinterChanged(int indexOfSelectedDicomPrinter);
    ///Omple el combobox amb les impressores i mostra com a seleccionada la que esta marca com a impressora per defecte
    void  fillSelectedDicomPrinterComboBox();
    ///activa/desactiva el fram que cont� poder escollir el interval i des de quina imatge fins quina imprimir en funci� del mode de selecci� d'imatges escollit
    void imageSelectionModeChanged();
    ///Slot que s'executa quan canviem de valor a trav�s del Slider l'interval d'imatges a imprimir
    void m_intervalImagesSliderValueChanged(int value);
    ///Slot que s'executa quan canviem de valor a trav�s del Slider a partir de quina imatge imprimir
    void m_fromImageSliderValueChanged(int value);
    ///Slot que s'executa quan canviem de valor a trav�s del Slider fins quina imatge imprimir
    void m_toImageSliderValueChanged(int value);
    ///Slot que s'exectua quan el LineEdit per especificar el valor del interval d'imatges a imprimir �s editat, per traspassar el seu valor al Slider
    void m_intervalImagesLineEditTextEdited(const QString &text);
    ///Slot que s'exectua quan el LineEdit per especificar a partir de quina imatges s'haa imprimir �s editat, per traspassar el seu valor al Slider
    void m_fromImageLineEditTextEdited(const QString &text);
    ///Slot que s'exectua quan el LineEdit per especificar fins quina imatge s'ha d'imprimir �s editat, per traspassar el seu valor al Slider
    void m_toImageLineEditTextEdited(const QString &text);
    ///Actualitza label indicant el n�mero de p�gines dicom print que s'imprimiran
    void updateNumberOfDicomPrintPagesToPrint();

    //M�tode que ens carrega les imatges a partir del UID d'estudi i s�rie entrat pels lineEdit, M�tode temporal fins que no ens passi un v�lum
    void loadImagesToPrint();

    ///Envia a imprimir les imatges seleccionades a la impressora seleccionada
    void print();
private:
    ImagePrintFactory		* m_factory;
	QPrinterConfigurationWidget *m_printerConfigurationWidgetProof;
    QList<Image*> m_imageListToPrint; //Temporal fins que tinguem el volum d'on agafar les imatges

    ///Crea les connexions
	void createConnections();

    ///Crea inputValidators pels lineEdit de la selecci� d'imatges
	void configureInputValidator();
    ///Configura els controls de selecci� d'imatges en funci� dels nombre d'imatges

    void setSelectionImagesValue(QList<Image*> imageList);
    ///Retorna una llista de p�gines per imprimir

    ///Retorna el DicomPrintJob que s'ha d'enviar a imprimir en funci� de la impressora i imatges seleccionades
    DicomPrintJob getDicomPrintJobToPrint();

    QList<DicomPrintPage> getDicomPrintPageListToPrint();

    ///Retorna les imatges s'han d'enviar a imprimir en funci� de lo definit a la selecci� d'imatges
    QList<Image*> getSelectedImagesToPrint();

    ///Retorna la impressora seleccionada
    DicomPrinter getSelectedDicomPrinter();

    ///Indica amb la selecci� actua el n�mero de p�gines a imprimir, sin� l'ha pogut calcu�lar correctament retorna 0
    int getNumberOfPagesToPrint();

    ///Retorna el n�mero d'imatges que caben en la p�gina en funci� del layout escollit
    int getNumberOfImagesPerFilm(QString filmLayout);

    ///ens retorna un DicomPrintPage amb els par�metres d'impressi� omplerts a partir d'una impressora. No afegeix les imatges ni n�mero de p�gina
    DicomPrintPage fillDicomPrintPagePrintSettings(DicomPrinter dicomPrinter);
};
} // end namespace udg.

#endif
