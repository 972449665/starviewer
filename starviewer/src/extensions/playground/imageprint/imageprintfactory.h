#ifndef UDGIMAGEPRINTFACTORY_H
#define UDGIMAGEPRINTFACTORY_H

/**
 * Factory d'objectes que s�n utilitzats a la impressi� d'imatges (Dicom, Paper...).
 * S'ha aplicat el patr� Abstract Factory.
 */

namespace udg {

    class Printer;
    class PrintMethod;
    class PrinterManager;
    class QPrinterConfigurationWidget;
    class QPrintingConfigurationWidget;
    class QPrintJobCreatorWidget;

class ImagePrintFactory 
{	
public:
    /// Retorna la impressora.
    virtual Printer * getPrinter()=0;
    
    /// Retorna el m�tode d'impressi�
    virtual PrintMethod * getPrintMethod()=0;
    
     /// Retorna el manipulador de la impressora (Afegir, Eliminar...).
    virtual PrinterManager  * getPrinterManager()=0;
    
    /// Retorna el Widget encarregat de manipular la impressora (Afegir, Elimnar...)
    virtual QPrinterConfigurationWidget * getPrinterConfigurationWidget()=0;
    
    /// Retorna el Widget encarregat de configurar els par�metres de la impressora per una impressi�.
    virtual QPrintingConfigurationWidget* getPrintingConfigurationWidget()=0;
    
    /// Retorna el widget encarregat de configurar el PrintJob per una impressi�.
    virtual QPrintJobCreatorWidget * getPrintJobCreatorWidget() =0;
};
} 
#endif
