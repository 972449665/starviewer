#ifndef UDGDICOMPRINTFACTORY_H
#define UDGDICOMPRINTFACTORY_H

#include "imageprintfactory.h"

/**
 * Implementaci� del Factory ImagePrint per la impressi� mitjan�ant DCMTK.
 * S'ha aplicat el patr� Abstract Factory.
 */

namespace udg 
{
    class Printer;
    class PrintMethod;
    class PrinterHandler;
    class QPrinterConfigurationWidget;
    class QPrintingConfigurationWidget;
    class QPrintJobCreatorWidget;

class DicomPrintFactory : public ImagePrintFactory
{

public:
	 
	DicomPrintFactory();
    ~DicomPrintFactory();
	
    Printer						* getPrinter();
    PrintMethod					* getPrintMethod();
    PrinterHandler              * getPrinterHandler();
    QPrinterConfigurationWidget * getPrinterConfigurationWidget();
    QPrintingConfigurationWidget* getPrintingConfigurationWidget();
    QPrintJobCreatorWidget		* getPrintJobCreatorWidget();

private:
    Printer                     * m_printer;
    PrintMethod                 * m_printMethod;
    PrinterHandler              * m_printerHandler;
    QPrinterConfigurationWidget * m_qPrinterconfigurationWidget;
    QPrintingConfigurationWidget* m_qPrintingConfigurationWidget;
    QPrintJobCreatorWidget      * m_qPrintJobCreatorWidget;
};
}

#endif
