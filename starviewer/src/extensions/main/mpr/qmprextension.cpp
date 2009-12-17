/***************************************************************************
 *   Copyright (C) 2005 by Grup de Gràfics de Girona                       *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#include "qmprextension.h"
#include "volume.h"
#include "series.h"
#include "q2dviewer.h"
#include "mathtools.h" // per càlculs d'interseccions
#include "q3dviewer.h"
#include "qcustomwindowleveldialog.h"
#include "logging.h"
#include "toolmanager.h"
#include "windowlevelpresetstooldata.h"
#include "drawer.h"
#include "mprsettings.h"
#include "screenshottool.h" 
#include "qexportertool.h"
#include "patientbrowsermenu.h" 
#include "toolproxy.h"
// qt
#include <QSlider> // pel control m_axialSlider
#include <QSplitter>
#include <QMessageBox>
// vtk
#include <vtkRenderer.h>
#include <vtkMath.h> // pel vtkMath::Cross
#include <vtkAxisActor2D.h>
#include <vtkProperty2D.h>
#include <vtkPlaneSource.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkImageReslice.h>
#include <vtkTransform.h>
#include <vtkLine.h>
#include <vtkImageChangeInformation.h> // per portar a l'origen
// pels events
#include <vtkCommand.h>

namespace udg {

QMPRExtension::QMPRExtension( QWidget *parent )
 : QWidget( parent ), m_axialZeroSliceCoordinate(.0)
{
    setupUi( this );
    MPRSettings().init();

    init();
    createActions();
    createConnections();
    createActors();
    readSettings();
    // ajustaments de window level pel combo box
    m_windowLevelComboBox->setPresetsData( m_axial2DView->getWindowLevelData() );
    m_sagital2DView->setWindowLevelData( m_axial2DView->getWindowLevelData() );
    m_coronal2DView->setWindowLevelData( m_axial2DView->getWindowLevelData() );
    m_windowLevelComboBox->selectPreset( m_axial2DView->getWindowLevelData()->getCurrentPreset() );

    initializeTools();

    m_thickSlab = 0.0;

    //TODO ocultem botons que no son del tot necessaris o que no es faran servir
    // Amb "visible" posem a l'abast o no el MIP 3D
    bool visible = false;
    m_thickSlabLabel->setVisible(visible);
    m_thickSlabSlider->setVisible(visible);
    m_thickSlabSpinBox->setVisible(visible);
    m_mipToolButton->setVisible(visible);
    // aquests els amaguem per guanyar espai
    m_windowLevelToolButton->setVisible(false);
    m_moveToolButton->setVisible(false);

    m_screenshotsExporterToolButton->setToolTip( tr("Export viewer image(s) to DICOM and send them to a PACS server") );
    m_viewerInformationToolButton->setToolTip( tr("Show/Hide viewer's textual information") );
    m_windowLevelComboBox->setToolTip( tr("Choose Window/Level Presets") );
}

QMPRExtension::~QMPRExtension()
{
    writeSettings();
    // fent això o no sembla que s'allibera la mateixa memòria gràcies als smart pointers
    m_sagitalReslice->Delete();
    m_coronalReslice->Delete();

    m_transform->Delete();

    m_sagitalOverAxialAxisActor->Delete();
    m_axialOverSagitalIntersectionAxis->Delete();
    m_coronalOverAxialIntersectionAxis->Delete();
    m_coronalOverSagitalIntersectionAxis->Delete();
    m_thickSlabOverAxialActor->Delete();
    m_thickSlabOverSagitalActor->Delete();

    m_axialPlaneSource->Delete();
    m_sagitalPlaneSource->Delete();
    m_coronalPlaneSource->Delete();
    m_thickSlabPlaneSource->Delete();

    if( m_pickedActorReslice )
        m_pickedActorReslice->Delete();

    if( m_mipViewer )
    {
        delete m_mipViewer;
    }
    delete m_coronal2DView;
}

void QMPRExtension::init()
{
    m_axialPlaneSource = vtkPlaneSource::New();
    m_axialPlaneSource->SetXResolution( 1 ); // així estan configurats a vtkImagePlaneWidget
    m_axialPlaneSource->SetYResolution( 1 );

    m_sagitalPlaneSource = vtkPlaneSource::New();
    m_sagitalPlaneSource->SetXResolution( 1 );
    m_sagitalPlaneSource->SetYResolution( 1 );

    m_coronalPlaneSource = vtkPlaneSource::New();
    m_coronalPlaneSource->SetXResolution( 1 );
    m_coronalPlaneSource->SetYResolution( 1 );

    m_thickSlabPlaneSource = vtkPlaneSource::New();
    m_thickSlabPlaneSource->SetXResolution( 1 );
    m_thickSlabPlaneSource->SetYResolution( 1 );

    m_sagitalReslice = vtkImageReslice::New();
    m_sagitalReslice->AutoCropOutputOn(); // perquè l'extent d'output sigui suficient i no es "mengi" dades
    m_sagitalReslice->SetInterpolationModeToCubic();

    m_coronalReslice = vtkImageReslice::New();
    m_coronalReslice->AutoCropOutputOn();
    m_coronalReslice->SetInterpolationModeToCubic();

    // configurem les annotacions que volem veure
    m_axial2DView->removeAnnotation( Q2DViewer::ScalarBarAnnotation );
    m_sagital2DView->removeAnnotation( Q2DViewer::PatientOrientationAnnotation | Q2DViewer::ScalarBarAnnotation | Q2DViewer::PatientInformationAnnotation | Q2DViewer::SliceAnnotation  );
    m_coronal2DView->removeAnnotation( Q2DViewer::PatientOrientationAnnotation | Q2DViewer::ScalarBarAnnotation | Q2DViewer::PatientInformationAnnotation | Q2DViewer::SliceAnnotation );
    showViewerInformation( m_viewerInformationToolButton->isChecked() );

    m_sagital2DView->disableContextMenu();
    m_coronal2DView->disableContextMenu();

    /// per defecte isomètric
    m_axialSpacing[0] = 1.;
    m_axialSpacing[1] = 1.;
    m_axialSpacing[2] = 1.;

    m_state = NONE;

    m_transform = vtkTransform::New();

    m_pickedActorPlaneSource = 0;
    m_pickedActorReslice = 0;
    m_mipViewer = 0;

    m_extensionToolsList << "ZoomTool" << "SlicingTool" << "TranslateTool" << "VoxelInformationTool" << "WindowLevelTool" << "ScreenShotTool" << "DistanceTool" << "PolylineROITool" << "EraserTool";
}

void QMPRExtension::createActions()
{
    m_horizontalLayoutAction = new QAction( 0 );
    m_horizontalLayoutAction->setText( tr("Switch horizontal layout") );
    m_horizontalLayoutAction->setStatusTip( tr("Switch horizontal layout") );
    m_horizontalLayoutAction->setIcon( QIcon(":/images/view_left_right.png") );
    m_horizontalLayoutToolButton->setDefaultAction( m_horizontalLayoutAction );

    m_mipAction = new QAction( 0 );
    m_mipAction->setText( tr("&MIP") );
    m_mipAction->setShortcut( tr("Ctrl+M") );
    m_mipAction->setStatusTip( tr("Maximum Intensity Projection") );
    m_mipAction->setIcon( QIcon(":/images/mip.png") );
    m_mipAction->setCheckable( true );
    m_mipToolButton->setDefaultAction( m_mipAction );
}

void QMPRExtension::initializeTools()
{
    // creem el tool manager
    m_toolManager = new ToolManager(this);
    // obtenim les accions de cada tool que volem
    m_zoomToolButton->setDefaultAction( m_toolManager->registerTool("ZoomTool") );
    m_slicingToolButton->setDefaultAction( m_toolManager->registerTool("SlicingTool") );
    m_moveToolButton->setDefaultAction( m_toolManager->registerTool("TranslateTool") );
    m_windowLevelToolButton->setDefaultAction( m_toolManager->registerTool("WindowLevelTool") );
    m_voxelInformationToolButton->setDefaultAction( m_toolManager->registerTool("VoxelInformationTool") );
    m_toolManager->registerTool("ScreenShotTool");
    m_screenShotToolButton->setToolTip( m_toolManager->getRegisteredToolAction("ScreenShotTool")->toolTip() );
    m_distanceToolButton->setDefaultAction( m_toolManager->registerTool("DistanceTool") );
    m_polylineROIToolButton->setDefaultAction( m_toolManager->registerTool("PolylineROITool") );
    m_angleToolButton->setDefaultAction( m_toolManager->registerTool( "AngleTool" ) );
    m_openAngleToolButton->setDefaultAction( m_toolManager->registerTool( "NonClosedAngleTool" ) );
    m_eraserToolButton->setDefaultAction( m_toolManager->registerTool("EraserTool") );
    m_toolManager->registerTool("WindowLevelPresetsTool");
    m_toolManager->registerTool("SlicingKeyboardTool");

    // definim els grups exclusius
    QStringList leftButtonExclusiveTools;
    leftButtonExclusiveTools << "ZoomTool" << "SlicingTool" << "DistanceTool" << "PolylineROITool" << "EraserTool" << "AngleTool" << "NonClosedAngleTool";
    m_toolManager->addExclusiveToolsGroup("LeftButtonGroup", leftButtonExclusiveTools);

    QStringList middleButtonExclusiveTools;
    middleButtonExclusiveTools << "TranslateTool";
    m_toolManager->addExclusiveToolsGroup("MiddleButtonGroup", middleButtonExclusiveTools);

    QStringList rightButtonExclusiveTools;
    rightButtonExclusiveTools << "WindowLevelTool";
    m_toolManager->addExclusiveToolsGroup("RighttButtonGroup", rightButtonExclusiveTools);

    // Activem les tools que volem tenir per defecte, això és com si clickéssim a cadascun dels ToolButton
    QStringList defaultTools;
    defaultTools << "WindowLevelPresetsTool" << "SlicingTool" << "WindowLevelTool" << "TranslateTool" << "ScreenShotTool" << "SlicingKeyboardTool";
    m_toolManager->triggerTools(defaultTools);

    // registrem al manager les tools que van als diferents viewers
    m_toolManager->setupRegisteredTools( m_axial2DView );
    // als altres visors volem totes les registrades, excepte l'slicing
    QStringList toolsList = m_toolManager->getRegisteredToolsList();
    toolsList.removeAt( toolsList.indexOf("SlicingTool") );
    m_toolManager->setViewerTools( m_sagital2DView, toolsList );
    m_toolManager->setViewerTools( m_coronal2DView, toolsList );
}

void QMPRExtension::createConnections()
{
    // conectem els sliders i demés visors
    connect( m_axial2DView , SIGNAL( sliceChanged(int) ) , m_axialSlider , SLOT( setValue(int) ) );
    connect( m_axialSlider , SIGNAL( valueChanged(int) ) , m_axial2DView, SLOT( setSlice(int) ) );

    connect( m_axial2DView, SIGNAL( sliceChanged(int) ), SLOT( axialSliceUpdated(int) ) );
    connect( m_axial2DView, SIGNAL( sliceChanged( int ) ), SLOT( updateProjectionLabel() ) );

    // gestionen els events de les finestres per poder manipular els plans
    connect( m_axial2DView, SIGNAL( eventReceived(unsigned long) ), SLOT( handleAxialViewEvents( unsigned long ) ) );
    connect( m_sagital2DView, SIGNAL( eventReceived(unsigned long) ), SLOT( handleSagitalViewEvents( unsigned long ) ) );

    connect( m_thickSlabSpinBox, SIGNAL( valueChanged(double) ), SLOT( updateThickSlab(double) ) );
    connect( m_thickSlabSlider, SIGNAL( valueChanged(int) ), SLOT( updateThickSlab(int) ) );

    // layouts
    connect( m_horizontalLayoutAction , SIGNAL( triggered() ), SLOT( switchHorizontalLayout() ) );
    connect( m_mipAction , SIGNAL( triggered(bool) ), SLOT( switchToMIPLayout(bool) ) );

    // Fem que no s'assigni automàticament l'input que s'ha seleccionat amb el menú de pacient, ja que fem tractaments adicionals
    // sobre el volum seleccionat i l'input final del visor pot diferir de l'inicial i és l'extensió qui decideix finalment quin input
    // se li vol donar a cada viewer. Capturem la senyal de quin volum s'ha escollit i a partir d'aquí fem el que calgui
    disconnect( m_axial2DView->getPatientBrowserMenu(), SIGNAL( selectedVolume(Volume *) ), m_axial2DView, SLOT( setInput(Volume *) ) );
    connect( m_axial2DView->getPatientBrowserMenu(), SIGNAL( selectedVolume(Volume *) ), SLOT( setInput(Volume *) ) );

    // mostrar o no la informacio del volum a cada visualitzador
    connect( m_viewerInformationToolButton, SIGNAL( toggled( bool ) ), SLOT( showViewerInformation( bool ) ) );

    // HACK per poder fer servir l'eina d'screenshot amb el botonet
    connect( m_axial2DView, SIGNAL(selected()), SLOT(changeSelectedViewer()) );
    connect( m_sagital2DView, SIGNAL(selected()), SLOT(changeSelectedViewer()) );
    connect( m_coronal2DView, SIGNAL(selected()), SLOT(changeSelectedViewer()) );
    connect( m_screenShotToolButton, SIGNAL( clicked() ), SLOT( screenShot() ) );

    // per mostrar exportació
    connect( m_screenshotsExporterToolButton, SIGNAL( clicked() ) , SLOT( showScreenshotsExporterDialog() ) );

}

void QMPRExtension::changeSelectedViewer()
{
    if( this->sender() == m_axial2DView )
    {
        m_axial2DView->setActive(true);
        
        m_sagital2DView->setActive(false);
        m_coronal2DView->setActive(false);
    }
    else if( this->sender() == m_sagital2DView )
    {
        m_sagital2DView->setActive(true);
        
        m_axial2DView->setActive(false);
        m_coronal2DView->setActive(false);
    }
    else if( this->sender() == m_coronal2DView )
    {
        m_coronal2DView->setActive(true);
        
        m_axial2DView->setActive(false);
        m_sagital2DView->setActive(false);
    }
}

void QMPRExtension::screenShot()
{
    ScreenShotTool *screenShotTool = 0;
    if( m_axial2DView->isActive() )
    {
        screenShotTool = dynamic_cast<ScreenShotTool *>( m_axial2DView->getToolProxy()->getTool("ScreenShotTool") );
    }
    else if( m_sagital2DView->isActive() )
    {
        screenShotTool = dynamic_cast<ScreenShotTool *>( m_sagital2DView->getToolProxy()->getTool("ScreenShotTool") );
    }
    else if( m_coronal2DView->isActive() )
    {
        screenShotTool = dynamic_cast<ScreenShotTool *>( m_coronal2DView->getToolProxy()->getTool("ScreenShotTool") );
    }

    if( screenShotTool )
        screenShotTool->singleCapture();
    else
        DEBUG_LOG("No hi ha tool d'screenshot disponible");
}

void QMPRExtension::showScreenshotsExporterDialog()
{
    Q2DViewer * viewer = 0;
    if( m_axial2DView->isActive() )
    {
        viewer = m_axial2DView;
    }
    else if( m_sagital2DView->isActive() )
    {
        viewer = m_sagital2DView;
    }
    else if( m_coronal2DView->isActive() )
    {
        viewer = m_coronal2DView;
    }

    if ( viewer )
    {
        QExporterTool exporter( viewer );
        exporter.exec();
    }
    else
    {
        QMessageBox::warning(this, tr("Export to DICOM") , tr("Please, select a viewer and try again.") );
    }
}


void QMPRExtension::showViewerInformation( bool show )
{
    m_axial2DView->enableAnnotation(
        Q2DViewer::WindowInformationAnnotation | Q2DViewer::PatientOrientationAnnotation |
        Q2DViewer::RulersAnnotation | Q2DViewer::SliceAnnotation | Q2DViewer::PatientInformationAnnotation |
        Q2DViewer::AcquisitionInformationAnnotation
        , show );

    m_sagital2DView->enableAnnotation(
        Q2DViewer::WindowInformationAnnotation | Q2DViewer::RulersAnnotation |
        Q2DViewer::AcquisitionInformationAnnotation
        , show );

    m_coronal2DView->enableAnnotation(
        Q2DViewer::WindowInformationAnnotation | Q2DViewer::RulersAnnotation |
        Q2DViewer::AcquisitionInformationAnnotation
        , show );
}

void QMPRExtension::updateProjectionLabel()
{
    m_projectionLabel->setText( m_axial2DView->getCurrentPlaneProjectionLabel() );
}

void QMPRExtension::switchHorizontalLayout()
{
    QWidget *leftWidget, *rightWidget;
    leftWidget = m_horizontalSplitter->widget( 0 );
    rightWidget = m_horizontalSplitter->widget( 1 );

    m_horizontalSplitter->insertWidget( 0 , rightWidget );
    m_horizontalSplitter->insertWidget( 1 , leftWidget );
}

void QMPRExtension::switchToMIPLayout( bool isMIPChecked )
{
    // desem la mida abans de canviar els widgets
    QList<int> splitterSize = m_horizontalSplitter->sizes();
    if( isMIPChecked )
    {
        if( !m_mipViewer )
        {
            m_mipViewer = new Q3DViewer;
            m_mipViewer->orientationMarkerOff();
            m_mipViewer->setRenderFunctionToMIP3D();
        }
        Volume *mipInput = new Volume;
        //TODO això es necessari perquè tingui la informació de la sèrie, estudis, pacient...
        mipInput->setImages( m_volume->getImages() );
        mipInput->setData( m_coronalReslice->GetOutput() );
        m_mipViewer->setInput( mipInput );
        m_mipViewer->render();
        m_mipViewer->show();
        // disposem la distribució de widgets
        m_horizontalSplitter->insertWidget( m_horizontalSplitter->indexOf( m_verticalSplitter ) , m_mipViewer );
        m_verticalSplitter->hide();
        m_horizontalSplitter->insertWidget( 2 , m_verticalSplitter );
    }
    else
    {
        m_horizontalSplitter->insertWidget( m_horizontalSplitter->indexOf( m_mipViewer ) , m_verticalSplitter );
        m_verticalSplitter->show();
        m_mipViewer->hide();
        m_horizontalSplitter->insertWidget( 2 , m_mipViewer );
    }
    // recuperem les mides
    m_horizontalSplitter->setSizes( splitterSize );
}

void QMPRExtension::handleAxialViewEvents( unsigned long eventID )
{
    switch( eventID )
    {
    case vtkCommand::LeftButtonPressEvent:
        if( detectAxialViewAxisActor() )
        {
            if ( m_axial2DView->getInteractor()->GetControlKey() )
                m_state = PUSHING;
            else
                m_state = ROTATING;
        }
    break;

    case vtkCommand::LeftButtonReleaseEvent:
        if( m_state != NONE )
            releaseAxialViewAxisActor();
    break;

    case vtkCommand::MouseMoveEvent:
        if( m_state == ROTATING )
            rotateAxialViewAxisActor();
        else if( m_state == PUSHING )
            pushAxialViewAxisActor();

        if ( m_pickedActorPlaneSource == m_coronalPlaneSource && m_coronal2DView->getDrawer()->getNumberOfDrawnPrimitives() > 0 )
            m_coronal2DView->getDrawer()->removeAllPrimitives();

        if ( m_pickedActorPlaneSource == m_sagitalPlaneSource && m_sagital2DView->getDrawer()->getNumberOfDrawnPrimitives() > 0 )
            m_sagital2DView->getDrawer()->removeAllPrimitives();

    break;

    default:
    break;
    }
}

void QMPRExtension::handleSagitalViewEvents( unsigned long eventID )
{
    switch( eventID )
    {
    case vtkCommand::LeftButtonPressEvent:
        if ( m_sagital2DView->getInteractor()->GetControlKey() )
            detectPushSagitalViewAxisActor();
        else
            detectSagitalViewAxisActor();
    break;

    case vtkCommand::LeftButtonReleaseEvent:
        if( m_state != NONE )
            releaseSagitalViewAxisActor();
    break;

    case vtkCommand::MouseMoveEvent:
        if( m_state == ROTATING )
            rotateSagitalViewAxisActor();
        else if( m_state == PUSHING )
        {
            if ( m_pickedActorPlaneSource == m_coronalPlaneSource )
                pushSagitalViewCoronalAxisActor();
            else
                pushSagitalViewAxialAxisActor();
        }

        if ( m_pickedActorPlaneSource == m_coronalPlaneSource && m_coronal2DView->getDrawer()->getNumberOfDrawnPrimitives() > 0 )
            m_coronal2DView->getDrawer()->removeAllPrimitives();

    break;

    default:
    break;
    }
}

bool QMPRExtension::detectAxialViewAxisActor()
{
    bool picked = false;
    double clickedWorldPoint[3];
    m_axial2DView->getEventWorldCoordinate( clickedWorldPoint );

    // detectem quin és l'actor més proper, l'identifiquem i llavors el deixem com a seleccionat
    double point[3] = { clickedWorldPoint[0] , clickedWorldPoint[1] , 0.0 };
    double *r1 , *r2;
    double distanceToCoronal , distanceToSagital;

    r1 = m_coronalOverAxialIntersectionAxis->GetPositionCoordinate()->GetValue();
    r2 = m_coronalOverAxialIntersectionAxis->GetPosition2Coordinate()->GetValue();
    distanceToCoronal = vtkLine::DistanceToLine( point , r1 , r2 );

    r1 = m_sagitalOverAxialAxisActor->GetPositionCoordinate()->GetValue();
    r2 = m_sagitalOverAxialAxisActor->GetPosition2Coordinate()->GetValue();
    distanceToSagital = vtkLine::DistanceToLine( point , r1 , r2 );

    // donem una "tolerància" mínima
    if( distanceToCoronal < 50.0 || distanceToSagital < 50.0 )
    {
        if( distanceToCoronal < distanceToSagital )
        {
            m_pickedActorPlaneSource = m_coronalPlaneSource;
            m_pickedActorReslice = m_coronalReslice;
        }
        else
        {
            m_pickedActorPlaneSource = m_sagitalPlaneSource;
            m_pickedActorReslice = m_sagitalReslice;
        }
        m_pickedActorReslice->SetInterpolationModeToNearestNeighbor();
        // desactivem les tools que puguin estar actives
        m_toolManager->disableAllToolsTemporarily();
        m_initialPickX = clickedWorldPoint[0];
        m_initialPickY = clickedWorldPoint[1];
        picked = true;
    }
    return picked;
}

void QMPRExtension::rotateAxialViewAxisActor()
{
    double clickedWorldPoint[3];
    m_axial2DView->getEventWorldCoordinate( clickedWorldPoint );

    double vec1[3], vec2[3];
    double axis[3];
    double direction[3];

    vec1[0] = m_initialPickX - m_pickedActorPlaneSource->GetCenter()[0];
    vec1[1] = m_initialPickY - m_pickedActorPlaneSource->GetCenter()[1];
    vec1[2] = 0.0;

    vec2[0] = clickedWorldPoint[0] - m_pickedActorPlaneSource->GetCenter()[0];
    vec2[1] = clickedWorldPoint[1] - m_pickedActorPlaneSource->GetCenter()[1];
    vec2[2] = 0.0;

    double degrees = MathTools::angleInDegrees( vec1 , vec2 );

    m_initialPickX = clickedWorldPoint[0];
    m_initialPickY = clickedWorldPoint[1];

    // TODO canviar per MathTools::crossProduct()
    vtkMath::Cross( vec1 , vec2 , direction );
    this->getRotationAxis( m_pickedActorPlaneSource , axis );
    double dot = MathTools::dotProduct( direction , axis );

    axis[0] *= dot;
    axis[1] *= dot;
    axis[2] *= dot;
    // TODO fer la normalització amb un mètode de MathTools
    vtkMath::Normalize( axis );
    rotateMiddle( degrees , axis , m_pickedActorPlaneSource );
    updatePlanes();
    updateControls();
}

void QMPRExtension::releaseAxialViewAxisActor()
{
    if( m_pickedActorReslice )
    {
        m_pickedActorReslice->SetInterpolationModeToCubic();
        // TODO no seria millor un restoreOverrideCursor?
        this->setCursor( QCursor( Qt::ArrowCursor ) );
        if( m_pickedActorPlaneSource == m_sagitalPlaneSource )
        {
            m_sagital2DView->refresh();
        }
        else
        {
            m_coronal2DView->refresh();
        }
        m_state = NONE;
        m_pickedActorReslice = 0;
        m_pickedActorPlaneSource = 0;
        // reactivem les tools
        m_toolManager->undoDisableAllToolsTemporarily();
    }
}

void QMPRExtension::detectSagitalViewAxisActor()
{
    double clickedWorldPoint[3];
    m_sagital2DView->getEventWorldCoordinate( clickedWorldPoint );

    // detectem quin és l'actor més proper, l'identifiquem i llavors el deixem com a seleccionat
    double point[3] = { clickedWorldPoint[0] , clickedWorldPoint[1] , 0.0 };
    double *r1 , *r2;
    double distanceToCoronal;

    r1 = m_coronalOverSagitalIntersectionAxis->GetPositionCoordinate()->GetValue();
    r2 = m_coronalOverSagitalIntersectionAxis->GetPosition2Coordinate()->GetValue();
    distanceToCoronal = vtkLine::DistanceToLine( point , r1 , r2 );

    // donem una "tolerància" mínima
    if( distanceToCoronal < 50.0 )
    {
        m_pickedActorReslice = m_coronalReslice;
        m_pickedActorReslice->SetInterpolationModeToNearestNeighbor();
        m_pickedActorPlaneSource = m_coronalPlaneSource;
        // desactivem les tools que puguin estar actives

       m_toolManager->disableAllToolsTemporarily();

        m_initialPickX = clickedWorldPoint[0];
        m_initialPickY = clickedWorldPoint[1];
        m_state = ROTATING;
    }
}

void QMPRExtension::rotateSagitalViewAxisActor()
{
    double clickedWorldPoint[3];
    m_sagital2DView->getEventWorldCoordinate( clickedWorldPoint );
    //
    double vec1[3], vec2[3];
    double axis[3];
    double direction[3];

    vec1[1] = m_initialPickX - m_pickedActorPlaneSource->GetCenter()[1];
    vec1[2] = m_initialPickY - m_pickedActorPlaneSource->GetCenter()[2];
    vec1[0] = 0.0;

    vec2[1] = clickedWorldPoint[0] - m_pickedActorPlaneSource->GetCenter()[1];
    vec2[2] = clickedWorldPoint[1] - m_pickedActorPlaneSource->GetCenter()[2];
    vec2[0] = 0.0;

    double degrees = MathTools::angleInDegrees( vec1 , vec2 );

    m_initialPickX = clickedWorldPoint[0];
    m_initialPickY = clickedWorldPoint[1];

    // TODO canviar per MathTools::crossProduct()
    vtkMath::Cross( vec1 , vec2 , direction );
    axis[0] = m_pickedActorPlaneSource->GetPoint1()[0] - m_pickedActorPlaneSource->GetOrigin()[0];
    axis[1] = m_pickedActorPlaneSource->GetPoint1()[1] - m_pickedActorPlaneSource->GetOrigin()[1];
    axis[2] = m_pickedActorPlaneSource->GetPoint1()[2] - m_pickedActorPlaneSource->GetOrigin()[2];

    double dot = MathTools::dotProduct( direction , axis );

    axis[0] *= dot;
    axis[1] *= dot;
    axis[2] *= dot;
    // TODO fer la normalització amb un mètode de MathTools
    vtkMath::Normalize( axis );
    rotateMiddle( degrees , axis , m_pickedActorPlaneSource );
    updatePlanes();
    updateControls();
}

void QMPRExtension::releaseSagitalViewAxisActor()
{
    if( m_pickedActorReslice )
    {
        this->setCursor( QCursor( Qt::ArrowCursor ) );
        m_pickedActorReslice->SetInterpolationModeToCubic();
        m_coronal2DView->refresh();
        m_state = NONE;
        m_pickedActorReslice = 0;
        m_pickedActorPlaneSource = 0;
        // reactivem les tools
        m_toolManager->undoDisableAllToolsTemporarily();
    }
}

void QMPRExtension::getRotationAxis( vtkPlaneSource *plane , double axis[3] )
{
    if( !plane )
        return;

    axis[0] = plane->GetPoint2()[0] - plane->GetOrigin()[0];
    axis[1] = plane->GetPoint2()[1] - plane->GetOrigin()[1];
    axis[2] = plane->GetPoint2()[2] - plane->GetOrigin()[2];
}

void QMPRExtension::pushSagitalViewCoronalAxisActor()
{
    this->setCursor( QCursor( Qt::ClosedHandCursor ) );
    double clickedWorldPoint[3];
    m_sagital2DView->getEventWorldCoordinate( clickedWorldPoint );
    // Get the motion vector
    //
    double v[3];
    v[1] = clickedWorldPoint[0] - m_initialPickX;
    v[2] = clickedWorldPoint[1] - m_initialPickY;
    v[0] = 0.0;

    m_pickedActorPlaneSource->Push( MathTools::dotProduct( v, m_pickedActorPlaneSource->GetNormal() ) );
    updatePlanes();
    updateControls();

    m_initialPickX = clickedWorldPoint[0];
    m_initialPickY = clickedWorldPoint[1];
}

void QMPRExtension::pushAxialViewAxisActor()
{
    this->setCursor( QCursor( Qt::ClosedHandCursor ) );
    double clickedWorldPoint[3];
    m_axial2DView->getEventWorldCoordinate( clickedWorldPoint );
    // Get the motion vector
    //
    double v[3];
    v[0] = clickedWorldPoint[0] - m_initialPickX;
    v[1] = clickedWorldPoint[1] - m_initialPickY;
    v[2] = 0.0;

    m_pickedActorPlaneSource->Push( MathTools::dotProduct( v, m_pickedActorPlaneSource->GetNormal() ) );
    updatePlanes();
    updateControls();

    m_initialPickX = clickedWorldPoint[0];
    m_initialPickY = clickedWorldPoint[1];
}

void QMPRExtension::detectPushSagitalViewAxisActor()
{
    double clickedWorldPoint[3];
    m_sagital2DView->getEventWorldCoordinate( clickedWorldPoint );

    // detectem quin és l'actor més proper, l'identifiquem i llavors el deixem com a seleccionat
    // únicament mourem la vista axial. Desde la vista sagital no podrem moure l'slice de la coronal
    double point[3] = { clickedWorldPoint[0] , clickedWorldPoint[1] , 0.0 };
    double *r1 , *r2;
    double distanceToAxial, distanceToCoronal;

    r1 = m_axialOverSagitalIntersectionAxis->GetPositionCoordinate()->GetValue();
    r2 = m_axialOverSagitalIntersectionAxis->GetPosition2Coordinate()->GetValue();
    distanceToAxial = vtkLine::DistanceToLine( point , r1 , r2 );

    r1 = m_coronalOverSagitalIntersectionAxis->GetPositionCoordinate()->GetValue();
    r2 = m_coronalOverSagitalIntersectionAxis->GetPosition2Coordinate()->GetValue();
    distanceToCoronal = vtkLine::DistanceToLine( point , r1 , r2 );

    // donem una "tolerància" mínima
    if( distanceToCoronal < 50.0 || distanceToAxial < 50.0 )
    {
        this->setCursor( QCursor( Qt::OpenHandCursor ) );
        //desactivem les tools perquè no facin interferència
        m_toolManager->disableAllToolsTemporarily();
        if( distanceToCoronal < distanceToAxial )
        {
            m_pickedActorPlaneSource = m_coronalPlaneSource;
            m_pickedActorReslice = m_coronalReslice;
        }
        else
        {
            m_pickedActorPlaneSource = m_axialPlaneSource;
            m_pickedActorReslice = m_sagitalReslice;
        }
        m_state = PUSHING;
        m_initialPickX = clickedWorldPoint[0];
        m_initialPickY = clickedWorldPoint[1];
    }
}

void QMPRExtension::pushSagitalViewAxialAxisActor()
{
    double clickedWorldPoint[3];
    m_sagital2DView->getEventWorldCoordinate( clickedWorldPoint );

    m_axial2DView->setSlice( m_axial2DView->getMaximumSlice() - static_cast<int>( clickedWorldPoint[1] / m_axialSpacing[2] ) );
    updatePlanes();
    updateControls();

    m_initialPickX = clickedWorldPoint[0];
    m_initialPickY = clickedWorldPoint[1];
}

void QMPRExtension::setInput( Volume *input )
{
    if( input->getNumberOfPhases() > 1 )
        m_phasesAlertLabel->setVisible(true);
    else
        m_phasesAlertLabel->setVisible(false);

    vtkImageChangeInformation *changeInfo = vtkImageChangeInformation::New();
    changeInfo->SetInput( input->getVtkData() );
    changeInfo->SetOutputOrigin( .0, .0, .0 );

    // TODO es crea un nou volum cada cop!
    m_volume = new Volume;
    m_volume->setImages( input->getImages() );
    m_volume->setData( changeInfo->GetOutput() );
    m_volume->setNumberOfPhases( input->getNumberOfPhases() );
    m_volume->setNumberOfSlicesPerPhase( input->getNumberOfSlicesPerPhase() );

    m_volume->getSpacing( m_axialSpacing );

    m_sagitalReslice->SetInput( m_volume->getVtkData() );
    m_coronalReslice->SetInput( m_volume->getVtkData() );

    // faltaria refrescar l'input dels 3 mpr
    m_axial2DView->setInput( m_volume );

    int extent[6];
    m_volume->getWholeExtent( extent );
    m_axialSlider->setMaximum(  extent[5] );

    double maxThickSlab = sqrt( (m_axialSpacing[0]*extent[1]) * (m_axialSpacing[0]*extent[1]) + (m_axialSpacing[1]*extent[3]) * (m_axialSpacing[1]*extent[3]) + (m_axialSpacing[2]*extent[5]) * (m_axialSpacing[2]*extent[5]) );
    m_thickSlabSlider->setMaximum( (int) maxThickSlab );
    m_thickSlabSpinBox->setMaximum( maxThickSlab );

    // posta a punt dels planeSource
    initOrientation();

    Volume *sagitalResliced = new Volume;
    //TODO això es necessari perquè tingui la informació de la sèrie, estudis, pacient...
    sagitalResliced->setImages( m_volume->getImages() );
    sagitalResliced->setData( m_sagitalReslice->GetOutput() );
    sagitalResliced->setNumberOfPhases( 1 );
    sagitalResliced->setNumberOfSlicesPerPhase( 1 );

    m_sagital2DView->setInput( sagitalResliced );

    Volume *coronalResliced = new Volume;
    //TODO això es necessari perquè tingui la informació de la sèrie, estudis, pacient...
    coronalResliced->setImages( m_volume->getImages() );
    coronalResliced->setData( m_coronalReslice->GetOutput() );
    coronalResliced->setNumberOfPhases( 1 );
    coronalResliced->setNumberOfSlicesPerPhase( 1 );
    m_coronal2DView->setInput( coronalResliced );
}

void QMPRExtension::initOrientation()
{
/**
    IMPORTANT!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    ara li donem a cada pla unes dimensions, extent , espaiat, etc d'acord com si aquests plans haguessin de ser ortogonals,però segons el pla de tall serà d'una manera o altre
    La vista axial mantindrà els espaiats i extents originals
    La vista sagital, com que únicament es podrà rotar sobre l'eix Y, mantindrà l'extent de la seva X igual que l'extent Y original. Els altres s'hauran d'adaptar a les distàncies corresponents a les diagonals zmax-xmax
    En la vista coronal, com que pot tenir qualsevol orientacio tindrà que adaptar els seus extents als màxims

*/
    int extent[6];
    m_volume->getWholeExtent(extent);
    double origin[3];
    m_volume->getOrigin(origin);
    double spacing[3];
    m_volume->getSpacing(spacing);

    // Prevent obscuring voxels by offsetting the plane geometry
    //
    double xbounds[] = {origin[0] + spacing[0] * (extent[0] - 0.5) ,
                        origin[0] + spacing[0] * (extent[1] + 0.5)};
    double ybounds[] = {origin[1] + spacing[1] * (extent[2] - 0.5),
                        origin[1] + spacing[1] * (extent[3] + 0.5)};
    double zbounds[] = {origin[2] + spacing[2] * (extent[4] - 0.5),
                        origin[2] + spacing[2] * (extent[5] + 0.5)};

    if ( spacing[0] < 0.0 )
    {
        double t = xbounds[0];
        xbounds[0] = xbounds[1];
        xbounds[1] = t;
    }
    if ( spacing[1] < 0.0 )
    {
        double t = ybounds[0];
        ybounds[0] = ybounds[1];
        ybounds[1] = t;
    }
    if ( spacing[2] < 0.0 )
    {
        double t = zbounds[0];
        zbounds[0] = zbounds[1];
        zbounds[1] = t;
    }
    //XY, z-normal : vista axial , en principi d'aquesta vista nomès canviarem la llesca
    m_axialPlaneSource->SetOrigin( xbounds[0] , ybounds[1] , zbounds[1] );
    m_axialPlaneSource->SetPoint1( xbounds[1] , ybounds[1] , zbounds[1] );
    m_axialPlaneSource->SetPoint2( xbounds[0] , ybounds[0] , zbounds[1] );
    m_axialZeroSliceCoordinate = zbounds[1];

    //YZ, x-normal : vista sagital
    // estem ajustant la mida del pla a les dimensions d'aquesta orientació
    // TODO podríem donar unes mides a cada punt que fossin suficientment grans com per poder mostrejar qualssevol orientació en el volum, potser fent una bounding box o simplement d'una forma més "bruta" doblant la longitud d'aquest pla :P
    m_sagitalPlaneSource->SetOrigin( xbounds[0] , ybounds[0] , zbounds[1] );
    m_sagitalPlaneSource->SetPoint1( xbounds[0] , ybounds[1] , zbounds[1] );
    m_sagitalPlaneSource->SetPoint2( xbounds[0] , ybounds[0] , zbounds[0] );
    m_sagitalPlaneSource->Push( -0.5 * ( xbounds[1] - xbounds[0] ) );

    //ZX, y-normal : vista coronal
    // ídem anterior
    // TODO comprovar si és correcte aquest ajustament de mides
    double maxZBound = sqrt( ybounds[1]*ybounds[1] + xbounds[1]*xbounds[1] );
    double maxXBound = sqrt( ybounds[1]*ybounds[1] + xbounds[1]*xbounds[1] );
    double diffXBound = maxXBound - xbounds[1];
    double diffZBound = maxZBound - zbounds[1];

    m_coronalPlaneSource->SetOrigin( xbounds[0] - diffXBound*0.5 , ybounds[1] , zbounds[1] + diffZBound*0.5 );
    m_coronalPlaneSource->SetPoint1( xbounds[1] + diffXBound*0.5 , ybounds[1] , zbounds[1] + diffZBound*0.5 );
    m_coronalPlaneSource->SetPoint2( xbounds[0] - diffXBound*0.5 , ybounds[1] , zbounds[0] - diffZBound*0.5 );
    // posem en la llesca central
    m_coronalPlaneSource->Push( -0.5 * ( ybounds[1] - ybounds[0] ) );

    updatePlanes();
    updateControls();
}

