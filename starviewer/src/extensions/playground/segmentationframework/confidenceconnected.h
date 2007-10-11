#ifndef UDGCONFIDENCECONNECTED_H
#define UDGCONFIDENCECONNECTED_H




#include <qobject.h>
#include <itkConfidenceConnectedImageFilter.h>
#include <itkImage.h>
#include <itkCastImageFilter.h>
#include <itkCurvatureFlowImageFilter.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>




#include <vector.h>
#include "volume.h"



using namespace std;

namespace udg {

/**
@author Grup de Gr�fics de Girona  ( GGG )
*/

/**
    Aquesta classe, confidenceConnected �s un m�tode de segmentaci� usant t�cniques de
    Region Growing. Est� pensat per trobar la  regi� que volem segmentar utilitzant
    una llavor i avaluant la intensitat dels p�xels vehins segons un criteri determinat.
    CRITERI:

    Aquesta classe prepara tot el tractament de de la imatge, rep tots els par�metres necessaris
    per executar el m�tode de segmentaci� (imatge a segmentar inclosa) i executa tot el proc�s de 
    segmentaci�.
*/





class ConfidenceConnected{
private:

    
    /// Definim un tipus d'imatge, el tipus d'imate del volum.
    typedef udg::Volume::ItkImageType     VolumeImageType;
    
    typedef VolumeImageType::IndexType    VolumeImageIndexType;
        
    /** Definim el tipus d'imatge amb qu� treballa el filtre de suavitzaci�.
       itk::CurvatureFlowImageFilter trevalla amb imatges de tipus double.*/
    typedef double                                                     InternalPixelType;
    typedef itk::Image< InternalPixelType,udg::Volume::VDimension >    InternalImageType;
    
    /** Definim tipus de Castings si els filtres necessiten alg�n tipus d'imatge determinat
       (InternalImageType) han de treballar amb aquest.
       Definim dos castings, un per passar del tipus d'imatge del volum al d'�s intern i
       un d'invers per poder guardar un nou volum*/
    typedef itk::CastImageFilter< VolumeImageType, InternalImageType >    CastingFilterTypeIn;
    typedef itk::CastImageFilter< InternalImageType, VolumeImageType >    CastingFilterTypeOut;
    
    /// Definim filtre de suavitzaci� de m�scara de mitjanes
    
    /// Definim filtre de suavitzaci�
    typedef itk::CurvatureFlowImageFilter< InternalImageType, InternalImageType >
            CurvatureFlowImageFilterType;
            
    /// Definim filtre de segmentaci� Region Growing
    typedef itk::ConfidenceConnectedImageFilter< InternalImageType, InternalImageType >
            ConfidenceConnectedImageFilterType;

    
            
    ///Atributs de la classe ConnectedThreshold
    
    ///El volum sobre el qual treballarem
    udg::Volume*                                 m_inputVolume;
    ///Un pont� al volum segmentat
    VolumeImageType::Pointer                     m_segmentedImage;
    ///El vector on guardarem les llavors.
    vector<VolumeImageIndexType>                 m_seedsVector;
    ///Castings
    CastingFilterTypeIn::Pointer                 m_casterInput;
    CastingFilterTypeOut::Pointer                m_casterOutput;
    ///Filtre de suavitzaci�
    CurvatureFlowImageFilterType::Pointer        m_smoothing;
    ///Filtres de segmentaci�
    ConfidenceConnectedImageFilterType::Pointer  m_confidence;
   
    
   





public:
    ConfidenceConnected();

    ~ConfidenceConnected();
    
    
    /// Afegeix una llavor (x,y,z) al vector de llavors, l'atribut m_seedsVector
    void addSeed( int x, int y, int z );
    
    /// Carrega el volum a tractar
    void setInputVolume( Volume* v );
    /** Carrega els par�metres necessaris per configurar el filtre de suavitzaci�
        ( iteracions i TimeStep )*/
    void setSmoothingParameters();
    /** Carrega els par�metres necessaris per configurar el filtre de segmentaci�
        ( radi, multiplier i les iteracions)*/
    void setConfidenceParameters( int radius, float multi, int itera);
    /** Aplica el metode de segmentaci� a la imatge passada amb els par�metres tamb�
        passats.*/
    bool applyMethod();
    
    /// Retorna el volum segmentat
    udg::Volume* getSegmentedVolume(); 

  

};

};

#endif
