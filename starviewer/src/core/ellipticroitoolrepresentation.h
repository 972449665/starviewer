#ifndef UDGELLIPTICROITOOLREPRESENTATION_H
#define UDGELLIPTICROITOOLREPRESENTATION_H

#include "roitoolrepresentation.h"

namespace udg {

/**
    Representaci� per la tool de ROI el�l�ptica
*/
class EllipticROIToolRepresentation : public ROIToolRepresentation {
Q_OBJECT
public:
    EllipticROIToolRepresentation(Drawer *drawer, QObject *parent = 0);
    ~EllipticROIToolRepresentation();

protected:
    void createHandlers();
};

}

#endif
