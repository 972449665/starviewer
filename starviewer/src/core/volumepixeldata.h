#ifndef UDGVOLUMEPIXELDATA_H
#define UDGVOLUMEPIXELDATA_H

#include <QObject>
#include <QVector>

#include <itkImage.h>
#include <vtkSmartPointer.h>
// Els filtres per passar itk<=>vtk: InsightApplications/auxiliary/vtk --> ho tenim a /tools

// Converts an ITK image into a VTK image and plugs a itk data pipeline to a VTK datapipeline.
#include "itkImageToVTKImageFilter.h"
// Converts a VTK image into an ITK image and plugs a vtk data pipeline to an ITK datapipeline.
#include "itkVTKImageToImageFilter.h"

class vtkImageData;

namespace udg {

class VolumePixelDataIterator;

/**
    Classe que té com a responsabilitat mantenir el pixel data d'un Volume.
    El pixel data d'un volume és el lloc de memòria on es guarden els diferents valors de voxel d'un Volume.
  */
class VolumePixelData : public QObject {
Q_OBJECT
public:
    /// Tipus de vòxel del volum
    typedef signed short int VoxelType;
    /// Tipus d'imatge intern per defecte d'itk
    typedef VoxelType ItkPixelType;
    static const unsigned int VDimension = 3;

    typedef itk::Image<ItkPixelType, VDimension> ItkImageType;
    typedef ItkImageType::Pointer ItkImageTypePointer;

    explicit VolumePixelData(QObject *parent = 0);

    /// Assignem/Retornem les dades en format ITK
    void setData(ItkImageTypePointer itkImage);
    ItkImageTypePointer getItkData();

    /// Assignem/Retornem les dades en format VTK
    void setData(vtkImageData *vtkImage);
    vtkImageData* getVtkData();

    /// Creem les dades a partir d'un buffer d'unsigned chars
    /// L'extent ha de ser coherent amb la mida de les dades del buffer i els bytesPerPixel indicats
    /// Si deleteData = cert, aquesta classe s'encarregarà de destruir el buffer quan es destrueixi aquest objecte
    /// Si deleteData = fals, (per defecte) no esborrarà el buffer
    /// Les característiques d'spacing i origin no s'assignaran amb aquest mètode. Això caldrà fer-ho accedint posteriorment a les dades vtk
    void setData(unsigned char *data, int extent[6], int bytesPerPixel, bool deleteData = false);

    /// Retorna cert si conté dades carregades.
    bool isLoaded() const;

    /// Obtenim el punter a les dades que es troben en l'índex donat. És un accés a molt baix nivell, ja que obtenim
    /// el punter de les dades. Retornem el punter transformat al tipus natiu de dades VoxelType.
    VoxelType* getScalarPointer(int x, int y, int z);
    VoxelType* getScalarPointer();

    /// Returns a VolumePixelDataIterator pointing to the voxel at index [x, y, z].
    VolumePixelDataIterator getIterator(int x, int y, int z);
    /// Returns a VolumePixelDataIterator pointing to the first voxel.
    VolumePixelDataIterator getIterator();

    /// Donada una coordenada de món, ens dóna l'índex del vòxel corresponent.
    /// Si la coordenada està dins del volum retorna true, false altrament.
    /// TODO S'espera que la coordenada sigui dins del món VTK!
    /// Caldria determinar si ha de ser així o hauria de ser DICOM o en un altre sistema.
    bool computeCoordinateIndex(const double coordinate[3], int index[3]);

    /// Donada una coordenada de món, ens dóna el valor del vòxel corresponent.
    /// Si la coordenada està dins del volum retorna true, false altrament.
    /// TODO S'espera que la coordenada sigui dins del món VTK!
    /// Caldria determinar si ha de ser així o hauria de ser DICOM o en un altre sistema.
    /// HACK S'introdueixen els paràmetres phaseNumber i numberOfPhases per poder calcular l'índex correcte dins del volum corresponent a la fase actual
    bool getVoxelValue(double coordinate[3], QVector<double> &voxelValue, int phaseNumber = 0, int numberOfPhases = 1);

    /// Donada una posició del volum, ens dona el valor coma double del vòxel corresponent
    /// Bàsicament crida el getScalarComponentAsDouble del vtkImageData
    /// TODO El mètode de vtkImageData incorpora el paràmetre int comp però no s'ha inclòs degut a que s'ha fet per realitzar un refactoring de volume i 
    /// es crida amb el paràmetre comp = 0 tal i com es feia abans desde la magicTool.
    /// TODO Intentar fer servir el getVoxelValue directament tot i que això no seria un refactor sinó una millora o canvi de comportament
    double getScalarComponentAsDouble(int x, int y, int z);

    /// S'encarrega de convertir el VolumePixelData en un pixel data neutre que permet que es faci servir en casos en
    /// els que ens quedem sense memòria o ens trobem amb altres problemes a l'hora d'intentar allotjar-ne un en memòria
    void convertToNeutralPixelData();

    /// Assigna/Obté l'origen
    void setOrigin(double origin[3]);
    void setOrigin(double x, double y, double z);
    void getOrigin(double origin[3]);

    /// Assigna/Obté l'espaiat del model al llarg dels eixos
    void setSpacing(double spacing[3]);
    void setSpacing(double x, double y, double z);
    void getSpacing(double spacing[3]);

    /// Retorna l'extent
    void getExtent(int extent[6]);

    /// Retorna el nombre de components escalars
    int getNumberOfScalarComponents();

    /// Retorna el nombre de components escalars
    int getScalarSize();
    
    /// Obté el tipus d'escalars
    int getScalarType();

    //  Obté el nombre de punts
    int getNumberOfPoints();
   
private:
    /// Filtres per importar/exportar
    typedef itk::ImageToVTKImageFilter<ItkImageType> ItkToVtkFilterType;
    typedef itk::VTKImageToImageFilter<ItkImageType> VtkToItkFilterType;

    /// Les dades en format vtk
    vtkSmartPointer<vtkImageData> m_imageDataVTK;

    /// Indica si conté dades carregades o no.
    bool m_loaded;

    /// Filtres per passar de vtk a itk
    ItkToVtkFilterType::Pointer m_itkToVtkFilter;
    VtkToItkFilterType::Pointer m_vtkToItkFilter;
};

}

#endif // UDGVOLUMEPIXELDATA_H
