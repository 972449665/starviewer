/***************************************************************************
 *   Copyright (C) 2005 by Grup de Gràfics de Girona                       *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#ifndef UDGQ3DVIEWER_H
#define UDGQ3DVIEWER_H

#include "qviewer.h"

#include "combiningvoxelshader.h"
#include "vtkVolumeRayCastSingleVoxelShaderCompositeFunction.h"

// FWD declarations
class vtkRenderer;
class vtkImageData;
class vtkRenderWindowInteractor;
class vtkInteractorStyle;
class vtkVolume;
class vtkVolumeProperty;
class vtkVolumeRayCastCompositeFunction;
class vtkVolumeRayCastIsosurfaceFunction;
class vtkVolumeRayCastMapper;


namespace udg {

// FWD declarations
class Volume;
class Q3DOrientationMarker;
class TransferFunction;
class ObscuranceMainThread;
class AmbientVoxelShader;
class DirectIlluminationVoxelShader;
class ObscuranceVoxelShader;
class vtk4DLinearRegressionGradientEstimator;
class Obscurance;
class ContourVoxelShader;

/**
Classe base per als visualitzadors 3D

@author Grup de Gràfics de Girona  ( GGG )
*/
class Q3DViewer : public QViewer{
Q_OBJECT
public:
    enum RenderFunction{ RayCasting, RayCastingObscurance,
                         MIP3D, IsoSurface , Texture2D , Texture3D, Contouring };
    enum ObscuranceQuality { Low, Medium, High };

    Q3DViewer( QWidget *parent = 0 );
    ~Q3DViewer();

    /// retorna el tipu de visualització que es té assignat
    RenderFunction getRenderFunction() const { return m_renderFunction; }

    /// retorna el tipu de visualització que es té assignat com a un string
    QString getRenderFunctionAsString();

    virtual void setInput( Volume* volume );

    virtual vtkRenderer *getRenderer();

    /// Obté el window level actual de la imatge
    double getCurrentColorWindow();
    double getCurrentColorLevel();
    void getCurrentWindowLevel( double wl[2] );

    void resetView( CameraOrientationType view );

public slots:
    /// assignem el tipus de visualització 3D que volem. RayCasting, MIP, reconstrucció de superfícies...
    void setRenderFunction(RenderFunction function);
    void setRenderFunctionToRayCasting();
    void setRenderFunctionToRayCastingObscurance();
    void setRenderFunctionToMIP3D();
    void setRenderFunctionToIsoSurface();
    void setRenderFunctionToTexture2D();
    void setRenderFunctionToTexture3D();
    void setRenderFunctionToContouring();

    void resetViewToAxial();
    void resetViewToSagital();
    void resetViewToCoronal();

    /// mètodes per controlar la visibilitat de l'orientation marker widget
    void enableOrientationMarker( bool enable );
    void orientationMarkerOn();
    void orientationMarkerOff();

    virtual void render();
    void reset();

    /// Li assignem la funció de transferència que volem aplicar
    void setTransferFunction( TransferFunction *transferFunction );
    TransferFunction * getTransferFunction( ){return m_transferFunction;}
    void setWindowLevel( double window , double level );
    void setNewTransferFunction( );

    /// Paràmetres d'il·luminació.
    void setShading( bool on );
    void setSpecular( bool on );
    void setSpecularPower( double power );

    /// Paràmetres de contorns.
    void setContour( bool on );
    void setContourThreshold( double threshold );

    /// Càlcul d'obscurances.
    void computeObscurance( ObscuranceQuality quality );
    void cancelObscurance();

    /// Paràmetres d'obscurances.
    void setObscurance( bool on );
    void setObscuranceFactor( double factor );

    /// Paràmetres d'isosuperfícies.
    void setIsoValue( int isoValue );

signals:
    /// TODO documentar els signals
    void obscuranceProgress( int progress );
    void obscuranceComputed();
    /// Es llança quan les obscurances són cancel·lades pel programa (no per l'usuari).
    void obscuranceCancelledByProgram();
    /// Informa del rang de valors del volum quan aquest canvia.
    void scalarRange( double min, double max );
    /// indica el nou window level
    void windowLevelChanged( double window , double level );
    void transferFunctionChanged ();

private:
    /// fa la visualització per raycasting
    void renderRayCasting();

    /// fa la visualització per raycasting amb obscurances
    void renderRayCastingObscurance();

    /// fa la visualització per contouring
    void renderContouring();

    /// fa la visualització per MIP3D
    void renderMIP3D();

    /// fa la visualització per reconstrucció de superfíces
    void renderIsoSurface();

    /// fa la visualització per textures 2D \TODO afegir comprovació de si el hard o suporta o no
    void renderTexture2D();

    /// fa la visualització per textures 3D \TODO afegir comprovació de si el hard o suporta o no
    void renderTexture3D();

    /// rescala les dades en el format adequat per als corresponents algorismes. Retorna fals si no hi ha cap volum assignat
    bool rescale();

    /// reinicia la orientació
    void resetOrientation();

