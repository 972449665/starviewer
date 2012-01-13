#ifndef UDGOVALROITOOL_H
#define UDGOVALROITOOL_H

#include "roitool.h"

namespace udg {

class Q2DViewer;
class DrawerText;

/**
    Tool per calcular l'�rea i la mitjana de grisos d'un �val
  */
class OvalROITool : public ROITool {
Q_OBJECT
public:
    OvalROITool(QViewer *viewer, QObject *parent = 0);
    ~OvalROITool();

    void handleEvent(long unsigned eventID);

protected:
    virtual void setTextPosition(DrawerText *text);

private:
    /// Estats de la Tool
    enum { Ready, FirstPointFixed };

    /// Gestiona les accions a realitzar quan es clica el ratol�
    void handlePointAddition();

    /// Simula la forma de l'�val quan tenim el primer punt i movem el mouse
    void simulateOval();

    /// Calcula el centre de l'�val a partir dels punts introduits mitjan�ant la interacci� de l'usuari
    void computeOvalCentre(double centre[3]);

    /// Actualitza els punts del pol�gon perqu� resulti el dibuix de l'�val
    void updatePolygonPoints();

    /// Dona el dibuix de l'�val per finalitzat
    void closeForm();

    /// Elimina la representacio temporal de la tool
    void deleteTemporalRepresentation();

    /// Equalitza la profunditat dels elements que formen l'el�lipse final.
    void equalizeDepth();

private slots:
    /// Inicialitza la tool
    void initialize();

private:
    /// Punts que annotem de la interacci� de l'usuari per crear l'�val
    double m_firstPoint[3];
    double m_secondPoint[3];

    /// Estat de la tool
    int m_state;
};

}

#endif
