#include <QString>
#include <QMap>

#include "patientorientation.h"

namespace udg {

/**
    Aquesta classe s'encarrega de dir-nos quines operacions de rotació i flip hem d'aplicar
    sobre una imatge 2D donades la orientació inicial i la orientació desitjada.

    Per fer-ho indicarem la orientació inicial i la desitjada a través dels mètodes setInitialOrientation()
    i setDesiredOrientation(). Un cop assignades les orientacions preguntarem les operacions que cal
    aplicar sobre la imatge 2D per obtenir la orientació que es desitji.

    L'ordre en que s'apliquin les operacions indicades *és únic* i ha de ser estrictament el següent:
       1. Aplicar rotacions
       2. Aplicar flips

    Aquesta classe només contempla operacions possibles dins d'un mateix pla 2D, per tant no ens pot donar
    una fòrmula que passi una imatge adquirida en axial a sagital, per exemple.
    En aquests casos, getNumberOfClockwiseTurnsToApply() i requiresHorizontalFlip() retornaran 0 i fals respectivament.
  */
class ImageOrientationOperationsMapper {
public:
    ImageOrientationOperationsMapper();
    ~ImageOrientationOperationsMapper();

    /// Indica la orientació inicial
    void setInitialOrientation(const PatientOrientation &initialOrientation);

    /// Indica la orientació desitjada
    void setDesiredOrientation(const PatientOrientation &desiredOrientation);

    /// Ens retorna el número de girs a aplicar en el sentit de les agulles del rellotge
    /// Els valors possibles són 0, 1, 2 ó 3
    int getNumberOfClockwiseTurnsToApply();

    /// Ens indica si cal aplicar un flip horitzontal o no
    bool requiresHorizontalFlip();

private:
    /// Inicialitza els valors de la taula on mapejarem les operacions
    /// a realitzar segons les possibles orientacions
    void initializeOrientationTable();

    /// Actualitza les operacions a realitzar segons les orientacions introduides
    void updateOperations();

private:
    /// Orientacions inicial i desitjada
    PatientOrientation m_initialOrientation;
    PatientOrientation m_desiredOrientation;

    /// Nombre de girs a aplicar
    int m_clockwiseTurns;

    /// Indica si cal aplicar flip o no
    bool m_horizontalFlip;

    /// Ens indicarà si cal trobar les rotacions i flips necessaris per la combinació actual d'orientacions
    bool m_hasToUpdateOperations;

    /// QMap per guardar les operacions de rotació i flip corresponents segons les orientacions donades.
    /// La clau és una QString en la que guardarem la orientació inicial i desitjada en el següent format:
    /// "initialRowDirectionLabel\\initialColumnDirectionLabel-desiredRowDirectionLabel\\desiredColumnDirectionLabel"
    /// El valor de cada clau serà una QString amb el número de rotacions seguida de un valor que indica
    /// si es fa flip o no, separats per una coma, com per exemple "3,false"
    QMap<QString, QString> m_orientationMappingTable;
};

} // End namespace udg
