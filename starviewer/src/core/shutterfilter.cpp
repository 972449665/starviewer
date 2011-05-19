#include "shutterfilter.h"

#include <vtkPolyDataToImageStencil.h>
#include "vtkAtamaiPolyDataToImageStencil2.h"
#include <vtkImageStencil.h>
#include <vtkLinearExtrusionFilter.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkPolyData.h>
#include <vtkRegularPolygonSource.h>
#include <vtkImageMask.h>
#include <vtkImageData.h>

#include "volume.h"
#include "logging.h"

// Dcmtk: presentation state
// DVPresentationState
#include <dvpstat.h>

namespace udg {

ShutterFilter::ShutterFilter()
{
    m_rectangularPolyDataStencil = 0;
    m_circularPolyDataStencil = 0;
    m_polygonalPolyDataStencil = 0;
    m_inputData = 0;
    // Per defecte el background és negre
    m_background = 0;
}

ShutterFilter::~ShutterFilter()
{
    clearAllShutters();
}

void ShutterFilter::setInput(Volume *volume)
{
    m_inputData = volume;
}

void ShutterFilter::setPresentationStateShutters(const QString &presentationStateFile)
{
    if (m_inputData)
    {
        DVPresentationState *presentationStateHandler = new DVPresentationState;
        DcmDataset *presentationStateData = NULL;
        DcmFileFormat presentationStateFileFormat;
        OFCondition status = presentationStateFileFormat.loadFile(qPrintable(presentationStateFile));
        if (status.good())
        {
            presentationStateData = presentationStateFileFormat.getDataset();
            status = presentationStateHandler->read(*presentationStateData);
            if (status.good())
            {
                // Obtenim els paràmetres de les coordenades per mapejar bé els punts a l'espai
                double origin[3];
                double spacing[3];
                m_inputData->getOrigin(origin);
                m_inputData->getSpacing(spacing);
                // Càlcul d'un background adequat amb el rang de dades. Els valors de background poden anar de 0 o 65535 i el rang de dades pot ser qualsevol
                // altre, com per exemple 127..255;
                double range[2];
                int backValue = presentationStateHandler->getShutterPresentationValue();
                m_inputData->getScalarRange(range);
                if (backValue == 0)
                {
                    m_background = range[0];
                }
                else
                {
                    m_background = backValue * (range[1] / backValue) + range[0];
                }

                DEBUG_LOG(QString("Valor de background del Shutter donat pel presentation state: %1").arg(m_background));

                if (presentationStateHandler->haveShutter(DVPSU_rectangular))
                {
                    DEBUG_LOG("Hi ha RECTANGULAR shutter al presentation state");
                    this->setRectangularShutter(origin[0] + (presentationStateHandler->getRectShutterLV() - 1) * spacing[0],
                                origin[0] + (presentationStateHandler->getRectShutterRV() - 1) * spacing[0],
                                origin[1] + (presentationStateHandler->getRectShutterUH() - 1) * spacing[1],
                                origin[1] + (presentationStateHandler->getRectShutterLH() - 1) * spacing[1]
                               );
                }
                else
                {
                    DEBUG_LOG("No hi ha RECTANGULAR shutter al presentation state");
                }

                if (presentationStateHandler->haveShutter(DVPSU_circular))
                {
                    DEBUG_LOG("Hi ha CIRCULAR shutter al presentation state");

                    double center[2] = {
                        origin[0] + (presentationStateHandler->getCenterOfCircularShutter_x() - 1) * spacing[0],
                        origin[1] + (presentationStateHandler->getCenterOfCircularShutter_y() - 1) * spacing[1]
                    };
                    // Radius of the cicular shutter with to respect to pixels in the image given as a number of pixels along the row direction-> per tant cal
                    // calcular bé quan mesura respecte les coordenades de món
                    double radius = (origin[0] + (presentationStateHandler->getRadiusOfCircularShutter() - 1) * spacing[0]) - origin[0];
                    this->setCircularShutter(center, radius);
                }
                else
                {
                    DEBUG_LOG("No hi ha CIRCULAR shutter al presentation state");
                }

                if (presentationStateHandler->haveShutter(DVPSU_polygonal))
                {
                    int vertices = presentationStateHandler->getNumberOfPolyShutterVertices();
                    QString msg = "Hi ha POLYGONAL shutter al presentation state, #de punts:";
                    msg += QString("%1 amb coordenades: ").arg(vertices);
                    // \TODO caldria passar això al mètode propi de la classe?
                    // Creem els punts
                    vtkPoints *points = vtkPoints::New();
                    vtkCellArray *vertexs = vtkCellArray::New();
                    vertexs->InsertNextCell(vertices + 1);
                    Sint32 polyX, polyY;
                    double x, y;
                    for (int i = 0; i < vertices; i++)
                    {
                        if (EC_Normal == presentationStateHandler->getPolyShutterVertex(i, polyX, polyY))
                        {
                            x = origin[0] + (polyX - 1.0) * spacing[0];
                            y = origin[1] + (polyY - 1.0) * spacing[1];
                            msg += QString("[%1,%2]\n").arg(x).arg(y);
                            points->InsertPoint(i, x, y, origin[2] - 1.0);
                            vertexs->InsertCellPoint(i);
                        }
                        else
                        {
                            msg += "???\\???,\n";
                        }
                    }
                    // Per tancar el polígon
                    vertexs->InsertCellPoint(0);
                    vtkPolyData *polyData = vtkPolyData::New();
                    polyData->SetPoints(points);
                    polyData->SetLines(vertexs);

                    vtkLinearExtrusionFilter *extruder = vtkLinearExtrusionFilter::New();
                    extruder->SetScaleFactor(10);
                    extruder->SetExtrusionTypeToNormalExtrusion();
                    // Assumirem ara per ara, només es fa en axial
                    extruder->SetVector(0, 0, 1);
                    extruder->SetInput(polyData);

                    clearPolygonalShutter();

                    // Creem l'stencil
                    m_polygonalPolyDataStencil = vtkAtamaiPolyDataToImageStencil2::New();
                    m_polygonalPolyDataStencil->SetInput(extruder->GetOutput());

                    points->Delete();
                    vertexs->Delete();
                    polyData->Delete();
                    extruder->Delete();

                    DEBUG_LOG(QString(msg));
                }
                else
                {
                    DEBUG_LOG("No hi ha POLYGONAL shutter al presentation state");
                }

                if (presentationStateHandler->haveShutter(DVPSU_bitmap))
                {
                    DEBUG_LOG("Hi ha BITMAP shutter al presentation state");
                    int overlays = presentationStateHandler->getNumberOfOverlaysInPresentationState();
                    DEBUG_LOG(QString("Nombre d'overlays en el PS: %1").arg(overlays));

                    for (int overlayIndex = 0; overlayIndex < overlays; overlayIndex++)
                    {
                        if (presentationStateHandler->overlayIsSuitableAsBitmapShutter(overlayIndex))
                        {
                            int layer = presentationStateHandler->getOverlayInPresentationStateActivationLayer(overlayIndex);

                            DEBUG_LOG(QString("L'overlay %1 és bo com a bitmap shutter i es pinta al layer #%2").arg(overlayIndex).arg(layer));

                            if (presentationStateHandler->overlayInPresentationStateIsROI(overlayIndex))
                            {
                                DEBUG_LOG("L'overlay és un ROI!!!!!!!!!!");
                            }
                            else
                            {
                                DEBUG_LOG("L'overlay és un BITMAP SHUTTER :D !!!!");
                            }

                            OFCondition status;
                            status = presentationStateHandler->activateOverlayAsBitmapShutter(overlayIndex);
                            // The overlay must not be activated on a graphic layer (i.e. getOverlayInPresentationStateActivationLayer(idx) != DVPS_IDX_NONE,
                            // otherwise this method fails.
                            //     idx  index of the overlay, must be < getNumberOfOverlaysInPresentationState().
                            DEBUG_LOG(QString("STATUS quo: ") + status.text());

                            bool isROI = false;
                            unsigned int width = 0, height = 0, left = 0, top = 0;
                            const void *data = 0;
                            // Color amb que es pinta l'overlay
                            Uint16 foreground = 0;
                            status = presentationStateHandler->getOverlayData(
                                    layer,
                                    overlayIndex,
                                    data,
                                    width,
                                    height,
                                    left,
                                    top,
                                    isROI,
                                    foreground
                                       );

                            DEBUG_LOG(status.text());
                            this->setBitmapShutter((unsigned char *)data, width, height, left, top, foreground);
                        }
                    }
                }
                else
                {
                    DEBUG_LOG("No hi ha BITMAP shutter al presentation state");
                }
            }
            else
            {
                DEBUG_LOG(QString("No s'han pogut carregar les dades del presentation state al corresponenr handler: ") + status.text());
            }
        }
        else
        {
            DEBUG_LOG(QString("No s'ha pogut carregar el fitxer de presentation state: ") + status.text());
        }

        delete presentationStateHandler;
    }
    else
    {
        DEBUG_LOG("No hi ha dades d'input, no es pot aplicar cap shutter");
    }
}

void ShutterFilter::setRectangularShutter(double leftVertical, double rightVertical, double upperHorizontal, double lowerHorizontal)
{
    DEBUG_LOG(QString("Coordenades shutter RECTANGULAR: [LV,RV,UH,LH] %1,%2,%3,%4")
                .arg(leftVertical).arg(rightVertical).arg(upperHorizontal).arg(lowerHorizontal));
    double origin[3];
    m_inputData->getOrigin(origin);

    vtkPoints *points = vtkPoints::New();
    // \TODO tenim un petit problema quan la coordenada x és 0 (potser és quan coincideix amb l'origen 0) Deu ser degut a un bug del filtre d'stencil. Per
    // evitar això fem que si coincideix l'origen x, li fem un petit increment perquè funcioni el retallat
    if (leftVertical == origin[0])
    {
        leftVertical += 0.001;
    }
    // Comencem per la cantonada inferior esquerre i continuem en sentit anti-horari
    points->InsertPoint(0, leftVertical, lowerHorizontal, origin[2] - 1.);
    points->InsertPoint(1, rightVertical, lowerHorizontal, origin[2] - 1.);
    points->InsertPoint(2, rightVertical, upperHorizontal, origin[2] - 1.);
    points->InsertPoint(3, leftVertical, upperHorizontal, origin[2] - 1.);

    vtkCellArray *vertexs = vtkCellArray::New();
    vertexs->InsertNextCell(5);
    for (int i = 0; i < 5; i++)
    {
        // 0,1,2,3,0
        vertexs->InsertCellPoint(i % 4);
    }

    vtkPolyData *polyData = vtkPolyData::New();
    polyData->SetPoints(points);
    polyData->SetLines(vertexs);

    vtkLinearExtrusionFilter *extruder = vtkLinearExtrusionFilter::New();
    extruder->SetScaleFactor(2);
    extruder->SetExtrusionTypeToNormalExtrusion();
    // Assumirem ara per ara, només es fa en axial
    extruder->SetVector(0, 0, 1);
    extruder->SetInput(polyData);

    clearRectangularShutter();

    m_rectangularPolyDataStencil = vtkPolyDataToImageStencil::New();
    m_rectangularPolyDataStencil->SetInput(extruder->GetOutput());

    points->Delete();
    vertexs->Delete();
    polyData->Delete();
    extruder->Delete();
}

void ShutterFilter::setPolygonalShutter(std::vector<double[2]> vtkNotUsed(vertexs))
{
    // \TODO per implementar
    DEBUG_LOG("Mètode no implementat");
}

void ShutterFilter::setCircularShutter(double center[2], double radius)
{
    DEBUG_LOG(QString("CIRCULAR shutter, centre: %1,%2 radi: %3").arg(center[0]).arg(center[1]).arg(radius));
    double origin[3];
    double spacing[3];
    m_inputData->getOrigin(origin);
    m_inputData->getSpacing(spacing);

    vtkRegularPolygonSource *circle = vtkRegularPolygonSource::New();
    circle->SetCenter(center[0], center[1], origin[2] - 1.0);
    circle->SetRadius(radius);
    // Com més sides, més línies, per tant el cercle tindria més resolució.
    circle->SetNumberOfSides(35);
    circle->GeneratePolygonOff();

    vtkLinearExtrusionFilter *extruder = vtkLinearExtrusionFilter::New();
    extruder->SetScaleFactor(2);
    extruder->SetExtrusionTypeToNormalExtrusion();
    // Assumirem ara per ara, només es fa en axial
    extruder->SetVector(0, 0, 1);
    extruder->SetInput(circle->GetOutput());

    clearCircularShutter();

    m_circularPolyDataStencil = vtkPolyDataToImageStencil::New();
    m_circularPolyDataStencil->SetInput(extruder->GetOutput());

    circle->Delete();
    extruder->Delete();
}

void ShutterFilter::setBitmapShutter(vtkImageData *vtkNotUsed(bitmap))
{
    // \TODO per implementar
    DEBUG_LOG("Mètode no implementat");
}

void ShutterFilter::setBitmapShutter(unsigned char *data, unsigned int vtkNotUsed(width), unsigned int vtkNotUsed(height), unsigned int vtkNotUsed(left),
                                     unsigned int vtkNotUsed(top), unsigned int vtkNotUsed(foreground))
{
    // Creem la màscara que farà de bitmap shutter
    // Coses a tenir en compte
    // La màscara ha de tenir el mateix extent que l'input
    // Ara mateix només ho fem per una imatge per tant anem a saco
    // S'ha de tenir en compte que podem tenir més shutters a part (poly, circle, etc) Fer bé la combinació.

    // Creem el vtkImageData a partir del bitmap del DICOM
    vtkImageData *bitmapShutter = vtkImageData::New();
    bitmapShutter->SetDimensions(m_inputData->getDimensions());
    bitmapShutter->SetWholeExtent(m_inputData->getWholeExtent());
//                 bitmapShutter->SetExtent(m_inputData->GetExtent());
//                 bitmapShutter->SetUpdateExtent(m_inputData->GetUpdateExtent());
    bitmapShutter->SetSpacing(m_inputData->getSpacing());
    bitmapShutter->SetOrigin(m_inputData->getOrigin());
//                 bitmapShutter->SetScalarType(m_inputData->GetScalarType());
    bitmapShutter->SetScalarTypeToUnsignedChar();
    bitmapShutter->SetNumberOfScalarComponents(1);
    bitmapShutter->AllocateScalars();

    int wholeExtent[6];
    unsigned char *unsignedData = (unsigned char *)data;
    m_inputData->getWholeExtent(wholeExtent);
    for (int i = 0; i <= wholeExtent[1]; i++)
    {
        for (int j = 0; j <= wholeExtent[3]; j++)
        {
            for (int k = 0; k <= wholeExtent[5]; k++)
            {
                //  \TODO mirar si això està bé
                int index = ((wholeExtent[1] + 1) * j * + i) + k * ((wholeExtent[1] + 1) * (wholeExtent[3] + 1));

                // Get scalar pointer to current pixel
                unsigned char *currentVoxel = (unsigned char *) bitmapShutter->GetScalarPointer(i, j, k);

                // Set scalar value accordingly
//                             *currentVoxel = (unsigned char *)data[index];
                *currentVoxel = unsignedData[index];
//                 DEBUG_LOG(QString("índex: %1, %2, %3 = %4 : valor: %5").arg(i).arg(j).arg(k).arg(index).arg(&currentVoxel));
            }
        }
    }

    vtkImageMask *bitmapShutterMask = vtkImageMask::New();
    bitmapShutterMask->SetImageInput(m_inputData->getVtkData());

//                 std::cout << "Overlays available (non-shadowed) in attached image: " << max << std::endl;
//                 for (size_t oidx = 0; oidx < max; oidx++)
//                 {
//                     std::cout << "  Overlay #" << oidx+1 << ": group=0x" << hex << m_defaultPresentationStateHandler->getOverlayInImageGroup(oidx) << dec << " label=\"";
//                     c = m_defaultPresentationStateHandler->getOverlayInImageLabel(oidx);
//                     if (c)
//                         std::cout << c;
//                     else
//                         std::cout << "(none)";
//                     std::cout << "\" description=\"";
//                     c = m_defaultPresentationStateHandler->getOverlayInImageDescription(oidx);
//                     if (c)
//                         std::cout << c;
//                     else
//                         std::cout << "(none)";
//                     std::cout << "\" type=";
//                     if (m_defaultPresentationStateHandler->overlayInImageIsROI(oidx))
//                         std::cout << "ROI";
//                     else
//                         std::cout << "graphic";
//                     std::cout << std::endl;
//                 }
//                 std::cout << std::endl;

    bitmapShutter->Delete();
    bitmapShutterMask->Delete();
}

void ShutterFilter::clearAllShutters()
{
    this->clearCircularShutter();
    this->clearRectangularShutter();
    this->clearPolygonalShutter();
    this->clearBitmapShutter();
}

void ShutterFilter::clearShutter(int shutterID)
{
    switch (shutterID)
    {
    case CircularShutter:
        clearCircularShutter();
    break;

    case PolygonalShutter:
        clearPolygonalShutter();
    break;

    case RectangularShutter:
        clearRectangularShutter();
    break;

    case BitmapShutter:
        clearBitmapShutter();
    break;
    }
}

void ShutterFilter::clearCircularShutter()
{
    if (m_circularPolyDataStencil)
    {
        m_circularPolyDataStencil->Delete();
        m_circularPolyDataStencil = 0;
    }
}

void ShutterFilter::clearPolygonalShutter()
{
    if (m_polygonalPolyDataStencil)
    {
        m_polygonalPolyDataStencil->Delete();
        m_polygonalPolyDataStencil = 0;
    }
}

void ShutterFilter::clearRectangularShutter()
{
    if (m_rectangularPolyDataStencil)
    {
        m_rectangularPolyDataStencil->Delete();
        m_rectangularPolyDataStencil = 0;
    }
}

void ShutterFilter::clearBitmapShutter()
{
    // \TODO per implementar
    DEBUG_LOG("Mètode no implementat");
}

vtkImageData *ShutterFilter::getOutput()
{
    // Si hi ha algun shutter l'apliquem sobre la imatge
    // \TODO falta verificar que la concatenació d'stencils sigui correcta
    // \TODO fer còpia local de l'output i no cal refer el pipeline si es demana més d'un cop
    if (m_inputData)
    {
        vtkImageStencil *rectangularStencil = 0, *polygonalStencil = 0, *circularStencil = 0;
        if (m_rectangularPolyDataStencil)
        {
            rectangularStencil = vtkImageStencil::New();
            rectangularStencil->SetInput(m_inputData->getVtkData());
            rectangularStencil->ReverseStencilOn();
            rectangularStencil->SetBackgroundValue(m_background);
            rectangularStencil->SetStencil(m_rectangularPolyDataStencil->GetOutput());
        }

        if (m_polygonalPolyDataStencil)
        {
            polygonalStencil = vtkImageStencil::New();
            // Si hi ha shutter rectangular el concatenem
            if (rectangularStencil)
            {
                polygonalStencil->SetInput(rectangularStencil->GetOutput());
            }
            else
            {
                polygonalStencil->SetInput(m_inputData->getVtkData());
            }
            // Això sembla que s'ha de fer així pel sentit en que ens donen els punts
            polygonalStencil->ReverseStencilOff();
            polygonalStencil->SetBackgroundValue(m_background);
            polygonalStencil->SetStencil(m_polygonalPolyDataStencil->GetOutput());
        }

        if (m_circularPolyDataStencil)
        {
            circularStencil = vtkImageStencil::New();
            // Si hi ha shutter poligonal el concatenem
            if (polygonalStencil)
            {
                circularStencil->SetInput(polygonalStencil->GetOutput());
            }
            // Sinó mire si n'hi de rectamgular per concatenar
            else if (rectangularStencil) 
            {
                circularStencil->SetInput(rectangularStencil->GetOutput());
            }
            else
            {
                circularStencil->SetInput(m_inputData->getVtkData());
            }

            circularStencil->ReverseStencilOn();
            circularStencil->SetBackgroundValue(m_background);
            circularStencil->SetStencil(m_circularPolyDataStencil->GetOutput());
        }

        // \TODO faltaria la part del bitmap shutter, encara per resoldre
        vtkImageData *output = 0;
        if (circularStencil)
        {
            output = circularStencil->GetOutput();
            // La solució bona aquí seria guardar output en un smart pointer (per retornar-lo i oblidar-nos-en) i després fer circularStencil->Delete()
        }
        else if (polygonalStencil)
        {
            output = polygonalStencil->GetOutput();
            // La solució bona aquí seria guardar output en un smart pointer (per retornar-lo i oblidar-nos-en) i després fer polygonalStencil->Delete()
        }
        else if (rectangularStencil)
        {
            output = rectangularStencil->GetOutput();
            // La solució bona aquí seria guardar output en un smart pointer (per retornar-lo i oblidar-nos-en) i després fer rectangularStencil->Delete()
        }

        if (output)
        {
            output->Update();
        }

        return output;
    }
    else
    {
        DEBUG_LOG("L'input és NUL. No es pot reproduir el shutter");
        return 0;
    }
}

}
