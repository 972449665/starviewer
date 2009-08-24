#ifndef UDGQDICOMPRINTINGCONFIGURATIONWIDGET_H
#define UDGQDICOMPRINTINGCONFIGURATIONWIDGET_H

#include "ui_qdicomprintingconfigurationwidgetbase.h"
#include "qprintingconfigurationwidget.h"

/**
 * Implementaci� de QPrintingConfigurationWidget per impressores DICOM en DCMTK.
 */

namespace udg {

class Printer;

class QDicomPrintingConfigurationWidget : public QPrintingConfigurationWidget , private::Ui::QDicomPrintingConfigurationWidgetBase
{
Q_OBJECT

public:
    QDicomPrintingConfigurationWidget();
    ~QDicomPrintingConfigurationWidget();
	
    void	  setPrinter(Printer& _printer);
    Printer *  getPrinter();

private slots:

    ///Test de la part d'impressi� mentre s'est� desenvolupant
    void testPrinting();

private:
    void createConnections();
    void createActions();
};
} 

#endif
