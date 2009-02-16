/***************************************************************************
 *   Copyright (C) 2007 by Grup de Gràfics de Girona                       *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/

#include "qvolume3dviewtestingextension.h"
#include "volume.h"
#include "toolmanager.h"
#include "transferfunctionio.h"
#include "renderingstyle.h"
#include "starviewerapplication.h"
// qt
#include <QAction>
#include <QFileDialog>
#include <QSettings>
#include <QStandardItemModel>
#include <QTimer>
// vtk
#include <vtkImageData.h>
#include <vtkRenderWindow.h>            // actualització ràpida
#include <vtkRenderWindowInteractor.h>  // actualització ràpida

namespace udg {

QVolume3DViewTestingExtension::QVolume3DViewTestingExtension( QWidget * parent )
 : QWidget( parent )
{
    setupUi( this );

    // creem el temporitzador (s'ha de fer abans del createConnections())
    m_timer = new QTimer( this );
    m_timer->setSingleShot( true );
    m_timer->setInterval( 1000 );   /// \todo Poso 1000 ms arbitràriament.

    initializeTools();
    loadClutPresets();
    loadRenderingStyles();
    createConnections();
    readSettings();
    updateUiForRenderingMethod( m_renderingMethodComboBox->currentIndex() );
    m_obscuranceCheckBox->hide(); m_obscuranceFactorLabel->hide(); m_obscuranceFactorDoubleSpinBox->hide();

    m_computingObscurance = false;

    m_firstInput = true;

    hideClutEditor();
}

QVolume3DViewTestingExtension::~QVolume3DViewTestingExtension()
{
    writeSettings();
}

void QVolume3DViewTestingExtension::initializeTools()
{
    m_toolManager = new ToolManager(this);
    // obtenim les accions de cada tool que volem
    m_zoomToolButton->setDefaultAction( m_toolManager->getToolAction("ZoomTool") );
    m_rotate3DToolButton->setDefaultAction( m_toolManager->getToolAction("Rotate3DTool") );
    m_windowLevelToolButton->setDefaultAction( m_toolManager->getToolAction("WindowLevelTool") );
    m_panToolButton->setDefaultAction( m_toolManager->getToolAction("TranslateTool") );

    // Activem les tools que volem tenir per defecte, això és com si clickéssim a cadascun dels ToolButton
    m_zoomToolButton->defaultAction()->trigger();
    m_panToolButton->defaultAction()->trigger();
    m_rotate3DToolButton->defaultAction()->trigger();
    // afegim eina d'screen-shot TODO cal posar botó a la interfície???
    m_toolManager->getToolAction("ScreenShotTool")->trigger();

    // registrem al manager les tools que van amb el viewer principal
    QStringList toolsList;
    toolsList << "ZoomTool" << "TranslateTool" << "Rotate3DTool" << "WindowLevelTool" << "ScreenShotTool";
    m_toolManager->setViewerTools( m_3DView, toolsList );

    QStringList rightButtonExclusiveTools;
    rightButtonExclusiveTools << "Rotate3DTool" << "WindowLevelTool";
    m_toolManager->addExclusiveToolsGroup("RightButtonGroup", rightButtonExclusiveTools);
}

void QVolume3DViewTestingExtension::loadClutPresets()
{
    DEBUG_LOG( "loadClutPresets()" );

    disconnect( m_clutPresetsComboBox, SIGNAL( currentIndexChanged(const QString&) ), this, SLOT( applyPresetClut(const QString&) ) );

    m_clutPresetsComboBox->clear();

    QString path = QDir::toNativeSeparators( UserDataRootPath + "cluts/" );
    m_clutsDir.setPath( path );

    if ( !m_clutsDir.exists() ) m_clutsDir.mkpath( path );
    if ( !m_clutsDir.isReadable() ) return; // no es pot accedir al directori

    QStringList nameFilters;
    nameFilters << "*.tf";

    QStringList clutList = m_clutsDir.entryList( nameFilters );

    foreach ( const QString & clutName, clutList )
    {
        TransferFunction * transferFunction = TransferFunctionIO::fromFile( m_clutsDir.absoluteFilePath( clutName ) );
        if ( transferFunction )
        {
            m_clutNameToFileName[transferFunction->name()] = clutName;
            delete transferFunction;
        }
    }

    m_clutPresetsComboBox->addItems( m_clutNameToFileName.keys() );
    m_clutPresetsComboBox->setCurrentIndex( -1 );

    connect( m_clutPresetsComboBox, SIGNAL( currentIndexChanged(const QString&) ), this, SLOT( applyPresetClut(const QString&) ) );
}

void QVolume3DViewTestingExtension::loadRenderingStyles()
{
    m_renderingStyleModel = new QStandardItemModel( this );

    // Per la primera versió creem tot això a pèl.
    // Per cada estil creem un item i li assignem unes dades que seran les que es faran servir a l'hora d'aplicar-lo.
    QStandardItem *item;
    RenderingStyle renderingStyle;
    TransferFunction *transferFunction;

    item = new QStandardItem( QIcon( ":/extensions/Volume3DViewTestingExtension/renderingstyles/spine2.png" ), tr("Spine") );
    renderingStyle.method = RenderingStyle::RayCasting;
    renderingStyle.diffuseLighting = true;
    renderingStyle.specularLighting = true;
    renderingStyle.specularPower = 64.0;
    transferFunction = TransferFunctionIO::fromXmlFile( ":/extensions/Volume3DViewTestingExtension/renderingstyles/spine2.xml" );
    renderingStyle.transferFunction = *transferFunction;
    delete transferFunction;
    renderingStyle.contour = false;
    renderingStyle.obscurance = false;
    item->setData( renderingStyle.toVariant() );
    m_renderingStyleModel->appendRow( item );

    item = new QStandardItem( QIcon( ":/extensions/Volume3DViewTestingExtension/renderingstyles/thorax1.png" ), tr("Thorax") );
    renderingStyle.method = RenderingStyle::RayCasting;
    renderingStyle.diffuseLighting = true;
    renderingStyle.specularLighting = true;
    renderingStyle.specularPower = 64.0;
    transferFunction = TransferFunctionIO::fromXmlFile( ":/extensions/Volume3DViewTestingExtension/renderingstyles/thorax1.xml" );
    renderingStyle.transferFunction = *transferFunction;
    delete transferFunction;
    renderingStyle.contour = false;
    renderingStyle.obscurance = false;
    item->setData( renderingStyle.toVariant() );
    m_renderingStyleModel->appendRow( item );

    item = new QStandardItem( QIcon( ":/extensions/Volume3DViewTestingExtension/renderingstyles/pelvis2.png" ), tr("Pelvis") );
    renderingStyle.method = RenderingStyle::RayCasting;
    renderingStyle.diffuseLighting = true;
    renderingStyle.specularLighting = true;
    renderingStyle.specularPower = 64.0;
    transferFunction = TransferFunctionIO::fromXmlFile( ":/extensions/Volume3DViewTestingExtension/renderingstyles/pelvis2.xml" );
    renderingStyle.transferFunction = *transferFunction;
    delete transferFunction;
    renderingStyle.contour = false;
    renderingStyle.obscurance = false;
    item->setData( renderingStyle.toVariant() );
    m_renderingStyleModel->appendRow( item );

    item = new QStandardItem( QIcon( ":/extensions/Volume3DViewTestingExtension/renderingstyles/cow1.png" ), tr("Circle of Willis") );
    renderingStyle.method = RenderingStyle::RayCasting;
    renderingStyle.diffuseLighting = true;
    renderingStyle.specularLighting = false;
    transferFunction = TransferFunctionIO::fromXmlFile( ":/extensions/Volume3DViewTestingExtension/renderingstyles/cow1.xml" );
    renderingStyle.transferFunction = *transferFunction;
    delete transferFunction;
    renderingStyle.contour = false;
    renderingStyle.obscurance = false;
    item->setData( renderingStyle.toVariant() );
    m_renderingStyleModel->appendRow( item );

    item = new QStandardItem( QIcon( ":/extensions/Volume3DViewTestingExtension/renderingstyles/carotids2.png" ), tr("Carotids") );
    renderingStyle.method = RenderingStyle::RayCasting;
    renderingStyle.diffuseLighting = true;
    renderingStyle.specularLighting = true;
    renderingStyle.specularPower = 64.0;
    transferFunction = TransferFunctionIO::fromXmlFile( ":/extensions/Volume3DViewTestingExtension/renderingstyles/carotids2.xml" );
    renderingStyle.transferFunction = *transferFunction;
    delete transferFunction;
    renderingStyle.contour = false;
    renderingStyle.obscurance = false;
    item->setData( renderingStyle.toVariant() );
    m_renderingStyleModel->appendRow( item );

    item = new QStandardItem( QIcon( ":/extensions/Volume3DViewTestingExtension/renderingstyles/bones4.png" ), tr("Bones") );
    renderingStyle.method = RenderingStyle::RayCasting;
    renderingStyle.diffuseLighting = true;
    renderingStyle.specularLighting = true;
    renderingStyle.specularPower = 64.0;
    transferFunction = TransferFunctionIO::fromXmlFile( ":/extensions/Volume3DViewTestingExtension/renderingstyles/bones4.xml" );
    renderingStyle.transferFunction = *transferFunction;
    delete transferFunction;
    renderingStyle.contour = false;
    renderingStyle.obscurance = false;
    item->setData( renderingStyle.toVariant() );
    m_renderingStyleModel->appendRow( item );

    item = new QStandardItem( QIcon( ":/extensions/Volume3DViewTestingExtension/renderingstyles/bonehires.png" ), tr("Bones 2") );
    renderingStyle.method = RenderingStyle::RayCasting;
    renderingStyle.diffuseLighting = true;
    renderingStyle.specularLighting = false;
    transferFunction = TransferFunctionIO::fromXmlFile( ":/extensions/Volume3DViewTestingExtension/renderingstyles/bonehires.xml" );
    renderingStyle.transferFunction = *transferFunction;
    delete transferFunction;
    renderingStyle.contour = false;
    renderingStyle.obscurance = false;
    item->setData( renderingStyle.toVariant() );
    m_renderingStyleModel->appendRow( item );

    item = new QStandardItem( QIcon( ":/extensions/Volume3DViewTestingExtension/renderingstyles/abdomenbones.png" ), tr("Abdomen bones") );
    renderingStyle.method = RenderingStyle::RayCasting;
    renderingStyle.diffuseLighting = true;
    renderingStyle.specularLighting = true;
    renderingStyle.specularPower = 64.0;
    transferFunction = TransferFunctionIO::fromXmlFile( ":/extensions/Volume3DViewTestingExtension/renderingstyles/abdomenbones.xml" );
    renderingStyle.transferFunction = *transferFunction;
    delete transferFunction;
    renderingStyle.contour = false;
    renderingStyle.obscurance = false;
    item->setData( renderingStyle.toVariant() );
    m_renderingStyleModel->appendRow( item );

    item = new QStandardItem( QIcon( ":/extensions/Volume3DViewTestingExtension/renderingstyles/abdomenrunoff1.png" ), tr("Abdomen run-off") );
    renderingStyle.method = RenderingStyle::RayCasting;
    renderingStyle.diffuseLighting = true;
    renderingStyle.specularLighting = true;
    renderingStyle.specularPower = 64.0;
    transferFunction = TransferFunctionIO::fromXmlFile( ":/extensions/Volume3DViewTestingExtension/renderingstyles/abdomenrunoff1.xml" );
    renderingStyle.transferFunction = *transferFunction;
    delete transferFunction;
    renderingStyle.contour = false;
    renderingStyle.obscurance = false;
    item->setData( renderingStyle.toVariant() );
    m_renderingStyleModel->appendRow( item );

    item = new QStandardItem( QIcon( ":/extensions/Volume3DViewTestingExtension/renderingstyles/abdomenslab.png" ), tr("Abdomen slab") );
    renderingStyle.method = RenderingStyle::RayCasting;
    renderingStyle.diffuseLighting = true;
    renderingStyle.specularLighting = true;
    renderingStyle.specularPower = 64.0;
    transferFunction = TransferFunctionIO::fromXmlFile( ":/extensions/Volume3DViewTestingExtension/renderingstyles/abdomenslab.xml" );
    renderingStyle.transferFunction = *transferFunction;
    delete transferFunction;
    renderingStyle.contour = false;
    renderingStyle.obscurance = false;
    item->setData( renderingStyle.toVariant() );
    m_renderingStyleModel->appendRow( item );

    m_renderingStyleListView->setModel( m_renderingStyleModel );
}

void QVolume3DViewTestingExtension::createConnections()
{
    // orientacions axial,sagital i coronal
    connect( m_axialOrientationButton , SIGNAL( clicked() ) , m_3DView , SLOT( resetViewToAxial() ) );
    connect( m_sagitalOrientationButton , SIGNAL( clicked() ) , m_3DView , SLOT( resetViewToSagital() ) );
    connect( m_coronalOrientationButton , SIGNAL( clicked() ) , m_3DView , SLOT( resetViewToCoronal() ) );

    // actualització del mètode de rendering
    connect( m_renderingMethodComboBox, SIGNAL( currentIndexChanged(int) ), this, SLOT( updateUiForRenderingMethod(int) ) );

    // shading
    connect( m_shadingCheckBox, SIGNAL( toggled(bool) ), m_specularCheckBox, SLOT( setEnabled(bool) ) );
    connect( m_specularCheckBox, SIGNAL( toggled(bool) ), m_specularPowerLabel, SLOT( setEnabled(bool) ) );
    connect( m_specularCheckBox, SIGNAL( toggled(bool) ), m_specularPowerDoubleSpinBox, SLOT( setEnabled(bool) ) );

    // contorns
    connect( m_contourCheckBox, SIGNAL( toggled(bool) ), m_contourThresholdLabel, SLOT( setEnabled(bool) ) );
    connect( m_contourCheckBox, SIGNAL( toggled(bool) ), m_contourThresholdDoubleSpinBox, SLOT( setEnabled(bool) ) );

    // obscurances
    connect( m_obscuranceComputeCancelPushButton, SIGNAL( clicked() ), this, SLOT( computeOrCancelObscurance() ) );
    connect( m_3DView, SIGNAL( obscuranceProgress(int) ), m_obscuranceProgressBar, SLOT( setValue(int) ) );
    connect( m_3DView, SIGNAL( obscuranceComputed() ), this, SLOT( endComputeObscurance() ) );
    connect( m_3DView, SIGNAL( obscuranceCancelledByProgram() ), this, SLOT( autoCancelObscurance() ) );
    connect( m_obscuranceCheckBox, SIGNAL( toggled(bool) ), m_obscuranceFactorLabel, SLOT( setEnabled(bool ) ) );
    connect( m_obscuranceCheckBox, SIGNAL( toggled(bool) ), m_obscuranceFactorDoubleSpinBox, SLOT( setEnabled(bool) ) );

    enableAutoUpdate();

    // clut editor
    connect( m_loadClutPushButton, SIGNAL( clicked() ), SLOT( loadClut() ) );
    connect( m_saveClutPushButton, SIGNAL( clicked() ), SLOT( saveClut() ) );
    connect( m_switchEditorPushButton, SIGNAL( clicked() ), SLOT( switchEditor() ) );
    connect( m_applyPushButton, SIGNAL( clicked() ), SLOT( applyEditorClut() ) );

    connect( m_customStyleToolButton, SIGNAL( clicked() ), SLOT( toggleClutEditor() ) );

    connect( m_3DView, SIGNAL( transferFunctionChanged () ), SLOT( changeViewerTransferFunction() ) );
    connect( this, SIGNAL( newTransferFunction () ), m_3DView, SLOT( setNewTransferFunction() ) );

    // visor 3d
    connect( m_3DView, SIGNAL( scalarRange(double,double) ), SLOT( setScalarRange(double,double) ) );

    // rendering styles
    connect( m_renderingStyleListView, SIGNAL( activated(const QModelIndex&) ), SLOT( applyRenderingStyle(const QModelIndex&) ) );

    connect(m_editorSplitter, SIGNAL( splitterMoved(int, int)), SLOT( setCustomStyleButtonStateBySplitter() ));

    // temporitzador
    connect( m_timer, SIGNAL( timeout() ), SLOT( render() ) );
}

void QVolume3DViewTestingExtension::setInput( Volume * input )
{
    m_input = input;
    m_3DView->setInput( m_input );

    applyClut( m_currentClut );
    this->render();
}

void QVolume3DViewTestingExtension::setScalarRange( double min, double max )
{
    unsigned short maximum = static_cast<unsigned short>( qRound( max ) );
    m_gradientEditor->setMaximum( maximum );
    m_editorByValues->setMaximum( maximum );
    m_isoValueSpinBox->setMaximum( maximum );

    if ( m_firstInput )
    {
        m_currentClut.addPoint( min, QColor( 0, 0, 0, 0 ) );
        m_currentClut.addPoint( max, QColor( 255, 255, 255, 255 ) );
        m_firstInput = false;
        emit newTransferFunction ();
    }
}


void QVolume3DViewTestingExtension::applyPresetClut( const QString & clutName )
{
    DEBUG_LOG( "applyPresetClut()" );

    const QString & fileName = m_clutNameToFileName[clutName];
    TransferFunction * transferFunction = TransferFunctionIO::fromFile( m_clutsDir.absoluteFilePath( QDir::toNativeSeparators( fileName ) ) );
    if ( transferFunction )
    {
        //m_currentClut = *transferFunction;
        applyClut( *transferFunction, true );
        this->render();
        //m_3DView->setTransferFunction( transferFunction );
    }
    //this->render();
}


void QVolume3DViewTestingExtension::applyClut( const TransferFunction & clut, bool preset )
{
    DEBUG_LOG( "applyClut()" );

    m_currentClut = clut;
    if ( !preset )
    {
        // cal fer el disconnect per evitar un bucle infinit
        disconnect( m_clutPresetsComboBox, SIGNAL( currentIndexChanged(const QString&) ), this, SLOT( applyPresetClut(const QString&) ) );
        m_clutPresetsComboBox->setCurrentIndex( -1 );
        connect( m_clutPresetsComboBox, SIGNAL( currentIndexChanged(const QString&) ), this, SLOT( applyPresetClut(const QString&) ) );
    }
    m_gradientEditor->setTransferFunction( m_currentClut );
    m_editorByValues->setTransferFunction( m_currentClut );
    m_3DView->setTransferFunction( new TransferFunction( m_currentClut ) );
    emit newTransferFunction();
//     this->render();
}

void QVolume3DViewTestingExtension::changeViewerTransferFunction( )
{
    //Actualitzem l'editor de cluts quan es canvia per la funció pel w/l del visor
    m_gradientEditor->setTransferFunction( *(m_3DView->getTransferFunction()) );
    m_editorByValues->setTransferFunction( *(m_3DView->getTransferFunction()) );
}

void QVolume3DViewTestingExtension::readSettings()
{
    QSettings settings;
    settings.beginGroup("Starviewer-App-3DTesting");
    settings.endGroup();
}

void QVolume3DViewTestingExtension::writeSettings()
{
    QSettings settings;
    settings.beginGroup("Starviewer-App-3DTesting");
    settings.endGroup();
}

void QVolume3DViewTestingExtension::computeOrCancelObscurance()
{
    this->setCursor( QCursor(Qt::WaitCursor) );

    if ( !m_computingObscurance )   // no s'estan calculant -> comencem
    {
        m_computingObscurance = true;

        enableObscuranceRendering( false );

        m_obscuranceComputeCancelPushButton->setText( tr("Cancel") );
        m_obscuranceProgressBar->setValue( 0 );

        switch ( m_obscuranceQualityComboBox->currentIndex() )
        {
            case 0: m_3DView->computeObscurance( Q3DViewer::Low ); break;
            case 1: m_3DView->computeObscurance( Q3DViewer::Medium ); break;
            case 2: m_3DView->computeObscurance( Q3DViewer::High ); break;
            default: ERROR_LOG( QString( "Valor inesperat per a la qualitat de les obscurances: %1" ).arg( m_obscuranceQualityComboBox->currentIndex() ) );
        }
    }
    else    // s'estan calculant -> aturem-ho
    {
        m_computingObscurance = false;

        m_obscuranceComputeCancelPushButton->setText( tr("Compute") );

        m_3DView->cancelObscurance();
    }

    this->setCursor( QCursor(Qt::ArrowCursor) );
}

void QVolume3DViewTestingExtension::autoCancelObscurance()
{
    Q_ASSERT( m_computingObscurance );

    this->setCursor( QCursor(Qt::WaitCursor) );
    m_computingObscurance = false;
    m_obscuranceComputeCancelPushButton->setText( tr("Compute") );
    this->setCursor( QCursor(Qt::ArrowCursor) );
}

void QVolume3DViewTestingExtension::endComputeObscurance()
{
    m_computingObscurance = false;
    m_obscuranceComputeCancelPushButton->setText( tr("Compute") );
    enableObscuranceRendering( true );
    m_obscuranceCheckBox->setChecked( true );   // les activem automàticament quan acaba el càlcul
}

void QVolume3DViewTestingExtension::enableObscuranceRendering( bool on )
{
    if ( !on ) m_obscuranceCheckBox->setChecked( false );

    m_obscuranceCheckBox->setEnabled( on );
    m_obscuranceCheckBox->setVisible( on );
    m_obscuranceFactorLabel->setVisible( on );
    m_obscuranceFactorDoubleSpinBox->setVisible( on );
    m_obscuranceProgressBar->setVisible( !on );
}

void QVolume3DViewTestingExtension::render()
{
    this->setCursor( QCursor(Qt::WaitCursor) );
    m_3DView->render();
    this->setCursor( QCursor(Qt::ArrowCursor) );
}

void QVolume3DViewTestingExtension::loadClut()
{
    QSettings settings;
    settings.beginGroup( "Starviewer-App-3DTesting" );
    QString customClutsDirPath = settings.value( "customClutsDir", QString() ).toString();

    QString transferFunctionFileName =
            QFileDialog::getOpenFileName( this, tr("Load CLUT"),
                                          customClutsDirPath, tr("Transfer function files (*.tf);;All files (*)") );

    if ( !transferFunctionFileName.isNull() )
    {
        TransferFunction * transferFunction = TransferFunctionIO::fromFile( transferFunctionFileName );
        QTransferFunctionEditor * currentEditor = qobject_cast<QTransferFunctionEditor*>( m_editorsStackedWidget->currentWidget() );
        currentEditor->setTransferFunction( *transferFunction );
        delete transferFunction;
        emit newTransferFunction();

        QFileInfo transferFunctionFileInfo( transferFunctionFileName );
        settings.setValue( "customClutsDir", transferFunctionFileInfo.absolutePath() );
    }

    settings.endGroup();
}


void QVolume3DViewTestingExtension::saveClut()
{
    QSettings settings;
    settings.beginGroup( "Starviewer-App-3DTesting" );
    QString customClutsDirPath = settings.value( "customClutsDir", QString() ).toString();

    QFileDialog saveDialog( this, tr("Save CLUT"), customClutsDirPath, tr("Transfer function files (*.tf);;All files (*)") );
    saveDialog.setAcceptMode( QFileDialog::AcceptSave );
    saveDialog.setDefaultSuffix( "tf" );

    if ( saveDialog.exec() == QDialog::Accepted )
    {
        QString transferFunctionFileName = saveDialog.selectedFiles().first();
        QTransferFunctionEditor * currentEditor = qobject_cast<QTransferFunctionEditor*>( m_editorsStackedWidget->currentWidget() );
        TransferFunctionIO::toFile( transferFunctionFileName, currentEditor->getTransferFunction() );

        QFileInfo transferFunctionFileInfo( transferFunctionFileName );
        settings.setValue( "customClutsDir", transferFunctionFileInfo.absolutePath() );
    }

    settings.endGroup();
}


void QVolume3DViewTestingExtension::switchEditor()
{
    QTransferFunctionEditor * currentEditor = qobject_cast<QTransferFunctionEditor*>( m_editorsStackedWidget->currentWidget() );
    const TransferFunction & currentTransferFunction = currentEditor->getTransferFunction();
    m_editorsStackedWidget->setCurrentIndex( 1 - m_editorsStackedWidget->currentIndex() );
    currentEditor = qobject_cast<QTransferFunctionEditor*>( m_editorsStackedWidget->currentWidget() );
    currentEditor->setTransferFunction( currentTransferFunction );
}


void QVolume3DViewTestingExtension::applyEditorClut()
{
    QTransferFunctionEditor *currentEditor = qobject_cast<QTransferFunctionEditor*>( m_editorsStackedWidget->currentWidget() );
    applyClut( currentEditor->getTransferFunction() );
    this->render();
}


void QVolume3DViewTestingExtension::toggleClutEditor()
{
    if ( m_editorSplitter->sizes()[1] == 0 )    // show
    {
        m_editorSplitter->setSizes( QList<int>() << 1 << 1 );
        m_gradientEditor->setTransferFunction( m_currentClut );
        m_editorByValues->setTransferFunction( m_currentClut );
    }
    else    // hide
        hideClutEditor();
}


void QVolume3DViewTestingExtension::hideClutEditor()
{
    m_editorSplitter->setSizes( QList<int>() << 0 << 0 );
}

void QVolume3DViewTestingExtension::setCustomStyleButtonStateBySplitter()
{
    m_customStyleToolButton->setChecked( m_editorSplitter->sizes()[1] != 0 );
}

void QVolume3DViewTestingExtension::applyRenderingStyle( const QModelIndex &index )
{
    disableAutoUpdate();

    QStandardItem *item = m_renderingStyleModel->itemFromIndex( index );
    RenderingStyle renderingStyle = RenderingStyle::fromVariant( item->data() );

    switch ( renderingStyle.method )
    {
        case RenderingStyle::RayCasting:
        case RenderingStyle::Texture3D:
        case RenderingStyle::Texture2D:
            if ( renderingStyle.method == RenderingStyle::RayCasting )
                m_renderingMethodComboBox->setCurrentIndex( renderingStyle.obscurance ? 1 : 0 );
            else if ( renderingStyle.method == RenderingStyle::Texture3D )
                m_renderingMethodComboBox->setCurrentIndex( 3 );
            else
                m_renderingMethodComboBox->setCurrentIndex( 4 );

            applyClut( renderingStyle.transferFunction );
            m_shadingCheckBox->setChecked( renderingStyle.diffuseLighting );

            if ( renderingStyle.diffuseLighting )
            {
                m_specularCheckBox->setChecked( renderingStyle.specularLighting );
                if ( renderingStyle.specularLighting ) m_specularPowerDoubleSpinBox->setValue( renderingStyle.specularPower );
            }

            if ( renderingStyle.method == RenderingStyle::RayCasting )
            {
                m_contourCheckBox->setChecked( renderingStyle.contour );
                if ( renderingStyle.contour ) m_contourThresholdDoubleSpinBox->setValue( renderingStyle.contourThreshold );

                if ( renderingStyle.obscurance )
                {
                    if ( !m_computingObscurance )   // si s'estan calculant no toquem res
                    {
                        /// \todo fer una comprovació més ben feta (amb un booleà)
                        if ( !m_obscuranceCheckBox->isVisible() ) // si no hi ha obscurances calcular-les
                        {
                            m_obscuranceQualityComboBox->setCurrentIndex( static_cast<int>( renderingStyle.obscuranceQuality ) );
                            m_obscuranceComputeCancelPushButton->click();
                        }
                        else // altrament aplicar-les i ja està
                            m_obscuranceCheckBox->setChecked( true );
                    }

                    m_obscuranceFactorDoubleSpinBox->setValue( renderingStyle.obscuranceFactor );
                }
            }

            break;

        case RenderingStyle::MIP:
            m_renderingMethodComboBox->setCurrentIndex( 2 );
            break;

        case RenderingStyle::IsoSurface:
            m_renderingMethodComboBox->setCurrentIndex( 5 );
            m_isoValueSpinBox->setValue( static_cast<int>( qRound( renderingStyle.isoValue ) ) );
            break;

        case RenderingStyle::Contouring:
            m_renderingMethodComboBox->setCurrentIndex( 6 );
            break;

        default:
            enableAutoUpdate();
            return;
    }

    updateView( false );
    enableAutoUpdate();
}

void QVolume3DViewTestingExtension::updateUiForRenderingMethod( int index )
{
    m_shadingOptionsWidget->hide();
    m_contourOptionsWidget->hide();
    m_obscuranceOptionsWidget->hide();
    m_isosurfaceOptionsWidget->hide();

    switch( index )
    {
        case 0: // ray casting
            m_shadingOptionsWidget->show();
            m_contourOptionsWidget->show();
            break;

        case 1: // ray casting + obscurances
            m_shadingOptionsWidget->show();
            m_contourOptionsWidget->show();
            m_obscuranceOptionsWidget->show();
            break;

        case 2: // mip
            break;

        case 3: // textures 3d
            m_shadingOptionsWidget->show();
            break;

        case 4: // textures 2d
            m_shadingOptionsWidget->show();
            break;

        case 5: // isosuperfícies
            m_isosurfaceOptionsWidget->show();
            break;

        case 6: // contouring
            break;
    }
}


void QVolume3DViewTestingExtension::updateView( bool fast )
{
    m_timer->stop();

    this->setCursor( QCursor(Qt::WaitCursor) );

    switch( m_renderingMethodComboBox->currentIndex() )
    {
        case 0:
            m_3DView->setRenderFunctionToRayCasting();
            break;

        case 1:
            m_3DView->setRenderFunctionToRayCastingObscurance();
            break;

        case 2:
            m_3DView->setRenderFunctionToMIP3D();
            break;

        case 3:
            m_3DView->setRenderFunctionToTexture3D();
            break;

        case 4:
            m_3DView->setRenderFunctionToTexture2D();
            break;

        case 5:
            m_3DView->setIsoValue( m_isoValueSpinBox->value() );    // necessari per la primera vegada
            m_3DView->setRenderFunctionToIsoSurface();
            break;

        case 6:
            m_3DView->setRenderFunctionToContouring();
            break;
    }

    m_3DView->setShading( m_shadingCheckBox->isChecked() );
    m_3DView->setSpecular( m_specularCheckBox->isChecked() );
    m_3DView->setSpecularPower( m_specularPowerDoubleSpinBox->value() );
    m_3DView->setContour( m_contourCheckBox->isChecked() );
    m_3DView->setContourThreshold( m_contourThresholdDoubleSpinBox->value() );
    m_3DView->setObscurance( m_obscuranceCheckBox->isChecked() );
    m_3DView->setObscuranceFactor( m_obscuranceFactorDoubleSpinBox->value() );
    m_3DView->setIsoValue( m_isoValueSpinBox->value() );

    if ( fast )
    {
        m_timer->start();   /// \todo Ara ho fem abans del render. Caldria comprovar en sistemes lents si és millor abans o després.
        m_3DView->getRenderWindow()->SetDesiredUpdateRate( m_3DView->getInteractor()->GetDesiredUpdateRate() );
    }

    m_3DView->render();

    if ( fast ) m_3DView->getRenderWindow()->SetDesiredUpdateRate( m_3DView->getInteractor()->GetStillUpdateRate() );

    this->setCursor( QCursor(Qt::ArrowCursor) );
}


void QVolume3DViewTestingExtension::enableAutoUpdate()
{
    // actualització del mètode de rendering
    connect( m_renderingMethodComboBox, SIGNAL( currentIndexChanged(int) ), this, SLOT( updateView() ) );

    // shading
    connect( m_shadingCheckBox, SIGNAL( toggled(bool) ), this, SLOT( updateView() ) );
    connect( m_specularCheckBox, SIGNAL( toggled(bool) ), this, SLOT( updateView() ) );
    connect( m_specularPowerDoubleSpinBox, SIGNAL( valueChanged(double) ), this, SLOT( updateView() ) );

    // contorns
    connect( m_contourCheckBox, SIGNAL( toggled(bool) ), this, SLOT( updateView() ) );
    connect( m_contourThresholdDoubleSpinBox, SIGNAL( valueChanged(double) ), this, SLOT( updateView() ) );

    // obscurances
    connect( m_obscuranceCheckBox, SIGNAL( toggled(bool) ), this, SLOT( updateView() ) );
    connect( m_obscuranceFactorDoubleSpinBox, SIGNAL( valueChanged(double) ), this, SLOT( updateView() ) );

    // isosuperfícies
    connect( m_isoValueSpinBox, SIGNAL( valueChanged(int) ), this, SLOT( updateView() ) );
}


void QVolume3DViewTestingExtension::disableAutoUpdate()
{
    // actualització del mètode de rendering
    disconnect( m_renderingMethodComboBox, SIGNAL( currentIndexChanged(int) ), this, SLOT( updateView() ) );

    // shading
    disconnect( m_shadingCheckBox, SIGNAL( toggled(bool) ), this, SLOT( updateView() ) );
    disconnect( m_specularCheckBox, SIGNAL( toggled(bool) ), this, SLOT( updateView() ) );
    disconnect( m_specularPowerDoubleSpinBox, SIGNAL( valueChanged(double) ), this, SLOT( updateView() ) );

    // contorns
    disconnect( m_contourCheckBox, SIGNAL( toggled(bool) ), this, SLOT( updateView() ) );
    disconnect( m_contourThresholdDoubleSpinBox, SIGNAL( valueChanged(double) ), this, SLOT( updateView() ) );

    // obscurances
    disconnect( m_obscuranceCheckBox, SIGNAL( toggled(bool) ), this, SLOT( updateView() ) );
    disconnect( m_obscuranceFactorDoubleSpinBox, SIGNAL( valueChanged(double) ), this, SLOT( updateView() ) );

    // isosuperfícies
    disconnect( m_isoValueSpinBox, SIGNAL( valueChanged(int) ), this, SLOT( updateView() ) );
}


}
