#include "polylinetemporalroitool.h"
#include "polylinetemporalroitooldata.h"
#include "q2dviewer.h"
#include "logging.h"
#include "drawerpolygon.h"
#include "drawer.h"
// Vtk
#include <vtkPoints.h>
#include <vtkLine.h>

namespace udg {

PolylineTemporalROITool::PolylineTemporalROITool(QViewer *viewer, QObject *parent)
 : PolylineROITool(viewer, parent)
{
    m_toolName = "PolylineTemporalROITool";
    m_hasSharedData = false;
    m_hasPersistentData = true;

    m_myData = new PolylineTemporalROIToolData;

    connect(this, SIGNAL(finished()), this, SLOT(start()));
}

PolylineTemporalROITool::~PolylineTemporalROITool()
{
}

ToolData *PolylineTemporalROITool::getToolData() const
{
    return m_myData;
}

void PolylineTemporalROITool::setToolData(ToolData *data)
{
    // Fem aquesta comparació perquè a vegades ens passa la data que ja tenim a m_myData
    if (m_myData != data)
    {
        // Creem de nou les dades
        m_toolData = data;
        m_myData = qobject_cast<PolylineTemporalROIToolData*>(data);
    }
}

void PolylineTemporalROITool::start()
{
    DEBUG_LOG("Start PolylineTemporalROI");
    double bounds[6];
    m_roiPolygon->getBounds(bounds);
    this->computeTemporalMean();
/*        double *intersection = new double[3];

        intersection[0] = (bounds[1]+bounds[0])/2.0;
        intersection[1] = (bounds[3]+bounds[2])/2.0;
        intersection[2] = (bounds[5]+bounds[4])/2.0;

        DrawerText * text = new DrawerText;

        const double * pixelSpacing = m_2DViewer->getInput()->getSeries()->getImage(0)->getPixelSpacing();

        if (pixelSpacing[0] == 0.0 && pixelSpacing[1] == 0.0)
        {
            double * spacing = m_2DViewer->getInput()->getSpacing();
            text->setText(tr("Area: %1 px2\nMean: %2").arg(m_roiPolygon->computeArea(m_2DViewer->getView(), spacing), 0, 'f', 0).arg(this->computeGrayMean(), 0, 'f', 2));
        }
        else
        {
            text->setText(tr("Area: %1 mm2\nMean: %2").arg(m_roiPolygon->computeArea(m_2DViewer->getView())).arg(this->computeGrayMean(), 0, 'f', 2));
        }

        text->setAttatchmentPoint(intersection);
        m_2DViewer->getDrawer()->draw(text, m_2DViewer->getView(), m_2DViewer->getCurrentSlice());
 */
}

void PolylineTemporalROITool::convertInputImageToTemporalImage()
{
    // Potser no cal i només recorrem l'input la regió que en marca la roi
    // d'aquesta manera no caldria fer la còpia
/*
    m_temporalImageHasBeenDefined = true;

    TemporalImageType::RegionType regiont;
    TemporalImageType::IndexType startt;
    startt[0]=0;
    startt[1]=0;
    startt[2]=0;
    startt[3]=0;

    TemporalImageType::SizeType sizet;
    sizet[0] = m_2DViewer->getInput()->getNumberOfPhases();  //les mostres temporals
    sizet[1] = m_2DViewer->getInput()->getItkData()->GetBufferedRegion().GetSize()[0];  //les X
    sizet[2] = m_2DViewer->getInput()->getItkData()->GetBufferedRegion().GetSize()[0];  //les Y
    sizet[3] = m_2DViewer->getInput()->getNumberOfSlicesPerPhase();  //les Z

    //Ho definim així perquè l'iterador passi per totes les mostres temporals
    regiont.SetSize(sizet);
    regiont.SetIndex(startt);

    m_temporalImage = TemporalImageType::New();
    m_temporalImage->SetRegions(regiont);
    m_temporalImage->Allocate();
*/
}

double PolylineTemporalROITool::computeTemporalMean()
{
    if (!m_myData->temporalImageHasBeenDefined())
    {
        return 0.0;
    }

    TemporalImageType::RegionType region = m_myData->getTemporalImage()->GetLargestPossibleRegion();
    // std::cout<<"Region="<<region<<std::endl;
    TemporalImageType::SizeType size = region.GetSize();
    // std::cout<<"Size="<<size<<std::endl;
    int temporalSize = size[0];
    QVector<double> mean (temporalSize);
    QVector<double> aux (temporalSize);
    int i, j;
    for (i = 0; i < temporalSize; i++)
    {
        mean[i] = 0.0;
    }

    int subId;
    int initialPosition;
    int endPosition;
    double intersectPoint[3];
    double *firstIntersection;
    double *secondIntersection;
    double pcoords[3];
    double t;
    double p0[3];
    double p1[3];
    int numberOfVoxels = 0;
    QList<double*> intersectionList;
    QList<int> indexList;
    vtkPoints *auxPoints;
    double rayP1[3];
    double rayP2[3];
    double verticalLimit;
    int currentView = m_2DViewer->getView();

    // El nombre de segments és el mateix que el nombre de punts del polígon
    int numberOfSegments = m_roiPolygon->getNumberOfPoints() - 1;

    // Taula de punters a vtkLine per a representar cadascun dels segments del polígon
    QVector<vtkLine*> segments;

    // Creem els diferents segments
    for (i = 0; i < numberOfSegments; i++)
    {
        vtkLine *line = vtkLine::New();
        line->GetPointIds()->SetNumberOfIds(2);
        line->GetPoints()->SetNumberOfPoints(2);

        const double *p1 = m_roiPolygon->getVertix(i);
        const double *p2 = m_roiPolygon->getVertix(i + 1);

        line->GetPoints()->InsertPoint(0, p1);
        line->GetPoints()->InsertPoint(1, p2);

        segments << line;
    }

    double bounds[6];
    m_roiPolygon->getBounds(bounds);
    double *spacing = m_2DViewer->getInput()->getSpacing();

    int rayPointIndex;
    int intersectionIndex;
    switch (currentView)
    {
    case Q2DViewer::Axial:
        // xmin
        rayP1[0] = bounds[0];
        // y
        rayP1[1] = bounds[2];
        // z
        rayP1[2] = bounds[4];
        // xmax
        rayP2[0] = bounds[1];
        // y
        rayP2[1] = bounds[2];
        // z
        rayP2[2] = bounds[4];

        rayPointIndex = 1;
        intersectionIndex = 0;
        verticalLimit = bounds[3];
    break;

    case Q2DViewer::Sagital:
        // xmin
        rayP1[0] = bounds[0];
        // ymin
        rayP1[1] = bounds[2];
        // zmin
        rayP1[2] = bounds[4];
        // xmin
        rayP2[0] = bounds[0];
        // ymin
        rayP2[1] = bounds[2];
        // zmax
        rayP2[2] = bounds[5];

        rayPointIndex = 1;
        intersectionIndex = 2;
        verticalLimit = bounds[3];
    break;

    case Q2DViewer::Coronal:
        // xmin
        rayP1[0] = bounds[0];
        // ymin
        rayP1[1] = bounds[2];
        // zmin
        rayP1[2] = bounds[4];
        // xmax
        rayP2[0] = bounds[1];
        // ymin
        rayP2[1] = bounds[2];
        // zmin
        rayP2[2] = bounds[4];

        rayPointIndex = 2;
        intersectionIndex = 0;
        verticalLimit = bounds[5];
    break;
    }

    while (rayP1[rayPointIndex] <= verticalLimit)
    {
        intersectionList.clear();
        indexList.clear();
        for (i = 0; i < numberOfSegments; i++)
        {
            auxPoints = segments[i]->GetPoints();
            auxPoints->GetPoint(0, p0);
            auxPoints->GetPoint(1, p1);
            if ((rayP1[rayPointIndex] <= p0[rayPointIndex] && rayP1[rayPointIndex] >= p1[rayPointIndex])
                || (rayP1[rayPointIndex] >= p0[rayPointIndex] && rayP1[rayPointIndex] <= p1[rayPointIndex]))
            {
                indexList << i;
            }
        }
        // Obtenim les interseccions entre tots els segments de la ROI i el raig actual
        foreach (int segment, indexList)
        {
            if (segments[segment]->IntersectWithLine(rayP1, rayP2, 0.0001, t, intersectPoint, pcoords, subId) > 0)
            {
                double *findedPoint = new double[3];
                findedPoint[0] = intersectPoint[0];
                findedPoint[1] = intersectPoint[1];
                findedPoint[2] = intersectPoint[2];
                intersectionList.append (findedPoint);
            }
        }

        if ((intersectionList.count() % 2) == 0)
        {
            int limit = intersectionList.count() / 2;
            for (i = 0; i < limit; i++)
            {
                initialPosition = i * 2;
                endPosition = initialPosition + 1;

                firstIntersection = intersectionList.at(initialPosition);
                secondIntersection = intersectionList.at(endPosition);

                // Tractem els dos sentits de les interseccions

                // D'esquerra cap a dreta
                if (firstIntersection[intersectionIndex] <= secondIntersection[intersectionIndex])
                {
                    while (firstIntersection[intersectionIndex] <= secondIntersection[intersectionIndex])
                    {
                        aux = this->getGraySerie(firstIntersection, temporalSize);
                        for (j = 0; j < temporalSize; j++)
                        {
                            mean[j] += aux[j];
                        }
                        numberOfVoxels++;
                        firstIntersection[intersectionIndex] += spacing[0];
                    }
                }
                // De dreta cap a esquerra
                else
                {
                    while (firstIntersection[intersectionIndex] >= secondIntersection[intersectionIndex])
                    {
                        aux = this->getGraySerie(firstIntersection, temporalSize);
                        for (j = 0; j < temporalSize; j++)
                        {
                            mean[j] += aux[j];
                        }
                        numberOfVoxels++;
                        firstIntersection[intersectionIndex] -= spacing[0];
                    }
                }
            }
        }
        else
        {
            DEBUG_LOG("EL NOMBRE D'INTERSECCIONS ENTRE EL RAIG I LA ROI Ã‰S IMPARELL!!");
        }

        // Fem el següent pas en la coordenada que escombrem
        rayP1[rayPointIndex] += spacing[1];
        rayP2[rayPointIndex] += spacing[1];

        while (!intersectionList.isEmpty())
        {
            double *intersection = intersectionList.takeFirst();
            delete[] intersection;
        }
    }

    // std::cout<<"Num of voxels:"<<numberOfVoxels<<std::endl;

    for (j = 0; j < temporalSize; j++)
    {
        mean[j] /= numberOfVoxels;
        // std::cout<<"i: "<<j<<": "<<mean[j]<<std::endl;
    }

    this->m_myData->setMeanVector(mean);

    // Destruim els diferents segments que hem creat per simular la roi
    for (i = 0; i < numberOfSegments; i++)
    {
        segments[i]->Delete();
    }

    return 0.0;
}

QVector<double> PolylineTemporalROITool::getGraySerie(double *coords, int size)
{
    QVector<double> v (size);
    double *origin = m_2DViewer->getInput()->getOrigin();
    double *spacing = m_2DViewer->getInput()->getSpacing();
    itk::Index<4> index;
    int xIndex, yIndex, zIndex;
    int i;

    Q2DViewer::getXYZIndexesForView(xIndex, yIndex, zIndex, m_2DViewer->getView());
    index[xIndex + 1] = (int)((coords[xIndex] - origin[xIndex])/spacing[xIndex]);
    index[yIndex + 1] = (int)((coords[yIndex] - origin[yIndex])/spacing[yIndex]);
    index[zIndex + 1] = m_2DViewer->getCurrentSlice();

    for (i = 0; i < size; i++)
    {
        index[0] = i;
        v[i] = m_myData->getTemporalImage()->GetPixel(index);
    }
    return v;
}

}