void QMPRExtension::createActors()
{
    // creem els axis actors
    m_sagitalOverAxialAxisActor = vtkAxisActor2D::New();
    m_coronalOverAxialIntersectionAxis = vtkAxisActor2D::New();
    m_coronalOverSagitalIntersectionAxis = vtkAxisActor2D::New();
    m_axialOverSagitalIntersectionAxis = vtkAxisActor2D::New();
    m_thickSlabOverAxialActor = vtkAxisActor2D::New();
    m_thickSlabOverSagitalActor = vtkAxisActor2D::New();

    m_sagitalOverAxialAxisActor->AxisVisibilityOn();
    m_sagitalOverAxialAxisActor->TickVisibilityOff();
    m_sagitalOverAxialAxisActor->LabelVisibilityOff();
    m_sagitalOverAxialAxisActor->TitleVisibilityOff();
    m_sagitalOverAxialAxisActor->GetProperty()->SetColor( 1. , .6 , .0 );

    m_coronalOverAxialIntersectionAxis->TickVisibilityOff();
    m_coronalOverAxialIntersectionAxis->LabelVisibilityOff();
    m_coronalOverAxialIntersectionAxis->TitleVisibilityOff();
    m_coronalOverAxialIntersectionAxis->GetProperty()->SetColor( .0 , 1. , 1. );

    m_coronalOverSagitalIntersectionAxis->AxisVisibilityOn();
    m_coronalOverSagitalIntersectionAxis->TickVisibilityOff();
    m_coronalOverSagitalIntersectionAxis->LabelVisibilityOff();
    m_coronalOverSagitalIntersectionAxis->TitleVisibilityOff();
    m_coronalOverSagitalIntersectionAxis->GetProperty()->SetColor( .0 , 1. , 1. );

    m_axialOverSagitalIntersectionAxis->AxisVisibilityOn();
    m_axialOverSagitalIntersectionAxis->TickVisibilityOff();
    m_axialOverSagitalIntersectionAxis->LabelVisibilityOff();
    m_axialOverSagitalIntersectionAxis->TitleVisibilityOff();
    m_axialOverSagitalIntersectionAxis->GetProperty()->SetColor( 1. , 1. , .0 );

    m_thickSlabOverAxialActor->AxisVisibilityOn();
    m_thickSlabOverAxialActor->TickVisibilityOff();
    m_thickSlabOverAxialActor->LabelVisibilityOff();
    m_thickSlabOverAxialActor->TitleVisibilityOff();
    m_thickSlabOverAxialActor->GetProperty()->SetColor( .0 , 1. , 1. );
    m_thickSlabOverAxialActor->GetProperty()->SetLineStipplePattern( 65280 );

    m_thickSlabOverSagitalActor->AxisVisibilityOn();
    m_thickSlabOverSagitalActor->TickVisibilityOff();
    m_thickSlabOverSagitalActor->LabelVisibilityOff();
    m_thickSlabOverSagitalActor->TitleVisibilityOff();
    m_thickSlabOverSagitalActor->GetProperty()->SetColor( .0 , 1. , 1. );
    m_thickSlabOverSagitalActor->GetProperty()->SetLineStipplePattern( 65280 );

    m_axial2DView->getRenderer()->AddViewProp( m_sagitalOverAxialAxisActor );
    m_axial2DView->getRenderer()->AddViewProp( m_coronalOverAxialIntersectionAxis );
    m_axial2DView->getRenderer()->AddViewProp( m_thickSlabOverAxialActor );
    m_sagital2DView->getRenderer()->AddViewProp( m_coronalOverSagitalIntersectionAxis );
    m_sagital2DView->getRenderer()->AddViewProp( m_axialOverSagitalIntersectionAxis );
    m_sagital2DView->getRenderer()->AddViewProp( m_thickSlabOverSagitalActor );

}

