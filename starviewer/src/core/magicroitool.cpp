#include "magicroitool.h"
#include "logging.h"
#include "q2dviewer.h"
#include "volume.h"
#include "drawer.h"
#include "drawerpolygon.h"
#include "drawertext.h"
#include "mathtools.h"

#include <qmath.h>

// Vtk
#include <vtkCommand.h>
#include <vtkRenderWindowInteractor.h>

namespace udg {

const int MagicROITool::MagicSize = 3;
const double MagicROITool::InitialMagicFactor = 0.0;

MagicROITool::MagicROITool(QViewer *viewer, QObject *parent)
: ROITool(viewer, parent)
{
    m_magicFactor = InitialMagicFactor;
    m_minX = 0;
    m_maxX = 0;
    m_minY = 0;
    m_maxY = 0;
    m_lowerLevel = 0.0;
    m_upperLevel = 0.0;
    m_toolName = "MagicROITool";

    m_roiPolygon = NULL;
    m_filledRoiPolygon = NULL;

    connect(m_2DViewer, SIGNAL(volumeChanged(Volume*)), SLOT(initialize()));
    connect(m_2DViewer, SIGNAL(sliceChanged(int)), SLOT(restartRegion()));
    connect(m_2DViewer, SIGNAL(phaseChanged(int)), SLOT(restartRegion()));
}

MagicROITool::~MagicROITool()
{
    deleteTemporalRepresentation();
}

void MagicROITool::deleteTemporalRepresentation()
{
    if (!m_roiPolygon.isNull())
    {
        m_roiPolygon->decreaseReferenceCount();
        delete m_roiPolygon;
        m_2DViewer->render();
    }
    if (!m_filledRoiPolygon.isNull())
    {
        m_filledRoiPolygon->decreaseReferenceCount();
        delete m_filledRoiPolygon;
        m_2DViewer->render();
    }
}

void MagicROITool::initialize()
{
    // Alliberem les primitives perquè puguin ser esborrades
    if (!m_roiPolygon.isNull())
    {
        m_roiPolygon->decreaseReferenceCount();
        delete m_roiPolygon;
        m_2DViewer->render();
    }
    if (!m_filledRoiPolygon.isNull())
    {
        m_filledRoiPolygon->decreaseReferenceCount();
        delete m_filledRoiPolygon;
        m_2DViewer->render();
    }

    m_roiPolygon = NULL;
    m_filledRoiPolygon = NULL;
}

void MagicROITool::handleEvent(unsigned long eventID)
{
    switch (eventID)
    {
        case vtkCommand::LeftButtonPressEvent:
            startRegion();
            break;
        case vtkCommand::LeftButtonReleaseEvent:
            endRegion();
            break;
        case vtkCommand::MouseMoveEvent:
            modifyRegionByFactor();
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

void MagicROITool::setTextPosition(DrawerText *text)
{
    double bounds[6];
    m_roiPolygon->getBounds(bounds);

    int xIndex, yIndex, zIndex;
    m_2DViewer->getView().getXYZIndexes(xIndex, yIndex, zIndex);

    double attachmentPoint[3];
    attachmentPoint[xIndex] = (bounds[xIndex * 2] + bounds[xIndex * 2 + 1]) / 2.0;
    attachmentPoint[zIndex] = bounds[zIndex * 2];
    if (m_2DViewer->getView() == OrthogonalPlane::XYPlane)
    {
        attachmentPoint[yIndex] = bounds[yIndex * 2 + 1];
    }
    else
    {
        attachmentPoint[yIndex] = bounds[yIndex * 2];
    }

    const double Padding = 5.0;
    double paddingY = 0.0;

    paddingY = -Padding;
    text->setVerticalJustification("Top");

    double attachmentPointInDisplay[3];
    // Passem attachmentPoint a coordenades de display
    m_2DViewer->computeWorldToDisplay(attachmentPoint[0], attachmentPoint[1], attachmentPoint[2], attachmentPointInDisplay);
    // Apliquem el padding i tornem a coordenades de món
    m_2DViewer->computeDisplayToWorld(attachmentPointInDisplay[0], attachmentPointInDisplay[1] + paddingY, attachmentPointInDisplay[2], attachmentPoint);

    text->setAttachmentPoint(attachmentPoint);
}

void MagicROITool::computeMaskBounds()
{
    int extent[6];
    m_2DViewer->getInput()->getWholeExtent(extent);

    int xIndex, yIndex, zIndex;
    m_2DViewer->getView().getXYZIndexes(xIndex, yIndex, zIndex);

    m_minX = extent[xIndex * 2];
    m_maxX = extent[(xIndex * 2) + 1];
    m_minY = extent[yIndex * 2];
    m_maxY = extent[(yIndex * 2) + 1];
}

double MagicROITool::getVoxelValue(int x, int y, int z, VolumePixelData *pixelData)
{
    int xIndex, yIndex, zIndex;
    m_2DViewer->getView().getXYZIndexes(xIndex, yIndex, zIndex);

    int index[3];
    index[xIndex] = x;
    index[yIndex] = y;
    index[zIndex] = z;

    double value = 0;

    value = pixelData->getScalarComponentAsDouble(index[0], index[1], index[2]);

    return value;

}

void MagicROITool::startRegion()
{
    if (m_2DViewer->hasInput())
    {
        if (m_2DViewer->getCurrentCursorImageCoordinate(m_pickedPosition))
        {
            m_pickedPositionInDisplayCoordinates = m_2DViewer->getEventPosition();
            m_magicFactor = InitialMagicFactor;
            m_roiPolygon = new DrawerPolygon;
            m_roiPolygon->increaseReferenceCount();
            m_2DViewer->getDrawer()->draw(m_roiPolygon);
            m_filledRoiPolygon = new DrawerPolygon();
            m_filledRoiPolygon->increaseReferenceCount();
            m_filledRoiPolygon->setFilled(true);
            m_filledRoiPolygon->setOpacity(0.5);
            m_2DViewer->getDrawer()->draw(m_filledRoiPolygon);

            this->generateRegion();
        }
    }
}

void MagicROITool::endRegion()
{
    if (m_roiPolygon)
    {
        this->generateRegion();
        this->printData();
        // Alliberem la primitiva perquè es pugui esborrar
        m_roiPolygon->decreaseReferenceCount();
        // Col·loquem el dibuix al lloc corresponent
        m_2DViewer->getDrawer()->erasePrimitive(m_roiPolygon);
        m_2DViewer->getDrawer()->draw(m_roiPolygon, m_2DViewer->getView(), m_2DViewer->getCurrentSlice());
        // Re-iniciem el punter
        m_roiPolygon = NULL;
    }

    if (m_filledRoiPolygon)
    {
        // Alliberem la primitiva perquè es pugui esborrar
        m_filledRoiPolygon->decreaseReferenceCount();
        // Esborrem el polígon ple del visor i el destruïm
        m_2DViewer->getDrawer()->erasePrimitive(m_filledRoiPolygon);
        delete m_filledRoiPolygon;
        m_filledRoiPolygon = NULL;
    }
}

void MagicROITool::restartRegion()
{
    if (!m_filledRoiPolygon.isNull())
    {
        m_filledRoiPolygon->decreaseReferenceCount();
        delete m_filledRoiPolygon;
    }

    if (!m_roiPolygon.isNull())
    {
        m_roiPolygon->decreaseReferenceCount();
        delete m_roiPolygon;
        
        startRegion();
    }
}

void MagicROITool::modifyRegionByFactor()
{
    if (m_roiPolygon)
    {
        const double ScaleFactor = 0.05;

        // Fem servir la distància al punt inicial que s'ha clicat. Es fa la distància de mahattan que és una bona aproximació i molt més ràpida de calcular
        int displacement =  (m_viewer->getEventPosition() - m_pickedPositionInDisplayCoordinates).manhattanLength();
        m_magicFactor = displacement * ScaleFactor;
        if (m_magicFactor < 0.0)
        {
            m_magicFactor = 0.0;
        }

        this->generateRegion();
    }
}

void MagicROITool::generateRegion()
{
    computeMaskBounds();
    
    VolumePixelData *pixelData = m_2DViewer->getInput()->getPixelData();
    if (m_2DViewer->isThickSlabActive())
    {
        pixelData = new VolumePixelData();
        pixelData->setData(m_2DViewer->getCurrentSlabProjection());
    }

    this->computeLevelRange(pixelData);

    // Posem a true els punts on la imatge està dins els llindard i connectat amb la llavor (region growing)
    this->computeRegionMask(pixelData);

    if (m_2DViewer->isThickSlabActive())
    {
        delete pixelData;
    }
    
    // Trobem els punts frontera i creem el polígon
    this->computePolygon();

    m_2DViewer->render();
}

void MagicROITool::computeLevelRange(VolumePixelData *pixelData)
{
    if (!pixelData)
    {
        return;
    }
    
    int index[3];
    pixelData->computeCoordinateIndex(m_pickedPosition, index);

    int xIndex, yIndex, zIndex;
    m_2DViewer->getView().getXYZIndexes(xIndex, yIndex, zIndex);

    int x = index[xIndex];
    int y = index[yIndex];
    // HACK Per poder crear les regions correctament quan tenim imatges amb fases
    // Com que els volums no suporten recontruccions, només hem de tractar el cas Axial
    // TODO Revisar això quan s'implementi el ticket #1247 (Suportar reconstruccions per volums amb fases)
    int z;
    if (m_2DViewer->getView() == OrthogonalPlane::XYPlane && !m_2DViewer->isThickSlabActive())
    {
        z = m_2DViewer->getInput()->getImageIndex(m_2DViewer->getCurrentSlice(), m_2DViewer->getCurrentPhase());
    }
    else
    {
        z = index[zIndex];
    }
    // Calculem la desviació estàndard dins la finestra que ens marca la magic size
    double standardDeviation = getStandardDeviation(x, y, z, pixelData);
    
    // Calculem els llindars com el valor en el pixel +/- la desviació estàndard * magic factor
    double value = this->getVoxelValue(x, y, z, pixelData);
    m_lowerLevel = value - m_magicFactor * standardDeviation;
    m_upperLevel = value + m_magicFactor * standardDeviation;
}

void MagicROITool::computeRegionMask(VolumePixelData *pixelData)
{
    // Busquem el voxel inicial
    int index[3];
    pixelData->computeCoordinateIndex(m_pickedPosition, index);
    
    int xIndex, yIndex, zIndex;
    m_2DViewer->getView().getXYZIndexes(xIndex, yIndex, zIndex);
    
    int x = index[xIndex];
    int y = index[yIndex];
    int z;
    // HACK Per poder crear les regions correctament quan tenim imatges amb fases
    // Com que els volums no suporten recontruccions, només hem de tractar el cas Axial
    // TODO Revisar això quan s'implementi el ticket #1247 (Suportar reconstruccions per volums amb fases)
    if (m_2DViewer->getView() == OrthogonalPlane::XYPlane && !m_2DViewer->isThickSlabActive())
    {
        z = m_2DViewer->getInput()->getImageIndex(m_2DViewer->getCurrentSlice(), m_2DViewer->getCurrentPhase());
    }
    else
    {
        z = index[zIndex];
    }

    // Creem la màscara
    if (m_minX == 0 && m_minY == 0)
    {
        m_mask = QVector<bool>((m_maxX + 1) * (m_maxY + 1), false);
    }
    else
    {
        DEBUG_LOG("ERROR: extension no comença a 0");
    }
    
    // TODO Desfà els índexs projectats a 2D als originals 3D per poder obtenir el valor
    // Corretgir-ho d'una millor manera perquè no calgui fer servir aquest mètode (guardar els índexs x,y,z o d'una altra manera)
    double value = this->getVoxelValue(x, y, z, pixelData);
    
    if ((value >= m_lowerLevel) && (value <= m_upperLevel))
    {
        int maskIndex = computeMaskVectorIndex(x, y, m_maxX);
        m_mask[maskIndex] = true;
    }
    else
    {
        DEBUG_LOG("Ha petat i sortim");
        return;
    }

    // Comencem el Region Growing
    QVector<int> movements;
    // First movement \TODO Codi duplicat amb main loop
    int i = 0;
    bool found = false;
    int maskIndex = 0;
    while (i < 4 && !found)
    {
        this->doMovement(x, y, i);
        // TODO Desfà els índexs projectats a 2D als originals 3D per poder obtenir el valor
        // Corretgir-ho d'una millor manera perquè no calgui fer servir aquest mètode (guardar els índexs x,y,z o d'una altra manera)
        value = this->getVoxelValue(x, y, z, pixelData);

        if ((value >= m_lowerLevel) && (value <= m_upperLevel))
        {
            maskIndex = computeMaskVectorIndex(x, y, m_maxX);
            m_mask[maskIndex] = true;
            found = true;
            movements.push_back(i);
        }
        if (!found)
        {
            this->undoMovement(x, y, i);
        }
        ++i;
    }

    // Main loop
    i = 0;
    while (movements.size() > 0)
    {
        found = false;
        while (i < 4 && !found)
        {
            this->doMovement(x, y, i);
            if ((x > m_minX) && (x < m_maxX) && (y > m_minY) && (y < m_maxY))
            {
                // TODO Desfà els índexs projectats a 2D als originals 3D per poder obtenir el valor
                // Corretgir-ho d'una millor manera perquè no calgui fer servir aquest mètode (guardar els índexs x,y,z o d'una altra manera)
                value = this->getVoxelValue(x, y, z, pixelData);
                maskIndex = computeMaskVectorIndex(x, y, m_maxX);
                if ((value >= m_lowerLevel) && (value <= m_upperLevel) && (!m_mask[maskIndex]))
                {
                    m_mask[maskIndex] = true;
                    found = true;
                    movements.push_back(i);
                    i = 0;
                }
            }
            if (!found)
            {
                this->undoMovement(x, y, i);
                ++i;
            }
        }
        if (!found)
        {
            this->undoMovement(x, y, movements.back());
            i = movements.back();
            ++i;
            movements.pop_back();
        }
    }
}

void MagicROITool::doMovement(int &x, int &y, int movement)
{
    switch (movement)
    {
        case MoveRight:
            x++;
            break;
        case MoveLeft:
            x--;
            break;
        case MoveUp:
            y++;
            break;
        case MoveDown:
            y--;
            break;
        default:
            DEBUG_LOG("Invalid movement");
    }
}

void MagicROITool::undoMovement(int &x, int &y, int movement)
{
    switch (movement)
    {
        case MoveRight:
            x--;
            break;
        case MoveLeft:
            x++;
            break;
        case MoveUp:
            y--;
            break;
        case MoveDown:
            y++;
            break;
        default:
            DEBUG_LOG("Invalid movement");
    }
}

void MagicROITool::computePolygon()
{
    int i = m_minX;
    int j;
    int maskIndex = 0;
    // Busquem el primer punt
    bool found = false;
    while ((i <= m_maxX) && !found)
    {
        j = m_minY;
        while ((j <= m_maxY) && !found)
        {
            maskIndex = computeMaskVectorIndex(i, j, m_maxX);
            if (m_mask[maskIndex])
            {
                found = true;
            }
            ++j;
        }
        ++i;
    }

    // L'índex és -1 pq els hem incrementat una vegada més    
    int x = i - 1;
    int y = j - 1;
    int z = m_2DViewer->getCurrentSlice();
    m_roiPolygon->removeVertices();
    m_filledRoiPolygon->removeVertices();
    
    this->addPoint(7, x, y, z);
    this->addPoint(1, x, y, z);
    
    int nextX;
    int nextY;
    int nextZ = z;

    int direction = 0;

    bool loop = false;
    bool next = false;
    while (!loop)
    {
        this->getNextIndex(direction, x, y, nextX, nextY);
        maskIndex = computeMaskVectorIndex(nextX, nextY, m_maxX);
        next = m_mask[maskIndex];
        while (!next && !loop)
        {
            if (MathTools::isOdd(direction) && !next)
            {
                this->addPoint(direction, x, y, z);
                loop = this->isLoopReached();
            }
            direction = this->getNextDirection(direction);
            this->getNextIndex(direction, x, y, nextX, nextY);
            maskIndex = computeMaskVectorIndex(nextX, nextY, m_maxX);
            next = m_mask[maskIndex];
        }
        x = nextX;
        y = nextY;
        direction = this->getInverseDirection(direction);
        direction = this->getNextDirection(direction);
    }

    m_roiPolygon->update();
    m_filledRoiPolygon->update();
    m_hasToComputeStatisticsData = true;
}

void MagicROITool::getNextIndex(int direction, int x, int y, int &nextX, int &nextY)
{
    switch (direction)
    {
        case LeftDown:
            nextX = x - 1;
            nextY = y - 1;
            break;
        case Down:
            nextX = x;
            nextY = y - 1;
            break;
        case RightDown:
            nextX = x + 1;
            nextY = y - 1;
            break;
        case Right:
            nextX = x + 1;
            nextY = y;
            break;
        case RightUp:
            nextX = x + 1;
            nextY = y + 1;
            break;
        case Up:
            nextX = x;
            nextY = y + 1;
            break;
        case LeftUp:
            nextX = x - 1;
            nextY = y + 1;
            break;
        case Left:
            nextX = x - 1;
            nextY = y;
            break;
        default:
            DEBUG_LOG("ERROR: This direction doesn't exist");
    }
}

int MagicROITool::getNextDirection(int direction)
{
    int nextDirection = direction + 1;
    return (nextDirection == 8)? 0: nextDirection;
}

int MagicROITool::getInverseDirection(int direction)
{
    return (direction + 4) % 8;
}

void MagicROITool::addPoint(int direction, int x, int y, double z)
{    
    double origin[3];
    double spacing[3];
    m_2DViewer->getInput()->getSpacing(spacing);
    m_2DViewer->getInput()->getOrigin(origin);

    int xIndex, yIndex, zIndex;
    m_2DViewer->getView().getXYZIndexes(xIndex, yIndex, zIndex);
 
    double point[3];
    switch (direction)
    {
        case Down:
            point[xIndex] = x * spacing[xIndex] + origin[xIndex];
            point[yIndex] = (y - 0.5) * spacing[yIndex] + origin[yIndex];
            break;
        case Right:
            point[xIndex] = (x + 0.5) * spacing[xIndex] + origin[xIndex];
            point[yIndex] = y * spacing[yIndex] + origin[yIndex];
            break;
        case Up:
            point[xIndex] = x * spacing[xIndex] + origin[xIndex];
            point[yIndex] = (y + 0.5) * spacing[yIndex] + origin[yIndex];
            break;
        case Left:
            point[xIndex] = (x - 0.5) * spacing[xIndex] + origin[xIndex];
            point[yIndex] = y * spacing[yIndex] + origin[yIndex];
            break;
        default:
            DEBUG_LOG("ERROR: This direction doesn't exist");
    }
    point[zIndex] = z * spacing[zIndex] + origin[zIndex];

    m_roiPolygon->addVertix(point);
    m_filledRoiPolygon->addVertix(point);
}

bool MagicROITool::isLoopReached()
{
    const double *firstVertix = this->m_roiPolygon->getVertix(0);
    const double *lastVertix = this->m_roiPolygon->getVertix(m_roiPolygon->getNumberOfPoints() - 1);
    return ((qAbs(firstVertix[0] - lastVertix[0]) < 0.0001) && (qAbs(firstVertix[1] - lastVertix[1]) < 0.0001));
}

double MagicROITool::getStandardDeviation(int x, int y, int z, VolumePixelData *pixelData)
{
    int minX = qMax(x - MagicSize, m_minX);
    int maxX = qMin(x + MagicSize, m_maxX);
    int minY = qMax(y - MagicSize, m_minY);
    int maxY = qMin(y + MagicSize, m_maxY);

    // Calculem la mitjana
    double mean = 0.0;
    double value;

    for (int i = minX; i <= maxX; ++i)
    {
        for (int j = minY; j <= maxY; ++j)
        {
            value = this->getVoxelValue(i, j, z, pixelData);
            mean += value;
        }
    }

    int numberOfSamples = (maxX - minX + 1) * (maxY - minY + 1);
    mean = mean / (double)numberOfSamples;

    // Calculem la desviació estandard
    double deviation = 0.0;
    for (int i = minX; i <= maxX; ++i)
    {
        for (int j = minY; j <= maxY; ++j)
        {
            value = this->getVoxelValue(i, j, z, pixelData);
            deviation += qPow(value - mean, 2);
        }
    }
    deviation = qSqrt(deviation / (double)numberOfSamples);
    return deviation;
}

int MagicROITool::computeMaskVectorIndex(int x, int y, int columns)
{
    return y * columns + x;
}

}
