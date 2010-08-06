/***************************************************************************
 *   Copyright (C) 2005-2006 by Grup de Gràfics de Girona                  *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#include <sqlite3.h>
#include <QString>

#include "localdatabasepatientdal.h"
#include "databaseconnection.h"
#include "logging.h"
#include "dicommask.h"

namespace udg {

LocalDatabasePatientDAL::LocalDatabasePatientDAL(DatabaseConnection *dbConnection)
{
    m_lastSqliteError = SQLITE_OK;
    m_dbConnection = dbConnection;
}

void LocalDatabasePatientDAL::insert(Patient *newPatient)
{
    m_lastSqliteError = sqlite3_exec( m_dbConnection->getConnection(), qPrintable(buildSqlInsert(newPatient)), 0, 0, 0);

    if (getLastError() != SQLITE_OK) logError(buildSqlInsert(newPatient));
}

void LocalDatabasePatientDAL::update(Patient *patientToUpdate)
{
    m_lastSqliteError = sqlite3_exec( m_dbConnection->getConnection(), qPrintable(buildSqlUpdate(patientToUpdate)), 0, 0, 0);

    if (getLastError() != SQLITE_OK) logError(buildSqlUpdate(patientToUpdate));
}

void LocalDatabasePatientDAL::del(const DicomMask &patientMaskToDelete)
{
    m_lastSqliteError = sqlite3_exec( m_dbConnection->getConnection(), qPrintable(buildSqlDelete(patientMaskToDelete)), 0, 0, 0);

    if (getLastError() != SQLITE_OK) logError(buildSqlDelete(patientMaskToDelete));
}

QList<Patient*> LocalDatabasePatientDAL::query(const DicomMask &patientMask)
{
    int columns , rows;
    char **reply = NULL , **error = NULL;
    QList<Patient*> patientList;

    m_lastSqliteError = sqlite3_get_table(m_dbConnection->getConnection(),
                                      qPrintable(buildSqlSelect(patientMask)),
                                    &reply, &rows, &columns, error);

    if (getLastError() != SQLITE_OK)
    {
        logError (buildSqlSelect(patientMask));
        return patientList;
    }

    //index = 1 ignorem les capçaleres
    for (int index = 1; index <= rows ; index++)
    {
        patientList.append(fillPatient(reply, index, columns));
    }

    return patientList;
}

int LocalDatabasePatientDAL::getLastError()
{
    return m_lastSqliteError;
}

Patient* LocalDatabasePatientDAL::fillPatient(char **reply, int row, int columns)
{
    Patient *patient = new Patient();

    patient->setID(reply[0 + row * columns]);
    patient->setFullName(reply[1 + row * columns]);
    patient->setBirthDate(reply[2 + row * columns]);
    patient->setSex(reply[3 + row * columns]);

    return patient;
}

QString LocalDatabasePatientDAL::buildSqlSelect(const DicomMask &patientMaskToSelect)
{
    QString selectSentence, whereSentence;

    selectSentence = "Select ID, Name, Birthdate, Sex "
                       "From Patient ";

    if (!patientMaskToSelect.getPatientId().isEmpty())
        whereSentence = QString(" Where ID = '%1' ").arg( DatabaseConnection::formatTextToValidSQLSyntax( patientMaskToSelect.getPatientId() ) );

    return selectSentence + whereSentence;
}

QString LocalDatabasePatientDAL::buildSqlInsert(Patient *newPatient)
{
    QString insertSentence = QString ("Insert into Patient  (ID, Name, Birthdate, Sex) "
                                                   "values ('%1', '%2', '%3', '%4')")
                                    .arg( DatabaseConnection::formatTextToValidSQLSyntax( newPatient->getID() ) )
                                    .arg( DatabaseConnection::formatTextToValidSQLSyntax( newPatient->getFullName() ) )
                                    .arg( newPatient->getBirthDate().toString("yyyyMMdd") )
                                    .arg( DatabaseConnection::formatTextToValidSQLSyntax( newPatient->getSex() ) );

    return insertSentence;
}

QString LocalDatabasePatientDAL::buildSqlUpdate(Patient *patientToUpdate)
{
    QString updateSentence = QString ("Update Patient Set   Name = '%1', "
                                                           "Birthdate = '%2', "
                                                           "Sex = '%3' "
                                                   "Where   ID = '%4'")
                                    .arg( DatabaseConnection::formatTextToValidSQLSyntax( patientToUpdate->getFullName() ) )
                                    .arg( patientToUpdate->getBirthDate().toString("yyyyMMdd") )
                                    .arg( DatabaseConnection::formatTextToValidSQLSyntax( patientToUpdate->getSex() ) )
                                    .arg( DatabaseConnection::formatTextToValidSQLSyntax( patientToUpdate->getID() ) );
    return updateSentence;
}

QString LocalDatabasePatientDAL::buildSqlDelete(const DicomMask &patientMaskToDelete)
{
    QString deleteSentence, whereSentence = "";

    deleteSentence = "Delete From Patient ";
    if (!patientMaskToDelete.getPatientId().isEmpty())
        whereSentence = QString(" Where ID = '%1'").arg( DatabaseConnection::formatTextToValidSQLSyntax( patientMaskToDelete.getPatientId() ) );

    return deleteSentence + whereSentence;
}

void LocalDatabasePatientDAL::logError(const QString &sqlSentence)
{
    //Ingnorem l'error de clau duplicada
    if (getLastError() != SQLITE_CONSTRAINT)
    {
        ERROR_LOG("S'ha produit l'error: " + QString().setNum(getLastError()) + ", " + m_dbConnection->getLastErrorMessage() + ", al executar la seguent sentencia sql " + sqlSentence);
    }
}

}
