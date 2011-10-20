#ifndef UDGDISPLAYSHUTTER_H
#define UDGDISPLAYSHUTTER_H

#include <QPolygon>

class QColor;
class QImage;

namespace udg {

/*
    Un display shutter consisteix en una forma geom�trica (cercle, rectangle o pol�gon)
    que defineix la zona que ha de ser visible en la imatge que s'associa. La part continguda dins de la forma �s la visible i la
    que queda fora �s la part oclosa.
 */
class DisplayShutter {
public:
    DisplayShutter();
    ~DisplayShutter();

    enum ShapeType { UndefinedShape = 0, RectangularShape, PolygonalShape, CircularShape };
    
    /// Assigna/Obt� la forma del shutter
    void setShape(ShapeType shape);
    ShapeType getShape() const;

    /// Ens retorna la forma en una cadena de text segons els valors establerts pel DICOM
    QString getShapeAsDICOMString() const;
    
    /// Valor de gris amb el que s'hauria de pintar la part opaca del shutter
    void setShutterValue(unsigned short int value);
    unsigned short int getShutterValue() const;

    /// Ens retorna el valor del shutter en forma de QColor
    QColor getShutterValueAsQColor() const;
    
    /// M�todes per assignar els punts per cada tipus de forma. En cas que el format de punts no es correspongui
    /// amb la forma ja assignada, retornar� fals i no es guardar� cap punt. En cas que no s'hagi definit cap forma
    /// (shape = UndefinedShape) el m�tode assignar� autom�ticament la forma que pertoqui per aquell m�tode guardant els punts indicats.
    
    /// Assigna el centre i el radi per la forma circular
    bool setPoints(const QPoint &centre, int radius);
    
    /// Assigna els v�rtexs per la forma poligonal
    bool setPoints(const QVector<QPoint> &vertices);
    
    /// Assigna els punts superior esquerre i inferior dret que defineixen la forma rectangular
    bool setPoints(const QPoint &topLeft, const QPoint &bottomRight);

    /// Assigna els punts en format text. Cada punt tindr� la forma x,x, dos valors enters (x) separats per coma (,) 
    /// Cada punt dins de la llista anir� separat per punt i coma (;)
    /// Per la forma rectangular tindrem 2 punts corresponents al punt superior esquerre i al punt inferior dret del rectangle
    /// Per la forma circular el format ser� centre;radi on radi ser� un valor enter
    /// Per la forma poligonal el format ser� un llistat de punts amb un m�nim de 3 elements
    /// En cas que el format de punts no es correspongui amb la forma del shutter, retornar� fals, cert altrament
    /// Si la forma �s indefinida, tractar� de veure si es correspon amb algun format de punts definit i li assignar� en cas de trobar-ne correspond�ncia
    bool setPoints(const QString &pointsString);
    
    /// Retorna el shutter en forma de QPolygon
    QPolygon getAsQPolygon() const;

    /// Retorna els punts del shutter en format d'string. El format ser� el mateix que el de setPoints(const QString &)
    QString getPointsAsString() const;

    /// Retorna el shutter com una QImage de mides width i height
    QImage getAsQImage(int width, int height) const;
    
    /// Donada una llista de shutters, ens retorna el shutter resultant de la intersecci� d'aquests. 
    /// En quant al color resultant, ser� la mitjana de tots els shutters de la llista.
    static DisplayShutter intersection(const QList<DisplayShutter> &shuttersList);

private:
    /// Comprova que el format de l'string de punts �s correcte segons la forma donada
    bool shapeMatchesPointsStringFormat(ShapeType shape, const QString &pointsString);

private:
    /// Forma del shutter
    ShapeType m_shape;

    /// Pol�gon que defineix el shutter. Independentment de la forma que tingui definida sempre es guardar� internament com un pol�gon.
    QPolygon m_shutterPolygon;
    
    /// Valor de gris amb el que s'ha de pintar la part opaca del shutter
    unsigned short int m_shutterValue;
};

} // End namespace udg

#endif
