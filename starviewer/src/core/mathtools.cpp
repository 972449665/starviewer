/***************************************************************************
 *   Copyright (C) 2005 by Grup de Gràfics de Girona                       *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/

#include "mathtools.h"
#include "logging.h"
#include <vtkMath.h>
#include <vtkPlane.h>
namespace udg{


/**
* DEFINICIÓ DE CONSTANTS
**/


const double MathTools::LOGARITHM_BASE2_E = 1.4426950408889634074;          /* log_2 e */
const long double MathTools::REVERS_PI_LONG = 0.3183098861837906715377675267450287L;  /* 1/pi */
const double MathTools::PI = 3.14159265358979323846;  /* pi */
const long double MathTools::PI_LONG = 3.14159265358979323846;  /* pi */
const long double MathTools::PI_DIV_2_LONG = 1.5707963267948966192313216916397514L;  /* pi/2 */


 ///logaritme en base 2
double MathTools::logTwo(const double x, const bool zero)
{
    if (x < 0)
        WARN_LOG( "MathTools::logTwo >> Log of negative number" );

    if (zero)
    {
        return (x == 0) ? 0 : double( log(double(x)) ) * LOGARITHM_BASE2_E;
    }
    else
    {
        if (x == 0)
            WARN_LOG( "MathTools::logTwo >> Log of zero" );

        return double( log(double(x)) ) * LOGARITHM_BASE2_E;
    }
}

/// Càlcul de l'entropia binària
double MathTools::binaryEntropy(const double p)
{
    if ( p < 0 || p > 1 )
    {
        WARN_LOG( QString("MathTools::binaryEntropy >> Value [%1] out of range (0..1) -> ").arg(p) );
    }

    return - p * MathTools::logTwo(p) - (1 - p) * MathTools::logTwo(1 - p) ;
}

double MathTools::divReal( const double dividend,  const double divisor, const bool zero, const double value )
{
    if (zero)
    {
        return (divisor == 0) ? value : dividend / divisor;
    }
    else
    {
        if (divisor == 0)
            WARN_LOG( "MathTools::divReal >> Division by zero" );
        return dividend / divisor;
    }
}

double MathTools::angleInRadians( double vec1[3] , double vec2[3] )
{
    return acos( vtkMath::Dot( vec1,vec2 ) / ( vtkMath::Norm(vec1)*vtkMath::Norm(vec2) ) );
}

double MathTools::angleInDegrees( double vec1[3] , double vec2[3] )
{
    return angleInRadians( vec1 , vec2 ) * vtkMath::DoubleRadiansToDegrees();
}

int MathTools::planeIntersection( double p[3] , double n[3], double q[3] , double m[3], double r[3] , double t[3] )
{

    if( angleInDegrees( n , m ) == 0.0 )
    {
        return 0;
    }
    //
    // solució extreta de http://vis.eng.uci.edu/courses/eecs104/current/GraphicsMath.pdf, pàg. 64
    // pla1 definit per (p,n); p: punt del pla, p.ex. origen; n: normal
    // pla2 definit per (q,m); q: punt del pla, p.ex. origen; m: normal
    // línia d'intersecció (r,t); r: punt de la recta que pertany a tots dos plans; t: vector director
    // u: vector perpendicular a n i t;
    // Cross: producte vectorial
    // Dot: producte escalar
    // * : multiplicació de vectors
    // + : suma de vectors
    //
    // ******* FORMULETA *************
    //
    // t = Cross(n,m)
    // u = Cross(n,t)
    // r = p + Dot(p-q)*u / Dot(u,m)
    //
    double u[3];
    vtkMath::Cross( n , m , t );
    vtkMath::Cross( n , t , u );

    double pq[3] , sum[3] , pqDotm , dot_u_m;

    pq[0] = q[0] - p[0];
    pq[1] = q[1] - p[1] ;
    pq[2] = q[2] - p[2];


    pqDotm = vtkMath::Dot( pq , m );
    dot_u_m = vtkMath::Dot( u , m );
    sum[0] = ( pqDotm * u[0] ) / dot_u_m;
    sum[1] = ( pqDotm * u[1] ) / dot_u_m;
    sum[2] = ( pqDotm * u[2] ) / dot_u_m;

    r[0] = sum[0] + p[0];
    r[1] = sum[1] + p[1];
    r[2] = sum[2] + p[2];

    return 1;
}

int MathTools::planeIntersection( double p[3] , double n[3], double q[3] , double m[3], double r[3] , double t[3] , double intersectionPoint[3] )
{
    //
    // solució extreta de http://vis.eng.uci.edu/courses/eecs104/current/GraphicsMath.pdf, pàg. 65
    // pla1 definit per (p,n); p: punt del pla, p.ex. origen, n: normal del pla
    // pla2 definit per (q,m);
    // pla3 definit per (r,t);
    // la intersecció serà un punt w
    // si w = p + a·n + b·m + c·t
    // llavors caldria resoldre el sistema lineal
    // pw · n  = 0 , qw · m = 0 , rw · t = 0
    // per a, b i c
    //
    // o bé calcular la línia d'intersecció entre dos plans i el punt d'intersecció de la línia amb el pla restant
    //
    // Sembla més fàcil la segona opció
    double point[3] , vector[3];
    planeIntersection( p , n , q , m , point , vector );
    //càlcul intersecció línia pla
    // Línia representada per punt i vector(p,t), pla per punt(origen) i normal (r,n), q és la intersecció
    // q = p + (pr·n)t / (t·n)

    double tt , point2[3];
    point2[0] = point[0] + vector[0];
    point2[1] = point[1] + vector[1];
    point2[2] = point[2] + vector[2];


    // li donem una recta definida per dos punts , i el pla definit per la normal i un punt. T és la coordenada paramètrica al llarg de la recta i el punt de la intersecció queda a intersectPoint

    if( vtkPlane::IntersectWithLine( point , point2 , t , r ,  tt , intersectionPoint ) == 0 )
    {
        // si retorna 0 és que o bé línia i pla no intersecten o són paralels entre sí
        if( tt == VTK_DOUBLE_MAX )
        {
            DEBUG_LOG( QString("No hi ha hagut intersecció! Valor coord paramètrica: %1").arg( tt )  );
            return -1;
        }
        else
            return 0;

    }

    return 1;
}

double* MathTools::directorVector( double point1[3], double point2[3] )
{
    double *vd = new double[3];
    
    for (int i = 0; i < 3; i++)
        vd[i] = point2[i] - point1[i];
    
    return vd;
}

double* MathTools::vectorialProduct( double vectorDirector1[3], double vectorDirector2[3] )
{
    double *vp = new double[3];
    vp[0] = (vectorDirector1[1] * vectorDirector2[2]) - (vectorDirector1[2] * vectorDirector2[1]);
    vp[1] = -1.0 * ((vectorDirector1[0] * vectorDirector2[2]) - (vectorDirector1[2] * vectorDirector2[0]));
    vp[2] = (vectorDirector1[0] * vectorDirector2[1]) - (vectorDirector1[1] * vectorDirector2[0]);
    
    return vp;
}

int MathTools::getSign( double number )
{
    int sign;
    
    if (number < 0. )
        sign = NEGATIVE;
    else
        sign = POSITIVE;
        
    return sign;       
}

bool MathTools::haveSameSign( double numberA, double numberB )
{
    return ( getSign( numberA ) == getSign( numberB ) );
}

}; // end namespace udg
