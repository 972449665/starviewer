#ifndef UDGPATIENTORIENTATION_H
#define UDGPATIENTORIENTATION_H

#include <QString>

namespace udg {

/**
    Classe que encapsular� l'atribut DICOM Patient Orientation (0020,0020)
  */
class PatientOrientation {
public:
    /// Etiquetes d'orientaci�
    static const QString LeftLabel;
    static const QString RightLabel;
    static const QString PosteriorLabel;
    static const QString AnteriorLabel;
    static const QString HeadLabel;
    static const QString FeetLabel;

    /// Donada una etiqueta d'orientaci�, ens retorna aquesta etiqueta per� amb els valors oposats.
    /// Per exemple, si l'etiqueta que ens donen �s RPF (Right-Posterior,Feet), el valor retornat seria LAH (Left-Anterior-Head)
    /// Les etiquetes v�lides i els seus oposats s�n les seg�ents:
    /// RightLabel:LeftLabel, AnteriorLabel:PosteriorLabel, HeadLabel:FeetLabel
    /// Si l'string donada no es correspon amb cap d'aquests valors, el valor transformat ser� '?'
    static QString getOppositeOrientationLabel(const QString &label);
};

} // End namespace udg

#endif