void QMPRExtension::axialSliceUpdated( int slice )
{
    // push relatiu que hem de fer = reubicar-nos a l'inici i colocar la llesca
    m_axialPlaneSource->Push( -(m_axialZeroSliceCoordinate - m_axialPlaneSource->GetOrigin()[2]) + (slice*m_axialSpacing[2]) );
    m_axialPlaneSource->Update();
    updateControls();
}

void QMPRExtension::updateControls()
{
    // posem la representació dels plans sobre cada vista
    updateIntersectionPoint();

    // Passem a sistema de coordenades de món
    m_sagitalOverAxialAxisActor->GetPositionCoordinate()->SetCoordinateSystemToWorld();
    m_sagitalOverAxialAxisActor->GetPosition2Coordinate()->SetCoordinateSystemToWorld();

    m_coronalOverSagitalIntersectionAxis->GetPositionCoordinate()->SetCoordinateSystemToWorld();
    m_coronalOverSagitalIntersectionAxis->GetPosition2Coordinate()->SetCoordinateSystemToWorld();

    m_coronalOverAxialIntersectionAxis->GetPositionCoordinate()->SetCoordinateSystemToWorld();
    m_coronalOverAxialIntersectionAxis->GetPosition2Coordinate()->SetCoordinateSystemToWorld();

    m_axialOverSagitalIntersectionAxis->GetPositionCoordinate()->SetCoordinateSystemToWorld();
    m_axialOverSagitalIntersectionAxis->GetPosition2Coordinate()->SetCoordinateSystemToWorld();

    m_thickSlabOverAxialActor->GetPositionCoordinate()->SetCoordinateSystemToWorld();
    m_thickSlabOverAxialActor->GetPosition2Coordinate()->SetCoordinateSystemToWorld();

    m_thickSlabOverSagitalActor->GetPositionCoordinate()->SetCoordinateSystemToWorld();
    m_thickSlabOverSagitalActor->GetPosition2Coordinate()->SetCoordinateSystemToWorld();

    m_thickSlabPlaneSource->SetOrigin( m_coronalPlaneSource->GetOrigin() );
    m_thickSlabPlaneSource->SetPoint1( m_coronalPlaneSource->GetPoint1() );
    m_thickSlabPlaneSource->SetPoint2( m_coronalPlaneSource->GetPoint2() );
    m_thickSlabPlaneSource->Push( m_thickSlab );

    double r[3] , t[3] , position1[3] , position2[3];

    // projecció sagital sobre axial i viceversa
    MathTools::planeIntersection( m_axialPlaneSource->GetOrigin() , m_axialPlaneSource->GetNormal(),  m_sagitalPlaneSource->GetOrigin(), m_sagitalPlaneSource->GetNormal() , r , t );

    position1[0] = r[0] - t[0]*2000;
    position1[1] = r[1] - t[1]*2000;
    position1[2] = r[2] - t[2]*2000;

    position2[0] = r[0] + t[0]*2000;
    position2[1] = r[1] + t[1]*2000;
    position2[2] = r[2] + t[2]*2000;

    m_sagitalOverAxialAxisActor->SetPosition(  position1[0] , position1[1] );
    m_sagitalOverAxialAxisActor->SetPosition2( position2[0] , position2[1] );

    m_axialOverSagitalIntersectionAxis->SetPosition(  position1[1] , position1[2] );
    m_axialOverSagitalIntersectionAxis->SetPosition2( position2[1] , position2[2] );

    // projecció coronal sobre sagital

    MathTools::planeIntersection( m_coronalPlaneSource->GetOrigin() , m_coronalPlaneSource->GetNormal() , m_sagitalPlaneSource->GetOrigin() , m_sagitalPlaneSource->GetNormal() , r , t );

    position1[0] = r[0] - t[0]*2000;
    position1[1] = r[1] - t[1]*2000;
    position1[2] = r[2] - t[2]*2000;

    position2[0] = r[0] + t[0]*2000;
    position2[1] = r[1] + t[1]*2000;
    position2[2] = r[2] + t[2]*2000;

    m_coronalOverSagitalIntersectionAxis->SetPosition( position1[1] , position1[2] );
    m_coronalOverSagitalIntersectionAxis->SetPosition2( position2[1] , position2[2] );

    // projecció thick slab sobre sagital
    MathTools::planeIntersection( m_thickSlabPlaneSource->GetOrigin() , m_thickSlabPlaneSource->GetNormal() , m_sagitalPlaneSource->GetOrigin() , m_sagitalPlaneSource->GetNormal() , r , t );

    position1[0] = r[0] - t[0]*2000;
    position1[1] = r[1] - t[1]*2000;
    position1[2] = r[2] - t[2]*2000;

    position2[0] = r[0] + t[0]*2000;
    position2[1] = r[1] + t[1]*2000;
    position2[2] = r[2] + t[2]*2000;

    m_thickSlabOverSagitalActor->SetPosition( position1[1] , position1[2] );
    m_thickSlabOverSagitalActor->SetPosition2( position2[1] , position2[2] );

    // projecció coronal sobre axial
    MathTools::planeIntersection( m_coronalPlaneSource->GetOrigin() , m_coronalPlaneSource->GetNormal() , m_axialPlaneSource->GetOrigin() , m_axialPlaneSource->GetNormal() , r , t );

    position1[0] = r[0] - t[0]*2000;
    position1[1] = r[1] - t[1]*2000;
    position1[2] = r[2] - t[2]*2000;

    position2[0] = r[0] + t[0]*2000;
    position2[1] = r[1] + t[1]*2000;
    position2[2] = r[2] + t[2]*2000;

    m_coronalOverAxialIntersectionAxis->SetPosition(  position1[0] , position1[1] );
    m_coronalOverAxialIntersectionAxis->SetPosition2( position2[0] , position2[1] );

    // projecció thick slab sobre axial
    MathTools::planeIntersection( m_thickSlabPlaneSource->GetOrigin() , m_thickSlabPlaneSource->GetNormal() , m_axialPlaneSource->GetOrigin() , m_axialPlaneSource->GetNormal() , r , t );

    position1[0] = r[0] - t[0]*2000;
    position1[1] = r[1] - t[1]*2000;
    position1[2] = r[2] - t[2]*2000;

    position2[0] = r[0] + t[0]*2000;
    position2[1] = r[1] + t[1]*2000;
    position2[2] = r[2] + t[2]*2000;

    m_thickSlabOverAxialActor->SetPosition( position1[0] , position1[1] );
    m_thickSlabOverAxialActor->SetPosition2( position2[0] , position2[1] );

//     Repintem l'escena
    m_axial2DView->refresh();
    m_sagital2DView->refresh();
    m_coronal2DView->refresh();
}

