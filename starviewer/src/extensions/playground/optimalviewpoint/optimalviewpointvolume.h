/***************************************************************************
 *   Copyright (C) 2006-2007 by Grup de Gràfics de Girona                  *
 *   http://iiia.udg.edu/GGG/index.html                                    *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/


#ifndef UDGMAGICMIRRORSVOLUME_H
#define UDGMAGICMIRRORSVOLUME_H


#include <QObject>

#include <QVector>


class vtkImageData;
class vtkVolume;
class vtkVolumeProperty;
class vtkVolumeRayCastCompositeFunction;
class vtkVolumeRayCastCompositeFunctionOptimalViewpoint;
class vtkVolumeRayCastMapper;


namespace udg {


class TransferFunction;


/**
 * Aquesta classe gestiona el tractament de volums de l'extensió Optimal
 * Viewpoint.
 *
 * Guarda tots els objectes necessaris per crear un vtkVolume a partir d'un
 * vtkImageData, permet definir els paràmetres de la visualització del volum amb
 * ray casting, pot segmentar un volum amb les dades d'un fitxer o amb un
 * algorisme propi, etc.
 *
 * Té signals i slots que permeten la comunicació amb OptimalViewpointPlane
 * mentre es fa una visualització.
 *
 * \todo S'hauria d'ampliar una mica més aquesta descripció.
 *
 * \author Grup de Gràfics de Girona (GGG) <vismed@ima.udg.edu>
 */
class OptimalViewpointVolume : public QObject
{

    Q_OBJECT

public:

    /// Construeix el volum a partir de l'imatge \a image. \a image no pot ser nul.
    OptimalViewpointVolume( vtkImageData * image, QObject * parent = 0 );
    virtual ~OptimalViewpointVolume();

    /// Retorna el vtkVolume corresponent a l'índex donat.
    vtkVolume * getMainVolume() const;
    vtkVolume * getPlaneVolume() const;

    void setShade( bool on );

    void setImageSampleDistance( double imageSampleDistance );
    double getImageSampleDistance() const { return m_imageSampleDistance; }

    void setSampleDistance( double sampleDistance );
    double getSampleDistance() const { return m_sampleDistance; }

    void setTransferFunction( const TransferFunction & transferFunction );

    /**
     * Estableix la funció de transferència d'opacitat pel vtkVolume
     * corresponent a l'índex donat.
     */
//     void setOpacityTransferFunction( vtkPiecewiseFunction * opacityTransferFunction);

    /**
     * Estableix la funció de transferència de color pel vtkVolume corresponent
     * a l'índex donat.
     */
//     void setColorTransferFunction( vtkColorTransferFunction * colorTransferFunction);

    /**
     * Sincronitza les tranformacions de tots els vtkVolumes. Concretament,
     * aplica la transformació del vtkVolume amb índex 0 a tots els altres
     * vtkVolumes.
     */
    void synchronize();

    void handle( int rayId, int offset );
    void endRay( int rayId );

    /// Carrega la segmentació del volum des dun fitxer de text.
    signed char loadSegmentationFromFile( const QString & segmentationFileName );
    /// Fa una segmentació automàtica del volum.
    unsigned char segmentateVolume( unsigned short iterations, unsigned char numberOfClusters, double noise );
    signed char rescale( int bins );

    void setSegmentationFileName( QString name );




    // nous paràmetres
    void setOpacityForComputing( bool on );
    static const int INTERPOLATION_NEAREST_NEIGHBOUR = 0,
                     INTERPOLATION_LINEAR_INTERPOLATE_CLASSIFY = 1,
                     INTERPOLATION_LINEAR_CLASSIFY_INTERPOLATE = 2;
    void setInterpolation( int interpolation );
    void setSpecular( bool on );
    void setSpecularPower( double specularPower );







    vtkImageData * getImage() const { return m_image; }
    vtkImageData * getLabeledImage() const { return m_labeledImage; }

    /// Retorna el valor de propietat mínim.
    unsigned char getRangeMin() const;
    /// Retorna el valor de propietat màxim.
    unsigned char getRangeMax() const;



    void setRenderCluster( bool renderCluster );
    void setClusterLimits( unsigned short first, unsigned short last );




public slots:

    void setExcessEntropy( double excessEntropy );
    void setComputing( bool on = true );

private:

    /// Genera la imatge etiquetada i la segmentada a partir dels limits donats.
    void labelize( const QVector< unsigned char > & limits );
    /// Genera una funció de transferència ajustada a la segmentació a partir dels límits donats.
    void generateAdjustedTransferFunction( const QVector< unsigned char > & limits );

    /// Model de vòxels original.
    vtkImageData * m_image;
    vtkImageData * m_labeledImage;
    vtkImageData * m_segmentedImage;

    /// Vector de volums.
    vtkVolume * m_mainVolume;
    vtkVolume * m_planeVolume;

    /// Vector de mappers.
    vtkVolumeRayCastMapper * m_mainMapper;
    vtkVolumeRayCastMapper * m_planeMapper;

    /// Vector de funcions de ray-cast.
    vtkVolumeRayCastCompositeFunction * m_mainVolumeRayCastFunction;
    vtkVolumeRayCastCompositeFunctionOptimalViewpoint * m_planeVolumeRayCastFunction;

    /// Vector de funcions de transferència d'opacitat.
//     vtkPiecewiseFunction * m_opacityTransferFunction;

    /// Vector de funcions de transferència de color.
//     vtkColorTransferFunction * m_colorTransferFunction;

    /// Vector de propietats de volum.
    vtkVolumeProperty * m_volumeProperty;


    vtkImageData * m_clusterImage;
    vtkVolume * m_clusterVolume;
    vtkVolumeRayCastMapper * m_clusterMapper;



    unsigned char * m_data;
    unsigned char * m_labeledData;
    unsigned char * m_segmentedData;
    int m_dataSize;

    double m_imageSampleDistance;
    double m_sampleDistance;

    double m_excessEntropy;

    QString m_segmentationFileName;

    /// Valors de propietat mínim i màxim del volum.
    unsigned char m_rangeMin, m_rangeMax;


    bool m_renderCluster;
    bool m_clusterFirst, m_clusterLast;


signals:

    /// S'emet quan es crea el volum i dóna els valors de propietat mínim i màxim.
    void scalarRange( unsigned char min, unsigned char max );
    void needsExcessEntropy();
    void visited( int rayId, unsigned char value );
    void rayEnd( int rayId );
    void adjustedTransferFunctionDefined( const TransferFunction & adjustedTransferFunction );


};



}



#endif
