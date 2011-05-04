#ifndef UDGQ3DORIENTATIONMARKER_H
#define UDGQ3DORIENTATIONMARKER_H

#include <QObject>

class vtkOrientationMarkerWidget;
class vtkRenderWindowInteractor;
class vtkAnnotatedCubeActor;

namespace udg {

/**
Classe que encapsula un vtkOrienatationMarker que servirà d'element d'orientació per a visors 3D. Per funcionar només cal crear una instància de la classe i indicar-li el vtkInteractor. La fem heretar de QObjecte per si cal connectar-li signals i slots des d'una altre classe
*/
class Q3DOrientationMarker : public QObject
{
Q_OBJECT
public:
    Q3DOrientationMarker(vtkRenderWindowInteractor *interactor, QObject *parent = 0);

    ~Q3DOrientationMarker();

    /// Li posem les etiquetes de texte que han d'anar per cada cara del cub
    void setOrientationText(QString right, QString left, QString posterior, QString anterior, QString superior, QString inferior);

public slots:
    /// Mètodes per controlar si s'habilita el widget o no
    void setEnabled(bool enable);
    void enable();
    void disable();

private:
    /// Widget per veure la orientació en 3D
    vtkOrientationMarkerWidget *m_markerWidget;

    ///
    vtkAnnotatedCubeActor *m_cubeActor;
};

}

#endif
