#ifndef UDGSLICINGWHEELTOOL_H
#define UDGSLICINGWHEELTOOL_H

#include "slicingtool.h"

namespace udg {

class Q2DViewer;
class ToolProxy;

/**
    Tool que hereta de SlicingTool que serveix per fer slicing amb la rodeta en un visor 2D
  */
class SlicingWheelTool : public SlicingTool {
Q_OBJECT
public:
    SlicingWheelTool(QViewer *viewer, QObject *parent = 0);
    ~SlicingWheelTool();

    void handleEvent(unsigned long eventID);

private:
    /// Updates the current slicing mode according to the slicing tool in the current viewer, if slicing tool exists.
    void updateSlicingModeAccordingToSlicingTool();

private:
    /// Guardem els estadistics dels girs de rodeta.
    QString m_wheelSteps;
};

}

#endif
