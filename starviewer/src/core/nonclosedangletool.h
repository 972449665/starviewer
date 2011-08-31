#ifndef UDGNONCLOSEDANGLETOOL_H
#define UDGNONCLOSEDANGLETOOL_H

#include "tool.h"
#include <QPointer>

namespace udg {

class Q2DViewer;
class DrawerLine;
class DrawerText;

/**
    Tool per calcular angles
  */
class NonClosedAngleTool : public Tool {
Q_OBJECT
public:
    /// Possibles estats de la tool
    enum { None, FirstLineFixed, SecondLineFixed };

    NonClosedAngleTool(QViewer *viewer, QObject *parent = 0);
    ~NonClosedAngleTool();

    void handleEvent(long unsigned eventID);

private:
    /// Gestiona quin punt de l'angle estem dibuixant. Es cridarà cada cop que
    /// haguem fet un clic amb el botó esquerre del mouse.
    void handlePointAddition();

    /// Ens permet anotar els punts de les línies.
    void annotateLinePoints();

    /// Gestiona quina de les línies estem dibuixant mentres es mou el mouse
    void handleLineDrawing();

    /// Ens simula la linia que estem dibuixant respecte el punt on està el mouse.
    void simulateLine(DrawerLine *line);

    /// Calcula l'angle de les dues línies dibuixades
    void computeAngle();

    /// Calcula la correcta posició del caption de l'angle segons els punts de l'angle
    void placeText(double *firstLineVertex, double *secondLineVertex, DrawerText *angleText);

private slots:
    /// Inicialitza l'estat de la tool
    void initialize();

private:
    /// Estats de la línia segons els punts
    enum { NoPoints, FirstPoint };

    /// Viewer 2D sobre el qual treballem
    Q2DViewer *m_2DViewer;

    /// Primera línia
    QPointer<DrawerLine> m_firstLine;

    /// Segona línia
    QPointer<DrawerLine> m_secondLine;

    /// Línia d'unió
    QPointer<DrawerLine> m_middleLine;

    /// Estat de la tool
    int m_state;

    /// Estat d'una línia
    int m_lineState;
};

}

#endif
