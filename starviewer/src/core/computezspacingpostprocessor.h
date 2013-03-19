#ifndef COMPUTEZSPACINGPOSTPROCESSOR_H
#define COMPUTEZSPACINGPOSTPROCESSOR_H

#include "postprocessor.h"

namespace udg {

/**
    Postprocessador que calcula el z-spacing d'un volum a partir de la dist�ncia entre llesques.
 */
class ComputeZSpacingPostprocessor : public Postprocessor {

public:

    /// Modifica el z-spacing del volum perqu� sigui igual a la dist�ncia entre les dues primeres llesques.
    virtual void postprocess(Volume *volume);

};

} // namespace udg

#endif // COMPUTEZSPACINGPOSTPROCESSOR_H
