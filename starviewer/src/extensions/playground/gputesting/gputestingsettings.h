#ifndef UDGGPUTESTINGSETTINGS_H
#define UDGGPUTESTINGSETTINGS_H

#include "defaultsettings.h"

namespace udg {

class GPUTestingSettings : public DefaultSettings
{
public:
    GPUTestingSettings();
    ~GPUTestingSettings();

    void init();

    /// Declaraci� de claus
    static const QString TransferFunctionFilesPath;
    static const QString CameraFilesPath;
};

} // end namespace udg 

#endif
