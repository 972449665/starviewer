/***************************************************************************
 *   Copyright (C) 2005-2007 by Grup de Gràfics de Girona                  *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#include "synchronizetooldata.h"
#include "q2dviewer.h"
#include "logging.h"

namespace udg {

SynchronizeToolData::SynchronizeToolData()
 : ToolData()
{
    m_increment = 0;
}


SynchronizeToolData::~SynchronizeToolData()
{
}

void SynchronizeToolData::setIncrement( double value )
{
    m_increment = value;
    emit( sliceChanged() );
}

double SynchronizeToolData::getIncrement()
{
    return m_increment;
}

void SynchronizeToolData::setWindowLevel( double window, double level )
{
    m_level = level;
    m_window = window;
    emit( windowLevelChanged() );
}

double SynchronizeToolData::getWindow()
{
    return m_window;
}

double SynchronizeToolData::getLevel()
{
    return m_level;
}

void SynchronizeToolData::setZoomFactor( double factor )
{
    m_zoomFactor = factor;
    emit( zoomFactorChanged() );
}

double SynchronizeToolData::getZoomFactor()
{
    return m_zoomFactor;
}

}
