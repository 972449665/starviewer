#ifndef UDGQABOUTDIALOG_H
#define UDGQABOUTDIALOG_H

#include "ui_qaboutdialogbase.h"

namespace udg {

/**
    Di�leg d'about de l'aplicaci�
  */
class QAboutDialog : public QDialog, private Ui::QAboutDialogBase {
Q_OBJECT
public:
    QAboutDialog(QWidget *parent = 0);
    ~QAboutDialog();
};

}

#endif