void QMPRExtension::updateIntersectionPoint()
{
    MathTools::planeIntersection( m_coronalPlaneSource->GetOrigin() , m_coronalPlaneSource->GetNormal() , m_sagitalPlaneSource->GetOrigin() , m_sagitalPlaneSource->GetNormal() , m_axialPlaneSource->GetOrigin() , m_axialPlaneSource->GetNormal() ,  m_intersectionPoint );
}

void QMPRExtension::updatePlanes()
{
    updatePlane( m_sagitalPlaneSource , m_sagitalReslice );
    updatePlane( m_coronalPlaneSource , m_coronalReslice );
    updateIntersectionPoint();
}

void QMPRExtension::updatePlane( vtkPlaneSource *planeSource , vtkImageReslice *reslice )
{
    if ( !reslice || !( vtkImageData::SafeDownCast( reslice->GetInput() ) )  )
    {
        return;
    }

    // Calculate appropriate pixel spacing for the reslicing
    //
    double spacing[3];
    m_volume->getSpacing( spacing );

    int i;

//     if ( this->RestrictPlaneToVolume )
//     {
        double origin[3];
        m_volume->getOrigin( origin );

        int extent[6];
        m_volume->getWholeExtent(extent);

        double bounds[] = { origin[0] + spacing[0]*extent[0], //xmin
                        origin[0] + spacing[0]*extent[1], //xmax
                        origin[1] + spacing[1]*extent[2], //ymin
                        origin[1] + spacing[1]*extent[3], //ymax
                        origin[2] + spacing[2]*extent[4], //zmin
                        origin[2] + spacing[2]*extent[5] };//zmax

        for ( i = 0; i <= 4; i += 2 ) // reverse bounds if necessary
        {
            if ( bounds[i] > bounds[i+1] )
            {
                double t = bounds[i+1];
                bounds[i+1] = bounds[i];
                bounds[i] = t;
            }
        }

        double abs_normal[3];
        planeSource->GetNormal(abs_normal);

        double planeCenter[3];
        planeSource->GetCenter(planeCenter);

        double nmax = 0.0;
        int k = 0;
        for ( i = 0; i < 3; i++ )
        {
            abs_normal[i] = fabs(abs_normal[i]);
            if ( abs_normal[i]>nmax )
            {
                nmax = abs_normal[i];
                k = i;
            }
        }
    // Force the plane to lie within the true image bounds along its normal
    //
        if ( planeCenter[k] > bounds[2*k+1] )
        {
            planeCenter[k] = bounds[2*k+1];
        }
        else if ( planeCenter[k] < bounds[2*k] )
        {
            planeCenter[k] = bounds[2*k];
        }
        planeSource->SetCenter(planeCenter);
//     }

    double planeAxis1[3];
    double planeAxis2[3];
    // obtenim els vectors
    planeAxis1[0] = planeSource->GetPoint1()[0] - planeSource->GetOrigin()[0];
    planeAxis1[1] = planeSource->GetPoint1()[1] - planeSource->GetOrigin()[1];
    planeAxis1[2] = planeSource->GetPoint1()[2] - planeSource->GetOrigin()[2];

    planeAxis2[0] = planeSource->GetPoint2()[0] - planeSource->GetOrigin()[0];
    planeAxis2[1] = planeSource->GetPoint2()[1] - planeSource->GetOrigin()[1];
    planeAxis2[2] = planeSource->GetPoint2()[2] - planeSource->GetOrigin()[2];

    // The x,y dimensions of the plane
    //
    // TODO fer la normalització amb un mètode de MathTools
    double planeSizeX = vtkMath::Normalize( planeAxis1 );
    double planeSizeY = vtkMath::Normalize( planeAxis2 );

    double normal[3];
    planeSource->GetNormal( normal );

    // Generate the slicing matrix
    //
    // Podria ser membre de classe, com era originalriament o passar per paràmetre
    vtkMatrix4x4 *resliceAxes = vtkMatrix4x4::New();
    resliceAxes->Identity();
    for ( i = 0; i < 3; i++ )
    {
        resliceAxes->SetElement(0,i,planeAxis1[i]);
        resliceAxes->SetElement(1,i,planeAxis2[i]);
        resliceAxes->SetElement(2,i,normal[i]);
    }

    double planeOrigin[4];
    planeSource->GetOrigin(planeOrigin);
    planeOrigin[3] = 1.0;
    double originXYZW[4];
    resliceAxes->MultiplyPoint(planeOrigin,originXYZW);

    resliceAxes->Transpose();
    double neworiginXYZW[4];
    double point[] =  {originXYZW[0],originXYZW[1],originXYZW[2],originXYZW[3]};
    resliceAxes->MultiplyPoint(point,neworiginXYZW);

    resliceAxes->SetElement(0,3,neworiginXYZW[0]);
    resliceAxes->SetElement(1,3,neworiginXYZW[1]);
    resliceAxes->SetElement(2,3,neworiginXYZW[2]);

    reslice->SetResliceAxes( resliceAxes );

    double spacingX = fabs(planeAxis1[0]*spacing[0])+\
                    fabs(planeAxis1[1]*spacing[1])+\
                    fabs(planeAxis1[2]*spacing[2]);

    double spacingY = fabs(planeAxis2[0]*spacing[0])+\
                    fabs(planeAxis2[1]*spacing[1])+\
                    fabs(planeAxis2[2]*spacing[2]);


    // Pad extent up to a power of two for efficient texture mapping

    // make sure we're working with valid values
    double realExtentX = ( spacingX == 0 ) ? 0 : planeSizeX / spacingX;

    int extentX;
    // Sanity check the input data:
    // * if realExtentX is too large, extentX will wrap
    // * if spacingX is 0, things will blow up.
    // * if realExtentX is naturally 0 or < 0, the padding will yield an
    //   extentX of 1, which is also not desirable if the input data is invalid.
    if (realExtentX > (VTK_INT_MAX >> 1) || realExtentX < 1)
    {
        WARN_LOG( "Invalid X extent. [" + QString::number( realExtentX ) + "] Perhaps the input data is empty?" );
        extentX = 0;
    }
    else
    {
        extentX = 1;
        while (extentX < realExtentX)
        {
            extentX = extentX << 1;
        }
    }

    // make sure extentY doesn't wrap during padding
    double realExtentY = ( spacingY == 0 ) ? 0 : planeSizeY / spacingY;

    int extentY;
    if (realExtentY > (VTK_INT_MAX >> 1) || realExtentY < 1)
    {
        WARN_LOG( "Invalid Y extent. [" + QString::number( realExtentY ) + "] Perhaps the input data is empty?" );
        extentY = 0;
    }
    else
    {
        extentY = 1;
        while (extentY < realExtentY)
        {
            extentY = extentY << 1;
        }
    }

    reslice->SetOutputSpacing( planeSizeX/extentX , planeSizeY/extentY , 1 );
    reslice->SetOutputOrigin( 0.0 , 0.0 , 0.0 );
    // \TODO li passem thickSlab que és double però això només accepta int's! Buscar si aquesta és la manera adequada. Potsre si volem fer servir doubles ho hauríem de combinar amb l'outputSpacing
    reslice->SetOutputExtent( 0 , extentX-1 , 0 , extentY-1 , 0 , static_cast<int>( m_thickSlab ) ); // obtenim una única llesca
    reslice->Update();
}

