#ifndef UDGDICOMSEQUENCEITEM_H
#define UDGDICOMSEQUENCEITEM_H

#include <QMap>

namespace udg {

class DICOMAttribute;
class DICOMValueAttribute;
class DICOMSequenceAttribute;
class DICOMTag;

/** 
    Classe encarregada de representar els Items de les seqüències de l'estàndard DICOM. Per treballar amb seqüències s'ha de fer ús de la classe
    \c DICOMSequenceAttribute.
  */
class DICOMSequenceItem {

public:
    DICOMSequenceItem();
    ~DICOMSequenceItem();

    /// Afegeix un DICOMAttribute a la llista d'atributs
    void addAttribute(DICOMAttribute *attribute);

    /// Obté tota la llista de DICOMAttribute
    QList<DICOMAttribute*> getAttributes();

    /// Retorna l'atribut associat al tag passat per parametre. Si no disposa d'ell retorna null.
    DICOMAttribute* getAttribute(const DICOMTag &tag);

    /// Retorna l'atribut associat al tag passat per paràmetre.
    /// Si no disposa d'ell o el tag no és un valor retorna null.
    DICOMValueAttribute* getValueAttribute(const DICOMTag &tag);

    /// Retorna l'atribut associat al tag passat per paràmetre.
    /// Si no disposa d'ell o el tag no és un retorna null.
    DICOMSequenceAttribute* getSequenceAttribute(const DICOMTag &tag);

    /// Retorna el contingut de l'item en forma de text. Útil per analitzar el contingut.
    QString toString();

private:
    /// Atribut per emmagatzemar els artributs que conté l'item. S'utilitza un QMap per optimitzar la cerca d'atributs.
    QMap<QString, DICOMAttribute*> m_attributeList;
};

}

#endif
