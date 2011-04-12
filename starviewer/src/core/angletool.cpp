#include "angletool.h"
#include "q2dviewer.h"
#include "logging.h"
#include "volume.h"
#include "drawer.h"
#include "drawerpolyline.h"
#include "drawertext.h"
#include "mathtools.h"
// VTK
#include <vtkRenderWindowInteractor.h>
#include <vtkCommand.h>

namespace udg {

AngleTool::AngleTool( QViewer *viewer, QObject *parent )
 : Tool(viewer, parent), m_mainPolyline(NULL), m_circlePolyline(NULL), m_state(None)
{
    m_toolName = "AngleTool";
    m_hasSharedData = false;

    m_2DViewer = qobject_cast<Q2DViewer *>( viewer );
    if( !m_2DViewer )
        DEBUG_LOG(QString("El casting no ha funcionat!!! És possible que viewer no sigui un Q2DViewer!!!-> ")+ viewer->metaObject()->className() );
    
    connect( m_2DViewer, SIGNAL( volumeChanged(Volume *) ), SLOT( initialize() ) );
}

AngleTool::~AngleTool()
{
    if ( m_state != None )
    {
        bool hasToRefresh = false;
        // Cal decrementar el reference count perquè 
        // l'annotació s'esborri si "matem" l'eina
        if ( m_mainPolyline )
        {
            m_mainPolyline->decreaseReferenceCount();
            delete m_mainPolyline;
            hasToRefresh = true;
        }
        
        if ( m_circlePolyline )
        {
            m_circlePolyline->decreaseReferenceCount();
            delete m_circlePolyline;
            hasToRefresh = true;
        }

        if( hasToRefresh )
            m_2DViewer->render();
    }
}

void AngleTool::handleEvent( long unsigned eventID )
{
    switch( eventID )
    {
        case vtkCommand::LeftButtonPressEvent:
            handlePointAddition();
        break;

        case vtkCommand::MouseMoveEvent:
            simulateCorrespondingSegmentOfAngle();
        break;
    }
}

void AngleTool::findInitialDegreeArc()
{
    // Per saber quin l'angle inicial, cal calcular l'angle que forma el primer segment anotat i un segment fictici totalment horitzontal.
    double horizontalP2[3], *vd1, *vd2, pv[3];
    double *p1 = m_mainPolyline->getPoint( 0 );
    double *p2 = m_mainPolyline->getPoint( 1 );

    int coord1, depthCoord;

    switch( m_2DViewer->getView() )
    {
        case QViewer::AxialPlane:
            coord1 = 0;
            depthCoord = 2;
            break;

        case QViewer::SagitalPlane:
            coord1 = 1;
            depthCoord = 0;
            break;

        case QViewer::CoronalPlane:
            coord1 = 2;
            depthCoord = 1;
            break;
    }

    for (int i = 0; i < 3; i++)
        horizontalP2[i] = p2[i];

    vd1 = MathTools::directorVector( p1, p2 );

    horizontalP2[coord1] += 10.0;
    vd2 = MathTools::directorVector( horizontalP2, p2 );
    MathTools::crossProduct(vd1, vd2, pv);

    if ( pv[depthCoord] > 0 )
    {
        m_initialDegreeArc =(int)MathTools::angleInDegrees( vd1, vd2 );
    }
    else
    {
        m_initialDegreeArc = -1 * (int)MathTools::angleInDegrees( vd1, vd2 );
    }
}

void AngleTool::annotateFirstPoint()
{
    m_mainPolyline = new DrawerPolyline;

    // Obtenim el punt clickat
    double clickedWorldPoint[3];
    m_2DViewer->getEventWorldCoordinate( clickedWorldPoint );
    // Afegim el punt a la polilínia
    m_mainPolyline->addPoint( clickedWorldPoint );
    // Així evitem que durant l'edició la primitiva pugui ser esborrada per events externs
    m_mainPolyline->increaseReferenceCount();
    m_2DViewer->getDrawer()->draw( m_mainPolyline , m_2DViewer->getView(), m_2DViewer->getCurrentSlice() );

    // Actualitzem l'estat de la tool
    m_state = FirstPointFixed;
}

void AngleTool::fixFirstSegment()
{
    // Posem l'estat de la tool a CenterFixed, així haurà agafat l'últim valor.
    m_state = CenterFixed;

    // Creem la polilínia per a dibuixar l'arc de circumferència i l'afegim al drawer
    m_circlePolyline = new DrawerPolyline;
    // Així evitem que durant l'edició la primitiva pugui ser esborrada per events externs
    m_circlePolyline->increaseReferenceCount();
    m_2DViewer->getDrawer()->draw( m_circlePolyline , m_2DViewer->getView(), m_2DViewer->getCurrentSlice() );
}

void AngleTool::drawCircle()
{
    double degreesIncrease, *newPoint, radius;
    int initialAngle, finalAngle;

    double *firstPoint = m_mainPolyline->getPoint(0);
    double *circleCentre = m_mainPolyline->getPoint(1);
    double *lastPoint = m_mainPolyline->getPoint(2);

    // Calculem l'angle que formen els dos segments
    double *firstSegment = MathTools::directorVector( firstPoint, circleCentre );
    double *secondSegment = MathTools::directorVector( lastPoint, circleCentre );
    m_currentAngle = MathTools::angleInDegrees( firstSegment, secondSegment );
    
    // Calculem el radi de l'arc de circumferència que mesurarà
    // un quart del segment més curt dels dos que formen l'angle
    double distance1 = MathTools::getDistance3D( firstPoint, circleCentre );
    double distance2 = MathTools::getDistance3D( circleCentre, lastPoint );
    radius = MathTools::minimum( distance1, distance2 ) / 4.0;

    // Calculem el rang de les iteracions per pintar l'angle correctament
    initialAngle = 360 - m_initialDegreeArc;
    finalAngle = int(360 - ( m_currentAngle+m_initialDegreeArc ) );

    double pv[3];
    MathTools::crossProduct(firstSegment, secondSegment, pv);

    int view = m_2DViewer->getView();
    int zIndex = Q2DViewer::getZIndexForView(view);
    if ( pv[zIndex] > 0 )
    {
        finalAngle = int(m_currentAngle-m_initialDegreeArc);
    }
    if ( (initialAngle-finalAngle) > 180 )
    {
        initialAngle = int( m_currentAngle-m_initialDegreeArc );
        finalAngle = -m_initialDegreeArc;
    }

    // Reconstruim l'arc de circumferència
    m_circlePolyline->deleteAllPoints();
    for ( int i = initialAngle; i > finalAngle; i-- )
    {
        degreesIncrease = i*1.0*MathTools::DegreesToRadiansAsDouble;
        newPoint = new double[3];

        /*
            TODO Aquí hauríem de fer alguna cosa d'aquest estil, però si ho fem així, 
            no se'ns dibuixa l'arc de circumferència que ens esperem sobre la vista coronal.
            Potser és degut a com obtenim els punts o per una altra causa. Caldria mirar-ho 
            per així evitar la consciència del pla en el que ens trobem
            newPoint[xIndex] = cos( degreesIncrease )*radius + circleCentre[xIndex];
            newPoint[yIndex] = sin( degreesIncrease )*radius + circleCentre[yIndex];
            newPoint[zIndex] = 0.0;
        */
        switch( view )
        {
            case QViewer::AxialPlane:
                newPoint[0] = cos( degreesIncrease )*radius + circleCentre[0];
                newPoint[1] = sin( degreesIncrease )*radius + circleCentre[1];
                newPoint[2] = 0.0;
                break;

            case QViewer::SagitalPlane:
                newPoint[0] = 0.0;
                newPoint[1] = cos( degreesIncrease )*radius + circleCentre[1];
                newPoint[2] = sin( degreesIncrease )*radius + circleCentre[2];
                break;

            case QViewer::CoronalPlane:
                newPoint[0] = sin( degreesIncrease )*radius + circleCentre[0];
                newPoint[1] = 0.0;
                newPoint[2] = cos( degreesIncrease )*radius + circleCentre[2];
                break;
        }
        m_circlePolyline->addPoint( newPoint );
    }

    m_circlePolyline->update();
}

void AngleTool::handlePointAddition()
{
    if( m_2DViewer->getInput() )
    {
        if ( m_2DViewer->getInteractor()->GetRepeatCount() == 0 )
        {
            if ( m_state == None )
                this->annotateFirstPoint();
            else if ( m_state == FirstPointFixed )
            {
                this->fixFirstSegment();
                this->findInitialDegreeArc();
            }
            else
            {
                // Voldrem enregistrar l'últim punt, pertant posem l'estat a none
                m_state = None;
                finishDrawing();
            }
        }
    }
}

void AngleTool::simulateCorrespondingSegmentOfAngle()
{
    if( !m_mainPolyline )
        return;

    if( m_state != None )
    {
        // Agafem la coordenada de pantalla
        double clickedWorldPoint[3];
        m_2DViewer->getEventWorldCoordinate( clickedWorldPoint );

        int pointIndex;
        if( m_state == FirstPointFixed )
            pointIndex = 1;
        else if( m_state == CenterFixed )
            pointIndex = 2;

        // Assignem el segon o tercer punt de l'angle segons l'estat
        m_mainPolyline->setPoint( pointIndex, clickedWorldPoint );
        m_mainPolyline->update();

        if( m_state == CenterFixed )
            drawCircle();
        
        m_2DViewer->render();
    }
}

void AngleTool::finishDrawing()
{
    // Així alliberem les primitives perquè puguin ser esborrades
    m_mainPolyline->decreaseReferenceCount();
    m_circlePolyline->decreaseReferenceCount();
    // Eliminem l'arc de circumferència (s'esborra automàticament del drawer)
    delete m_circlePolyline;

    // afegim l'annotació textual
    DrawerText *text = new DrawerText;
    text->setText( tr("%1 degrees").arg( m_currentAngle,0,'f',1) );
    text->shadowOn();
    placeText( text );
    m_2DViewer->getDrawer()->draw( text , m_2DViewer->getView(), m_2DViewer->getCurrentSlice() );
}

void AngleTool::placeText( DrawerText *angleText )
{
    const double Padding = 5.0; // padding de 5 pixels

    double *p1 = m_mainPolyline->getPoint(0);
    double *p2 = m_mainPolyline->getPoint(1);
    double *p3 = m_mainPolyline->getPoint(2);
    double position[3];
    int view = m_2DViewer->getView();
    int horizontalCoord = Q2DViewer::getXIndexForView(view);
    int verticalCoord = Q2DViewer::getYIndexForView(view);

    // Mirem on estan horitzontalment els punts p1 i p3 respecte del p2
    if ( p1[0] <= p2[0] )
    {
        angleText->setHorizontalJustification( "Left" );

        if ( p3[horizontalCoord] <= p2[horizontalCoord] )
        {
            angleText->setAttachmentPoint( p2 );
        }
        else
        {
            double p2InDisplay[3];
            m_2DViewer->computeWorldToDisplay(p2[0], p2[1], p2[2], p2InDisplay);    // passem p2 a coordenades de display

            // apliquem el padding
            if (p2[verticalCoord] <= p3[verticalCoord])
            {
                p2InDisplay[1] -= Padding;
            }
            else
            {
                p2InDisplay[1] += Padding;
            }
            m_2DViewer->computeDisplayToWorld(p2InDisplay[0], p2InDisplay[1], p2InDisplay[2], position);    // tornem a coordenades de món
            // ara position és l'attachment point que volem

            angleText->setAttachmentPoint(position);
        }
    }
    else
    {
        angleText->setHorizontalJustification( "Right" );

        if ( p3[horizontalCoord] <= p2[horizontalCoord] )
        {
            angleText->setAttachmentPoint( p2 );
        }
        else
        {
            double p2InDisplay[3];
            m_2DViewer->computeWorldToDisplay(p2[0], p2[1], p2[2], p2InDisplay);    // passem p2 a coordenades de display

            // apliquem el padding
            if (p2[verticalCoord] <= p3[verticalCoord])
            {
                p2InDisplay[1] += Padding;
            }
            else
            {
                p2InDisplay[1] -= Padding;
            }
            m_2DViewer->computeDisplayToWorld(p2InDisplay[0], p2InDisplay[1], p2InDisplay[2], position);    // tornem a coordenades de món
            // ara position és l'attachment point que volem

            angleText->setAttachmentPoint(position);
        }
    }
}

void AngleTool::initialize()
{
    // Alliberem les primitives perquè puguin ser esborrades
    if( m_mainPolyline )
    {
        m_mainPolyline->decreaseReferenceCount();
        delete m_mainPolyline;
    }
    
    if( m_circlePolyline )
    {
        m_circlePolyline->decreaseReferenceCount();
        delete m_circlePolyline;
    }

    m_mainPolyline = NULL;
    m_circlePolyline = NULL;
    m_state = None;
}

}
