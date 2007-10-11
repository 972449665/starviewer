/***************************************************************************
 *   Copyright (C) 2005 by Grup de Gràfics de Girona                       *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#include "roitooldata.h"
#include <QString>

namespace udg {

ROIToolData::ROIToolData( ROIToolData::PointsList points, QString typeOfROI )
{
    setPoints( points );
    m_typeOfROI = typeOfROI;
}

ROIToolData::~ROIToolData()
{}
    
void ROIToolData::setPoints( ROIToolData::PointsList points )
{
   for ( int i = 0; i < points.count(); i++ ) 
    {
        m_points << points[i];
    }
    
    emit listOfPointsChanged();
}
    
};  // end namespace udg

