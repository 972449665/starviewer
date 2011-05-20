#include <QString>

namespace udg {

/**
    Classe amb la que encapsularem els diferents tipus d'orientacions que podem
    manegar com poden ser els plans d'adquisici� o la orienataci� del pacient
  */
class Orientation {
public:
    /// Tipus d'orientacions del pla
    enum PlaneOrientationType { Axial, Sagittal, Coronal, Oblique, NotAvailable };

    /// Orientaci� del pacient
    static const QString LeftLabel;
    static const QString RightLabel;
    static const QString PosteriorLabel;
    static const QString AnteriorLabel;
    static const QString HeadLabel;
    static const QString FeetLabel;

    /// Donada una orientaci� de pla, ens retorna la corresponent etiqueta per mostrar a l'interf�cie
    static const QString getPlaneOrientationLabel(PlaneOrientationType orientation);
};

} // End namespace udg
