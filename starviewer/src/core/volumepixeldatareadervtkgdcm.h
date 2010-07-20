#ifndef UDGVOLUMEPIXELDATAREADERVTKGDCM_H
#define UDGVOLUMEPIXELDATAREADERVTKGDCM_H

#include "volumepixeldatareader.h"

class vtkGDCMImageReader;
class vtkEventQtSlotConnect;

namespace udg {

/**
    Lector de dades d'imatge per Volume. 
    Aquest lector fa �s de la interf�cie vtkGDCM per llegir les dades.
 */
class VolumePixelDataReaderVTKGDCM : public VolumePixelDataReader {
Q_OBJECT
public:
    VolumePixelDataReaderVTKGDCM(QObject *parent = 0);
    ~VolumePixelDataReaderVTKGDCM();

    int read(const QStringList &filenames);

private slots:
    /// Captura el senyal de vtk perqu� poguem notificar el progr�s de la lectura
    void slotProgress();

private:
    // Lector vtkGDCM + progress
    vtkGDCMImageReader *m_vtkGDCMReader;
    vtkEventQtSlotConnect *m_vtkQtConnections;
};

} // End namespace udg

#endif
