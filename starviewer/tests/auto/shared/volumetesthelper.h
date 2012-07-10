#ifndef VOLUMETESTHELPER_H
#define VOLUMETESTHELPER_H

#include <QString>

namespace udg{
    class Volume;
}

namespace testing
{

/// Classe que retorna Imatges per utilitzar per testing
class VolumeTestHelper
{
public:
    /// Torna un Volume amb el n�mero d'imatges, fase i imatges per fase en cada s�rie indicat.
    static udg::Volume* createVolume(int numberOfImages = 0, int m_numberOfPhases = 1, int m_numberOfSlicesPerPhase = 1);

    /// Torna un Volume amb el num d'imatges, fases i imatges, a m�s d'assignar-li al vtkImageData l'origin, spacing, dimensions i l'extent passats per par�metre
    static udg::Volume* createVolumeWithParameters(int numberOfImages, int m_numberOfPhases, int m_numberOfSlicesPerPhase, double origin[3], double spacing[3], int dimensions[3], int extent[6]);

    /// Elimina les imges del volum cridant el destructor de l'estructura Patient/Study/Series/Image que pertany a cada image
    static void cleanUp(udg::Volume *volume);
};

}

#endif // VOLUMETESTHELPER_H
