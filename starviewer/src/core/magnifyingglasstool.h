#ifndef UDGMAGNIFYINGGLASSTOOL_H
#define UDGMAGNIFYINGGLASSTOOL_H

#include "tool.h"

namespace udg {

class MagnifyingGlassToolData;
class Q2DViewer;
class Volume;

class MagnifyingGlassTool : public Tool {
Q_OBJECT
public:
    MagnifyingGlassTool(QViewer *viewer, QObject *parent = 0);
    ~MagnifyingGlassTool();

    void handleEvent(unsigned long eventID);

    void setToolData(ToolData *data);
    
protected:
    /// Viewer 2D sobre el qual treballem
    Q2DViewer *m_2DViewer;

private slots:
    /// Actualitza tots els par�metres la vista magnificada
    void updateMagnifiedView();

    /// Actualitza la posici� del widget segons el moviment del cursor
    void updateMagnifyingGlassWidgetPosition();

    /// Actualitza la posici� de la imatge dins de la vista magnificada
    void updateMagnifiedImagePosition();

    /// M�todes per sincronitzar el visor magnificat amb els canvis del visor de refer�ncia 
    void setVolume(Volume *volume);
    void setView(int view);
    void setSlice(int slice);
    void setPhase(int phase);
    void setWindowLevel(double window, double level);
    void setZoom(double zoom);
    void setSlabThickness(int thickness);
    void updateRotationFactor(int rotationFactor);
    void horizontalFlip();

private:
    /// Crea les connexions
    void createConnections();

private:
    /// Dades espec�fiques de la tool
    MagnifyingGlassToolData *m_myData;
    double m_window;
    double m_level;
};

}

#endif
