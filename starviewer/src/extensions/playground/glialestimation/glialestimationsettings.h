#ifndef UDGGLIALESTIMATIONSETTINGS_H
#define UDGGLIALESTIMATIONSETTINGS_H

#include "defaultsettings.h"

namespace udg {

class GlialEstimationSettings : public DefaultSettings
{
public:
    GlialEstimationSettings();
    ~GlialEstimationSettings();

    void init();

    /// Declaraci� de claus (aquesta extensi� no t� encara cap setting!)
};

} // end namespace udg 

#endif