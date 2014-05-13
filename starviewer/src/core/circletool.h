#ifndef UDGCIRCLETOOL_H
#define UDGCIRCLETOOL_H

#include "tool.h"

#include "vector3.h"

#include <QPointer>

namespace udg {

class DrawerPolygon;
class Q2DViewer;

/**
    Eina per dibuixar un cercle amb una marca al centre.
 */
class CircleTool : public Tool {

    Q_OBJECT

public:

    CircleTool(QViewer *viewer, QObject *parent = 0);
    ~CircleTool();

    /// Decideix qu� s'ha de fer per cada event rebut.
    virtual void handleEvent(unsigned long eventId);

private:

    /// Comen�a a dibuixar el cercle.
    void startDrawing();
    /// Acaba de dibuixar el cercle.
    void endDrawing();
    /// Avorta el dibuix del cercle.
    void abortDrawing();
    /// Actualitza el cercle quan es mou el ratol� mentre s'est� dibuixant.
    void updateCircle();
    /// Obt� el punt on acaba la bounding box del cercle, assegurant-se que entre aquest i l'inicial es forma un quadrat.
    void getEndPoint();
    /// Actualitza els punts del pol�gon perqu� resulti el dibuix del cercle.
    void updatePolygonPoints();
    /// Retorna el centre del cercle.
    Vector3 getCenter() const;
    /// Equalitza la profunditat dels elements que formen el cercle final.
    void equalizeDepth();

private slots:

    /// Inicialitza l'eina.
    void initialize();

private:

    /// Visor 2D sobre el qual treballem.
    Q2DViewer *m_2DViewer;
    /// El cercle.
    QPointer<DrawerPolygon> m_circle;
    /// Cert mentre s'est� dibuixant un cercle.
    bool m_isDrawing;
    /// Punt on comen�a la bounding box del cercle.
    Vector3 m_startPoint;
    /// Punt on acaba la bounding box del cercle.
    Vector3 m_endPoint;

};

}

#endif
