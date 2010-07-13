#ifndef UDGVOLUMEPIXELDATAREADER_H
#define UDGVOLUMEPIXELDATAREADER_H

#include <QObject>

class vtkImageData;

namespace udg {

/**
    Classe encarregada de llegir �nicament les dades de p�xel d'arxius d'imatge m�dica.
    Aquesta classe �nicament actuar� d'interf�cie pels diferents possibles motors de lectura
    que podem tenir (itk, gdcm, dcmtk, etc).
    Les dades es retornaran en format vtkImageData.
 */
class VolumePixelDataReader : public QObject {
Q_OBJECT
public:
    /// Tipus d'error que podem tenir al llegir
    enum PixelDataReadError { NoError = 1, SizeMismatch, InvalidFileName, MissingFile, OutOfMemory, ZeroSpacingNotAllowed, UnknownError };
    
    VolumePixelDataReader(QObject *parent = 0);
    ~VolumePixelDataReader();

    /// Donada una llista de noms de fitxer, la llegeix i omple
    /// l'estructura d'imatge que fem servir internament.
    /// Ens retorna un enter que ens indicar� si hi ha hagut alguna mena d'error en el 
    /// proc�s de lectura, seguint els valors enumerats definits a aquesta mateixa classe
    virtual int read(const QStringList &filenames) = 0;
    
    /// Ens retorna les dades en format vtkImageData
    vtkImageData* getVTKData();

signals:
    /// Ens indica el progr�s del proc�s de lectura
    void progress(int progress);

protected:
    /// Les dades d'imatge en format vtk
    vtkImageData *m_vtkImageData;

};

} // End namespace udg

#endif
