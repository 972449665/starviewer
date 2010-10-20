/***************************************************************************
 *   Copyright (C) 2005-2007 by Grup de Gr�fics de Girona                  *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#ifndef UDGTOOLHANDLERROTATION_H
#define UDGTOOLHANDLERROTATION_H

#include "toolhandler.h"

#include <QPointer>

namespace udg {

class Drawer;
class DrawerPoint;

/**
    Handlers per crear rotacions. La seva representaci� �s un punt.
*/
class ToolHandlerRotation : public ToolHandler {
Q_OBJECT
public:    
    ToolHandlerRotation(Drawer *drawer, double *point, QObject *parent = 0);
    ~ToolHandlerRotation();

    void update();

    /// Estableix el centre de rotaci�
    void setRotationCenter(double *center);

    /// Estableix una nova posici�
    void setPosition(double *point);

public slots:
    void handleEvents(long unsigned eventID, double *point);

    /// Amaguen i mostren el handler segons l'estat de la ToolRepresentation
    void hide();
    void show();

    void isClickNearMe(double *point);

    /// Despla�a el handler
    void move(double *movement);

private:
    /// Refresca el handler al visor
    void repaintHandler();

    /// Calcula l'angle de rotaci�
    double calculateRotationAngle(double *point);

private:
    /// Drawer on es pintar� el handler
    Drawer *m_drawer;

    /// Punt que fa de handler
    QPointer<DrawerPoint> m_point;

    /// Centre de rotaci� de la figura
    double m_rotationCenter[3];
    
};

}

#endif
