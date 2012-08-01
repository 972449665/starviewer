#ifndef UDG_DICOMFORMATTEDVALUESCONVERTER_H
#define UDG_DICOMFORMATTEDVALUESCONVERTER_H

#include "windowlevel.h"
#include <QList>

class QString;

namespace udg {

/** 
    Classe helper per convertir dades en format DICOM a estructures de dades i classes pr�pies
 */
class DICOMFormattedValuesConverter {
public:
    DICOMFormattedValuesConverter();
    ~DICOMFormattedValuesConverter();

    /// Donats els valors en string de window width/center i explanation construeix la llista d'objectes Window Level pertinent
    /// Si els valors de les cadenes windowWidthString i windowCenterString no s�n Decimal Strings es retornar� una llista buida
    /// Si el nombre d'elements de windowWidthString i windowCenterString no �s el mateix es crear� una llista que tingui tants
    /// elements com la llista amb menys elements. Els elements de la llista m�s gran seran descartats
    /// explanationString no cal que tingui el mateix nombre d'elements que windowWidthString i windowCenterString, ja que es considera opcional.
    /// En cas que el nombre d'elements d'explanationString difereixi, nom�s s'afegiran aquells que estiguin dins del rang del nombre de window levels
    static QList<WindowLevel> parseWindowLevelValues(const QString &windowWidthString, const QString &windowCenterString, const QString &explanationString = QString());
};

}

#endif
