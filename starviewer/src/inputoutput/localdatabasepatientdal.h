#ifndef UDGLOCALDATABASEPATIENTDAL_H
#define UDGLOCALDATABASEPATIENTDAL_H

#include <QList>

#include "localdatabasebasedal.h"
#include "patient.h"

namespace udg {

class DicomMask;

/** Aquesta classe s'encarrega de dur a terme les operacions relacionades amb l'objecte estudi de la cache de l'aplicació.
*/
class LocalDatabasePatientDAL : public LocalDatabaseBaseDAL {
public:

    LocalDatabasePatientDAL(DatabaseConnection *dbConnection);

    ///insereix el nou pacient, i emplena el camp DatabaseID de Patient amb el ID de Pacient de la BD.
    void insert(Patient *newPatient);

    ///updata el pacient
    void update(Patient *patientToUpdate);

    ///Esborra els estudis pacients que compleixen amb els criteris de la màscara, només té en compte el Patient Id
    void del(qlonglong patientID);

    ///Cerca els pacients que compleixen amb els criteris de la màscara de cerca, només té en compte el Patient Id
    QList<Patient*> query(const DicomMask &patientMaskToQuery);
private :

    ///Construeix la sentència sql per inserir el nou pacient
    QString buildSqlInsert(Patient *newPatient);

    ///Construeix la sentència updatar el pacient
    QString buildSqlUpdate(Patient *patientToUpdate);

    ///Construeix la setència per fer select de pacients a partir de la màscara, només té en compte el PatientID
    QString buildSqlSelect(const DicomMask &patientMaskToSelect);

    ///Construeix la setència per esborrar pacients a partir de la màscara, només té en compte el Patient Id
    QString buildSqlDelete(qlonglong patientID);

    ///Emplena un l'objecte series de la fila passada per paràmetre
    Patient* fillPatient(char **reply, int row, int columns);
};
}

#endif
