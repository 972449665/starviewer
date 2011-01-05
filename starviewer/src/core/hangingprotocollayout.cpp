/***************************************************************************
 *   Copyright (C) 2005-2007 by Grup de Gràfics de Girona                  *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#include "hangingprotocollayout.h"

namespace udg {

HangingProtocolLayout::HangingProtocolLayout(QObject *parent)
: QObject(parent)
{
}


HangingProtocolLayout::~HangingProtocolLayout()
{
}

void HangingProtocolLayout::setNumberOfScreens(int numberOfScreens)
{
    m_numberOfScreens = numberOfScreens;
}

void HangingProtocolLayout::setVerticalPixelsList(const QList<int> &verticalPixelsList)
{
    m_verticalPixelsList = verticalPixelsList;
}

void HangingProtocolLayout::setHorizontalPixelsList(const QList<int> &horizontalPixelsList)
{
    m_horizontalPixelsList = horizontalPixelsList;
}

void HangingProtocolLayout::setDisplayEnvironmentSpatialPositionList(const QList<QString> &displayEnvironmentSpatialPosition)
{
    m_displayEnvironmentSpatialPosition = displayEnvironmentSpatialPosition;
}

int HangingProtocolLayout::getNumberOfScreens() const
{
    return m_numberOfScreens;
}

QList<int> HangingProtocolLayout::getVerticalPixelsList() const
{
    return m_verticalPixelsList;
}

QList<int> HangingProtocolLayout::getHorizontalPixelsList() const
{
    return m_horizontalPixelsList;
}

QList<QString> HangingProtocolLayout::getDisplayEnvironmentSpatialPositionList() const
{
    return m_displayEnvironmentSpatialPosition;
}

}