void QMPRExtension::getSagitalXVector( double x[3] )
{
    double* p1 = m_sagitalPlaneSource->GetPoint1();
    double* o =  m_sagitalPlaneSource->GetOrigin();
    x[0] = p1[0] - o[0];
    x[1] = p1[1] - o[1];
    x[2] = p1[2] - o[2];
}

void QMPRExtension::getSagitalYVector( double y[3] )
{
    double* p1 = m_sagitalPlaneSource->GetPoint2();
    double* o =  m_sagitalPlaneSource->GetOrigin();
    y[0] = p1[0] - o[0];
    y[1] = p1[1] - o[1];
    y[2] = p1[2] - o[2];
}

void QMPRExtension::getCoronalXVector( double x[3] )
{
    double* p1 = m_coronalPlaneSource->GetPoint1();
    double* o =  m_coronalPlaneSource->GetOrigin();
    x[0] = p1[0] - o[0];
    x[1] = p1[1] - o[1];
    x[2] = p1[2] - o[2];
}

void QMPRExtension::getAxialXVector( double x[3] )
{
    double* p1 = m_axialPlaneSource->GetPoint1();
    double* o =  m_axialPlaneSource->GetOrigin();
    x[0] = p1[0] - o[0];
    x[1] = p1[1] - o[1];
    x[2] = p1[2] - o[2];
}

