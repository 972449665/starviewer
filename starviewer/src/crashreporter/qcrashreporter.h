#ifndef UDGQCRASHREPORTER_H
#define UDGQCRASHREPORTER_H

#include <QWidget>
#include "ui_qcrashreporterbase.h"

namespace udg {

/** Aquesta classe és l'encarregada de fer tota la gestió del crash reporter. Bàsicament el que fa és informar a l'usuari que l'aplicació ha petat, 
  * permet veure la trama del problema i enviar la informació a l'equip de desenvolupament si ho creu oportú.
  */
class QCrashReporter : public QWidget, Ui::QCrashReporterBase {
Q_OBJECT

public:
    QCrashReporter( const QStringList& argv , QWidget *parent = 0);

public Q_SLOTS:
    void quitButtonClickedSlot();
    void restartButtonClickedSlot();

private:
    bool restart(const QString &path);
    void sendReport();
    void maybeSendReport();

private:
    QString m_minidumpPath;
};

}

#endif
