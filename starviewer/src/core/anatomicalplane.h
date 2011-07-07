#ifndef UDGANATOMICALPLANE_H
#define UDGANATOMICALPLANE_H

class QString;

namespace udg {

class PatientOrientation;

/**
    Classe que encapsular� les orientacions anat�miques del pacient
  */
class AnatomicalPlane {
public:
    /// Tipus d'orientacions anat�miques
    enum AnatomicalPlaneType { Axial, Sagittal, Coronal, Oblique, NotAvailable };

    /// Donada una orientaci� anat�mica, ens retorna la corresponent etiqueta per mostrar a l'interf�cie
    static const QString getLabel(AnatomicalPlaneType orientation);

    /// Ens retorna l'etiqueta/tipus del pla anat�mic que es correspon amb el PatientOrientation donat
    static const QString getLabelFromPatientOrientation(const PatientOrientation &orientation);
    static const AnatomicalPlaneType getPlaneTypeFromPatientOrientation(const PatientOrientation &orientation);
};

} // End namespace udg

#endif
