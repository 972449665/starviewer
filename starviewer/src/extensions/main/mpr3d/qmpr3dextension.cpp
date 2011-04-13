#include "qmpr3dextension.h"
#include "q3dmprviewer.h"
#include "logging.h"
#include "toolmanager.h"
#include "volume.h"
#include "series.h"
#include "windowlevelpresetstooldata.h"
#include "mpr3dsettings.h"
#include <QToolButton>
#include <QAction>

namespace udg {

QMPR3DExtension::QMPR3DExtension( QWidget *parent )
 : QWidget( parent )
{
    setupUi( this );
    MPR3DSettings().init();

    initializeTools();
    createConnections();

    m_axialViewEnabledButton->setChecked( true );
    m_sagitalViewEnabledButton->setChecked( true );
    m_coronalViewEnabledButton->setChecked( true );
    m_mpr3DView->orientationMarkerOff();

    // ajustaments de window level pel combo box
    m_windowLevelComboBox->setPresetsData( m_mpr3DView->getWindowLevelData() );
    m_windowLevelComboBox->selectPreset( m_mpr3DView->getWindowLevelData()->getCurrentPreset() );
}

QMPR3DExtension::~QMPR3DExtension()
{
}

void QMPR3DExtension::initializeTools()
{
    m_toolManager = new ToolManager(this);
    // obtenim les accions de cada tool que volem
    m_zoomToolButton->setDefaultAction( m_toolManager->registerTool("ZoomTool") );
    m_rotate3DToolButton->setDefaultAction( m_toolManager->registerTool("Rotate3DTool") );
    m_moveToolButton->setDefaultAction( m_toolManager->registerTool("TranslateTool") );
    m_screenShotToolButton->setDefaultAction( m_toolManager->registerTool("ScreenShotTool") );
    m_toolManager->registerTool("WindowLevelPresetsTool");

    // Activem les tools que volem tenir per defecte, això és com si clickéssim a cadascun dels ToolButton
    // En aquest cas activem totes les registrades
    m_toolManager->triggerTools( m_toolManager->getRegisteredToolsList() );

    // registrem al manager les tools que van amb el viewer principal
    m_toolManager->setupRegisteredTools( m_mpr3DView );

    // Action tools
    m_axialOrientationButton->setDefaultAction( m_toolManager->registerActionTool( "AxialViewActionTool" ) );
    m_sagitalOrientationButton->setDefaultAction( m_toolManager->registerActionTool( "SagitalViewActionTool" ) );
    m_coronalOrientationButton->setDefaultAction( m_toolManager->registerActionTool( "CoronalViewActionTool" ) );
    m_toolManager->enableRegisteredActionTools( m_mpr3DView );
}

void QMPR3DExtension::createConnections()
{
    connect( m_axialViewEnabledButton , SIGNAL( toggled(bool) ) , m_mpr3DView , SLOT( setAxialVisibility(bool) ) );
    connect( m_sagitalViewEnabledButton , SIGNAL( toggled(bool) ) , m_mpr3DView , SLOT( setSagitalVisibility(bool) ) );
    connect( m_coronalViewEnabledButton , SIGNAL( toggled(bool) ) , m_mpr3DView , SLOT( setCoronalVisibility(bool) ) );

    connect( m_mpr3DView, SIGNAL( volumeChanged(Volume *) ), SLOT( updateExtension(Volume *) ) );
}

void QMPR3DExtension::setInput( Volume *input )
{
    m_volume = input;
    m_mpr3DView->setInput( m_volume );
    updateExtension(m_volume);
    INFO_LOG("QMPR3DExtension:: Donem Input ");
}

void QMPR3DExtension::updateExtension( Volume * volume )
{
    m_volume = volume;
    if( m_volume->getNumberOfPhases() > 1 )
        m_phasesAlertLabel->setVisible(true);
    else
        m_phasesAlertLabel->setVisible(false);

    double wl[2];
    m_mpr3DView->getCurrentWindowLevel( wl );
    // TODO faltaria actualitzar els botons que calgui i/o altre elements de l'interfície

}

};  // end namespace udg