void QMPRExtension::getAxialYVector( double y[3] )
{
    double* p2 = m_axialPlaneSource->GetPoint2();
    double* o =  m_axialPlaneSource->GetOrigin();
    y[0] = p2[0] - o[0];
    y[1] = p2[1] - o[1];
    y[2] = p2[2] - o[2];
}
void QMPRExtension::getCoronalYVector( double y[3] )
{
    double* p1 = m_coronalPlaneSource->GetPoint2();
    double* o =  m_coronalPlaneSource->GetOrigin();
    y[0] = p1[0] - o[0];
    y[1] = p1[1] - o[1];
    y[2] = p1[2] - o[2];
}

bool QMPRExtension::isParallel( double axis[3] )
{
    double xyzAxis[3] = {1,0,0};
    // TODO hauríem de tenir un mètode MathTools::areParallel(vector1, vector2)
    if( MathTools::angleInDegrees( xyzAxis , axis ) == 0.0 )
    {
        return true;
    }
    xyzAxis[0] = -1;
    xyzAxis[1] = 0;
    xyzAxis[2] = 0;
    if( MathTools::angleInDegrees( xyzAxis , axis ) == 0.0 )
    {
        return true;
    }

    xyzAxis[0] = 0;
    xyzAxis[1] = 0;
    xyzAxis[2] = 1;
    if( MathTools::angleInDegrees( xyzAxis , axis ) == 0.0 )
    {
        return true;
    }
    xyzAxis[0] = 0;
    xyzAxis[1] = 0;
    xyzAxis[2] = -1;
    if( MathTools::angleInDegrees( xyzAxis , axis ) == 0.0 )
    {
        return true;
    }

    xyzAxis[0] = 0;
    xyzAxis[1] = 1;
    xyzAxis[2] = 0;
    if( MathTools::angleInDegrees( xyzAxis , axis ) == 0.0 )
    {
        return true;
    }
    xyzAxis[0] = 0;
    xyzAxis[1] = -1;
    xyzAxis[2] = 0;
    if( MathTools::angleInDegrees( xyzAxis , axis ) == 0.0 )
    {
        return true;
    }

    return false;
}

