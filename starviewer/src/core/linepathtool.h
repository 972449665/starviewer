/***************************************************************************
 *   Copyright (C) 2010 by Grup de Gr�fics de Girona                       *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#ifndef UDGLINEPATHTOOL_H
#define UDGLINEPATHTOOL_H

#include "tool.h"
#include "volume.h"
#include <QPointer>

namespace udg {

class Q2DViewer;
class DrawerPolyline;

class LinePathTool : public Tool
{
Q_OBJECT
public:
    LinePathTool( QViewer *viewer, QObject *parent = 0 );
    ~LinePathTool();

    void handleEvent( long unsigned eventID );

signals:
    /// Indica que s'ha acabat de dibuixar i emet la polil�nia que s'ha dibuixat
    void finished( QPointer<DrawerPolyline> polyline );

private slots:
    /// Inicialitza l'estat de la tool
    void initialize();

private:
    /// Gestiona quin punt de la polil�nia estem dibuixant. Es cridar� cada cop que 
    /// haguem fet un clic amb el bot� esquerre del mouse.
    void handlePointAddition();

    /// Ens permet anotar el seg�ent punt de la polil�nia. Si la primitiva no ha sigut creada, abans d'afegir el nou punt, la crea.
    void annotateNewPoint();

    /// M�tode que d�na per finalitzada la pol�linia que s'ha dibuixat
    void closeForm();

private:
    /// Viewer 2D sobre el qual treballem
    Q2DViewer *m_2DViewer;

    /// Polil�nia que ens marca la forma que hem anat editant.
    QPointer<DrawerPolyline> m_polyline;
};
}  //  end namespace udg

#endif
