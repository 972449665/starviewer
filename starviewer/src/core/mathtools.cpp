#include "mathtools.h"
#include "logging.h"

#include <cmath>
#include <vtkMath.h>
#include <vtkPlane.h>
#include <vtkLine.h>

#include <QVector3D>

namespace udg {

/// Definició de constants
// log_2 e
const double MathTools::NumberEBase2Logarithm = 1.4426950408889634074;
// 1/pi
const long double MathTools::ReversePiNumberLong = 0.3183098861837906715377675267450287L;
// pi
const double MathTools::PiNumber = 3.14159265358979323846;
// pi
const long double MathTools::PiNumberLong = 3.14159265358979323846;
// pi/2
const long double MathTools::PiNumberDivBy2Long = 1.5707963267948966192313216916397514L;
const double MathTools::Epsilon = 1E-9;
const double MathTools::DegreesToRadiansAsDouble = 0.017453292519943295;
const double MathTools::RadiansToDegreesAsDouble = 57.29577951308232;
// TODO Potser seria més conevnient fer servir std::numeric_limits<double>::max(). Caldria incloure <limits>
const double MathTools::DoubleMaximumValue = VTK_DOUBLE_MAX;

double MathTools::logTwo(const double x, const bool zero)
{
    if (x < 0)
    {
        WARN_LOG("MathTools::logTwo >> Log of negative number");
    }

    if (zero)
    {
        return (x == 0) ? 0 : double(log(double(x))) * NumberEBase2Logarithm;
    }
    else
    {
        if (x == 0)
        {
            WARN_LOG("MathTools::logTwo >> Log of zero");
        }

        return double(log(double(x))) * NumberEBase2Logarithm;
    }
}

double MathTools::angleInRadians(const QVector3D &vec1, const QVector3D &vec2)
{
    return acos(QVector3D::dotProduct(vec1, vec2) / (vec1.length() * vec2.length()));
}

double MathTools::angleInDegrees(const QVector3D &vec1, const QVector3D &vec2)
{
    return angleInRadians(vec1, vec2) * MathTools::RadiansToDegreesAsDouble;
}

int MathTools::planeIntersection(double p[3], double n[3], double q[3], double m[3], double r[3], double t[3])
{
    if (angleInDegrees(QVector3D(n[0], n[1], n[2]), QVector3D(m[0], m[1], m[2])) == 0.0)
    {
        return 0;
    }
    // Solució extreta de http://vis.eng.uci.edu/courses/eecs104/current/GraphicsMath.pdf, pàg. 64
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

    double u[3];
    MathTools::crossProduct(n, m, t);
    MathTools::crossProduct(n, t, u);

    double pq[3], sum[3], pqDotm, dot_u_m;

    pq[0] = q[0] - p[0];
    pq[1] = q[1] - p[1];
    pq[2] = q[2] - p[2];

    pqDotm = MathTools::dotProduct(pq, m);
    dot_u_m = MathTools::dotProduct(u, m);
    sum[0] = (pqDotm * u[0]) / dot_u_m;
    sum[1] = (pqDotm * u[1]) / dot_u_m;
    sum[2] = (pqDotm * u[2]) / dot_u_m;

    r[0] = sum[0] + p[0];
    r[1] = sum[1] + p[1];
    r[2] = sum[2] + p[2];

    return 1;
}

int MathTools::planeIntersection(double p[3], double n[3], double q[3], double m[3], double r[3], double t[3], double intersectionPoint[3])
{

    // Solució extreta de http://vis.eng.uci.edu/courses/eecs104/current/GraphicsMath.pdf, pàg. 65
    // pla1 definit per (p,n); p: punt del pla, p.ex. origen, n: normal del pla
    // pla2 definit per (q,m);
    // pla3 definit per (r,t);
    // la intersecció serà un punt w
    // si w = p + a·n + b·m + c·t
    // llavors caldria resoldre el sistema lineal
    // pw · n  = 0, qw · m = 0, rw · t = 0
    // per a, b i c
    //
    // o bé calcular la línia d'intersecció entre dos plans i el punt d'intersecció de la línia amb el pla restant
    //
    // Sembla més fàcil la segona opció
    double point[3], vector[3];
    planeIntersection(p, n, q, m, point, vector);
    // Càlcul intersecció línia pla
    // Línia representada per punt i vector(p,t), pla per punt(origen) i normal (r,n), q és la intersecció
    // q = p + (pr·n)t / (t·n)

    double tt, point2[3];
    point2[0] = point[0] + vector[0];
    point2[1] = point[1] + vector[1];
    point2[2] = point[2] + vector[2];

    // Li donem una recta definida per dos punts, i el pla definit per la normal i un punt. T és la coordenada paramètrica al llarg de la recta i el punt
    // de la intersecció queda a intersectPoint
    if (vtkPlane::IntersectWithLine(point, point2, t, r, tt, intersectionPoint) == 0)
    {
        // Si retorna 0 és que o bé línia i pla no intersecten o són paralels entre sí
        if (tt == MathTools::DoubleMaximumValue)
        {
            DEBUG_LOG(QString("No hi ha hagut intersecció! Valor coord paramètrica: %1").arg(tt));
            return -1;
        }
        else
        {
            return 0;
        }
    }

    return 1;
}

QVector3D MathTools::directorVector(const QVector3D &point1, const QVector3D &point2)
{
    return point2 - point1;
}

double MathTools::modulus(double vector[3])
{
    return sqrt(pow(vector[0], 2) + pow(vector[1], 2) + pow(vector[2], 2));
}

double MathTools::dotProduct(double vector1[3], double vector2[3])
{
    return ((vector1[0] * vector2[0]) + (vector1[1] * vector2[1]) + (vector1[2] * vector2[2]));
}

void MathTools::crossProduct(double vectorDirector1[3], double vectorDirector2[3], double crossProductVector[3])
{
    crossProductVector[0] = vectorDirector1[1] * vectorDirector2[2] - vectorDirector1[2] * vectorDirector2[1];
    crossProductVector[1] = vectorDirector1[2] * vectorDirector2[0] - vectorDirector1[0] * vectorDirector2[2];
    crossProductVector[2] = vectorDirector1[0] * vectorDirector2[1] - vectorDirector1[1] * vectorDirector2[0];
}

double MathTools::normalize(double vector[3])
{
    return vtkMath::Normalize(vector);
}

double MathTools::minimum(double a, double b)
{
    double min;

    if (a < b)
    {
        min = a;
    }
    else
    {
        min = b;
    }

    return min;
}

double MathTools::maximum(double a, double b)
{
    double max;

    if (a > b)
    {
        max = a;
    }
    else
    {
        max = b;
    }

    return max;
}

double MathTools::cubeRoot(double x)
{
    return std::pow(x, 1.0 / 3.0);
}

double MathTools::getDistance3D(const double firstPoint[3], const double secondPoint[3])
{
    double xx = firstPoint[0] - secondPoint[0];
    double yy = firstPoint[1] - secondPoint[1];
    double zz = firstPoint[2] - secondPoint[2];
    double value = pow(xx, 2) + pow(yy, 2) + pow(zz, 2);
    return sqrt(value);
}

double MathTools::getPointToFiniteLineDistance(double point[3], double lineFirstPoint[3], double lineSecondPoint[3], double closestPoint[3])
{
    double parametricCoordinate;

    // vtkLine::DistanceToLine() ens retorna la distància al quadrat, per això fem sqrt()
    return sqrt(vtkLine::DistanceToLine(point, lineFirstPoint, lineSecondPoint, parametricCoordinate, closestPoint));
}

double* MathTools::infiniteLinesIntersection(double *p1, double *p2, double *p3, double *p4, int &state)
{
    //  Solution by Wolfram Mathematics
    //
    //   http://mathworld.wolfram.com/Line-LineIntersection.html
    //
    double *intersection;

    intersection = new double[3];
    intersection[0] = 0;
    intersection[1] = 0;
    intersection[2] = 0;

    // Line 1: x = x1 + (x2 - x1)s
    // Line 2: x = x3 + (x4 - x3)t
    double s;
    // Director vectors for each line
    double dv1[3], dv2[3], dv3[3];

    dv1[0] = p2[0] - p1[0];
    dv1[1] = p2[1] - p1[1];
    dv1[2] = p2[2] - p1[2];

    dv2[0] = p4[0] - p3[0];
    dv2[1] = p4[1] - p3[1];
    dv2[2] = p4[2] - p3[2];

    dv3[0] = p3[0] - p1[0];
    dv3[1] = p3[1] - p1[1];
    dv3[2] = p3[2] - p1[2];

    // Coplanarity test
    double cross[3];
    MathTools::crossProduct(dv1, dv2, cross);

    double dot = MathTools::dotProduct(dv1, cross);

    // Coplanarity check
    if (MathTools::closeEnough(dot, 0.0))
    {
        double numerator1[3], numerator2[3], denominator1[3];
        double numerator, denominator;

        MathTools::crossProduct(dv3, dv2, numerator1);
        MathTools::crossProduct(dv1, dv2, numerator2);

        numerator = MathTools::dotProduct(numerator1, numerator2);

        MathTools::crossProduct(dv1, dv2, denominator1);

        denominator = pow(MathTools::modulus(denominator1), 2);

        if (MathTools::closeEnough(denominator, 0.0))
        {
            state = ParallelLines;
            return intersection;
        }
        else
        {
            s = numerator / denominator;

            intersection[0] = p1[0] + (s * dv1[0]);
            intersection[1] = p1[1] + (s * dv1[1]);
            intersection[2] = p1[2] + (s * dv1[2]);

            state = LinesIntersect;
            return intersection;
        }
    }
    // Skew Lines
    else
    {
        state = SkewIntersection;
        return intersection;
    }

}

double MathTools::truncate(double x)
{
    return std::floor(x);
}

int MathTools::roundToNearestInteger(double x)
{
    return vtkMath::Round(x);
}

bool MathTools::closeEnough(float f1, float f2)
{
    return fabsf((f1 - f2) / ((f2 == 0.0f) ? 1.0f : f2)) < Epsilon;
}

float MathTools::degreesToRadians(float degrees)
{
    return (degrees * PiNumber) / 180.0f;
}

float MathTools::radiansToDegrees(float radians)
{
    return (radians * 180.0f) / PiNumber;
}

bool MathTools::isNaN(double x)
{
    return x != x;
}

unsigned int MathTools::roundUpToPowerOf2(unsigned int v)
{
    // Font: http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    // Si v == 0 el codi de sobre retorna 0. Ho solucionem amb aquesta suma.
    v += (v == 0);
    return v;
}

}; // End namespace udg