    /// S'encarrega de decidir quina és la millor orientació
    /// depenent del tipus d'input que tenim. Generalment serà 
    /// Coronal, però depenent del tipus de Sèrie podria ser una altra
    void setDefaultOrientationForCurrentInput();

private slots:
    // TODO falta documentar el mètode
    void endComputeObscurance();

protected:
    /// el renderer
    vtkRenderer* m_renderer;

    /// la funció que es fa servir pel rendering
    RenderFunction m_renderFunction;

private:
    /// La imatge d'entrada dels mappers.
    vtkImageData *m_imageData;

    /// Widget per veure la orientació en 3D
    Q3DOrientationMarker *m_orientationMarker;

    /// El prop 3D de vtk per representar el volum
    vtkVolume *m_vtkVolume;

    /// Propietats que defineixen com es renderitzarà el volum
    vtkVolumeProperty *m_volumeProperty;

    /// Mapper del volum.
    vtkVolumeRayCastMapper *m_volumeMapper;

    /// Voxel shaders.
    AmbientVoxelShader *m_ambientVoxelShader;
    DirectIlluminationVoxelShader *m_directIlluminationVoxelShader;
    ContourVoxelShader *m_contourVoxelShader;
    typedef CombiningVoxelShader<AmbientVoxelShader, ContourVoxelShader> AmbientContourVoxelShader;
    AmbientContourVoxelShader *m_ambientContourVoxelShader;
    typedef CombiningVoxelShader<DirectIlluminationVoxelShader, ContourVoxelShader> DirectIlluminationContourVoxelShader;
    DirectIlluminationContourVoxelShader *m_directIlluminationContourVoxelShader;
    ObscuranceVoxelShader *m_obscuranceVoxelShader;
    typedef CombiningVoxelShader<AmbientVoxelShader, ObscuranceVoxelShader> AmbientObscuranceVoxelShader;
    AmbientObscuranceVoxelShader *m_ambientObscuranceVoxelShader;
    typedef CombiningVoxelShader<DirectIlluminationVoxelShader, ObscuranceVoxelShader> DirectIlluminationObscuranceVoxelShader;
    DirectIlluminationObscuranceVoxelShader *m_directIlluminationObscuranceVoxelShader;
    typedef CombiningVoxelShader<AmbientContourVoxelShader, ObscuranceVoxelShader> AmbientContourObscuranceVoxelShader;
    AmbientContourObscuranceVoxelShader *m_ambientContourObscuranceVoxelShader;
    typedef CombiningVoxelShader<DirectIlluminationContourVoxelShader, ObscuranceVoxelShader> DirectIlluminationContourObscuranceVoxelShader;
    DirectIlluminationContourObscuranceVoxelShader *m_directIlluminationContourObscuranceVoxelShader;

    /// Funcions de ray cast.
    vtkVolumeRayCastCompositeFunction *m_volumeRayCastFunction;
    vtkVolumeRayCastSingleVoxelShaderCompositeFunction<AmbientContourVoxelShader> *m_volumeRayCastAmbientContourFunction;
    vtkVolumeRayCastSingleVoxelShaderCompositeFunction<DirectIlluminationContourVoxelShader> *m_volumeRayCastDirectIlluminationContourFunction;
    vtkVolumeRayCastSingleVoxelShaderCompositeFunction<AmbientObscuranceVoxelShader> *m_volumeRayCastAmbientObscuranceFunction;
    vtkVolumeRayCastSingleVoxelShaderCompositeFunction<DirectIlluminationObscuranceVoxelShader> *m_volumeRayCastDirectIlluminationObscuranceFunction;
    vtkVolumeRayCastSingleVoxelShaderCompositeFunction<AmbientContourObscuranceVoxelShader> *m_volumeRayCastAmbientContourObscuranceFunction;
    vtkVolumeRayCastSingleVoxelShaderCompositeFunction<DirectIlluminationContourObscuranceVoxelShader> *m_volumeRayCastDirectIlluminationContourObscuranceFunction;
    vtkVolumeRayCastIsosurfaceFunction *m_volumeRayCastIsosurfaceFunction;

    /// La funció de transferència que s'aplica
    TransferFunction *m_transferFunction;
    TransferFunction *m_newTransferFunction;

    /// Booleà per saber si estem fent el primer render (per reiniciar l'orientació).
    bool m_firstRender;

    /// Thread de control del càlcul d'obscurances.
    ObscuranceMainThread *m_obscuranceMainThread;

    /// Booleà que indica si els contorns estan activats.
    bool m_contourOn;

    /// Obscurances.
    Obscurance *m_obscurance;
    /// Booleà que indica si les obscurances estan activades.
    bool m_obscuranceOn;

    /// Valors de window i level
    double m_window;
    double m_level;
    double m_range;
    double m_shift;

    /// Estimador de gradient que farem servir per les obscurances (i per la resta després de calcular les obscurances).
    vtk4DLinearRegressionGradientEstimator *m_4DLinearRegressionGradientEstimator;

    /// Orientació que tenim
    int m_currentOrientation;
};

};  //  end  namespace udg

#endif
