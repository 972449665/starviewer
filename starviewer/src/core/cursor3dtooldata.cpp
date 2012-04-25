#include "cursor3dtooldata.h"
#include "imageplane.h"

namespace udg {

Cursor3DToolData::Cursor3DToolData(QObject *parent)
 : ToolData(parent), m_referenceImagePlane(0)
{
    m_originPointPosition = new double[3];
}

Cursor3DToolData::~Cursor3DToolData()
{
    delete[] m_originPointPosition;
}

QString Cursor3DToolData::getFrameOfReferenceUID() const
{
    return m_frameOfReferenceUID;
}

ImagePlane* Cursor3DToolData::getImagePlane() const
{
    return m_referenceImagePlane;
}

double* Cursor3DToolData::getOriginPointPosition() const
{
    return m_originPointPosition;
}

void Cursor3DToolData::setFrameOfReferenceUID(const QString &frameOfReference)
{
    m_frameOfReferenceUID = frameOfReference;
}

void Cursor3DToolData::setImagePlane(ImagePlane *imagePlane)
{
    m_referenceImagePlane = imagePlane;
    emit changed();
}

void Cursor3DToolData::setOriginPointPosition(double position[3])
{
    for (int i = 0; i < 3; i++)
    {
        m_originPointPosition[i] = position[i];
    }
    emit changed();
}

QString Cursor3DToolData::getInstanceUID() const
{
    return m_instanceUID;
}

void Cursor3DToolData::setInstanceUID(const QString &instanceUID)
{
    m_instanceUID = instanceUID;
}

void Cursor3DToolData::hideCursors()
{
    emit turnOffCursor();
}

}
