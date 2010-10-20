/***************************************************************************
 *   Copyright (C) 2005-2007 by Grup de Gr�fics de Girona                  *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#ifndef UDGNONCLOSEDANGLETOOLREPRESENTATION_H
#define UDGNONCLOSEDANGLETOOLREPRESENTATION_H

#include "toolrepresentation.h"

#include <QPointer>

namespace udg {

class DrawerLine;
class DrawerText;

/**
    Representacio de l'eina d'angles oberts. (C�lcul)
*/
class NonClosedAngleToolRepresentation : public ToolRepresentation {
Q_OBJECT
public:
    NonClosedAngleToolRepresentation(Drawer *drawer, QObject *parent = 0);
    ~NonClosedAngleToolRepresentation();

    /// Se li passa les linies ja dibuixada per un outliner
    void setLines(DrawerLine *firstLine, DrawerLine *secondLine);

    /// Se li passen els parametres necessaris pel calcul
    void setParams(int view, int slice);

    /// Calcula la distancia i escriu el text
    void calculate();

private:
    /// Calcula l'angle de les dues l�nies dibuixades
    void computeAngle();

    /// Calcula la correcta posici� del caption de l'angle segons els punts de l'angle
    void textPosition(double *p1, double *p2);

    /// Crea els handlers
    void createHandlers();

    /// Mapeja els handlers amb els punts corresponents
    QList<double *> mapHandlerToPoint(ToolHandler *toolHandler);

private slots:
    /// Edita els v�rtexs de les DrawerPrimitive
    void moveAllPoints(double *movement);

private:
    /// Primera l�nia
    QPointer<DrawerLine> m_firstLine;

    /// Segona l�nia
    QPointer<DrawerLine> m_secondLine;

    /// L�nia d'uni�
    QPointer<DrawerLine> m_middleLine;

    /// Text amb el resultat del c�lcul
    QPointer<DrawerText> m_text;

    /// Par�metres necessaris per la representaci�
    int m_view;
    int m_slice;
};
}

#endif
