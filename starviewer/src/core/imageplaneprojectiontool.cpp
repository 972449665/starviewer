#include "imageplaneprojectiontool.h"
#include "imageplaneprojectiontooldata.h"
#include "q2dviewer.h"
#include "imageplane.h"
#include "drawerline.h"
#include "volume.h"
#include "logging.h"
#include "series.h"
#include "image.h"
#include "drawer.h"
#include "mathtools.h" // per c�lculs d'interseccions
#include "tooldata.h"
#include "toolconfiguration.h"
// Vtk's
#include <vtkCommand.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkTransform.h>
#include <vtkImageReslice.h>
// Qt's
#include <QStringList>
#include <QColor>

namespace udg {

ImagePlaneProjectionTool::ImagePlaneProjectionTool( QViewer *viewer, QObject *parent )
:Tool(viewer,parent), m_reslice(0)
{
    m_toolName = "ImagePlaneProjectionTool";
    m_hasSharedData = true;

    m_2DViewer = qobject_cast<Q2DViewer *>( viewer );
    if( !m_2DViewer )
        DEBUG_LOG(QString("El casting no ha funcionat!!! �s possible que viewer no sigui un Q2DViewer!!!-> ")+ viewer->metaObject()->className() );

    m_myData = new ImagePlaneProjectionToolData;
    m_toolData = m_myData;

    m_state = NONE;

    m_volume = NULL;

    m_enabled = false;

    m_thickness = 1;
}

ImagePlaneProjectionTool::~ImagePlaneProjectionTool()
{
    if ( m_reslice )
    {
        m_reslice->Delete();
    }
}

void ImagePlaneProjectionTool::setEnabled( bool enabled )
{
    if ( !m_toolConfiguration )
    {
        DEBUG_LOG(QString("ImagePlaneProjectionTool: �s obligatori associar a la tool una configuraci� correcta.") );
        return;
    }

    QString typeConfiguration = m_toolConfiguration->getValue( "typeConfiguration" ).toString();

    if ( !m_enabled && enabled )
    {
        // Tornem a activar els signals i slots necess�ries segons el tipus de configuraci� de la tool
        enableConnections();
    }
    else if ( m_enabled && !enabled )
    {
        // Desactivem els signals i slots creats segons el tipus de configuraci� de la tool
        disableConnections();
    }

    m_enabled = enabled;
}

void ImagePlaneProjectionTool::enableConnections()
{
    if ( !m_toolConfiguration )
    {
        DEBUG_LOG(QString("ImagePlaneProjectionTool: �s obligatori associar a la tool una configuraci� correcta.") );
        return;
    }

    QString typeConfiguration = m_toolConfiguration->getValue( "typeConfiguration" ).toString();
    if ( typeConfiguration == QString( "PRODUCER" ) )
    {
        // Cada cop que el viewer canv�i d'input s'ha d'inicialitzar el pla projectat per cada l�nia de projecci� de la tool 
        // i el volum compartit amb la resta de visors amb la tool activa
        connect( m_2DViewer, SIGNAL( volumeChanged( Volume * ) ), SLOT( initializeImagePlanesUpdteVolume( Volume * ) ) );

        // Cada cop que el viewer canv�i l'imatge o l'slab s'ha d'actualitzar el pla projectat per cada l�nia de projecci� de la tool
        connect( m_2DViewer, SIGNAL( sliceChanged( int ) ), SLOT( updateProjections()) );
        connect( m_2DViewer, SIGNAL( slabThicknessChanged( int ) ), SLOT( updateProjections()) );
    }

    if ( typeConfiguration == QString( "PRODUCER" ) || typeConfiguration == QString( "PRODUCER&CONSUMER" ) )
    {
        // Quan es modifica el pla projectar per una l�nia d'un viewer amb la tool configurada com a productor
        // cal comprovar si algun altre visor productor t� alguna l�nia amb la mateixa orientaci� que cal actualitzar
        // ( X visors diferents poden haver de mostrar l�nies amb el mateix pla de projecci� )
        connect( m_myData, SIGNAL( imagePlaneUpdated( QString ) ), SLOT( checkImagePlaneBindUpdated( QString ) ) );

        // Quan es modifica el n�mero d'imatges a mostrar al volum de reconstrucci� d'un visor consumidor, cal mostrar
        // el gruix agafat en la direcci� del pla de projecci� indicada per la corresponent l�nia
        connect( m_myData, SIGNAL( thicknessUpdated( QString ) ), SLOT( checkThicknessProjectedLineUpdated( QString ) ) );
    }

    if ( typeConfiguration == QString( "PRODUCER&CONSUMER" ) )
    {
        // Quan es modifica el volum de treball a les dades compartides els visors amb la tool configurada com a 
        // productor i consumidor inicialitzen el pla projectat per cada l�nia de projecci� de la tool comprovant
        // primer si ja existeix a les dades un pla per la orientaci� corresponent.
        connect( m_myData, SIGNAL( dataInitialized() ), SLOT( initializeImagePlanesCheckingData() ) );
    }
    
    if ( typeConfiguration == QString( "CONSUMER" ) || typeConfiguration == QString( "PRODUCER&CONSUMER" ) )
    {
        // Quan es modifica el pla projectat per una l�nia d'un viewer amb la tool configurada com a productor
        // cal comprovar si es tracta de la l�nia associada a l'actual viewer i si �s aix� mostrar el nou pla
        // (aquesta l�nia de projecci� est� mostrada a un viewer amb la tool configurada com a productor)
        connect( m_myData, SIGNAL( imagePlaneUpdated( QString ) ), SLOT( checkProjectedLineBindUpdated( QString ) ) );
    }
}

void ImagePlaneProjectionTool::disableConnections()
{
    if ( !m_toolConfiguration )
    {
        DEBUG_LOG(QString("ImagePlaneProjectionTool: �s obligatori associar a la tool una configuraci� correcta.") );
        return;
    }

    QString typeConfiguration = m_toolConfiguration->getValue( "typeConfiguration" ).toString();
    if ( typeConfiguration == QString( "PRODUCER" ) )
    {
        disconnect( m_2DViewer, SIGNAL( volumeChanged( Volume * ) ), this, SLOT( initializeImagePlanesUpdteVolume( Volume * ) ) );
        disconnect( m_2DViewer, SIGNAL( sliceChanged( int ) ), this, SLOT( updateProjections()) );
        disconnect( m_2DViewer, SIGNAL( slabThicknessChanged( int ) ), this, SLOT( updateProjections()) );
    }

    if ( typeConfiguration == QString( "PRODUCER" ) || typeConfiguration == QString( "PRODUCER&CONSUMER" ) )
    {
        disconnect( m_myData, SIGNAL( imagePlaneUpdated( QString ) ), this, SLOT( checkImagePlaneBindUpdated( QString ) ) );
        disconnect( m_myData, SIGNAL( thicknessUpdated( QString ) ), this, SLOT( checkThicknessProjectedLineUpdated( QString ) ) );
    }

    if ( typeConfiguration == QString( "PRODUCER&CONSUMER" ) )
    {
        disconnect( m_myData, SIGNAL( dataInitialized() ), this, SLOT( initializeImagePlanesCheckingData() ) );
    }
    
    if ( typeConfiguration == QString( "CONSUMER" ) || typeConfiguration == QString( "PRODUCER&CONSUMER" ) )
    {
        disconnect( m_myData, SIGNAL( imagePlaneUpdated( QString ) ), this, SLOT( checkProjectedLineBindUpdated( QString ) ) );
    }
}

void ImagePlaneProjectionTool::setConfiguration( ToolConfiguration *configuration )
{
    checkValuesConfiguration( configuration );
    m_toolConfiguration = configuration;
    applyConfiguration();
    m_enabled = true;
}

void ImagePlaneProjectionTool::checkValuesConfiguration( ToolConfiguration *configuration )
{
    if ( !( configuration->containsValue( "typeConfiguration" ) ) )
    {
        DEBUG_LOG(QString("ImagePlaneProjectionTool: No s'ha indicat el tipus de configuraci� a aplicar al viewer.") );
        return;
    }

    //Comprovem que s'hagin assignat els par�metres corresponents al tipus de configuraci� assignada
    QString typeConfiguration = configuration->getValue( "typeConfiguration" ).toString();
    if ( typeConfiguration == QString( "PRODUCER&CONSUMER" ) )
    {
        checkConfigurationProducer( configuration );
        checkConfigurationConsumer( configuration );
    }
    else if (  typeConfiguration == QString( "PRODUCER" ) )
    {
        checkConfigurationProducer( configuration );
    }
    else if ( typeConfiguration == QString( "CONSUMER" ) )
    {
        checkConfigurationConsumer( configuration );
    }
    else
    {
        DEBUG_LOG(QString("ImagePlaneProjectionTool: Tipus de configuraci� indicada incorrecte. Els valors possibles s�n: PRODUCER o CONSUMER.") );
        return;
    }
}

void ImagePlaneProjectionTool::checkConfigurationProducer( ToolConfiguration *configuration )
{
    int numProjectedLines = configuration->getValue( "numProjectedLines" ).toInt();
    QStringList namesProjectedLines = configuration->getValue( "namesProjectedLines" ).toStringList();
    QStringList initOrientationsProjectedLines = configuration->getValue( "initOrientationsProjectedLines" ).toStringList();
    QList< QVariant > colorsProjectedLines = configuration->getValue( "colorsProjectedLines" ).toList();

    if ( numProjectedLines != namesProjectedLines.length() 
          || numProjectedLines != initOrientationsProjectedLines.length() 
          || numProjectedLines != colorsProjectedLines.length() )
    {
        DEBUG_LOG( QString("ImagePlaneProjectionTool: No s'ha indicat l'identificaci�, la orientaci� i el color de totes les l�nies a projectar.") );
        return;
    }

    foreach ( QString orientation, initOrientationsProjectedLines ) 
    {
        if ( orientation != QString("HORIZONTAL") && orientation != QString("VERTICAL") )
        {
            DEBUG_LOG( QString("ImagePlaneProjectionTool: No s'ha indicat un identificador d'orientaci� correcte.") );
            return;
        }
    }

    for ( int i = 0; i < colorsProjectedLines.length(); i++ ) 
    {
        if( !colorsProjectedLines.at( i ).canConvert<QColor>() )
            DEBUG_LOG( QString("ImagePlaneProjectionTool: No s'ha indicat un color correcte.") );
    }
}

void ImagePlaneProjectionTool::checkConfigurationConsumer( ToolConfiguration *configuration )
{
    if ( !( configuration->containsValue( "nameProjectedLine" ) ) )
    {
        DEBUG_LOG( QString("ImagePlaneProjectionTool: No s'ha indicat el nom de la l�nia de projecci� associada a la tool amb configuraci� consumidor.") );
        return;
    }
}

void ImagePlaneProjectionTool::applyConfiguration()
{
    if ( !m_toolConfiguration )
    {
        DEBUG_LOG(QString("ImagePlaneProjectionTool: �s obligatori associar a la tool una configuraci� correcta.") );
        return;
    }

    // Activem totes les connexions (signals-slots) de la tool segons la seva configuraci�
    enableConnections();

    QString typeConfiguration = m_toolConfiguration->getValue( "typeConfiguration" ).toString();
    if ( typeConfiguration == QString( "PRODUCER" ) || typeConfiguration == QString( "PRODUCER&CONSUMER" ) )
    {
        // Inicialitzem les l�nies a projectar per la tool
        initProjectedLines();
    }

    if ( typeConfiguration == QString( "CONSUMER" ) || typeConfiguration == QString( "PRODUCER&CONSUMER" ) )
    {
        // Inicialitzem el nom de la l�nia de projecci� associada a la tool
        m_nameProjectedLineBind = m_toolConfiguration->getValue( "nameProjectedLine" ).toString();
        
        // Inicialitzem el reslice del viewer
        m_reslice = vtkImageReslice::New();
        m_reslice->AutoCropOutputOn();
        m_reslice->SetInterpolationModeToCubic();
    }
}

void ImagePlaneProjectionTool::initProjectedLines()
{
    QStringList names = m_toolConfiguration->getValue( "namesProjectedLines" ).toStringList();
    QStringList orientations = m_toolConfiguration->getValue( "initOrientationsProjectedLines" ).toStringList();
    QList< QVariant> colorsProjectedLines = m_toolConfiguration->getValue( "colorsProjectedLines" ).toList();

    // S'han de netejar les lines mostrades i tornar-les a inicialitzar.
    m_2DViewer->clearViewer();
    
    // Inicialitzem les diferents l�nies de projecci� visibles al viewer
    for ( int i = 0; i < names.length(); i++)
    {
        QString name = names.at( i );
        QString orientation = orientations.at( i );
        DrawerLine *projectedLine = new DrawerLine();
        projectedLine->setColor( colorsProjectedLines.at(i).value<QColor>() );
        projectedLine->setLineWidth( 3 );
        m_projectedLines.insert( projectedLine, ( QStringList () << name << orientation ) );
        m_imagePlanes.insert( name, new ImagePlane() );
    }
}

void ImagePlaneProjectionTool::initializeImagePlanesUpdteVolume( Volume *volume )
{
    m_volume = volume;

    // Guardem a les dades compartides el nou volum de treball
    m_myData->setVolume( volume );

    // S'han de netejar les lines mostrades i tornar-les a inicialitzar.
    m_2DViewer->clearViewer();
    
    // Inicialitzem l'imagePlane representat per cada linia de projecci�, la dibuixem 
    // i actualitzem les dades compartides indicant el pla projectat
    QMapIterator< DrawerLine *, QStringList > iterator( m_projectedLines );
    while (iterator.hasNext()) 
    {
        iterator.next();
        initializeImagePlane( iterator.key(), m_myData->getVolume() );
    }

    // Indiquem que s'han inicialitzat les dades perqu� si existeix algun visor productor&consumidor pugui
    // actualitzar les dades mostrades
    m_myData->emitDataInitialized();
}

void ImagePlaneProjectionTool::initializeImagePlanesCheckingData()
{
    // S'han de netejar les lines mostrades i tornar-les a inicialitzar.
    m_2DViewer->clearViewer();

    QMapIterator< DrawerLine *, QStringList > iterator( m_projectedLines );
    while ( iterator.hasNext() ) 
    {
        iterator.next();
        DrawerLine *projectedLine = iterator.key();
        QString name = iterator.value().at(0);
        ImagePlane *imagePlane;
        imagePlane = m_myData->getProjectedLineImagePlane( name );
        if ( imagePlane != NULL )
        {
            updateProjection( projectedLine, imagePlane, false, false );
        }
        else
        {
            initializeImagePlane( projectedLine, m_myData->getVolume() );
        }
    }
}

void ImagePlaneProjectionTool::initializeImagePlane( DrawerLine *projectedLine, Volume *volume )
{
    // Inicialitzem el pla que ha de projectar la tool sobre el Viewer configurat com a productor
    
    if ( volume != NULL )
    {
        QStringList infoProjectedLine = m_projectedLines[ projectedLine ];
        
        // Pla que ha de projectar la tool sobre el viewer 
        ImagePlane *imagePlane = new ImagePlane();

        // Imatges que composen el volum de dades 
        QList<Image*> images;
        images = volume->getImages();

        // L�mits i orientaci� de la primera imatge del volum 
        Image *firtImage;
        firtImage = images.at(0);

        ImagePlane *firstImagePlane = new ImagePlane();
        firstImagePlane->fillFromImage(firtImage);

        QList< QVector<double> > boundsFirstImagePlane;
        boundsFirstImagePlane = firstImagePlane->getCentralBounds();
        QVector<double> tlhcFirstImagePlane = boundsFirstImagePlane.at(0);
        QVector<double> trhcFirstImagePlane = boundsFirstImagePlane.at(1);
        QVector<double> brhcFirstImagePlane = boundsFirstImagePlane.at(2);
        QVector<double> blhcFirstImagePlane = boundsFirstImagePlane.at(3);

        double rowDirectionVector[3];
        firstImagePlane->getRowDirectionVector(rowDirectionVector);
        int rows = firstImagePlane->getRows();
        double rowLength = firstImagePlane->getRowLength();

        double columnDirectionVector[3];
        firstImagePlane->getColumnDirectionVector(columnDirectionVector);
        int columns = firstImagePlane->getColumns();
        double columnLength = firstImagePlane->getColumnLength();

        // L'espaiat cal obtenir-lo del volum perqu� no disposem de la dist�ncia entre imatges d'una altra manera
        double spacing[3];
        volume->getSpacing(spacing);

        // Informaci� sobre la mida i direcci� de la pila d'imatges
        int zDepth;
        zDepth = images.size();
        double zDirectionVector[3];
        volume->getStackDirection(zDirectionVector);
        
        // L�mits de l'�ltima imatge del volum 
        Image *lastImage;
        lastImage = images.at(zDepth-1);

        ImagePlane *lastImagePlane = new ImagePlane();
        lastImagePlane->fillFromImage(lastImage);

        QList< QVector<double> > boundsLastImagePlane;
        boundsLastImagePlane = lastImagePlane->getCentralBounds();
        QVector<double> tlhcLastImagePlane = boundsLastImagePlane.at(0);
        QVector<double> trhcLasttImagePlane = boundsLastImagePlane.at(1);
        QVector<double> brhcLastImagePlane = boundsLastImagePlane.at(2);
        QVector<double> blhcLastImagePlane = boundsLastImagePlane.at(3);

        double maxXBound, maxYBound, maxZBound;
        double imageOrigin[3];

        // Ajust de la mida del pla a les dimensions de la corresponent orientaci�
        QString orientation = infoProjectedLine.at( 1 );
        if ( orientation == QString("VERTICAL") )
        {
            //YZ, x-normal
            // No cal sumar spacings perqu� ja ho fa a dins l'imagePlane
            maxYBound = sqrt( ( ( double ) zDepth*zDepth + rows*rows ) );
            maxZBound = sqrt( ( ( double ) zDepth*zDepth + rows*rows ) );
            
            imageOrigin[0] = tlhcLastImagePlane[0] + rowDirectionVector[0] * rowLength / 2;
            imageOrigin[1] = tlhcLastImagePlane[1] + rowDirectionVector[1] * rowLength / 2;
            imageOrigin[2] = tlhcLastImagePlane[2] + rowDirectionVector[2] * rowLength / 2;
            
            imagePlane->setOrigin( imageOrigin );
            imagePlane->setRows( zDepth );
            imagePlane->setColumns( rows );
            imagePlane->setSpacing( spacing[0], spacing[2] );
            imagePlane->setThickness( spacing[1] );
            imagePlane->setRowDirectionVector( columnDirectionVector );
            imagePlane->setColumnDirectionVector( -zDirectionVector[0], -zDirectionVector[1], -zDirectionVector[2] );
        }
        else if ( orientation == QString("HORIZONTAL") )
        {
            //XZ, y-normal
            // No cal sumar spacings perqu� ja ho fa a dins l'imagePlane
            maxZBound = sqrt( ( ( double ) columns*columns + zDepth*zDepth ) );
            maxXBound = sqrt( ( ( double ) columns*columns + zDepth*zDepth ) );

            imageOrigin[0] = tlhcLastImagePlane[0] + columnDirectionVector[0] * columnLength / 2;
            imageOrigin[1] = tlhcLastImagePlane[1] + columnDirectionVector[1] * columnLength / 2;
            imageOrigin[2] = tlhcLastImagePlane[2] + columnDirectionVector[2] * columnLength / 2;
            
            imagePlane->setOrigin( imageOrigin );
            imagePlane->setRows( zDepth );
            imagePlane->setColumns( columns );
            imagePlane->setSpacing( spacing[1], spacing[2] );
            imagePlane->setThickness( spacing[0] );
            imagePlane->setRowDirectionVector( rowDirectionVector );
            imagePlane->setColumnDirectionVector( -zDirectionVector[0], -zDirectionVector[1], -zDirectionVector[2] );
        }
        
        // Quan es modifica l'ImagePlane de la tool s'actualitza la projecci� de la l�nia corresponent
        updateProjection( projectedLine, imagePlane, false, true );
    }
}

void ImagePlaneProjectionTool::updateProjections()
{
    // Cada cop que al viewer canvia d'input, l'imatge o l'slab s'ha d'obtenir el pla representat per 
    // cada l�nia projectada i actualitzar la projeccci�. Tamb� s'actualitza el canvi de pla a les dades compartides
    QMapIterator< DrawerLine *, QStringList > iterator( m_projectedLines );
    while (iterator.hasNext()) 
    {
        iterator.next();
        DrawerLine *projectedLine = iterator.key();
        QStringList infoProjectedLine = iterator.value();
        ImagePlane *imagePlane = m_myData->getProjectedLineImagePlane( infoProjectedLine.at( 0 ) );
        updateProjection( projectedLine, imagePlane );
    }
}

void ImagePlaneProjectionTool::updateProjection( DrawerLine *projectedLine, ImagePlane *imagePlane )
{
    updateProjection( projectedLine, imagePlane, true, true );
}

void ImagePlaneProjectionTool::updateProjection( DrawerLine *projectedLine, ImagePlane *imagePlane, bool projectedLineDrawed, bool updateToolData )
{
    ImagePlane *localizerPlane;

    QString typeConfiguration = m_toolConfiguration->getValue( "typeConfiguration" ).toString();
    if ( typeConfiguration == QString("PRODUCER") )
    {
        localizerPlane = m_2DViewer->getCurrentImagePlane();
    }
    else if ( typeConfiguration == QString("PRODUCER&CONSUMER") )
    {
        localizerPlane = m_myData->getProjectedLineImagePlane( m_nameProjectedLineBind );
    }

    if( !(imagePlane && localizerPlane) )
        return;

    // primer mirem que siguin plans diferents
    if( *imagePlane != *localizerPlane )
    {
        double firstIntersectionPoint[3], secondIntersectionPoint[3];

        // Calculem totes les possibles interseccions
        int numberOfIntersections = imagePlane->getIntersections( localizerPlane, firstIntersectionPoint, secondIntersectionPoint );

        if( numberOfIntersections > 0 )
        {
            // Passem els punts a coordenades de m�n
            m_2DViewer->projectDICOMPointToCurrentDisplayedImage( firstIntersectionPoint, firstIntersectionPoint );
            m_2DViewer->projectDICOMPointToCurrentDisplayedImage( secondIntersectionPoint, secondIntersectionPoint );

            // Actualitzem la projecci� del pla (pintem la l�nia)
            // La l�nia projectada indica la intersecci� entre el pla projectat per la l�nia i el pla actual del Viewer
            // ( Nom�s cal tenir en compte eixos x i y de la l�nia projectada que s�n els que es veuen per pantalla
            //  aix� despr�s podrem detectar si l'usuari vol moure la l�nia, altrament no podr�em perqu� la dist�ncia sortiria molt gran )
            if ( typeConfiguration == QString("PRODUCER") )
            {
                firstIntersectionPoint[2] = 0;
                secondIntersectionPoint[2] = 0;
            }
            else
            {
                double firstPoint[3];
                firstPoint[0] = firstIntersectionPoint[0];
                firstPoint[1] = firstIntersectionPoint[1];
                firstPoint[2] = firstIntersectionPoint[2];

                double secondPoint[3];
                secondPoint[0] = secondIntersectionPoint[0];
                secondPoint[1] = secondIntersectionPoint[1];
                secondPoint[2] = secondIntersectionPoint[2];

                double origin[3];
                localizerPlane->getOrigin( origin );

                // Cal agafar les coordenades x i y en funci� de la orientaci� de la l�nia projectada
                // i restal-li l'origen perqu� en el reslice l'origen �s el [0,0,0]
                if ( m_nameProjectedLineBind == "VERTICAL_LINE" )
                {
                    //YZ, x-normal
                    firstIntersectionPoint[0] = abs( firstPoint[1] - origin[1] );
                    firstIntersectionPoint[1] = abs( firstPoint[2] - origin[2] );
                    firstIntersectionPoint[2] = 0;

                    secondIntersectionPoint[0] = abs( secondPoint[1] - origin[1] );
                    secondIntersectionPoint[1] = abs( secondPoint[2] - origin[2] ); 
                    secondIntersectionPoint[2] = 0;
                }
                else if ( m_nameProjectedLineBind == "HORIZONTAL_LINE" )
                {
                    //XZ, y-normal
                    firstIntersectionPoint[0] = abs( firstPoint[0] - origin[0] );
                    firstIntersectionPoint[1] = abs( firstPoint[2] - origin[2] );
                    firstIntersectionPoint[2] = 0;

                    secondIntersectionPoint[0] = abs( secondPoint[0] - origin[0] );
                    secondIntersectionPoint[1] = abs( secondPoint[2] - origin[2] );
                    secondIntersectionPoint[2] = 0;
                }
            }
            
            projectedLine->setFirstPoint( firstIntersectionPoint );
            projectedLine->setSecondPoint( secondIntersectionPoint );
            if ( !projectedLine->isVisible() )
            {
                projectedLine->visibilityOn();
            }

            if ( !projectedLineDrawed )
                m_2DViewer->getDrawer()->draw( projectedLine, Q2DViewer::Top2DPlane );
            else
                projectedLine->update();

            QStringList infoProjectedLine = m_projectedLines[ projectedLine ];
            QString name = infoProjectedLine.at( 0 );

            ImagePlane *copiaImagePlane = new ImagePlane( imagePlane );

            // Actualitzem l'imagePlane projectat per la l�nia a l'objecte actual
            m_imagePlanes.insert( name, copiaImagePlane );
                     
            applyThicknessProjectedLine( name, projectedLine );

            if ( updateToolData )
            {
                // Actualitzem l'imagePlane projectat per la l�nia a les dades compartides de la tool
                m_myData->setProjectedLineImagePlane( name, imagePlane );
            }
        }
        else
        {
            if( projectedLine->isVisible() )
            {
                projectedLine->visibilityOff();
                projectedLine->update();
            }
        }
    }
    else
    {
        if( projectedLine->isVisible() )
        {
            projectedLine->visibilityOff();
            projectedLine->update();
        }
    }

    m_2DViewer->render();
}

void ImagePlaneProjectionTool:: checkImagePlaneBindUpdated( QString nameProjectedLine )
{
    if ( m_imagePlanes.contains( nameProjectedLine ) )
    {
        ImagePlane *myImagePlane = m_imagePlanes.value( nameProjectedLine );
        ImagePlane *myDataImagePlane = m_myData->getProjectedLineImagePlane( nameProjectedLine );

        if ( myImagePlane != myDataImagePlane )
        {
            QMapIterator< DrawerLine *, QStringList > iterator( m_projectedLines );
            bool searched = false;
            DrawerLine *projectedLine;
            while ( iterator.hasNext() && !searched ) 
            {
                iterator.next();
                projectedLine = iterator.key();
                QString name = iterator.value().at(0);
                searched = ( nameProjectedLine == name );
            }

            if ( searched ) updateProjection( projectedLine, myDataImagePlane, true, false );
        }
    }
}

void ImagePlaneProjectionTool::checkProjectedLineBindUpdated( QString nameProjectedLine )
{
    if (m_nameProjectedLineBind == nameProjectedLine )
    {
        showImagePlaneProjectedLineBind();
    }
}

void ImagePlaneProjectionTool::showImagePlaneProjectedLineBind()
{
    // Han modificat el pla projectat per la l�nia associada a l'actual viewer amb la tool configurada com a consumidor
    Volume *volume = m_myData->getVolume();

    if ( volume != NULL )
    {
        if ( m_volume != volume)
        {
            // S'ha modificat el volum amb el que s'ha de treballar als visors que tenen activada la tool,
            // per tant cal inicialitzar el reslice
            m_volume = volume;
            initReslice( volume );
        }
        else
        {
            updateReslice( volume );
        }
    }
}

void ImagePlaneProjectionTool::initReslice( Volume *volume )
{
    m_reslice->SetInput( volume->getVtkData() );

    updateReslice( volume );
}

void ImagePlaneProjectionTool::updateReslice( Volume *volume )
{
    if ( volume == NULL || !m_reslice ||  !( vtkImageData::SafeDownCast( m_reslice->GetInput() ) ) )
    {
        return;
    }

    // Baixem la resoluci�
    m_reslice->SetInterpolationModeToNearestNeighbor();

    ImagePlane *imagePlane = m_myData->getProjectedLineImagePlane( m_nameProjectedLineBind );

    // Obtenim els vectors directors dels tres eixos de l'image plane que volem mostrar
    double xDirectionVector[3], yDirectionVector[3], zDirectionVector[3];
    imagePlane->getRowDirectionVector( xDirectionVector );
    imagePlane->getColumnDirectionVector( yDirectionVector );
    imagePlane->getNormalVector( zDirectionVector );
    
    // Assignem a la matriu ResliceAxes els eixos de la llesca de sortida
    m_reslice->SetResliceAxesDirectionCosines( xDirectionVector, yDirectionVector, zDirectionVector );

    double origin[3];
    imagePlane->getOrigin(origin);

    // Especifiquem quin ha de ser el punt d'origen de la matriu ResliceAxes
    m_reslice->SetResliceAxesOrigin( origin );

    double spacing[2];
    imagePlane->getSpacing(spacing);

    // Indiquem la dist�ncia entre les llesques de sortida que es podran anar obtenint.
    m_reslice->SetOutputSpacing( spacing[0] , spacing[1] , 1 ); // thickness = 1

    // Indiquem quin ha de ser considerat l'origen de les llesques de sortida
    //m_reslice->SetOutputOrigin( origin[0] , origin[1] , origin[2] );
    m_reslice->SetOutputOrigin( 0 , 0 , 0 );

    double rows = imagePlane->getRows();
    double columns = imagePlane->getColumns();

    // L�mits de les llesques de sortida
    m_reslice->SetOutputExtent( 0 , columns - 1 , 0 , rows - 1 , 0 , m_thickness ); // assignem el gruix indicat per l'usuari

    // Fem efectius els canvis fets anteriorment sobre el vtkImageReslace
    m_reslice->Update();
    
    // Tornem ha augmentar la resoluci�
    m_reslice->SetInterpolationModeToCubic();

    // !!!Les seg�ents quatre instruccions estaven al final del m�tode initReslice,
    // !!!per� al modificar el thickness del reslice no s'actualitzava al visor
    // Assignem la informaci� de la s�rie, estudis, pacient... 
    Volume *reslicedVolume = new Volume;
    reslicedVolume->setImages( volume->getImages() );
    reslicedVolume->setData( m_reslice->GetOutput() );
    m_2DViewer->setInput( reslicedVolume );

    // Visualitzem els canvis al viewer
    m_2DViewer->render();
}

void ImagePlaneProjectionTool::setToolData(ToolData * data)
{
    if ( !m_toolConfiguration )
    {
        DEBUG_LOG(QString("ImagePlaneProjectionTool: �s obligatori associar a la tool una configuraci� correcta.") );
        return;
    }

    QString typeConfiguration = m_toolConfiguration->getValue( "typeConfiguration" ).toString();

    // Desfem els vincles anteriors
    if ( typeConfiguration == QString( "PRODUCER" ) )
    {
        disconnect( m_myData, SIGNAL( imagePlaneUpdated( QString ) ), this, SLOT( checkImagePlaneBindUpdated( QString ) ) );
        disconnect( m_myData, SIGNAL( thicknessUpdated( QString ) ), this, SLOT( checkThicknessProjectedLineUpdated( QString ) ) );
    }
    if ( typeConfiguration == QString( "CONSUMER" ) )
    {
        disconnect( m_myData, SIGNAL( imagePlaneUpdated( QString ) ), this, SLOT( checkProjectedLineBindUpdated( QString ) ) );
    }
    if ( typeConfiguration == QString( "PRODUCER&CONSUMER" ) )
    {
        disconnect( m_myData, SIGNAL( dataInitialized() ), this, SLOT( initializeImagePlanesCheckingData() ) );
        disconnect( m_myData, SIGNAL( imagePlaneUpdated( QString ) ), this, SLOT( checkImagePlaneBindUpdated( QString ) ) );
        disconnect( m_myData, SIGNAL( imagePlaneUpdated( QString ) ), this, SLOT( checkProjectedLineBindUpdated( QString ) ) );
        disconnect( m_myData, SIGNAL( thicknessUpdated( QString ) ), this, SLOT( checkThicknessProjectedLineUpdated( QString ) ) );
    }

    delete m_myData;

    // Creem de nou les dades
    m_toolData = data;
    m_myData = qobject_cast<ImagePlaneProjectionToolData *>(data);
    
    if ( typeConfiguration == QString( "PRODUCER" ) )
    {
        connect( m_myData, SIGNAL( imagePlaneUpdated( QString ) ), SLOT( checkImagePlaneBindUpdated( QString ) ) );
        connect( m_myData, SIGNAL( thicknessUpdated( QString ) ), SLOT( checkThicknessProjectedLineUpdated( QString ) ) );

        if ( m_volume != NULL )
        {
            // Quan es modifiquen les dades compartides de la tool cal tornar-les a actualitzar assignant 
            // el volum i el pla projectat de cada l�nia del viewer
            initializeImagePlanesUpdteVolume( m_volume );
        }
    }
    else
    {
        Volume *volume = m_myData->getVolume();

        if ( typeConfiguration == QString( "PRODUCER&CONSUMER" ) )
        {
            connect( m_myData, SIGNAL( dataInitialized() ), SLOT( initializeImagePlanesCheckingData() ) );
            connect( m_myData, SIGNAL( imagePlaneUpdated( QString ) ), SLOT( checkImagePlaneBindUpdated( QString ) ) );
            connect( m_myData, SIGNAL( thicknessUpdated( QString ) ), SLOT( checkThicknessProjectedLineUpdated( QString ) ) );
            
            if (volume != NULL)
            {
                initializeImagePlanesCheckingData();
            }
        }
        
        if ( typeConfiguration == QString( "CONSUMER" ) || typeConfiguration == QString( "PRODUCER&CONSUMER" ) )
        {
            connect( m_myData, SIGNAL( imagePlaneUpdated( QString ) ), SLOT( checkProjectedLineBindUpdated( QString ) ) );
            
            if (volume != NULL)
            {
                // Quan es modifiquen les dades compartides de la tool i ja ha estat assignat el volum de treball
                // cal mostrar el pla projectat per la seva l�nia associada
                showImagePlaneProjectedLineBind();
            }
        }
    }
}

void ImagePlaneProjectionTool::handleEvent( long unsigned eventID )
{
    if ( !m_enabled )
        return;
    
    if( !m_toolConfiguration )
    {
        DEBUG_LOG("No s'ha assignat cap configuraci�. La tool no funcionar� com s'espera.");
        return;
    }

    QString typeConfiguration = m_toolConfiguration->getValue( "typeConfiguration" ).toString();
    if ( typeConfiguration == QString("PRODUCER") || typeConfiguration == QString("PRODUCER&CONSUMER") )
    {
        switch( eventID )
        {
            case vtkCommand::LeftButtonPressEvent:
                detectManipulationProjectedLine();
                break;

            case vtkCommand::LeftButtonReleaseEvent:
                releaseProjectedLine();
                break;

            case vtkCommand::MouseMoveEvent:
                if( m_state == ROTATING )
                {
                    rotateProjectedLine();
                }
                else if( m_state == PUSHING )
                {
                    pushProjectedLine();
                }
                break;

            default:
                break;
        }
    }
}

void ImagePlaneProjectionTool::detectManipulationProjectedLine()
{
    double clickedWorldPoint[3];
    m_2DViewer->getEventWorldCoordinate( clickedWorldPoint );

    // detectem si el clic ha estat aprop d'alguna l�nia projectada
    double point[3] = { clickedWorldPoint[0] , clickedWorldPoint[1] , 0.0 };
    double distanceToProjectedLine;
    DrawerLine *projectedLine;
    bool picked = false;

    QMapIterator< DrawerLine *, QStringList > iterator( m_projectedLines );
    while (iterator.hasNext() && !picked ) 
    {
        iterator.next();
        projectedLine = iterator.key();
    
        distanceToProjectedLine = projectedLine->getDistanceToPoint( point );

        // donem una "toler�ncia" m�nima
        if( distanceToProjectedLine < 50.0 )
        {
            m_pickedProjectedLine = projectedLine;

            QStringList infoProjectedLine = m_projectedLines[ m_pickedProjectedLine ];
            m_pickedProjectedLineName = infoProjectedLine.at( 0 );

            if ( m_2DViewer->getInteractor()->GetControlKey() )
            {
                m_state = PUSHING;
            }
            else
            {
                m_state = ROTATING;
            }

            m_initialPickX = clickedWorldPoint[0];
            m_initialPickY = clickedWorldPoint[1];

            m_2DViewer->setCursor( QCursor( Qt::OpenHandCursor ) );

            picked = true;
        }
    }
}

void ImagePlaneProjectionTool::rotateProjectedLine()
{
    double clickedWorldPoint[3];
    m_2DViewer->getEventWorldCoordinate( clickedWorldPoint );

    double vec1[3], vec2[3];
    double axis[3];
    double direction[3];
    double center[3];

    ImagePlane *imagePlane = m_myData->getProjectedLineImagePlane( m_pickedProjectedLineName );

    imagePlane->getCenter( center );

    vec1[0] = m_initialPickX - center[0];
    vec1[1] = m_initialPickY - center[1];
    vec1[2] = 0.0;

    vec2[0] = clickedWorldPoint[0] - center[0];
    vec2[1] = clickedWorldPoint[1] - center[1];
    vec2[2] = 0.0;

    double degrees = MathTools::angleInDegrees( vec1 , vec2 );

    m_initialPickX = clickedWorldPoint[0];
    m_initialPickY = clickedWorldPoint[1];

    MathTools::crossProduct( vec1 , vec2 , direction );
    this->getRotationAxisImagePlane( imagePlane, axis );
    double dot = MathTools::dotProduct( direction , axis );

    axis[0] *= dot;
    axis[1] *= dot;
    axis[2] *= dot;

    MathTools::normalize( axis );
    this->rotateMiddleImagePlane( imagePlane, degrees , axis );

    this->updateProjection( m_pickedProjectedLine, imagePlane );
}

void ImagePlaneProjectionTool::getRotationAxisImagePlane( ImagePlane *imagePlane, double axis[3] )
{
    if( !imagePlane )
        return;

    double columnDirectionVector[3];
    imagePlane->getColumnDirectionVector( columnDirectionVector );

    axis[0] = -columnDirectionVector[0];
    axis[1] = -columnDirectionVector[1];
    axis[2] = -columnDirectionVector[2];
}

void ImagePlaneProjectionTool::rotateMiddleImagePlane( ImagePlane *imagePlane, double degrees , double rotationAxis[3] )
{
    vtkTransform *m_transform = vtkTransform::New();
    m_transform->Identity();
    
    double centre[3];
    imagePlane->getCenter( centre );
    
    m_transform->Translate( centre[0], centre[1], centre[2] );
    m_transform->RotateWXYZ( degrees , rotationAxis );
    m_transform->Translate( -centre[0], -centre[1], -centre[2] );

    // ara que tenim la transformaci�, l'apliquem als punts del pla ( origen, punt1 , punt2)
    
    double rowDirectionVector[3], columnDirectionVector[3], origin[3];
    imagePlane->getRowDirectionVector( rowDirectionVector );
    imagePlane->getColumnDirectionVector( columnDirectionVector );
    imagePlane->getOrigin( origin );

    double point1[3], point2[3];
    for ( int i=0; i < 3; i++ )
    {
        point1[i] = rowDirectionVector[i] + origin[i];
        point2[i] = columnDirectionVector[i] + origin[i];
    }

    double newOrigin[3];
    m_transform->TransformPoint( origin, newOrigin );
    imagePlane->setOrigin( newOrigin );

    double newPoint1[3], newPoint2[3];
    m_transform->TransformPoint( point1 , newPoint1 );
    m_transform->TransformPoint( point2 , newPoint2 );

    for ( int i=0; i < 3; i++ )
    {
        rowDirectionVector[i] = newPoint1[i] - newOrigin[i];
        columnDirectionVector[i] = newPoint2[i] - newOrigin[i];
    }
    imagePlane->setRowDirectionVector( rowDirectionVector );  
    imagePlane->setColumnDirectionVector( columnDirectionVector );
}

void ImagePlaneProjectionTool::pushProjectedLine()
{
    ImagePlane *imagePlane = m_myData->getProjectedLineImagePlane( m_pickedProjectedLineName );

    double clickedWorldPoint[3];
    m_2DViewer->getEventWorldCoordinate( clickedWorldPoint );

    // Get the motion vector
    double v[3];
    v[0] = clickedWorldPoint[0] - m_initialPickX;
    v[1] = clickedWorldPoint[1] - m_initialPickY;
    v[2] = 0.0;

    double normal[3];
    imagePlane->getNormalVector( normal );
    imagePlane->push( MathTools::dotProduct( v, normal ) );

    this->updateProjection( m_pickedProjectedLine, imagePlane );

    m_initialPickX = clickedWorldPoint[0];
    m_initialPickY = clickedWorldPoint[1];
}

void ImagePlaneProjectionTool::releaseProjectedLine()
{
    m_state = NONE;
    m_2DViewer->setCursor( QCursor( Qt::ArrowCursor ) );
}

void ImagePlaneProjectionTool::setNumImagesReconstruction( int numImages )
{
    if ( !m_toolConfiguration )
    {
        DEBUG_LOG(QString("ImagePlaneProjectionTool: �s obligatori associar a la tool una configuraci� correcta.") );
        return;
    }

    // C�lcul del gruix segons el n�mero d'imatges indicat
    double thickness = 1;
    if ( numImages > 1 )
    {
        // Dist�ncia entre els p�xels de les imatges que formen el volum
        double spacing[3];
        m_myData->getVolume()->getSpacing( spacing );

        // Calculem la dist�ncia m�xima entre la primera i la �lltima imatge que composaran la reconstrucci�
        // agafant com a refer�ncia la dist�ncia entre els p�xels de les imatges del volum
        double pixelsDistance = sqrt( spacing[0] * spacing[0] + spacing[1] * spacing[1] );
        thickness = numImages * pixelsDistance;
    } 

    if ( thickness != m_thickness )
    {
        m_thickness = thickness;
        m_myData->setProjectedLineThickness( m_nameProjectedLineBind, m_thickness );
        updateReslice( m_myData->getVolume() );
    }
}

void ImagePlaneProjectionTool:: checkThicknessProjectedLineUpdated( QString nameProjectedLine )
{
    if ( m_imagePlanes.contains( nameProjectedLine ) )
    {
        QMapIterator< DrawerLine *, QStringList > iterator( m_projectedLines );
        bool searched = false;
        DrawerLine *projectedLine;
        while ( iterator.hasNext() && !searched ) 
        {
            iterator.next();
            projectedLine = iterator.key();
            QString name = iterator.value().at(0);
            searched = ( nameProjectedLine == name );
        }

        if ( searched ) 
        {
            m_thickness = m_myData->getProjectedLineThickness( nameProjectedLine );
            applyThicknessProjectedLine( nameProjectedLine , projectedLine );
        }
    }
}

void ImagePlaneProjectionTool:: applyThicknessProjectedLine( QString nameProjectedLine, DrawerLine *projectedLine )
{
    // Inicialitzem o netegem les l�nies que mostraran quins s�n els
    // l�mits de les imatges que s'han reconstru�t al visor consumidor
    if ( !m_upLineThickness )
    {
        // L�nia thickness superior
        m_upLineThickness = new DrawerLine;
        m_upLineThickness->setLinePattern( DrawerPrimitive::DiscontinuousLinePattern );
        m_upLineThickness->setColor( Qt::yellow );
        m_upLineThickness->setLineWidth( 3 );
        m_2DViewer->getDrawer()->draw( m_upLineThickness , Q2DViewer::Top2DPlane );

        // L�nia thickness inferior
        m_downLineThickness = new DrawerLine;
        m_downLineThickness->setLinePattern( DrawerPrimitive::DiscontinuousLinePattern );
        m_downLineThickness->setColor( Qt::yellow );
        m_downLineThickness->setLineWidth( 3 );
        m_2DViewer->getDrawer()->draw( m_downLineThickness , Q2DViewer::Top2DPlane );
    }
    else
    {
        m_upLineThickness->visibilityOff();
        m_upLineThickness->update();

        m_downLineThickness->visibilityOff();
        m_downLineThickness->update();

        m_2DViewer->render();
    }

    QString typeConfiguration = m_toolConfiguration->getValue( "typeConfiguration" ).toString();

    //DEBUG_LOG(QString("Thickness = %1").arg(m_thickness));

    // Si el gruix no �s m�s gran que 1 no cal mostrar aquestes l�nies perqu� 
    // significa que el volum reconstru�t nom�s t� una imatge
    if ( m_thickness > 1 )
    {
        // Punts a despla�ar = punts que defineixen la l�nia de projecci�
        double *firstPoint = projectedLine->getFirstPoint();
        double *secondPoint = projectedLine->getSecondPoint();

        // Dist�ncia moviment
        double distance = m_thickness / 2;

        // Direcci� moviment 
        double normalVectorImagePlane[3];
        ImagePlane *imagePlane = m_myData->getProjectedLineImagePlane( nameProjectedLine );
        imagePlane->getNormalVector( normalVectorImagePlane );
        MathTools::normalize( normalVectorImagePlane );
        double normalVector[3];
        if ( typeConfiguration == QString("PRODUCER") )
        {
            // Direcci� = normal del pla indicat per la l�nia de projecci�
            normalVector[0] = normalVectorImagePlane[0];
            normalVector[1] = normalVectorImagePlane[1];
            normalVector[2] = normalVectorImagePlane[2];
        }
        else if ( typeConfiguration == QString("PRODUCER&CONSUMER") )
        {
            if ( m_nameProjectedLineBind == "VERTICAL_LINE" )
            {
                //YZ, x-normal
                normalVector[0] = normalVectorImagePlane[1];
                normalVector[1] = normalVectorImagePlane[2];
                normalVector[2] = normalVectorImagePlane[0];
            }
            else if ( m_nameProjectedLineBind == "HORIZONTAL_LINE" )
            {
                //XZ, y-normal
                normalVector[0] = normalVectorImagePlane[0];
                normalVector[1] = normalVectorImagePlane[2];
                normalVector[2] = normalVectorImagePlane[1];
            }
        }

        // Mostrem l�nia thickness superior = despla�ament positiu l�nia de projecci�
        double upFirstPoint[3];
        upFirstPoint[0] = firstPoint[0] + ( distance * normalVector[0] );
        upFirstPoint[1] = firstPoint[1] + ( distance * normalVector[1] );
        upFirstPoint[2] = 0;

        double upSecondPoint[3];
        upSecondPoint[0] = secondPoint[0] + ( distance * normalVector[0] );
        upSecondPoint[1] = secondPoint[1] + ( distance * normalVector[1] );
        upSecondPoint[2] = 0;

        m_upLineThickness->setFirstPoint( upFirstPoint );
        m_upLineThickness->setSecondPoint( upSecondPoint );
        if ( !m_upLineThickness->isVisible() )
        {
            m_upLineThickness->visibilityOn();
        }
        m_upLineThickness->update();

        // Mostrem l�nia thickness inferior = despla�ament negatiu l�nia de projecci�
        double downFirstPoint[3];
        downFirstPoint[0] = firstPoint[0] - ( distance * normalVector[0] );
        downFirstPoint[1] = firstPoint[1] - ( distance * normalVector[1] );
        downFirstPoint[2] = 0;

        double downSecondPoint[3];
        downSecondPoint[0] = secondPoint[0] - ( distance * normalVector[0] );
        downSecondPoint[1] = secondPoint[1] - ( distance * normalVector[1] );
        downSecondPoint[2] = 0;
        m_downLineThickness->setFirstPoint( downFirstPoint );
        m_downLineThickness->setSecondPoint( downSecondPoint );
        if ( !m_downLineThickness->isVisible() )
        {
            m_downLineThickness->visibilityOn();
        }
        m_downLineThickness->update();

        //DEBUG_LOG(QString("First point [%1,%2,%3]").arg(firstPoint[0]).arg(firstPoint[1]).arg(firstPoint[2]));
        //DEBUG_LOG(QString("Second point [%1,%2,%3]").arg(secondPoint[0]).arg(secondPoint[1]).arg(secondPoint[2]));
        //DEBUG_LOG(QString("Normal vector [%1,%2,%3]").arg(normalVector[0]).arg(normalVector[1]).arg(normalVector[2]));
        //DEBUG_LOG(QString("Distance = %1").arg(distance));
        //DEBUG_LOG(QString("First point upLine [%1,%2,%3]").arg(m_upLineThickness->getFirstPoint()[0]).arg(m_upLineThickness->getFirstPoint()[1]).arg(m_upLineThickness->getFirstPoint()[2]));
        //DEBUG_LOG(QString("Second point upLine [%1,%2,%3]").arg(m_upLineThickness->getSecondPoint()[0]).arg(m_upLineThickness->getSecondPoint()[1]).arg(m_upLineThickness->getSecondPoint()[2]));
        //DEBUG_LOG(QString("First point downLine [%1,%2,%3]").arg(m_downLineThickness->getFirstPoint()[0]).arg(m_downLineThickness->getFirstPoint()[1]).arg(m_downLineThickness->getFirstPoint()[2]));
        //DEBUG_LOG(QString("Second point downLine [%1,%2,%3]").arg(m_downLineThickness->getSecondPoint()[0]).arg(m_downLineThickness->getSecondPoint()[1]).arg(m_downLineThickness->getSecondPoint()[2]));
        
        m_2DViewer->render();
    }
}

}  //  end namespace udg
