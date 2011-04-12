/***************************************************************************
 *   Copyright (C) 2005-2007 by Grup de Gràfics de Girona                  *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#include "drawer.h"
#include "q2dviewer.h"
#include "drawerprimitive.h"
#include "logging.h"
#include "mathtools.h"
// vtk
#include <vtkRenderer.h>
#include <QColor>

namespace udg {

Drawer::Drawer(Q2DViewer *viewer, QObject *parent)
 : QObject(parent), m_currentPlane(0), m_currentSlice(0)
{
    m_2DViewer = viewer;
    connect(m_2DViewer, SIGNAL(sliceChanged(int)), SLOT(refresh()));
    connect(m_2DViewer, SIGNAL(viewChanged(int)), SLOT(refresh()));
}

Drawer::~Drawer()
{
}

void Drawer::draw(DrawerPrimitive *primitive, int plane, int slice)
{
    switch (plane)
    {
        case QViewer::AxialPlane:
            m_axialPrimitives.insert(slice, primitive);
            break;

        case QViewer::SagitalPlane:
            m_sagitalPrimitives.insert(slice, primitive);
            break;

        case QViewer::CoronalPlane:
            m_coronalPrimitives.insert(slice, primitive);
            break;

        case QViewer::Top2DPlane:
            m_top2DPlanePrimitives << primitive;
            break;

        default:
            DEBUG_LOG("Pla no definit!");
            return;
            break;
    }

    // En el cas que el pla sigui Axial/Sagital/Coronal, cal comprovar 
    // la visibilitat de la primitiva segons la llesca
    if (m_2DViewer->getView() == plane && plane != QViewer::Top2DPlane)
    {
        if (slice < 0 || m_2DViewer->getCurrentSlice() == slice)
        {
            primitive->setVisibility(true);
        }
        else
        {
            primitive->setVisibility(false);
        }
    }
    
    // Procedim a "pintar-la"
    vtkProp *prop = primitive->getAsVtkProp();
    if (prop)
    {
        connect(primitive, SIGNAL(dying(DrawerPrimitive *)), SLOT(erasePrimitive(DrawerPrimitive *)));
        m_2DViewer->getRenderer()->AddViewProp(prop);
        if (primitive->isVisible())
        {
            m_2DViewer->render();
        }
    }
}

void Drawer::clearViewer()
{
    QMultiMap<int, DrawerPrimitive*> primitivesContainer;
    switch (m_currentPlane)
    {
        case QViewer::AxialPlane:
            primitivesContainer = m_axialPrimitives;
            break;

        case QViewer::SagitalPlane:
            primitivesContainer = m_sagitalPrimitives;
            break;

        case QViewer::CoronalPlane:
            primitivesContainer = m_coronalPrimitives;
            break;

        case QViewer::Top2DPlane:
            DEBUG_LOG("Esborrar primitives de Top2dPlane no implementat!");
            return;
            break;

        default:
            DEBUG_LOG("Pla no definit!");
            return;
            break;
    }

    // Obtenim les primitives de la vista i llesca actuals
    QList<DrawerPrimitive*> list = primitivesContainer.values(m_currentSlice);
    // Eliminem totes aquelles primitives que estiguin a la llista i que no tinguin "propietaris"
    // Al fer delete es cridarà el mètode erasePrimitive() que ja s'encarrega de fer la "feina bruta"
    foreach (DrawerPrimitive *primitive, list)
    {
        if (!primitive->hasOwners())
        {
            delete primitive;
        }
    }
    m_2DViewer->render();
}

void Drawer::addToGroup(DrawerPrimitive *primitive, const QString &groupName)
{
    // No comprovem si ja existeix ni si està en cap altre de les llistes, no cal.
    m_primitiveGroups.insert(groupName, primitive);
}

void Drawer::refresh()
{
    if (m_currentPlane == m_2DViewer->getView())
    {
        if (m_currentSlice != m_2DViewer->getCurrentSlice())
        {
            // Cal fer invisible el que es veia en aquest pla i llesca i fer visible el que hi ha a la nova llesca
            hide(m_currentPlane, m_currentSlice);
            m_currentSlice = m_2DViewer->getCurrentSlice();
            show(m_currentPlane, m_currentSlice);
        }
    }
    else
    {
        // Cal fer invisible el que es veia en aquest pla i llesca i fer visible el que hi ha al nou pla i llesca
        hide(m_currentPlane, m_currentSlice);
        m_currentSlice = m_2DViewer->getCurrentSlice();
        m_currentPlane = m_2DViewer->getView();
        show(m_currentPlane, m_currentSlice);
    }
}

void Drawer::removeAllPrimitives()
{
    QList <DrawerPrimitive*> list = m_axialPrimitives.values();
    QList <DrawerPrimitive*> sagitalList = m_sagitalPrimitives.values();
    QList <DrawerPrimitive*> coronalList = m_coronalPrimitives.values();

    list += sagitalList;
    list += coronalList;
    list += m_top2DPlanePrimitives;

    foreach (DrawerPrimitive *primitive, list)
    {
        // TODO Atenció amb aquest tractament pel sucedani d'smart pointer.
        // Només esborrarem si ningú és propietari
        if (!primitive->hasOwners())
        {
            m_2DViewer->getRenderer()->RemoveViewProp(primitive->getAsVtkProp());
            delete primitive;
        }
    }
}

void Drawer::erasePrimitive(DrawerPrimitive *primitive)
{
    if (!primitive)
    {
        return;
    }
    // TODO Atenció amb aquest tractament pel sucedani d'smart pointer.
    // HACK Només esborrarem si ningú és propietari
    if (primitive->hasOwners())
    {
        DEBUG_LOG("No esborrem la primitiva. Tenim propietaris");
        return;
    }

    // Mirem si està en algun grup
    QMutableMapIterator<QString, DrawerPrimitive*> groupsIterator(m_primitiveGroups);
    while (groupsIterator.hasNext())
    {
        groupsIterator.next();
        if (primitive == groupsIterator.value())
        {
            groupsIterator.remove();
        }
    }

    // Busquem en el pla axial
    if (erasePrimitiveFromContainer(primitive, m_axialPrimitives))
    {
        // En principi una mateixa primitiva només estarà en una de les llistes
        return;
    }

    // Busquem en el pla sagital
    if (erasePrimitiveFromContainer(primitive, m_sagitalPrimitives))
    {
        return;
    }

    // Busquem en el pla coronal
    if (erasePrimitiveFromContainer(primitive, m_coronalPrimitives))
    {
        return;
    }

    // Busquem en la capa superior
    if (m_top2DPlanePrimitives.contains(primitive))
    {
        m_top2DPlanePrimitives.removeAt(m_top2DPlanePrimitives.indexOf(primitive));
        m_2DViewer->getRenderer()->RemoveViewProp(primitive->getAsVtkProp());
        m_2DViewer->render();
    }
}

void Drawer::hide(int plane, int slice)
{
    QList<DrawerPrimitive*> primitivesList;
    switch (plane)
    {
        case QViewer::AxialPlane:
            primitivesList = m_axialPrimitives.values(slice);
            break;

        case QViewer::SagitalPlane:
            primitivesList = m_sagitalPrimitives.values(slice);
            break;

        case QViewer::CoronalPlane:
            primitivesList = m_coronalPrimitives.values(slice);
            break;

        case QViewer::Top2DPlane:
            primitivesList = m_top2DPlanePrimitives;
            break;
    }
    foreach (DrawerPrimitive *primitive, primitivesList)
    {
        if (primitive->isVisible())
        {
            primitive->visibilityOff();
            primitive->update();
        }
    }
}

void Drawer::show(int plane, int slice)
{
    QList< DrawerPrimitive*> primitivesList;
    switch (plane)
    {
        case QViewer::AxialPlane:
            primitivesList = m_axialPrimitives.values(slice);
            break;

        case QViewer::SagitalPlane:
            primitivesList = m_sagitalPrimitives.values(slice);
            break;

        case QViewer::CoronalPlane:
            primitivesList = m_coronalPrimitives.values(slice);
            break;

        case QViewer::Top2DPlane:
            primitivesList = m_top2DPlanePrimitives;
            break;
    }
    
    foreach (DrawerPrimitive *primitive, primitivesList)
    {
        if (primitive->isModified() || !primitive->isVisible())
        {
            primitive->visibilityOn();
            primitive->update();
        }
    }
}

int Drawer::getNumberOfDrawnPrimitives()
{
    return (m_axialPrimitives.size() + m_sagitalPrimitives.size() + m_coronalPrimitives.size());
}

void Drawer::hideGroup(const QString &groupName)
{
    bool hasToRender = false;
    QList<DrawerPrimitive*> primitiveList = m_primitiveGroups.values(groupName);
    foreach (DrawerPrimitive *primitive, primitiveList)
    {
        if (primitive->isModified() || primitive->isVisible())
        {
            primitive->visibilityOff();
            primitive->update();
            hasToRender = true;
        }
    }
    
    if (hasToRender)
    {
        m_2DViewer->render();
    }
}

void Drawer::showGroup(const QString &groupName)
{
    bool hasToRender = false;
    QList<DrawerPrimitive*> primitiveList = m_primitiveGroups.values(groupName);
    foreach (DrawerPrimitive *primitive, primitiveList)
    {
        if (primitive->isModified() || !primitive->isVisible())
        {
            primitive->visibilityOn();
            primitive->update();
            hasToRender = true;
        }
    }
    
    if (hasToRender)
    {
        m_2DViewer->render();
    }
}

DrawerPrimitive* Drawer::getPrimitiveNearerToPoint(double point[3], int view, int slice)
{
    double distance;
    double minimumDistance = 1.0;
    QList<DrawerPrimitive*> primitivesList;

    DrawerPrimitive *nearestPrimitive = 0;

    switch (view)
    {
        case QViewer::AxialPlane:
            primitivesList = m_axialPrimitives.values(slice);
            break;

        case QViewer::SagitalPlane:
            primitivesList = m_sagitalPrimitives.values(slice);
            break;

        case QViewer::CoronalPlane:
            primitivesList = m_coronalPrimitives.values(slice);
            break;

        default:
            break;
    }

    foreach (DrawerPrimitive *primitive, primitivesList)
    {
        distance = primitive->getDistanceToPoint(point);
        if (distance <= minimumDistance)
        {
            minimumDistance = distance;
            nearestPrimitive = primitive;
        }
    }
    return nearestPrimitive;
}

void Drawer::erasePrimitivesInsideBounds(double bounds[6], int view, int slice)
{
    QList<DrawerPrimitive*> primitivesList;

    switch (view)
    {
        case QViewer::AxialPlane:
            primitivesList = m_axialPrimitives.values(slice);
            break;

        case QViewer::SagitalPlane:
            primitivesList = m_sagitalPrimitives.values(slice);
            break;

        case QViewer::CoronalPlane:
            primitivesList = m_coronalPrimitives.values(slice);
            break;

        default:
            break;
    }

    foreach (DrawerPrimitive *primitive, primitivesList)
    {
        if (isPrimitiveInside(primitive,view,bounds))
        {
            erasePrimitive(primitive);
        }
    }
}

bool Drawer::isPrimitiveInside(DrawerPrimitive *primitive, int view, double bounds[6])
{
    // Comprovem que els bounds de la primitiva estiguin continguts 
    // dins dels que ens han passat per paràmetre
    double primitiveBounds[6];
    primitive->getBounds(primitiveBounds);

    int xIndex = Q2DViewer::getXIndexForView(view);
    int yIndex = Q2DViewer::getYIndexForView(view);

    bool inside = false;
    if (bounds[xIndex * 2] <= primitiveBounds[xIndex * 2] && bounds[xIndex * 2 + 1] >= primitiveBounds[xIndex * 2 + 1] && bounds[yIndex * 2] <= primitiveBounds[yIndex * 2] && bounds[yIndex * 2 + 1] >= primitiveBounds[yIndex * 2 + 1])
        inside = true;

    return inside;
}

bool Drawer::erasePrimitiveFromContainer(DrawerPrimitive *primitive, QMultiMap<int, DrawerPrimitive*> &primitiveContainer)
{
    bool found = false;
    QMutableMapIterator<int, DrawerPrimitive*> containerIterator(primitiveContainer);
    while (containerIterator.hasNext() && !found)
    {
        containerIterator.next();
        if (primitive == containerIterator.value())
        {
            found = true;
            containerIterator.remove();
            m_2DViewer->getRenderer()->RemoveViewProp(primitive->getAsVtkProp());
        }
    }

    return found;
}


}
