#ifndef UDGQDIAGNOSISTESTRESULTWIDGET_H
#define UDGQDIAGNOSISTESTRESULTWIDGET_H

#include "ui_qdiagnosistestresultwidgetbase.h"

#include <QWidget>

class QMouseEvent;

namespace udg {

class DiagnosisTest;
class DiagnosisTestResult;

/**
    Aquest widget ens mostrar un objecte DiagnosisTest amb el seu resultat DiagnosisTestResult
  */

class QDiagnosisTestResultWidget : public QWidget, private Ui::QDiagnosisTestResultWidgetBase {
Q_OBJECT
public:
    QDiagnosisTestResultWidget(DiagnosisTest *diagnosisTest, DiagnosisTestResult diagnosisTestResult);

    /// Expandeix el contingut
    void expand();

    /// Contrau el contingut
    void contract();

    /// Indica si es pot expandir. Es poden expandir tots excepte els que mostra un DiagnosisTestResult que no ha fallat
    bool isExpandable();

    // Assignar la mida del widget pare per poder calcular la mida correcte dels labels amb wordwrap
    void setParentWidgetWidth(int parentWidgetWidth);

signals:
    ///Signal que s'emet quan s'ha canviat la mida del Widget
    void clicked(QDiagnosisTestResultWidget *);

protected:
    /// Comprova si s'ha fet click al frame que mostra la descripci� del test, si �s aix� es mostren/s'amaguen la drescripci� de l'error del test i la soluci�
    void mouseReleaseEvent (QMouseEvent * event);
    /// Retorna la mida adequada del widget
    QSize sizeHint() const;

private:

    ///Retorna la icoona que s'ha de mostrar en funci� del resultat del test
    QPixmap getIconLabel(const DiagnosisTestResult &diagnosisTestResult) const;

    ///Indica si el click s'ha fet a l'�rea del Frame que mostrar la descripci� del test
    bool clickIsInAreaTestDescriptionFrame(QMouseEvent *event);

    /// Calcula quantes l�nies de text ocupa un QString en un QLabel amb word wrap
    int countNumberOfLinesWithWordWrap(QTextEdit *textEdit) const;

private:
    bool m_isExpanded;
    bool m_isExpandable;

    /// Nombre de l�nies que ocupa la descripci� del resultat del test en un label amb word wrap
    int m_diagnosisTestResultDescriptionLines;
    /// Nombre de l�nies que ocupa la soluci� del resultat del test en un label amb word wrap
    int m_diagnosisTestResultSolutionLines;

    // Mida del widget pare
    int m_parentWidgetWidth;
};

}
#endif 
