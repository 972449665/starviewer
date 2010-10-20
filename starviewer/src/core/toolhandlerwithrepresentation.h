/***************************************************************************
 *   Copyright (C) 2005-2007 by Grup de Gr�fics de Girona                  *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#ifndef UDGTOOLHANDLERWITHREPRESENTATION_H
#define UDGTOOLHANDLERWITHREPRESENTATION_H

#include "toolhandler.h"

#include <QPointer>

namespace udg {

class Drawer;
class DrawerPoint;

/**
    Handlers per v�rtexs. La seva representaci� �s un punt.
*/
class ToolHandlerWithRepresentation : public ToolHandler {
Q_OBJECT
public:    
    ToolHandlerWithRepresentation(Drawer *drawer, double *point, QColor color = Qt::white, QObject *parent = 0);
    ~ToolHandlerWithRepresentation();

    void update();

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

private:
    /// Drawer on es pintara el handler
    Drawer *m_drawer;

    /// Punt que fa de handler
    QPointer<DrawerPoint> m_point;
};

}

#endif
