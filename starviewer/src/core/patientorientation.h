#ifndef UDGPATIENTORIENTATION_H
#define UDGPATIENTORIENTATION_H

#include <QString>

namespace udg {

/**
    Classe que encapsular� l'atribut DICOM Patient Orientation (0020,0020). Per m�s informaci� consultar l'apartat C.7.6.1.1.1 (PS 3.3)
    En cas que aquest atribut s'assigni directament del contingut de l'arxiu DICOM constar� de dues cadenes de texte separades per \\
    que representen la direcci� de les files i columnes de la imatge respecte al pacient. 
    En cas que l'atribut es calculi a partir de la classe ImageOrientation, a m�s a m�s li afegirim una tercera cadena que indicaria 
    la direcci� en que s'apilen les imatges, �til per les reconstruccions.
  */
class PatientOrientation {
public:
    /// Etiquetes d'orientaci�
    /// Si Anatomical Orientation Type (0010,2210) �s buit o t� el valor de BIPED, 
    /// aquestes seran les abreviacions designades per les direccions anat�miques
    /// Si Anatomical Orientation Type (0010,2210) t� el valor de QUADRUPED, es farien servir altres abreviacions (animals)
    /// TODO De moment nom�s tenim en compte les orientacions de tipus "BIPED". Consultar C.7.6.1.1.1 (PS 3.3) per a m�s informaci�
    static const QString LeftLabel;
    static const QString RightLabel;
    static const QString PosteriorLabel;
    static const QString AnteriorLabel;
    static const QString HeadLabel;
    static const QString FeetLabel;

    /// Assigna la cadena d'orientaci� en format DICOM. Si aquesta t� alguna inconsist�ncia, retornar� fals, cert altrament.
    /// S'accepten cadenes buides, amb 2 i 3 �tems separats per \\ i que continguin les etiquetes d'orientaci� estipulades pel DICOM
    bool setDICOMFormattedPatientOrientation(const QString &patientOrientation);
    
    /// Retornna la cadena de la orientaci� de pacient en format DICOM
    QString getDICOMFormattedPatientOrientation() const;
    
    /// Donada una etiqueta d'orientaci�, ens retorna aquesta etiqueta per� amb els valors oposats.
    /// Per exemple, si l'etiqueta que ens donen �s RPF (Right-Posterior,Feet), el valor retornat seria LAH (Left-Anterior-Head)
    /// Les etiquetes v�lides i els seus oposats s�n les seg�ents:
    /// RightLabel:LeftLabel, AnteriorLabel:PosteriorLabel, HeadLabel:FeetLabel
    /// Si l'string donada no es correspon amb cap d'aquests valors, el valor transformat ser� '?'
    static QString getOppositeOrientationLabel(const QString &label);

private:
    /// Ens valida una cadena d'orientaci� de pacient en format DICOM. Acceptarem cadenes amb 2 o 3 elements o buides.
    /// Si la cadena �s correcta retorna cert, fals altrament
    bool validateDICOMFormattedPatientOrientationString(const QString &string);

private:
    /// La cadena d'orientaci� de pacient. Es guardar� com en el format DICOM, admetent que sigui buida o contingui 2 o 3 elements.
    QString m_patientOrientationString;
};

} // End namespace udg

#endif
