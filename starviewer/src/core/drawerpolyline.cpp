#include "drawerpolyline.h"
#include "logging.h"
#include "mathtools.h"
// Vtk
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkActor2D.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProperty2D.h>
// Qt
#include <QVector>

namespace udg {

DrawerPolyline::DrawerPolyline(QObject *parent)
 : DrawerPrimitive(parent), m_vtkPolydata(0), m_vtkPoints(0), m_vtkCellArray(0), m_vtkActor(0), m_vtkMapper(0)
{
    m_vtkActor = NULL;
}

DrawerPolyline::~DrawerPolyline()
{
    emit dying(this);

    if (m_vtkPolydata)
    {
        m_vtkPolydata->Delete();
    }

    if (m_vtkPoints)
    {
        m_vtkPoints->Delete();
    }

    if (m_vtkCellArray)
    {
        m_vtkCellArray->Delete();
    }

    if (m_vtkActor)
    {
        m_vtkActor->Delete();
    }
    if (m_vtkMapper)
    {
        m_vtkMapper->Delete();
    }
}

void DrawerPolyline::addPoint(double point[3])
{
    double *localPoint = new double[3];
    for (int i = 0; i < 3; i++)
    {
        localPoint[i] = point[i];
    }

    m_pointsList << localPoint;
    emit changed();
}

void DrawerPolyline::setPoint(int i, double point[3])
{
    if (i >= m_pointsList.count() || i < 0)
    {
        addPoint(point);
    }
    else
    {
        double *array = m_pointsList.takeAt(i);
        for (int j = 0; j < 3; j++)
        {
            array[j] = point[j];
        }

        m_pointsList.insert(i, array);
        emit changed();
    }
}

double* DrawerPolyline::getPoint(int position)
{
    if (position >= m_pointsList.count())
    {
        double *array = new double[3];
        return array;
    }
    else
    {
        return m_pointsList.at(position);
    }
}

void DrawerPolyline::removePoint(int i)
{
    m_pointsList.removeAt(i);
    emit changed();
}

void DrawerPolyline::deleteAllPoints()
{
    m_pointsList.clear();
}

vtkProp* DrawerPolyline::getAsVtkProp()
{
    if (!m_vtkActor)
    {
        buildVtkPoints();
        // Creem el pipeline de l'm_vtkActor
        m_vtkActor = vtkActor2D::New();
        m_vtkMapper = vtkPolyDataMapper2D::New();

        m_vtkActor->SetMapper(m_vtkMapper);
        m_vtkMapper->SetInput(m_vtkPolydata);
        // Li donem els atributs
        updateVtkActorProperties();
    }
    return m_vtkActor;
}

void DrawerPolyline::update()
{
    switch (m_internalRepresentation)
    {
        case VTKRepresentation:
            updateVtkProp();
            break;

        case OpenGLRepresentation:
            break;
    }
}

void DrawerPolyline::updateVtkProp()
{
    if (m_vtkActor)
    {
        m_vtkPolydata->Reset();
        buildVtkPoints();
        updateVtkActorProperties();
        this->setModified(false);
    }
    else
    {
        DEBUG_LOG("No es pot actualitzar la polilínia, ja que encara no s'ha creat!");
    }
}

void DrawerPolyline::buildVtkPoints()
{
    if (!m_vtkPolydata)
    {
        m_vtkPolydata = vtkPolyData::New();
        m_vtkPoints = vtkPoints::New();
        m_vtkCellArray = vtkCellArray::New();
    }

    // Especifiquem el nombre de vèrtexs que té la polilinia
    int numberOfVertices = m_pointsList.count();
    m_vtkCellArray->InsertNextCell(numberOfVertices);
    m_vtkPoints->SetNumberOfPoints(numberOfVertices);

    // Donem els punts
    int i = 0;
    foreach (double *vertix, m_pointsList)
    {
        m_vtkPoints->InsertPoint(i, vertix);
        m_vtkCellArray->InsertCellPoint(i);
        i++;
    }

    // Assignem els punts al polydata
    m_vtkPolydata->SetPoints(m_vtkPoints);

    m_vtkPolydata->SetLines(m_vtkCellArray);
}

void DrawerPolyline::updateVtkActorProperties()
{
    // Sistema de coordenades
    m_vtkMapper->SetTransformCoordinate(this->getVtkCoordinateObject());
    // Estil de la línia
    m_vtkActor->GetProperty()->SetLineStipplePattern(m_linePattern);
    // Assignem gruix de la línia
    m_vtkActor->GetProperty()->SetLineWidth(m_lineWidth);
    // Assignem opacitat de la línia
    m_vtkActor->GetProperty()->SetOpacity(m_opacity);
    // Mirem la visibilitat de l'm_vtkActor
    m_vtkActor->SetVisibility(this->isVisible());
    // Assignem color
    QColor color = this->getColor();
    m_vtkActor->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
}

int DrawerPolyline::getNumberOfPoints()
{
    return m_pointsList.count();
}

double DrawerPolyline::getDistanceToPoint(double *point3D, double closestPoint[3])
{
    double minimumDistanceFound = MathTools::DoubleMaximumValue;

    if (!m_pointsList.isEmpty())
    {
        // Recorrem tots els punts de la polilínia calculant la distància a cadascun dels
        // segments que uneixen cada vèrtex
        double distance;
        double localClosestPoint[3];
        int i = 0;
        while (i < m_pointsList.count() - 1)
        {
            double startPoint[3] = { m_pointsList.at(i)[0], m_pointsList.at(i)[1], m_pointsList.at(i)[2] };
            double endPoint[3] = { m_pointsList.at(i + 1)[0], m_pointsList.at(i + 1)[1], m_pointsList.at(i + 1)[2] };
            distance = MathTools::getPointToFiniteLineDistance(point3D, startPoint, endPoint, localClosestPoint);
            if (distance < minimumDistanceFound)
            {
                minimumDistanceFound = distance;
                closestPoint[0] = localClosestPoint[0];
                closestPoint[1] = localClosestPoint[1];
                closestPoint[2] = localClosestPoint[2];
            }

            ++i;
        }
    }

    return minimumDistanceFound;
}

void DrawerPolyline::getBounds(double bounds[6])
{
    if (m_vtkPolydata)
    {
        m_vtkPolydata->GetBounds(bounds);
    }
    else
    {
        memset(bounds, 0.0, sizeof(double) * 6);
    }
}

QList<double *> DrawerPolyline::getPointsList()
{
    return m_pointsList;
}

}
