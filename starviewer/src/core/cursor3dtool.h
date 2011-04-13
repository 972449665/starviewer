#ifndef UDGCURSOR3DTOOL_H
#define UDGCURSOR3DTOOL_H

#include "tool.h"

namespace udg {

class Cursor3DToolData;
class Q2DViewer;
class DrawerCrossHair;

/**
    Implementació de la tool del cursor 3D.
*/
class Cursor3DTool : public Tool {
Q_OBJECT
public:
    /// Estats de la tool
    enum { None, Computing };

    Cursor3DTool(QViewer *viewer, QObject *parent = 0);
    ~Cursor3DTool();

    void setToolData(ToolData * data);

    void handleEvent(long unsigned eventID);

    void setVisibility(bool visible);

private slots:
    /// Actualitza les línies a projectar sobre la imatge segons les dades de la tool
    void updateProjectedPoint();

    /// Actualitza el frame of reference de les dades a partir del volum donat
    void updateFrameOfReference();

    /// Aquest slot es crida per actualitzar les dades que marquen quin és el pla de referència
    /// Es cridarà quan el viewer sigui actiu o quan el viewer actiu canvïi d'input
    void refreshReferenceViewerData();

    /// Actualitza la posició de la tool
    void updatePosition();

    /// Amaga el crossHair al canviar de llesca
    void hideCrossHair();

private:
    /// Projecta el pla de referència sobre el pla de localitzador
    void projectPoint();

    /// Inicialitza la posició de la tool
    void initializePosition();

    /// Desactiva la tool
    void removePosition();

    /// Busca la llesca que es troba més a prop del punt i retorna la distancia
    int getNearestSlice(double point[3], double &distance);

private:
    /// Dades específiques de la tool
    Cursor3DToolData *m_myData;

    /// Viewer 2D sobre el qual treballem
    Q2DViewer *m_2DViewer;

    /// Ens guardem el frame of reference del nostre viewer, per no haver de "preguntar cada cop"
    QString m_myFrameOfReferenceUID;

    /// Ens guardem el l'instance UID del nostre viewer, per no haver de "preguntar cada cop"
    QString m_myInstanceUID;

    /// Controlar l'estat de la tool
    int m_state;

    /// Objecte crosshair per representar el punt
    DrawerCrossHair *m_crossHair;
};

}

#endif
