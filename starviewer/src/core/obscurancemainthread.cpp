#include "obscurancemainthread.h"

#include <vtkEncodedGradientEstimator.h>
#include <vtkImageData.h>
#include <vtkMultiThreader.h>
#include <vtkPointData.h>
#include <vtkVolume.h>
#include <vtkVolumeRayCastMapper.h>

#include "logging.h"
#include "obscurancethread.h"
#include "vector3.h"
#include "viewpointgenerator.h"


namespace udg {


bool ObscuranceMainThread::hasColor( Variant variant )
{
    return variant >= OpacityColorBleeding;
}


ObscuranceMainThread::ObscuranceMainThread( int numberOfDirections, double maximumDistance, Function function, Variant variant, bool doublePrecision, QObject *parent )
 : QThread( parent ),
   m_numberOfDirections( numberOfDirections ), m_maximumDistance( maximumDistance ), m_function( function ), m_variant( variant ),
   m_doublePrecision( doublePrecision ),
   m_volume( 0 ),
   m_obscurance( 0 )
{
}


ObscuranceMainThread::~ObscuranceMainThread()
{
    if ( m_volume ) m_volume->Delete();
}


bool ObscuranceMainThread::hasColor() const
{
    return hasColor( m_variant );
}


void ObscuranceMainThread::setVolume( vtkVolume *volume )
{
    m_volume = volume; m_volume->Register( 0 );
}


void ObscuranceMainThread::setTransferFunction( const TransferFunction &transferFunction )
{
    m_transferFunction = transferFunction;
}


void ObscuranceMainThread::setSaliency( const double *saliency, double fxSaliencyA, double fxSaliencyB, double fxSaliencyLow, double fxSaliencyHigh )
{
    m_saliency = saliency;
    m_fxSaliencyA = fxSaliencyA;
    m_fxSaliencyB = fxSaliencyB;
    m_fxSaliencyLow = fxSaliencyLow;
    m_fxSaliencyHigh = fxSaliencyHigh;
}


Obscurance* ObscuranceMainThread::getObscurance() const
{
    return m_obscurance;
}


void ObscuranceMainThread::stop()
{
    m_stopped = true;
}


void ObscuranceMainThread::run()
{
    Q_ASSERT( m_volume );

    m_stopped = false;

    vtkVolumeRayCastMapper *mapper = vtkVolumeRayCastMapper::SafeDownCast( m_volume->GetMapper() );
    vtkEncodedGradientEstimator *gradientEstimator = mapper->GetGradientEstimator();
    gradientEstimator->GetEncodedNormals(); /// \todo fent això aquí crec que va més ràpid, però s'hauria de comprovar i provar també amb l'Update()

    // Creem els threads
    int numberOfThreads = vtkMultiThreader::GetGlobalDefaultNumberOfThreads();  /// \todo QThread::idealThreadCount() amb Qt >= 4.3
    QVector<ObscuranceThread *> threads(numberOfThreads);

    // variables necessàries
    vtkImageData *image = mapper->GetInput();
    unsigned short *data = reinterpret_cast<unsigned short*>( image->GetPointData()->GetScalars()->GetVoidPointer( 0 ) );
    int dataSize = image->GetPointData()->GetScalars()->GetSize();
    int dimensions[3];
    image->GetDimensions( dimensions );
    vtkIdType vtkIncrements[3];
    image->GetIncrements( vtkIncrements );

    int increments[3];
    increments[0] = vtkIncrements[0];
    increments[1] = vtkIncrements[1];
    increments[2] = vtkIncrements[2];

    m_obscurance = new Obscurance( dataSize, hasColor(), m_doublePrecision );

    for ( int i = 0; i < numberOfThreads; i++ )
    {
        ObscuranceThread * thread = new ObscuranceThread( i, numberOfThreads, m_transferFunction );
        thread->setGradientEstimator( gradientEstimator );
        thread->setData( data, dataSize, dimensions, increments );
        thread->setObscuranceParameters( m_maximumDistance, m_function, m_variant, m_obscurance );
        thread->setSaliency( m_saliency, m_fxSaliencyA, m_fxSaliencyB, m_fxSaliencyLow, m_fxSaliencyHigh );
        threads[i] = thread;
    }

    // estructures de dades reaprofitables
    QVector<Vector3> lineStarts;

    const QVector<Vector3> directions = getDirections();
    int nDirections = directions.size();

    // iterem per les direccions
    for ( int i = 0; i < nDirections && !m_stopped; i++ )
    {
        const Vector3 &direction = directions.at( i );

        DEBUG_LOG( QString( "Direcció %1: %2" ).arg( i ).arg( direction.toString() ) );

        // direcció dominant (0 = x, 1 = y, 2 = z)
        int dominant;
        Vector3 absDirection( qAbs( direction.x ), qAbs( direction.y ), qAbs( direction.z ) );
        if ( absDirection.x >= absDirection.y )
        {
            if ( absDirection.x >= absDirection.z ) dominant = 0;
            else dominant = 2;
        }
        else
        {
            if ( absDirection.y >= absDirection.z ) dominant = 1;
            else dominant = 2;
        }

        // vector per avançar
        Vector3 forward;
        switch ( dominant )
        {
            case 0: forward = Vector3( direction.x, direction.y, direction.z ); break;
            case 1: forward = Vector3( direction.y, direction.z, direction.x ); break;
            case 2: forward = Vector3( direction.z, direction.x, direction.y ); break;
        }
        forward /= qAbs( forward.x );   // la direcció x passa a ser 1 o -1
        DEBUG_LOG( QString( "forward = " ) + forward.toString() );

        // dimensions i increments segons la direcció dominant
        int x = dominant, y = ( dominant + 1 ) % 3, z = ( dominant + 2 ) % 3;
        int dimX = dimensions[x], dimY = dimensions[y], dimZ = dimensions[z];
        int incX = increments[x], incY = increments[y], incZ = increments[z];
        int sX = 1, sY = 1, sZ = 1;
        qptrdiff startDelta = 0;
        if ( forward.x < 0.0 )
        {
            startDelta += incX * ( dimX - 1 );
//             incX = -incX;
            forward.x = -forward.x;
            sX = -1;
        }
        if ( forward.y < 0.0 )
        {
            startDelta += incY * ( dimY - 1 );
//             incY = -incY;
            forward.y = -forward.y;
            sY = -1;
        }
        if ( forward.z < 0.0 )
        {
            startDelta += incZ * ( dimZ - 1 );
//             incZ = -incZ;
            forward.z = -forward.z;
            sZ = -1;
        }
        DEBUG_LOG( QString( "forward = " ) + forward.toString() );
        // ara els 3 components són positius

        // llista dels vòxels que són començament de línia
        getLineStarts( lineStarts, dimX, dimY, dimZ, forward );

//         int incXYZ[3] = { incX, incY, incZ };
        int xyz[3] = { x, y, z };
        int sXYZ[3] = { sX, sY, sZ };

        // iniciem els threads
        for ( int j = 0; j < numberOfThreads; j++ )
        {
            ObscuranceThread * thread = threads[j];
            thread->setPerDirectionParameters( direction, forward, xyz, sXYZ, lineStarts, startDelta );
            thread->start();
        }

        // esperem que acabin els threads
        for ( int j = 0; j < numberOfThreads; j++ )
        {
            threads[j]->wait();
        }

        emit progress( 100 * ( i + 1 ) / nDirections );
    }

    // destruïm els threads
    for ( int j = 0; j < numberOfThreads; j++ )
    {
        delete threads[j];
    }

    if ( m_stopped )    // si han cancel·lat el procés ja podem plegar
    {
        emit progress( 0 );
        delete m_obscurance; m_obscurance = 0;
        return;
    }

    m_obscurance->normalize();

    emit computed();
}


void ObscuranceMainThread::getLineStarts( QVector<Vector3> &lineStarts, int dimX, int dimY, int dimZ, const Vector3 &forward )
{
    lineStarts.resize( 0 );

    // tots els (0,y,z) són començament de línia
    Vector3 lineStart;  // (0,0,0)
    for ( int iy = 0; iy < dimY; ++iy )
    {
        lineStart.y = iy;
        for ( int iz = 0; iz < dimZ; ++iz )
        {
            lineStart.z = iz;
            lineStarts << lineStart;
//             DEBUG_LOG( QString( "line start: (%1,%2,%3)" ).arg( lineStart.x ).arg( lineStart.y ).arg( lineStart.z ) );
        }
    }
    DEBUG_LOG( QString( "line starts: %1" ).arg( lineStarts.size() ) );

    // més començaments de línia
    Vector3 rv; // (0,0,0)
    Voxel v = { 0, 0, 0 }, pv = v;

    // iterar per la línia que comença a (0,0,0)
    while ( v.x < dimX )
    {
        if ( v.y != pv.y )
        {
            lineStart.x = rv.x; lineStart.y = rv.y - v.y;   // [y] = 0
            for ( double iz = rv.z - v.z; iz < dimZ; iz++ )
            {
                lineStart.z = iz;
                lineStarts << lineStart;
            }
        }
        if ( v.z != pv.z )
        {
            lineStart.x = rv.x; lineStart.z = rv.z - v.z;   // [z] = 0
            for ( double iy = rv.y - v.y; iy < dimY; iy++ )
            {
                lineStart.y = iy;
                lineStarts << lineStart;
            }
        }

        // avançar el vòxel
        rv += forward;
        pv = v;
        v.x = qRound( rv.x ); v.y = qRound( rv.y ); v.z = qRound( rv.z );
    }
    DEBUG_LOG( QString( "line starts: %1" ).arg( lineStarts.size() ) );
}


QVector<Vector3> ObscuranceMainThread::getDirections() const
{
    ViewpointGenerator viewpointGenerator;

    if ( m_numberOfDirections >= 0 ) viewpointGenerator.setToQuasiUniform( m_numberOfDirections );
    else
    {
        switch ( m_numberOfDirections )
        {
            default:
            case -4: viewpointGenerator.setToUniform4(); break;
            case -6: viewpointGenerator.setToUniform6(); break;
            case -8: viewpointGenerator.setToUniform8(); break;
            case -12: viewpointGenerator.setToUniform12(); break;
            case -20: viewpointGenerator.setToUniform20(); break;
        }
    }

    return viewpointGenerator.viewpoints();
}


}
