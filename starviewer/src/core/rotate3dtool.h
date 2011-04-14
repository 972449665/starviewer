#ifndef UDGROTATE3DTOOL_H
#define UDGROTATE3DTOOL_H

#include "tool.h"

class vtkInteractorStyle;

namespace udg {

class QViewer;

/**
    Eina per rotacions tridimensionals ( pensat per visors 3D )
*/
class Rotate3DTool : public Tool {
Q_OBJECT
public:
    enum { None , Rotating, Spinning };

    Rotate3DTool( QViewer *viewer, QObject *parent = 0 );
    ~Rotate3DTool();

    void handleEvent( unsigned long eventID );

private slots:
    /// Comença el translate
    void startRotate3D();

    /// Calcula el nou translate
    void doRotate3D();

    /// Atura l'estat de translate
    void endRotate3D();
    
private:
    /// Interactor style que omplirem en el constructor depenent del visor
    vtkInteractorStyle *m_interactorStyle;
    
    /// Estat de la tool
    int m_state;
};

}

#endif
