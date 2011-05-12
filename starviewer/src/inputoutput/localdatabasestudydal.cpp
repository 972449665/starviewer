#include <sqlite3.h>
#include <QString>
#include <QDate>

#include "patient.h"
#include "localdatabasestudydal.h"
#include "databaseconnection.h"
#include "logging.h"
#include "dicommask.h"

namespace udg {

LocalDatabaseStudyDAL::LocalDatabaseStudyDAL(DatabaseConnection *dbConnection)
 : LocalDatabaseBaseDAL(dbConnection)
{
}

void LocalDatabaseStudyDAL::insert(Study *newStudy, const QDate &lastAccessDate)
{
    m_lastSqliteError = sqlite3_exec(m_dbConnection->getConnection(), qPrintable(buildSqlInsert(newStudy, lastAccessDate)), 0, 0, 0);

    if (getLastError() != SQLITE_OK)
    {
        logError(buildSqlInsert(newStudy, lastAccessDate));
    }
}

void LocalDatabaseStudyDAL::update(Study *studyToUpdate, const QDate &lastAccessDate)
{
    m_lastSqliteError = sqlite3_exec(m_dbConnection->getConnection(), qPrintable(buildSqlUpdate(studyToUpdate, lastAccessDate)), 0, 0, 0);

    if (getLastError() != SQLITE_OK)
    {
        logError(buildSqlUpdate(studyToUpdate, lastAccessDate));
    }
}

void LocalDatabaseStudyDAL::del(const DicomMask &studyMaskToDelete)
{
    m_lastSqliteError = sqlite3_exec(m_dbConnection->getConnection(), qPrintable(buildSqlDelete(studyMaskToDelete)), 0, 0, 0);

    if (getLastError() != SQLITE_OK)
    {
        logError(buildSqlDelete(studyMaskToDelete));
    }
}

QList<Study*> LocalDatabaseStudyDAL::queryOrderByLastAccessDate(const DicomMask &studyMask, QDate lastAccessDateMinor, QDate lastAccessDateEqualOrMajor)
{
    int columns, rows;
    char **reply = NULL, **error = NULL;
    QList<Study*> studyList;
    QString sqlSentence = buildSqlSelect(studyMask, lastAccessDateMinor, lastAccessDateEqualOrMajor) + " Order by LastAccessDate";

    m_lastSqliteError = sqlite3_get_table(m_dbConnection->getConnection(), qPrintable(sqlSentence), &reply, &rows, &columns, error);

    if (getLastError() != SQLITE_OK)
    {
        logError (sqlSentence);
        return studyList;
    }

    //index = 1 ignorem les capçaleres
    for (int index = 1; index <= rows; index++)
    {
        studyList.append(fillStudy(reply, index, columns));
    }

    sqlite3_free_table(reply);

    return studyList;
}

QList<Study*> LocalDatabaseStudyDAL::query(const DicomMask &studyMask, QDate lastAccessDateMinor, QDate lastAccessDateEqualOrMajor)
{
    int columns, rows;
    char **reply = NULL, **error = NULL;
    QList<Study*> studyList;

    m_lastSqliteError = sqlite3_get_table(m_dbConnection->getConnection(),
                                          qPrintable(buildSqlSelect(studyMask, lastAccessDateMinor, lastAccessDateEqualOrMajor)),
                                          &reply, &rows, &columns, error);

    if (getLastError() != SQLITE_OK)
    {
        logError (buildSqlSelect(studyMask, lastAccessDateMinor, lastAccessDateEqualOrMajor));
        return studyList;
    }

    //index = 1 ignorem les capçaleres
    for (int index = 1; index <= rows; index++)
    {
        studyList.append(fillStudy(reply, index, columns));
    }

    sqlite3_free_table(reply);

    return studyList;
}

QList<Patient*> LocalDatabaseStudyDAL::queryPatientStudy(const DicomMask &patientStudyMaskToQuery, QDate lastAccessDateMinor, QDate lastAccessDateEqualOrMajor)
{
    int columns, rows;
    char **reply = NULL, **error = NULL;
    QList<Patient*> patientList;

    m_lastSqliteError = sqlite3_get_table(m_dbConnection->getConnection(),
                                          qPrintable(buildSqlSelectStudyPatient(patientStudyMaskToQuery, lastAccessDateMinor, lastAccessDateEqualOrMajor)),
                                          &reply, &rows, &columns, error);
    if (getLastError() != SQLITE_OK)
    {
        logError (buildSqlSelectStudyPatient(patientStudyMaskToQuery, lastAccessDateMinor, lastAccessDateEqualOrMajor));
        return patientList;
    }

    //index = 1 ignorem les capçaleres
    for (int index = 1; index <= rows; index++)
    {
        Patient *patient = fillPatient(reply, index, columns);
        patient->addStudy(fillStudy(reply, index, columns));

        patientList.append(patient);
    }

    sqlite3_free_table(reply);

    return patientList;
}

qlonglong LocalDatabaseStudyDAL::getPatientIDFromStudyInstanceUID(const QString &studyInstanceUID)
{
    int columns, rows;
    char **reply = NULL, **error = NULL;
    qlonglong patientID = -1;

    m_lastSqliteError = sqlite3_get_table(m_dbConnection->getConnection(), qPrintable(buildSqlGetPatientIDFromStudyInstanceUID(studyInstanceUID)),
        &reply, &rows, &columns, error);

    if (getLastError() != SQLITE_OK)
    {
        logError(buildSqlGetPatientIDFromStudyInstanceUID(studyInstanceUID));
    }
    else
    {
        if (rows >= 1) //A la row 0 hi ha el header
        {
            //Si cerquem per UID només podem tenir un resultat, ja que UID és camp clau al a taula Study
            patientID = QString(reply[1]).toLongLong();
        }
    }

    sqlite3_free_table(reply);

    return patientID;
}

Study* LocalDatabaseStudyDAL::fillStudy(char **reply, int row, int columns)
{
    QString studyInstanceUID;
    Study *study = new Study();
    QStringList modalities;

    study->setInstanceUID(reply[0 + row * columns]);
    study->setID(reply[2 + row * columns]);
    study->setPatientAge(QString(reply[3 + row * columns]));
    study->setWeight(QString(reply[4 + row * columns]).toDouble());
    study->setHeight(QString(reply[5 + row * columns]).toDouble());
    study->setDate(reply[7 + row * columns]);
    study->setTime(reply[8 + row * columns]);
    study->setAccessionNumber(reply[9 + row * columns]);
    study->setDescription(reply[10 + row * columns]);
    study->setReferringPhysiciansName(reply[11 + row * columns]);
    study->setRetrievedDate(QDate().fromString(reply[13 + row * columns], "yyyyMMdd"));
    study->setRetrievedTime(QTime().fromString(reply[14 + row * columns], "hhmmss"));

    //Afegim la modalitat que estan separades per "/"
    modalities = QString(reply[6 + row * columns]).split("/");
    foreach (QString modality, modalities)
    {
        study->addModality(modality);
    }

    return study;
}

Patient* LocalDatabaseStudyDAL::fillPatient(char **reply, int row, int columns)
{
    Patient *patient = new Patient();

    patient->setDatabaseID(QString(reply[16 + row * columns]).toLongLong());
    patient->setID(reply[17 + row * columns]);
    patient->setFullName(reply[18 + row * columns]);
    patient->setBirthDate(reply[19 + row * columns]);
    patient->setSex(reply[20 + row * columns]);

    return patient;
}

//TODO: Si només acceptem com a paràmatre de filtrar de la DICOMMask l'studyInstanceUID el que s'hauria de fer és directament passar un QString amb StudyInstanceUID
QString LocalDatabaseStudyDAL::buildSqlSelect(const DicomMask &studyMaskToSelect, const QDate &lastAccessDateMinor, const QDate &lastAccessDateEqualOrMajor)
{
    QString selectSentence, whereSentence;

    selectSentence = "Select InstanceUID, PatientID, ID, PatientAge, PatientWeigth, PatientHeigth, Modalities, Date, Time, "
                            "AccessionNumber, Description, ReferringPhysicianName, LastAccessDate, RetrievedDate, RetrievedTime, "
                            "State "
                       "From Study ";

    if (!studyMaskToSelect.getStudyInstanceUID().isEmpty())
    {
        whereSentence = QString(" Where InstanceUID = '%1' ").arg(DatabaseConnection::formatTextToValidSQLSyntax(studyMaskToSelect.getStudyInstanceUID()));
    }

    if (lastAccessDateMinor.isValid())
    {
        if (!whereSentence.isEmpty())
        {
            whereSentence += " and ";
        }
        else
        {
            whereSentence = " where ";
        }

        whereSentence += QString(" LastAccessDate < '%1' ").arg(lastAccessDateMinor.toString("yyyyMMdd"));
    }

    if (lastAccessDateEqualOrMajor.isValid())
    {
        if (!whereSentence.isEmpty())
        {
            whereSentence += " and ";
        }
        else
        {
            whereSentence = " where ";
        }

        whereSentence += QString(" '%1' <= LastAccessDate ").arg(lastAccessDateEqualOrMajor.toString("yyyyMMdd"));
    }

    return selectSentence + whereSentence;
}

QString LocalDatabaseStudyDAL::buildSqlSelectStudyPatient(const DicomMask &studyMaskToSelect, const QDate &lastAccessDateMinor, const QDate &lastAccessDateEqualOrMajor)
{
    QString selectSentence, whereSentence, orderBySentence;

    selectSentence = "Select InstanceUID, PatientID, Study.ID, PatientAge, PatientWeigth, PatientHeigth, Modalities, Date, Time, "
                            "AccessionNumber, Description, ReferringPhysicianName, LastAccessDate, RetrievedDate, RetrievedTime, "
                            "Study.State, Patient.ID, Patient.DICOMPatientId, Patient.Name, Patient.Birthdate, Patient.Sex "
                       "From Study, Patient ";

    whereSentence = "Where Study.PatientID = Patient.ID ";

    if (!studyMaskToSelect.getStudyInstanceUID().isEmpty())
    {
        whereSentence += QString(" and InstanceUID = '%1' ").arg(DatabaseConnection::formatTextToValidSQLSyntax(studyMaskToSelect.getStudyInstanceUID()));
    }

    if (!studyMaskToSelect.getPatientId().isEmpty() && studyMaskToSelect.getPatientId() != "*")
    {
        whereSentence += QString(" and Patient.DICOMPatientID like '%%1%' ").arg(DatabaseConnection::formatTextToValidSQLSyntax(studyMaskToSelect.getPatientId().replace("*", "")));
    }
    if (!studyMaskToSelect.getPatientName().isEmpty() && studyMaskToSelect.getPatientName() != "*")
    {
        whereSentence += QString(" and Patient.Name like '%%1%' ").arg(DatabaseConnection::formatTextToValidSQLSyntax(studyMaskToSelect.getPatientName().replace("*", "")));
    }

    //Si filtrem per data
    if (studyMaskToSelect.getStudyDate().length() == 8)
    {
        whereSentence += QString(" and Date = '%1'").arg(studyMaskToSelect.getStudyDate());
    }
    else if (studyMaskToSelect.getStudyDate().length() == 9)
    {
        if (studyMaskToSelect.getStudyDate().at(0) == '-')
        {
            whereSentence += QString(" and Date <= '%1'").arg(studyMaskToSelect.getStudyDate().mid(1, 8));
        }
        else if (studyMaskToSelect.getStudyDate().at(8) == '-')
        {
            whereSentence += QString(" and Date >= '%1'").arg(studyMaskToSelect.getStudyDate().mid(0, 8));
        }
    }
    else if (studyMaskToSelect.getStudyDate().length() == 17)
    {
        whereSentence += QString(" and Date between '%1' and '%2'").arg(studyMaskToSelect.getStudyDate().mid(0, 8)).arg(studyMaskToSelect.getStudyDate().mid(9, 8));
    }

    if (lastAccessDateMinor.isValid())
    {
        whereSentence += QString(" and LastAccessDate < '%1' ").arg(lastAccessDateMinor.toString("yyyyMMdd"));
    }
    if (lastAccessDateEqualOrMajor.isValid())
    {
        whereSentence += QString(" and '%1' <= LastAccessDate ").arg(lastAccessDateEqualOrMajor.toString("yyyyMMdd"));
    }

    if (studyMaskToSelect.getSeriesModality().length() > 0)
    {
        whereSentence += QString(" and Modalities like '%%1%' ").arg(studyMaskToSelect.getSeriesModality());
    }

    orderBySentence = " Order by Patient.Name";

    return selectSentence + whereSentence + orderBySentence;
}

QString LocalDatabaseStudyDAL::buildSqlGetPatientIDFromStudyInstanceUID(const QString &studyInstanceUID)
{
    QString selectSentence = QString ("Select PatientID "
                                      " From Study "
                                      "Where InstanceUID = '%1'")
                                    .arg(DatabaseConnection::formatTextToValidSQLSyntax(studyInstanceUID));

    return selectSentence;
}

QString LocalDatabaseStudyDAL::buildSqlInsert(Study *newStudy, const QDate &lastAcessDate)
{
    QString insertSentence = QString ("Insert into Study   (InstanceUID, PatientID, ID, PatientAge, PatientWeigth, PatientHeigth, "
                                                           "Modalities, Date, Time, AccessionNumber, Description, "
                                                           "ReferringPhysicianName, LastAccessDate, RetrievedDate, "
                                                           "RetrievedTime , State) "
                                                   "values ('%1', %2, '%3', '%4', %5, %6, '%7', '%8', '%9', '%10', '%11', "
                                                            "'%12', '%13', '%14', '%15', %16)")
                                    .arg(DatabaseConnection::formatTextToValidSQLSyntax(newStudy->getInstanceUID()))
                                    .arg(newStudy->getParentPatient()->getDatabaseID())
                                    .arg(DatabaseConnection::formatTextToValidSQLSyntax(newStudy->getID()))
                                    .arg(DatabaseConnection::formatTextToValidSQLSyntax(newStudy->getPatientAge()))
                                    .arg(newStudy->getWeight())
                                    .arg(newStudy->getHeight())
                                    .arg(DatabaseConnection::formatTextToValidSQLSyntax(newStudy->getModalitiesAsSingleString()))
                                    .arg(newStudy->getDate().toString("yyyyMMdd"))
                                    .arg(newStudy->getTime().toString("hhmmss"))
                                    .arg(DatabaseConnection::formatTextToValidSQLSyntax(newStudy->getAccessionNumber()))
                                    .arg(DatabaseConnection::formatTextToValidSQLSyntax(newStudy->getDescription()))
                                    .arg(DatabaseConnection::formatTextToValidSQLSyntax(newStudy->getReferringPhysiciansName()))
                                    .arg(lastAcessDate.toString("yyyyMMdd"))
                                    .arg(newStudy->getRetrievedDate().toString("yyyyMMdd"))
                                    .arg(newStudy->getRetrievedTime().toString("hhmmss"))
                                    .arg("0");

    return insertSentence;
}

QString LocalDatabaseStudyDAL::buildSqlUpdate(Study *studyToUpdate, const QDate &lastAccessDate)
{
    QString updateSentence = QString ("Update Study set ID = '%1', "
                                                       "PatientAge = '%2',"
                                                       "PatientWeigth = %3, "
                                                       "PatientHeigth = %4, "
                                                       "Modalities = '%5', "
                                                       "Date = '%6', "
                                                       "Time = '%7', "
                                                       "AccessionNumber = '%8', "
                                                       "Description = '%9', "
                                                       "ReferringPhysicianName = '%10', "
                                                       "LastAccessDate = '%11', "
                                                       "RetrievedDate = '%12', "
                                                       "RetrievedTime = '%13', "
                                                       "State = %14 "
                                                "Where InstanceUid = '%15'")
                                    .arg(DatabaseConnection::formatTextToValidSQLSyntax(studyToUpdate->getID()))
                                    .arg(DatabaseConnection::formatTextToValidSQLSyntax(studyToUpdate->getPatientAge()))
                                    .arg(studyToUpdate->getWeight())
                                    .arg(studyToUpdate->getHeight())
                                    .arg(DatabaseConnection::formatTextToValidSQLSyntax(studyToUpdate->getModalitiesAsSingleString()))
                                    .arg(studyToUpdate->getDate().toString("yyyyMMdd"))
                                    .arg(studyToUpdate->getTime().toString("hhmmss"))
                                    .arg(DatabaseConnection::formatTextToValidSQLSyntax(studyToUpdate->getAccessionNumber()))
                                    .arg(DatabaseConnection::formatTextToValidSQLSyntax(studyToUpdate->getDescription()))
                                    .arg(DatabaseConnection::formatTextToValidSQLSyntax(studyToUpdate->getReferringPhysiciansName()))
                                    .arg(lastAccessDate.toString("yyyyMMdd"))
                                    .arg(studyToUpdate->getRetrievedDate().toString("yyyyMMdd"))
                                    .arg(studyToUpdate->getRetrievedTime().toString("hhmmss"))
                                    .arg("0")
                                    .arg(DatabaseConnection::formatTextToValidSQLSyntax(studyToUpdate->getInstanceUID()));

    return updateSentence;
}

//TODO: Si només acceptem com a paràmtre per eliminar de la DICOMMask l'studyInstanceUID el que s'hauria de fer és directament passar un QString amb StudyInstanceUID
QString LocalDatabaseStudyDAL::buildSqlDelete(const DicomMask &studyMaskToDelete)
{
    QString deleteSentence, whereSentence = "";

    deleteSentence = "Delete From Study ";
    if (!studyMaskToDelete.getStudyInstanceUID().isEmpty())
    {
        whereSentence = QString(" Where InstanceUID = '%1'").arg(DatabaseConnection::formatTextToValidSQLSyntax(studyMaskToDelete.getStudyInstanceUID()));
    }

    return deleteSentence + whereSentence;
}

}
