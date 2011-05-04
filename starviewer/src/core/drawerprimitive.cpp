#include "drawerprimitive.h"
//vtk
#include <vtkCoordinate.h>

namespace udg {

DrawerPrimitive::DrawerPrimitive(QObject *parent)
: QObject(parent), m_internalRepresentation(VTKRepresentation), m_isVisible(true), m_coordinateSystem(WorldCoordinateSystem), m_color(QColor(255, 165, 0)), m_isFilled(false), m_linePattern(ContinuousLinePattern), m_lineWidth(1.0), m_opacity(1.0), m_modified(false), m_referenceCount(0)
{
    connect(this, SIGNAL(changed()), SLOT(setModified()));
}

DrawerPrimitive::~DrawerPrimitive()
{
}

void DrawerPrimitive::setVisibility(bool visible)
{
    m_isVisible = visible;
    emit changed();
}

void DrawerPrimitive::visibilityOn()
{
    setVisibility(true);
}

void DrawerPrimitive::visibilityOff()
{
    setVisibility(false);
}

bool DrawerPrimitive::isVisible() const
{
    return m_isVisible;
}

void DrawerPrimitive::setCoordinateSystem(int system)
{
    m_coordinateSystem = system;
    emit changed();
}

int DrawerPrimitive::getCoordinateSystem() const
{
    return m_coordinateSystem;
}

void DrawerPrimitive::setColor(QColor color)
{
    m_color = color;
    emit changed();
}

QColor DrawerPrimitive::getColor() const
{
    return m_color;
}

void DrawerPrimitive::setFilled(bool fill)
{
    m_isFilled = fill;
    emit changed();
}

bool DrawerPrimitive::isFilled() const
{
    return m_isFilled;
}

void DrawerPrimitive::setLinePattern(int pattern)
{
    m_linePattern = pattern;
    emit changed();
}

int DrawerPrimitive::getLinePattern() const
{
    return m_linePattern;
}

void DrawerPrimitive::setLineWidth(double width)
{
    m_lineWidth = width;
    emit changed();
}

double DrawerPrimitive::getLineWidth() const
{
    return m_lineWidth;
}

void DrawerPrimitive::setOpacity(double opacity)
{
    m_opacity = opacity;
    emit changed();
}

double DrawerPrimitive::getOpacity() const
{
    return m_opacity;
}

vtkProp* DrawerPrimitive::getAsVtkProp()
{
    return 0;
}

bool DrawerPrimitive::isModified() const
{
    return m_modified;
}

// HACK Mètodes de sucedani d'smart pointer
void DrawerPrimitive::increaseReferenceCount()
{
    m_referenceCount++;
}

void DrawerPrimitive::decreaseReferenceCount()
{
    if (m_referenceCount > 0)
    {
        m_referenceCount--;
    }
}

int DrawerPrimitive::getReferenceCount() const
{
    return m_referenceCount;
}

bool DrawerPrimitive::hasOwners() const
{
    return m_referenceCount > 0;
}

// FI Mètodes de sucedani d'smart pointer

void DrawerPrimitive::setModified(bool modified)
{
    m_modified = modified;
}

vtkCoordinate* DrawerPrimitive::getVtkCoordinateObject()
{
    vtkCoordinate *coordinate = vtkCoordinate::New();
    switch (m_coordinateSystem)
    {
        case WorldCoordinateSystem:
            coordinate->SetCoordinateSystemToWorld();
            break;

        case DisplayCoordinateSystem:
            coordinate->SetCoordinateSystemToDisplay();
            break;
    }
    return coordinate;
}

}