void QMPRExtension::rotateMiddle( double degrees , double rotationAxis[3] ,  vtkPlaneSource* plane )
{
//     vtkMath::Normalize( rotationAxis );
    m_transform->Identity();
    m_transform->Translate( plane->GetCenter()[0], plane->GetCenter()[1], plane->GetCenter()[2] );
    m_transform->RotateWXYZ( degrees , rotationAxis );
    m_transform->Translate( -plane->GetCenter()[0], -plane->GetCenter()[1], -plane->GetCenter()[2] );
    // ara que tenim la transformació, l'apliquem als punts del pla ( origen, punt1 , punt2)
    double newPoint[3];
    m_transform->TransformPoint( plane->GetPoint1() , newPoint );
    plane->SetPoint1( newPoint );
    m_transform->TransformPoint( plane->GetPoint2() , newPoint );
    plane->SetPoint2( newPoint );
    m_transform->TransformPoint( plane->GetOrigin() , newPoint );
    plane->SetOrigin( newPoint );
    plane->Update();
}

void QMPRExtension::rotate( double degrees , double rotationAxis[3] ,  vtkPlaneSource* plane )
{
//    Normalitzem l'eix de rotació, serà molt millor per les operacions a fer
    // TODO fer la normalització amb un mètode de MathTools
    vtkMath::Normalize( rotationAxis );

    if( isParallel( rotationAxis ) )
    {
        m_transform->Identity();
        m_transform->Translate( m_intersectionPoint[0], m_intersectionPoint[1], m_intersectionPoint[2] );
        m_transform->RotateWXYZ( degrees , rotationAxis );
        m_transform->Translate( -m_intersectionPoint[0], -m_intersectionPoint[1], -m_intersectionPoint[2] );
    }
    else
    {
    // (0)
        m_transform->Identity();
    // (1)
        m_transform->Translate( m_intersectionPoint[0], m_intersectionPoint[1], m_intersectionPoint[2] );
    // (2)
        double alpha, lp;
        lp = sqrt( rotationAxis[1]*rotationAxis[1] + rotationAxis[2]*rotationAxis[2] );
        alpha = asin( rotationAxis[1] / lp ) ;
//         alpha = acos( rotationAxis[2] / lp );
        m_transform->RotateX( -alpha * vtkMath::DoubleRadiansToDegrees()  ); // o RotateWXYZ( alpha , [1,0,0])
    // (3)
        double beta , l;
        l = sqrt( rotationAxis[0]*rotationAxis[0] + rotationAxis[1]*rotationAxis[1] + rotationAxis[2]*rotationAxis[2] );
        beta =  asin( rotationAxis[0] );
        m_transform->RotateY( beta * vtkMath::DoubleRadiansToDegrees()  ); // o RotateWXYZ( beta , [0,1,0])
    // (4)
        m_transform->RotateZ( degrees ); // o RotateWXYZ( degrees , [0,0,1])
    // (5)
        m_transform->RotateY( -beta * vtkMath::DoubleRadiansToDegrees() ); // o RotateWXYZ( -beta , [0,1,0])
    // (6)
        m_transform->RotateX( alpha * vtkMath::DoubleRadiansToDegrees()  ); // o RotateWXYZ( -alpha , [1,0,0])
    // (7)
        m_transform->Translate( -m_intersectionPoint[0], -m_intersectionPoint[1], -m_intersectionPoint[2] );

    }
    // ara que tenim la transformació, l'apliquem als punts del pla ( origen, punt1 , punt2)
    double newPoint[3];
    m_transform->TransformPoint( plane->GetPoint1() , newPoint );
    plane->SetPoint1( newPoint );
    m_transform->TransformPoint( plane->GetPoint2() , newPoint );
    plane->SetPoint2( newPoint );
    m_transform->TransformPoint( plane->GetOrigin() , newPoint );
    plane->SetOrigin( newPoint );

    plane->Update();
}

