/*************************************************************************************
  Copyright (C) 2014 Laboratori de Gràfics i Imatge, Universitat de Girona &
  Institut de Diagnòstic per la Imatge.
  Girona 2014. All rights reserved.
  http://starviewer.udg.edu

  This file is part of the Starviewer (Medical Imaging Software) open source project.
  It is subject to the license terms in the LICENSE file found in the top-level
  directory of this distribution and at http://starviewer.udg.edu/license. No part of
  the Starviewer (Medical Imaging Software) open source project, including this file,
  may be copied, modified, propagated, or distributed except according to the
  terms contained in the LICENSE file.
 *************************************************************************************/

#ifndef UDGSINGLEVOLUMEDISPLAYUNITHANDLER_H
#define UDGSINGLEVOLUMEDISPLAYUNITHANDLER_H

#include "genericvolumedisplayunithandler.h"

namespace udg {

/**
    VolumeDisplayUnit handler to handle a single input.
    Maximum number of inputs will be one.
*/
class SingleVolumeDisplayUnitHandler : public GenericVolumeDisplayUnitHandler {
public:
    SingleVolumeDisplayUnitHandler();
    virtual ~SingleVolumeDisplayUnitHandler();

    virtual int getMaximumNumberOfInputs() const;

    /// Returns the prop that should be added to the renderer.
    virtual vtkImageSlice* getImageProp() const;
};

} // End namespace udg

#endif
