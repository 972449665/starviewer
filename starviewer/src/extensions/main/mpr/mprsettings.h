#ifndef UDGMPRSETTINGS_H
#define UDGMPRSETTINGS_H

#include "defaultsettings.h"

namespace udg {

class MPRSettings : public DefaultSettings
{
public:
    MPRSettings();
    ~MPRSettings();

    void init();

    /// Declaraci� de claus
    static const QString HorizontalSplitterGeometry;
    static const QString VerticalSplitterGeometry;
};

} // end namespace udg

#endif
