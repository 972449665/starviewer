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

#include "synccriterion.h"

#include "qviewer.h"

namespace udg {

SyncCriterion::SyncCriterion()
{
}

SyncCriterion::~SyncCriterion()
{
}

bool SyncCriterion::isCriterionSatisfied(QViewer *viewer1, QViewer *viewer2)
{
    if (viewer1 && viewer2)
    {
        if (!viewer1->hasInput() || !viewer2->hasInput())
        {
            return false;
        }
        else
        {
            return criterionIsMet(viewer1, viewer2);
        }
    }
    else
    {
        return false;
    }
}

} // End namespace udg
