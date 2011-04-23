#ifndef QCOLORTRANSFERFUNCTIONGRAPHICALVIEW_H
#define QCOLORTRANSFERFUNCTIONGRAPHICALVIEW_H

#include <QGraphicsView>

namespace udg {

class ColorTransferFunction;

/**
    Vista gr�fica d'una funci� de transfer�ncia de color.
 */
class QColorTransferFunctionGraphicalView : public QGraphicsView {

    Q_OBJECT

public:

    /// Constructor.
    explicit QColorTransferFunctionGraphicalView(QWidget *parent = 0);

    /// Assigna la funci� de transfer�ncia de color.
    void setColorTransferFunction(const ColorTransferFunction &colorTransferFunction);
    /// Demana que s'actualitzi el fons despr�s de moure el ratol�
    /// \todo Es podria fer privat i el node com a classe aniuada privada.
    void requestBackgroundUpdate();
    // void fitInView();    // per ajustar el zoom autom�ticament

signals:

    /// S'emet quan s'afegeix un node.
    void nodeAdded(double x, const QColor &color);
    /// S'emet quan s'esborra un node.
    void nodeRemoved(double x);
    /// S'emet quan es mou un o m�s nodes/punts de color.
    void nodesMoved(const QList< QPair<double, double> > &moves);

protected:

    ///@{
    /// Gesti� d'esdeveniments. \todo Explicar una mica qu� fem a cadascun.
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void wheelEvent(QWheelEvent *event);
    ///@}

private:

    Q_DISABLE_COPY(QColorTransferFunctionGraphicalView)

    /// Actualitza el gradient del fons.
    void updateBackground();
    /// Afegeix un nou node a x.
    void addNode(double x);
    /// Esborra un node a x si existeix.
    void removeNode(double x);
    /// Fa els preparatius per comen�ar a moure nodes.
    void beginMoveNodes();
    /// Finalitza el moviment de nodes.
    void endMoveNodes();

private:

    /// Escena que guarda els nodes.
    QGraphicsScene *m_scene;    // aix� no cal, s'hereta del pare
    /// Nivell de zoom actual.
    double m_zoom;
    /// Indica si s'ha d'actualitzar el fons despr�s de moure el ratol�.
    bool m_backgroundUpdateRequested;

};

} // namespace udg

#endif // QCOLORTRANSFERFUNCTIONGRAPHICALVIEW_H
