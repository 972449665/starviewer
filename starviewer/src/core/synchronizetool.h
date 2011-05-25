#ifndef UDGSYNCHRONIZETOOL_H
#define UDGSYNCHRONIZETOOL_H

#include "tool.h"

namespace udg {

class Q2DViewer;
class SynchronizeToolData;

/**
    Tool de sincronització entre visualitzadors.
  */
class SynchronizeTool : public Tool {
Q_OBJECT
public:
    SynchronizeTool(QViewer *viewer, QObject *parent = 0);
    ~SynchronizeTool();

    void handleEvent(unsigned long eventID);

    /// Assignem una configuracio
    void setConfiguration(ToolConfiguration *configuration);

    /// Per posar dades (compartides)
    virtual void setToolData(ToolData *data);

    /// Per obtenir les dades (per compartir)
    virtual ToolData* getToolData() const;

    /// Habilita o deshabilita la tool
    /// En estat habilitat, rebrà els signals de sincronització
    /// En estat deshabilitat, no sincronitzarà res respecte la resta
    /// Per defecte la tool es troba en estat deshabilitat
    void setEnabled(bool enabled);

private slots:
    /// Posa l'increment a les dades
    void setIncrement(int slice);

    /// Posa el window level a les dades
    void setWindowLevel(double window, double level);

    /// Posa el factor de zoom
    void setZoomFactor(double factor);

    /// Posa el pan
    void setPan(double *motionVector);

    /// Aplica els canvis de canvi de llesca
    void applySliceChanges();

    /// Aplica els canvis del window level
    void applyWindowLevelChanges();

    /// Aplica els canvis de zoom
    void applyZoomFactorChanges();

    /// Aplica els canvis de pan
    void applyPanChanges();

    /// Reseteja els paràmetres amb els que es realitza la sincronització
    /// S'invoca quan es canvia de volum, vista o s'activa l'eina
    void reset();

private:
    /// Llesca anterior per saber l'increment
    int m_lastSlice;

    /// Thickness perdut per arrodoniment
    double m_roundLostSpacingBetweenSlices;

    /// Dades de la tool
    SynchronizeToolData *m_toolData;

    /// Visualitzador 2d al que pertany la tool
    Q2DViewer *m_q2dviewer;
};

}

#endif
