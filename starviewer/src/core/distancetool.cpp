#include "distancetool.h"

#include "q2dviewer.h"
#include "logging.h"
#include "drawer.h"
#include "drawerline.h"
#include "drawertext.h"
#include "image.h"
#include "volume.h"
#include "series.h"
#include "measurementmanager.h"

// Vtk's
#include <vtkRenderWindowInteractor.h>
#include <vtkCommand.h>

namespace udg {

DistanceTool::DistanceTool(QViewer *viewer, QObject *parent)
 : Tool(viewer, parent)
{
    m_toolName = "DistanceTool";
    m_hasSharedData = false;

    m_2DViewer = qobject_cast<Q2DViewer*>(viewer);
    if (!m_2DViewer)
    {
        DEBUG_LOG(QString("El casting no ha funcionat!!! És possible que viewer no sigui un Q2DViewer!!!-> ") + viewer->metaObject()->className());
    }

    connect(m_2DViewer, SIGNAL(volumeChanged(Volume*)), SLOT(initialize()));
    initialize();
}

DistanceTool::~DistanceTool()
{
    deleteTemporalRepresentation();
}

void DistanceTool::deleteTemporalRepresentation()
{
    bool hasToRefresh = false;
    // Cal decrementar el reference count perquè
    // l'annotació s'esborri si "matem" l'eina
    if (m_line)
    {
        m_line->decreaseReferenceCount();
        delete m_line;
        hasToRefresh = true;
    }

    if (hasToRefresh)
    {
        m_2DViewer->render();
    }

    m_lineState = NoPointFixed;
}

void DistanceTool::handleEvent(long unsigned eventID)
{
    switch (eventID)
    {
        case vtkCommand::LeftButtonPressEvent:
            handlePointAddition();
            break;
        case vtkCommand::MouseMoveEvent:
            simulateLine();
            break;
        case vtkCommand::KeyPressEvent:
            int keyCode = m_2DViewer->getInteractor()->GetKeyCode();
            if (keyCode == 27) // ESC
            {
                deleteTemporalRepresentation();
            }
            break;
    }
}

double DistanceTool::computeDistance()
{
    double distance;
    // HACK Comprovem si l'imatge té pixel spacing per saber si la mesura ha d'anar en píxels o mm
    // TODO Proporcionar algun mètode alternatiu per no haver d'haver de fer aquest hack
    double *vtkSpacing = m_2DViewer->getInput()->getSpacing();
    const double *pixelSpacing = m_2DViewer->getInput()->getImage(0)->getPixelSpacing();

    if (pixelSpacing[0] == 0.0 && pixelSpacing[1] == 0.0)
    {
        distance = m_line->computeDistance(vtkSpacing);
    }
    else
    {
        bool distanceIsComputed = false;
        // En cas de Ultrasons es fa un tractament especial perquè VTK no agafa l'spacing correcte.
        // TODO S'hauria d'unificar.
        // Podem tenir imatges de la mateixa sèrie amb spacings diferents
        if (m_2DViewer->getInput()->getImage(0)->getParentSeries()->getModality() == "US")
        {
            Image *image = m_2DViewer->getCurrentDisplayedImage();
            if (image)
            {
                const double *usSpacing = image->getPixelSpacing();
                double *firstPoint = m_line->getFirstPoint();
                double *secondPoint = m_line->getSecondPoint();

                double xx = (firstPoint[0] - secondPoint[0]) / vtkSpacing[0] * usSpacing[0];
                double yy = (firstPoint[1] - secondPoint[1]) / vtkSpacing[1] * usSpacing[1];
                double value = std::pow(xx, 2) + std::pow(yy, 2);
                distance = std::sqrt(value);
                distanceIsComputed = true;
            }
            else
            {
                // S'ha aplicat una reconstrucció, per tant l'spacing que es donarà serà el de vtk
                // TODO Això en algun moment desapareixerà ja que caldria deshabilitar les reconstruccions per
                // modalitats en les que les reconstruccions no tinguin sentit
                distanceIsComputed = false;
            }
        }

        if (!distanceIsComputed)
        {
            distance = m_line->computeDistance();
        }
    }

    return distance;
}

QString DistanceTool::getMeasurementText()
{
    // Compute distance
    double distance = computeDistance();
    // Determine units and precision of the measurement
    int decimalPrecision = 2;
    Image *image = m_2DViewer->getInput()->getImage(0);
    if (MeasurementManager::getMeasurementUnits(image) == MeasurementManager::Pixels)
    {
        decimalPrecision = 0;
    }
    QString units = MeasurementManager::getMeasurementUnitsAsQString(image);

    return tr("%1 %2").arg(distance, 0, 'f', decimalPrecision).arg(units);
}

void DistanceTool::handlePointAddition()
{
    if (m_2DViewer->getInput())
    {
        if (m_2DViewer->getInteractor()->GetRepeatCount() == 0)
        {
            this->annotateNewPoint();
        }
    }
}

void DistanceTool::annotateNewPoint()
{
    if (!m_line)
    {
        m_line = new DrawerLine;
        // Així evitem que durant l'edició la primitiva pugui ser esborrada per events externs
        m_line->increaseReferenceCount();
    }

    double clickedWorldPoint[3];
    m_2DViewer->getEventWorldCoordinate(clickedWorldPoint);

    // Afegim el punt
    if (m_lineState == NoPointFixed)
    {
        m_line->setFirstPoint(clickedWorldPoint);
        m_line->setSecondPoint(clickedWorldPoint);
        m_lineState = FirstPointFixed;

        m_2DViewer->getDrawer()->draw(m_line);
    }
    else
    {
        m_line->setSecondPoint(clickedWorldPoint);
        m_line->update();

        // Posem el text
        DrawerText *text = new DrawerText;
        text->setText(getMeasurementText());

        // Coloquem el text a l'esquerra o a la dreta del segon punt segons la forma de la línia.
        int xIndex = Q2DViewer::getXIndexForView(m_2DViewer->getView());
        double *firstPoint = m_line->getFirstPoint();
        double *secondPoint = m_line->getSecondPoint();

        // Apliquem un padding de 5 pixels
        const double Padding = 5.0;
        double textPadding;
        if (firstPoint[xIndex] <= secondPoint[xIndex])
        {
            textPadding = Padding;
            text->setHorizontalJustification("Left");
        }
        else
        {
            textPadding = -Padding;
            text->setHorizontalJustification("Right");
        }

        double secondPointInDisplay[3];
        // Passem secondPoint a coordenades de display
        m_2DViewer->computeWorldToDisplay(secondPoint[0], secondPoint[1], secondPoint[2], secondPointInDisplay);
        // Apliquem el padding i tornem a coordenades de món
        double temporalWorldPoint[4];
        m_2DViewer->computeDisplayToWorld(secondPointInDisplay[0] + textPadding, secondPointInDisplay[1], secondPointInDisplay[2], temporalWorldPoint);
        double textPoint[3];
        textPoint[0] = temporalWorldPoint[0];
        textPoint[1] = temporalWorldPoint[1];
        textPoint[2] = temporalWorldPoint[2];

        text->setAttachmentPoint(textPoint);
        m_2DViewer->getDrawer()->draw(text, m_2DViewer->getView(), m_2DViewer->getCurrentSlice());
        
        // Alliberem la primitiva perquè pugui ser esborrada
        m_line->decreaseReferenceCount();
        m_2DViewer->getDrawer()->erasePrimitive(m_line);
        equalizeDepth();
        // Coloquem la primitiva en el pla corresponent
        m_2DViewer->getDrawer()->draw(m_line, m_2DViewer->getView(), m_2DViewer->getCurrentSlice());
        // Reiniciem l'estat de la tool
        m_lineState = NoPointFixed;
        m_line = NULL;
    }
}

void DistanceTool::simulateLine()
{
    if (m_line)
    {
        double clickedWorldPoint[3];
        m_2DViewer->getEventWorldCoordinate(clickedWorldPoint);

        m_line->setSecondPoint(clickedWorldPoint);
        m_line->update();
        m_2DViewer->render();
    }
}

void DistanceTool::initialize()
{
    if (m_line)
    {
        // Així alliberem la primitiva perquè pugui ser esborrada
        m_line->decreaseReferenceCount();
        delete m_line;
    }
    m_lineState = NoPointFixed;
    m_line = NULL;
}

void DistanceTool::equalizeDepth()
{
    // Assignem al primer punt la z del segon
    int zIndex = Q2DViewer::getZIndexForView(m_2DViewer->getView());
    double z = m_line->getSecondPoint()[zIndex];
    double *firstPoint = m_line->getFirstPoint();
    firstPoint[zIndex] = z;
    m_line->setFirstPoint(firstPoint);
    m_line->update();
}

}
