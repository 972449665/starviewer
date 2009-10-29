/***************************************************************************
 *   Copyright (C) 2005-2007 by Grup de Gràfics de Girona                  *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#include "distancetool.h"

#include "q2dviewer.h"
#include "logging.h"
#include "drawer.h"
#include "drawerline.h"
#include "drawertext.h"
#include "image.h"
#include "volume.h"
#include "series.h"
// Vtk's
#include <vtkRenderWindowInteractor.h>
#include <vtkCommand.h>
#include <vtkLine.h>

namespace udg {

DistanceTool::DistanceTool( QViewer *viewer, QObject *parent )
 : Tool(viewer, parent)
{
    m_toolName = "DistanceTool";
    m_hasSharedData = false;

    m_2DViewer = qobject_cast<Q2DViewer *>( viewer );
    if( !m_2DViewer )
        DEBUG_LOG(QString("El casting no ha funcionat!!! És possible que viewer no sigui un Q2DViewer!!!-> ")+ viewer->metaObject()->className() );

    m_line = NULL;
    m_hasFirstPoint = false;
    m_hasSecondPoint = false;

    //DEBUG_LOG("DISTANCE TOOL CREADA ");
}

DistanceTool::~DistanceTool()
{
    if ( m_line )
    {
        delete m_line;
    }
}

void DistanceTool::handleEvent( long unsigned eventID )
{
    switch( eventID )
    {
        case vtkCommand::LeftButtonPressEvent:
            if( m_2DViewer->getInput() )
            {
                if ( m_2DViewer->getInteractor()->GetRepeatCount() == 0 )
                {
                    this->annotateNewPoint();
                    m_2DViewer->getDrawer()->refresh();
                }
            }
            break;
        case vtkCommand::MouseMoveEvent:
            if( m_line )
            {
                this->simulateLine();
                m_2DViewer->getDrawer()->refresh();
            }
            break;
    }
}

void DistanceTool::annotateNewPoint()
{
    if ( !m_line )
    {
        m_line = new DrawerLine;
        m_hasFirstPoint = false;
        m_hasSecondPoint = false;
    }

    double position[4];
    double computed[3];

    //capturem l'event de clic esquerre
    int *xy = m_2DViewer->getEventPosition();

    m_2DViewer->computeDisplayToWorld( m_2DViewer->getRenderer() , xy[0], xy[1], 0, position );
    computed[0] = position[0];
    computed[1] = position[1];
    computed[2] = position[2];

    //afegim el punt
    if( !m_hasFirstPoint )
    {
        m_line->setFirstPoint( computed );
        m_hasFirstPoint = true;
    }
    else
    {
        m_line->setSecondPoint( computed );
        //actualitzem els atributs de la linia

        if( !m_hasSecondPoint )
        {
            m_2DViewer->getDrawer()->draw( m_line , m_2DViewer->getView(), m_2DViewer->getCurrentSlice() );
            m_hasSecondPoint = true;
        }
        else
            m_line->update( DrawerPrimitive::VTKRepresentation );

        //Posem el text
        double *leftPoint = m_line->getLeftPoint( m_2DViewer->getView() );

        switch( m_2DViewer->getView() )
        {
            case Q2DViewer::Axial:
                leftPoint[0] -= 2.;
                break;

            case Q2DViewer::Sagital:
                leftPoint[1] -= 2.;
                break;

            case Q2DViewer::Coronal:
                leftPoint[0] -= 2.;
                break;
        }

        DrawerText * text = new DrawerText;
        // HACK Comprovem si l'imatge té pixel spacing per saber si la mesura ha d'anar en píxels o mm
        // TODO proporcionar algun mètode alternatiu per no haver d'haver de fer aquest hack
        const double *pixelSpacing = m_2DViewer->getInput()->getImage(0,0)->getPixelSpacing();

        if ( pixelSpacing[0] == 0.0 && pixelSpacing[1] == 0.0 )
        {
            double * spacing = m_2DViewer->getInput()->getSpacing();
            text->setText( tr("%1 px").arg( m_line->computeDistance( spacing ), 0, 'f', 0 ) );
        }
        else
        {
            text->setText( tr("%1 mm").arg( m_line->computeDistance(), 0, 'f', 2 ) );
        }
        text->setAttatchmentPoint( leftPoint );
        text->setHorizontalJustification( "Right" );
        text->update( DrawerPrimitive::VTKRepresentation );
        m_2DViewer->getDrawer()->draw( text , m_2DViewer->getView(), m_2DViewer->getCurrentSlice() );

        m_line = NULL;//Acabem la linia. Encara no sabem com s'obtindran per modificar
    }
}

void DistanceTool::simulateLine()
{
    double position[4];
    double computed[3];

    //capturem l'event de clic esquerre
    int *xy = m_2DViewer->getEventPosition();

    m_2DViewer->computeDisplayToWorld( m_2DViewer->getRenderer() , xy[0], xy[1], 0, position );
    computed[0] = position[0];
    computed[1] = position[1];
    computed[2] = position[2];

    m_line->setSecondPoint( computed );
    //actualitzem els atributs de la linia

    if( !m_hasSecondPoint )
    {
        m_2DViewer->getDrawer()->draw( m_line , m_2DViewer->getView(), m_2DViewer->getCurrentSlice() );
        m_hasSecondPoint = true;
    }
    else
        m_line->update( DrawerPrimitive::VTKRepresentation );

}

}
