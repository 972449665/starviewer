#ifndef UDGQPRINTINGCONFIGURATIONWIDGET_H
#define UDGQPRINTINGCONFIGURATIONWIDGET_H

#include "QWidget"

/**
 * Interf�cie pels Widgets encarregats de configurar els par�metres de la impressora per una impressi�.
 */

namespace udg {

class Printer;

class QPrintingConfigurationWidget : public QWidget{
Q_OBJECT

public:
    virtual void	  setPrinter(Printer& _printer) =0;
    virtual Printer * getPrinter() =0;
};
}

#endif
