/***************************************************************************
 *   Copyright (C) 2005-2007 by Grup de Gr�fics de Girona                  *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#ifndef UDGPOLYLINEANGLEOUTLINER_H
#define UDGPOLYLINEANGLEOUTLINER_H

#include <QObject>
#include <QPointer>

#include "outliner.h"

namespace udg {

class Q2DViewer;
class DrawerPolyline;

/**
    Classe que dibuixa una DrawerPolyline en forma d'angle
*/
class PolylineAngleOutliner : public Outliner {
Q_OBJECT
public:
    PolylineAngleOutliner(Q2DViewer *viewer, QObject *parent = 0);
    ~PolylineAngleOutliner();

    /// Provisional, aquest calcul s'haura de fer a angletoolrepresentation ja que s'haura de poder editar sense passar per l'outliner
    double getAngleDegrees();

public slots:
    void handleEvent(long unsigned eventID);

private:
    /// Calcula l'angle que fa el primer segment annotat amb l'eix horitzontal
    void findInitialDegreeArc();

    /// Ens permet anotar el primer v�rtex de l'angle.
    void annotateFirstPoint();

    /// Gestiona quin punt de l'angle estem dibuixant. Es cridar� cada cop que 
    /// haguem fet un clic amb el bot� esquerre del mouse.
    void handlePointAddition();
    
    /// Ens simula el segment de l'angle segons els punts annotats
    void simulateCorrespondingSegmentOfAngle();

    /// Dibuixa l'arc de circumfer�ncia que hi ha entre els dos segments 
    /// quan estem definint l'angle
    void drawCircle();
    
    /// Ajustem el primer segment i creem la polil�nia de l'arc de circumfer�ncia
    void fixFirstSegment();
    
    /// Acabem el dibuix de l'angle afegint l'annotaci� textual i eliminant l'arc de circumfer�ncia
    void finishDrawing();

private:
    /// Possibles estats de l'outliner
    enum { CenterFixed, FirstPointFixed, None };

    /// Polil�nia per dibuixar l'angle
    QPointer<DrawerPolyline> m_mainPolyline;

    /// Polil�nia de la circumfer�ncia de l'angle.
    QPointer<DrawerPolyline> m_circlePolyline;
        
    /// Estat de la tool
    int m_state;

    /// Graus que formen inicialment el primer segement amb un segment horitzontal fictici
    /// Ens servir� per calcular cap a on hem de dibuixar l'arc de circumfer�ncia quan 
    /// estem annotant el segon segment de l'angle
    int m_initialDegreeArc;

    /// Angle que formen en tot moment els dos segments
    double m_currentAngle;

};

}

#endif

