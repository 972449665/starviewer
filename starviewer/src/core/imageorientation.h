#ifndef UGDIMAGEORIENTATION_H
#define UGDIMAGEORIENTATION_H

#include <QString>
#include <QVector3D>

namespace udg {

/**
    Aquesta classe encapsula l�atribut DICOM Image Orientation (Patient) (0020,0037) que defineix les direccions dels vectors de la primera fila i columna 
    de la imatge respecte al pacient. Per m�s informaci� consultar PS 3.3, secci� C.7.6.2.1.1.

    En format DICOM aquest atribut consta de 6 valors separats per '\', essent els 3 primers el vector de la fila i els 3 �ltims el vector de la columna.

    Aquesta classe, a m�s a m�s guarda la normal del pla que formen aquests dos vectors.
  */
class ImageOrientation {
public:
    ImageOrientation();
    ~ImageOrientation();
    
    /// Constructor a partir d'un string d'orientaci� en format DICOM
    ImageOrientation(const QString &dicomFormattedImageOrientationString);
    
    /// Constructor a partir de dos vectors 3D
    ImageOrientation(const QVector3D &rowVector, const QVector3D &columnVector);
    
    /// Assigna la orientaci� proporcionada en el format estipulat pel DICOM: 6 valors separats per '\'
    /// Si la cadena no est� en el format esperat, es mant� el valor que tenia fins aquell moment i es retorna fals, cert altrament.
    bool setDICOMFormattedImageOrientation(const QString &imageOrientation);

    /// Ens retorna la orientaci� en el format estipulat pel DICOM: 6 valors separats per '\'
    /// En cas que el valor no s'hagi assignat la cadena ser� buida.
    QString getDICOMFormattedImageOrientation() const;

    /// Assigna la orientaci� a trav�s dels 2 vectors 3D corresponents a les direccions de la fila i de la columna respectivament
    void setRowAndColumnVectors(const QVector3D &rowVector, const QVector3D &columnVector);
    
    /// Ens retorna els vectors fila, columna i normal respectivament.
    /// En cas que no s'hagi assignat cap orientaci�, els vectors no tindran cap element
    QVector3D getRowVector() const;
    QVector3D getColumnVector() const;
    QVector3D getNormalVector() const;

    /// Ens diu si cont� algun valor o no
    bool isEmpty() const;

    /// Operador c�pia
    ImageOrientation& operator=(const ImageOrientation &imageOrientation);

    /// Operador igualtat
    bool operator==(const ImageOrientation &imageOrientation) const;

private:
    /// Crea els vectors de doubles (row, column, normal) a partir del valor de l'string d'orientaci�
    /// Si l'string �s buit, els vectors tamb� ho seran
    void buildVectorsFromOrientationString();

    /// Inicialitza correctament els vectors de doubles que contenen les orientacions
    void initializeVectors();

private:
    /// Guarda la orientaci� en el format estipulat pel DICOM: 6 valors separats per '\'
    QString m_imageOrientationString;

    /// Els 3 vectors de la orientaci� de la imatge.
    /// Els vectors fila i columna els obtindrem directament a partir de la cadena assignada amb setDICOMFormattedImageOrientation()
    /// El vector normal de la orientaci� de la imatge es calcular� fent el producte vectorial dels vectors fila i colummna
    QVector3D m_rowVector;
    QVector3D m_columnVector;
    QVector3D m_normalVector;
};

} // End namespace udg

#endif
