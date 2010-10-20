/***************************************************************************
 *   Copyright (C) 2005-2007 by Grup de Gr�fics de Girona                  *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#ifndef UDGPOLYLINEROIOUTLINER_H
#define UDGPOLYLINEROIOUTLINER_H

#include <QObject>
#include <QPointer>

#include "outliner.h"

namespace udg {

class Q2DViewer;
class DrawerPolyline;
class DrawerPolygon;

/**
    Classe que dibuixa una DrawerPolyline en forma de ROI
*/
class PolylineROIOutliner : public Outliner {
Q_OBJECT
public:
    PolylineROIOutliner(Q2DViewer *viewer, QObject *parent = 0);
    ~PolylineROIOutliner();

public slots:
    void handleEvent(long unsigned eventID);

private:
    /// Gestiona quin punt de la ROI estem dibuixant. Es cridar� cada cop que 
    /// haguem fet un clic amb el bot� esquerre del mouse.
    void handlePointAddition();

    /// Ens permet anotar el seg�ent punt de la polil�nia. Si la primitiva no ha sigut creada, abans d'afegir el nou punt, la crea.
    void annotateNewPoint();

    /// Ens simula com quedaria la polil�nia que estem editant si la tanqu�ssim. Ens serveix per a veure din�micament l'evoluci� de la polil�nia.
    void simulateClosingPolyline();

    /// M�tode que tanca la forma de la polil�nia que s'ha dibuixat
    void closeForm();

private:
    /// Polil�nia principal: �s la polil�nia que ens marca la forma que hem anat editant.
    QPointer<DrawerPolyline> m_mainPolyline;

    /// Polil�nia de tancament: es la polil�nia que ens simula com quedaria la polil�nia principal si es tanques, es a dir, uneix l'�ltim punt anotat i el primer punt de la polil�nia.
    QPointer<DrawerPolyline> m_closingPolyline;

    /// Pol�gon que defineix la ROI
    QPointer<DrawerPolygon> m_roiPolygon;

};

}

#endif