void QMPRExtension::updateThickSlab( double value )
{
    m_thickSlab = value;
    m_thickSlabSlider->setValue( (int) value );
    updatePlane( m_coronalPlaneSource , m_coronalReslice );
    updateControls();
}

void QMPRExtension::updateThickSlab( int value )
{
    m_thickSlab = (double) value;
    m_thickSlabSpinBox->setValue( m_thickSlab );
    updatePlane( m_coronalPlaneSource , m_coronalReslice );
    updateControls();
}

void QMPRExtension::readSettings()
{
    Settings settings;

    if( settings.getValue( MPRSettings::HorizontalSplitterGeometry).toByteArray().isEmpty() )
    {
        QList<int> list;
        list << this->size().width()/2 << this->size().width()/2;
        m_horizontalSplitter->setSizes( list );
    }
    else
        settings.restoreGeometry( MPRSettings::HorizontalSplitterGeometry, m_horizontalSplitter );

    if( settings.getValue( MPRSettings::VerticalSplitterGeometry).toByteArray().isEmpty() )
    {
        QList<int> list;
        list << this->size().height()/2 << this->size().height()/2;
        m_verticalSplitter->setSizes( list );
    }
    else
        settings.restoreGeometry( MPRSettings::VerticalSplitterGeometry, m_verticalSplitter );
}

void QMPRExtension::writeSettings()
{
    Settings settings;

    settings.saveGeometry( MPRSettings::HorizontalSplitterGeometry, m_horizontalSplitter );
    settings.saveGeometry( MPRSettings::VerticalSplitterGeometry, m_verticalSplitter );
}

};  // end namespace udg


