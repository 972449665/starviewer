#ifndef UDGANATOMICALPLANE_H
#define UDGANATOMICALPLANE_H

class QString;

namespace udg {

/**
    Classe que encapsular� les orientacions anat�miques del pacient
  */
class AnatomicalPlane {
public:
    /// Tipus d'orientacions anat�miques
    enum AnatomicalPlaneType { Axial, Sagittal, Coronal, Oblique, NotAvailable };

    /// Donada una orientaci� anat�mica, ens retorna la corresponent etiqueta per mostrar a l'interf�cie
    static const QString getLabel(AnatomicalPlaneType orientation);

    /// Ens diu quin �s el pla de projecci� d'una imatge segons les etiquetes d'orientaci� (R/L,A/P,F/H)
    /// El format ser� "direcci�Files\\direcci�Columnes"
    /// Valors: AXIAL, SAGITAL, CORONAL, OBLIQUE o N/A
    static const QString getProjectionLabelFromPlaneOrientation(const QString &orientation);
};

} // End namespace udg

#endif
