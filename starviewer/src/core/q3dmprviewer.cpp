#include "q3dmprviewer.h"

#include "windowlevelpresetstooldata.h"
#include "transferfunction.h"

// Includes vtk
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkCellPicker.h>
#include <vtkProperty.h>
#include <vtkImagePlaneWidget.h>
#include <vtkInteractorStyle.h>
#include <vtkCommand.h>
#include <vtkWindowToImageFilter.h>
#include <vtkLookupTable.h>

// Per crear la bounding box del model
#include <vtkOutlineFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>

// Per el marcador d'orientació
#include "q3dorientationmarker.h"

// Includes propis
#include "volume.h"
#include "image.h"
#include "logging.h"

namespace udg {

class PlanesInteractionCallback : public vtkCommand {
public:
    static PlanesInteractionCallback* New()
    {
        return new PlanesInteractionCallback;
    }
    Q3DMPRViewer *m_viewer;
    virtual void Execute(vtkObject *vtkNotUsed(caller), unsigned long event, void *vtkNotUsed(callData))
    {
        static double lastWindowLevel[2] = { 0., 0. };
        if (m_viewer)
        {
            switch (event)
            {
                case vtkCommand::StartInteractionEvent:
                    m_viewer->getCurrentWindowLevel(lastWindowLevel);
                    break;

                case vtkCommand::EndInteractionEvent:
                    break;

                case vtkCommand::InteractionEvent:
                    // Actualitzem únicament si ha canviat el window level
                    double wl[2];
                    m_viewer->getCurrentWindowLevel(wl);
                    if (wl[0] != lastWindowLevel[0] || wl[1] != lastWindowLevel[1])
                    {
                        m_viewer->getWindowLevelData()->setCustomWindowLevel(wl[0], wl[1]);
                    }
                    break;
            }
            m_viewer->planeInteraction();
        }
    }
};

Q3DMPRViewer::Q3DMPRViewer(QWidget *parent)
 : QViewer(parent)
{
    this->initializePlanes();
    // Assignem el color de fons de pantalla
    // TODO Podríem tenir un mètode genèric per fer això
    m_renderer->SetBackground(0.4392, 0.5020, 0.5647);
    // Interacció
    m_axialPlaneVisible = true;
    m_sagitalPlaneVisible = true;
    m_coronalPlaneVisible = true;

    m_outlineActor = 0;
    m_orientationMarker = 0;
    m_axialResliced = 0;
    m_sagitalResliced = 0;
    m_coronalResliced = 0;

    this->createActors();
    this->addActors();
}

Q3DMPRViewer::~Q3DMPRViewer()
{
    m_axialImagePlaneWidget->Delete();
    m_sagitalImagePlaneWidget->Delete();
    m_coronalImagePlaneWidget->Delete();
    m_outlineActor->Delete();
    delete m_orientationMarker;
}

void Q3DMPRViewer::setInput(Volume *volume)
{
    m_mainVolume = volume;
    this->createOutline();
    // Li proporcionem les dades als plans
    this->updatePlanesData();
    // Ajustem els valors del window Level per defecte
    this->updateWindowLevelData();
    // Li donem la orientació per defecte
    this->resetViewToAcquisitionPlane();
    render();
    // Indiquem el canvi de volum
    emit volumeChanged(getMainInput());
}

void Q3DMPRViewer::createActors()
{
    m_outlineActor = vtkActor::New();
    m_orientationMarker = new Q3DOrientationMarker(this->getInteractor());
}

void Q3DMPRViewer::addActors()
{
    if (!m_outlineActor)
    {
        DEBUG_LOG("Error! Intentant afegir actors que no s'han creat encara");
    }
    else
    {
        m_renderer->AddViewProp(m_outlineActor);
    }
}

void Q3DMPRViewer::getCurrentRenderedItemBounds(double bounds[6])
{
    m_outlineActor->GetBounds(bounds);
}

void Q3DMPRViewer::initializePlanes()
{
    vtkCellPicker *picker = vtkCellPicker::New();
    picker->SetTolerance(0.005);

    // Assignem les propietats per defecte
    vtkProperty *ipwProp = vtkProperty::New();

    // Creem tres vistes ortogonals utilitzant la classe ImagePlaneWidget
    //
    m_axialImagePlaneWidget = vtkImagePlaneWidget::New();
    m_sagitalImagePlaneWidget = vtkImagePlaneWidget::New();
    m_coronalImagePlaneWidget = vtkImagePlaneWidget::New();
    // Els 3 widgets s'utilizen per visualizar el model
    // (mostra imatges en 2D amb 3 orientacions diferents)

    // Pla AXIAL
    m_axialImagePlaneWidget->DisplayTextOn();
    m_axialImagePlaneWidget->SetPicker(picker);
    m_axialImagePlaneWidget->RestrictPlaneToVolumeOn();
    m_axialImagePlaneWidget->SetKeyPressActivationValue('z');
    m_axialImagePlaneWidget->GetPlaneProperty()->SetColor(1., 1., .0);
    m_axialImagePlaneWidget->SetTexturePlaneProperty(ipwProp);
    m_axialImagePlaneWidget->TextureInterpolateOn();
    m_axialImagePlaneWidget->SetResliceInterpolateToCubic();

    // Pla SAGITAL
    m_sagitalImagePlaneWidget->DisplayTextOn();
    m_sagitalImagePlaneWidget->SetPicker(picker);
    m_sagitalImagePlaneWidget->RestrictPlaneToVolumeOn();
    m_sagitalImagePlaneWidget->SetKeyPressActivationValue('x');
    m_sagitalImagePlaneWidget->GetPlaneProperty()->SetColor(1., .6, .0);
    m_sagitalImagePlaneWidget->SetTexturePlaneProperty(ipwProp);
    m_sagitalImagePlaneWidget->TextureInterpolateOn();
    m_sagitalImagePlaneWidget->SetLookupTable(m_axialImagePlaneWidget->GetLookupTable());
    m_sagitalImagePlaneWidget->SetResliceInterpolateToCubic();

    // Pla CORONAL
    m_coronalImagePlaneWidget->DisplayTextOn();
    m_coronalImagePlaneWidget->SetPicker(picker);
    m_coronalImagePlaneWidget->SetKeyPressActivationValue('y');
    m_coronalImagePlaneWidget->GetPlaneProperty()->SetColor(.0, 1., 1.);
    m_coronalImagePlaneWidget->SetTexturePlaneProperty(ipwProp);
    m_coronalImagePlaneWidget->TextureInterpolateOn();
    m_coronalImagePlaneWidget->SetLookupTable(m_axialImagePlaneWidget->GetLookupTable());
    m_coronalImagePlaneWidget->SetResliceInterpolateToCubic();

    picker->Delete();
    ipwProp->Delete();

    // INTERACCIÓ
    m_axialImagePlaneWidget->SetInteractor(getInteractor());
    m_sagitalImagePlaneWidget->SetInteractor(getInteractor());
    m_coronalImagePlaneWidget->SetInteractor(getInteractor());

    PlanesInteractionCallback *planesInteractionCallback = PlanesInteractionCallback::New();
    planesInteractionCallback->m_viewer = this;
//     m_axialImagePlaneWidget->AddObserver(vtkCommand::InteractionEvent, planesInteractionCallback);
//     m_sagitalImagePlaneWidget->AddObserver(vtkCommand::InteractionEvent, planesInteractionCallback);
//     m_coronalImagePlaneWidget->AddObserver(vtkCommand::InteractionEvent, planesInteractionCallback);
    m_axialImagePlaneWidget->AddObserver(vtkCommand::AnyEvent, planesInteractionCallback);
    m_sagitalImagePlaneWidget->AddObserver(vtkCommand::AnyEvent, planesInteractionCallback);
    m_coronalImagePlaneWidget->AddObserver(vtkCommand::AnyEvent, planesInteractionCallback);
    planesInteractionCallback->Delete();
}

void Q3DMPRViewer::updatePlanesData()
{
    if (hasInput())
    {
        m_axialImagePlaneWidget->SetInput(getMainInput()->getVtkData());
        if (!m_axialResliced)
        {
            m_axialResliced = new Volume();
            m_axialResliced->setData(m_axialImagePlaneWidget->GetResliceOutput());
        }
        else
        {
            m_axialResliced->setData(m_axialImagePlaneWidget->GetResliceOutput());
        }
        // TODO això es necessari perquè tingui la informació de la sèrie, estudis, pacient...
        m_axialResliced->setImages(getMainInput()->getImages());

        m_sagitalImagePlaneWidget->SetInput(getMainInput()->getVtkData());
        if (!m_sagitalResliced)
        {
            m_sagitalResliced = new Volume();
            m_sagitalResliced->setData(m_sagitalImagePlaneWidget->GetResliceOutput());
        }
        else
        {
            m_sagitalResliced->setData(m_sagitalImagePlaneWidget->GetResliceOutput());
        }
        // TODO això es necessari perquè tingui la informació de la sèrie, estudis, pacient...
        m_sagitalResliced->setImages(getMainInput()->getImages());

        m_coronalImagePlaneWidget->SetInput(getMainInput()->getVtkData());
        if (!m_coronalResliced)
        {
            m_coronalResliced = new Volume();
            m_coronalResliced->setData(m_coronalImagePlaneWidget->GetResliceOutput());
        }
        else
        {
            m_coronalResliced->setData(m_coronalImagePlaneWidget->GetResliceOutput());
        }
        // TODO això es necessari perquè tingui la informació de la sèrie, estudis, pacient...
        m_coronalResliced->setImages(getMainInput()->getImages());
    }
    else
    {
        DEBUG_LOG("No es poden inicialitzar les dades dels plans. No hi ha dades d'entrada");
    }
}

void Q3DMPRViewer::createOutline()
{
    if (hasInput())
    {
        // Creem l'outline
        vtkOutlineFilter *outlineFilter = vtkOutlineFilter::New();
        outlineFilter->SetInput(getMainInput()->getVtkData());
        vtkPolyDataMapper *outlineMapper = vtkPolyDataMapper::New();
        outlineMapper->SetInput(outlineFilter->GetOutput());
        m_outlineActor->SetMapper(outlineMapper);
        outlineFilter->Delete();
        outlineMapper->Delete();
    }
    else
    {
        DEBUG_LOG("Intentant crear outline sense haver donat input abans");
    }
}

void Q3DMPRViewer::resetViewToAxial()
{
    this->resetPlanes();
    this->setCameraOrientation(OrthogonalPlane::XYPlane);
}

void Q3DMPRViewer::resetViewToSagital()
{
    this->resetPlanes();
    this->setCameraOrientation(OrthogonalPlane::YZPlane);
}

void Q3DMPRViewer::resetViewToCoronal()
{
    this->resetPlanes();
    this->setCameraOrientation(OrthogonalPlane::XZPlane);
}

void Q3DMPRViewer::setSagitalVisibility(bool enable)
{
    m_sagitalPlaneVisible = enable;
    if (enable)
    {
        m_sagitalImagePlaneWidget->On();
    }
    else
    {
        m_sagitalImagePlaneWidget->Off();
    }
}

void Q3DMPRViewer::setCoronalVisibility(bool enable)
{
    m_coronalPlaneVisible = enable;
    if (enable)
    {
        m_coronalImagePlaneWidget->On();
    }
    else
    {
        m_coronalImagePlaneWidget->Off();
    }
}

void Q3DMPRViewer::setAxialVisibility(bool enable)
{
    m_axialPlaneVisible = enable;
    if (enable)
    {
        m_axialImagePlaneWidget->On();
    }
    else
    {
        m_axialImagePlaneWidget->Off();
    }
}

void Q3DMPRViewer::resetPlanes()
{
    if (hasInput())
    {
        int *size = getMainInput()->getDimensions();

        m_axialImagePlaneWidget->SetPlaneOrientationToZAxes();
        m_axialImagePlaneWidget->SetSliceIndex(size[2]/2);

        m_sagitalImagePlaneWidget->SetPlaneOrientationToXAxes();
        m_sagitalImagePlaneWidget->SetSliceIndex(size[0]/2);

        m_coronalImagePlaneWidget->SetPlaneOrientationToYAxes();
        m_coronalImagePlaneWidget->SetSliceIndex(size[1]/2);

        if (m_axialPlaneVisible)
        {
            m_axialImagePlaneWidget->On();
        }
        if (m_sagitalPlaneVisible)
        {
            m_sagitalImagePlaneWidget->On();
        }
        if (m_coronalPlaneVisible)
        {
            m_coronalImagePlaneWidget->On();
        }
    }
}

void Q3DMPRViewer::setWindowLevel(double window, double level)
{
    if (hasInput())
    {
        // Amb un n'hi ha prou ja que cada vtkImagePlaneWidget comparteix la mateixa LUT
        m_axialImagePlaneWidget->SetWindowLevel(window, level);
    }
    else
    {
        DEBUG_LOG("Intentant posar un window level sense donar un input abans");
    }
}

void Q3DMPRViewer::setTransferFunction(TransferFunction *transferFunction)
{
    m_transferFunction = transferFunction;
    // Apliquem la funció de transferència sobre cadascun dels plans
    // TODO Aplicar la funció de color: caldrà fer un pipeline similar al del 2D
}

void Q3DMPRViewer::enableOutline(bool enable)
{
    m_isOutlineEnabled = enable;
    m_outlineActor->SetVisibility(m_isOutlineEnabled);
}

void Q3DMPRViewer::outlineOn()
{
    this->enableOutline(true);
}

void Q3DMPRViewer::outlineOff()
{
    this->enableOutline(false);
}

void Q3DMPRViewer::enableOrientationMarker(bool enable)
{
    m_orientationMarker->setEnabled(enable);
}

void Q3DMPRViewer::orientationMarkerOn()
{
    this->enableOrientationMarker(true);
}

void Q3DMPRViewer::orientationMarkerOff()
{
    this->enableOrientationMarker(false);
}

void Q3DMPRViewer::getCurrentWindowLevel(double wl[2])
{
    m_axialImagePlaneWidget->GetWindowLevel(wl);
}

void Q3DMPRViewer::planeInteraction()
{
    emit planesHasChanged();
}

Volume *Q3DMPRViewer::getAxialResliceOutput()
{
    return m_axialResliced;
}

Volume *Q3DMPRViewer::getSagitalResliceOutput()
{
    return m_sagitalResliced;
}

Volume *Q3DMPRViewer::getCoronalResliceOutput()
{
    return m_coronalResliced;
}

double *Q3DMPRViewer::getAxialPlaneOrigin()
{
    return m_axialImagePlaneWidget->GetOrigin();
}

double *Q3DMPRViewer::getAxialPlaneNormal()
{
    return m_axialImagePlaneWidget->GetNormal();
}

void Q3DMPRViewer::getAxialPlaneOrigin(double origin[3])
{
    m_axialImagePlaneWidget->GetOrigin(origin);
}

void Q3DMPRViewer::getAxialPlaneNormal(double normal[3])
{
    m_axialImagePlaneWidget->GetNormal(normal);
}

double *Q3DMPRViewer::getSagitalPlaneOrigin()
{
    return m_sagitalImagePlaneWidget->GetOrigin();
}

double *Q3DMPRViewer::getSagitalPlaneNormal()
{
    return m_sagitalImagePlaneWidget->GetNormal();
}

void Q3DMPRViewer::getSagitalPlaneOrigin(double origin[3])
{
    m_sagitalImagePlaneWidget->GetOrigin(origin);
}

void Q3DMPRViewer::getSagitalPlaneNormal(double normal[3])
{
    m_sagitalImagePlaneWidget->GetNormal(normal);
}

double *Q3DMPRViewer::getCoronalPlaneOrigin()
{
    return m_coronalImagePlaneWidget->GetOrigin();
}

double *Q3DMPRViewer::getCoronalPlaneNormal()
{
    return m_coronalImagePlaneWidget->GetNormal();
}

void Q3DMPRViewer::getCoronalPlaneOrigin(double origin[3])
{
    m_coronalImagePlaneWidget->GetOrigin(origin);
}

void Q3DMPRViewer::getCoronalPlaneNormal(double normal[3])
{
    m_coronalImagePlaneWidget->GetNormal(normal);
}

};  // End namespace udg
