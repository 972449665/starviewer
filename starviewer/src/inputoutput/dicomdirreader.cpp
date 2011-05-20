#include "dicomdirreader.h"

// Llegeix el dicom dir
#include <dcdicdir.h>
#include <ofstring.h>
// Make sure OS specific configuration is included first
#include <osconfig.h>
// Provide the information for the tags
#include <dcdeftag.h>
#include <QStringList>
#include <QDir>
#include <QFile>

#include "status.h"
#include "patient.h"
#include "study.h"
#include "series.h"
#include "image.h"
#include "dicommask.h"
#include "logging.h"
#include "patientfiller.h"

namespace udg {

DICOMDIRReader::DICOMDIRReader()
{
    m_dicomdir = NULL;
}

DICOMDIRReader::~DICOMDIRReader()
{
}

Status DICOMDIRReader::open(const QString &dicomdirFilePath)
{
    Status state;

    // No existeix cap comanda per tancar un dicomdir, quan en volem obrir un de nou, l'única manera d'obrir un nou dicomdir, és a través del construtor de
    // DcmDicomDir, passant el path per paràmetre, per això si ja existia un Dicomdir ober, fem un delete, per tancar-lo
    if (m_dicomdir != NULL)
    {
        delete m_dicomdir;
    }

    // Guardem el directori on es troba el dicomdir
    QFileInfo dicomdirFileInfo(dicomdirFilePath);
    m_dicomdirAbsolutePath = dicomdirFileInfo.absolutePath();

    // L'estàndard del dicom indica que l'estructura del dicomdir ha d'estar guardada en un fitxer anomeant "DICOMDIR". En linux per
    // defecte en les unitats vfat, mostra els noms de fitxer que són shortname (8 o menys caràcters) en minúscules, per tant
    // quan el dicomdir estigui guardat en unitats vfat i el volguem obrir trobarem que el fitxer ones guarda la informació del
    // dicomdir es dirà "dicomdir" en minúscules, per aquest motiu busquem el fitxer dicomdir tan en majúscules com minúscules

    // Busquem el nom del fitxer que conté les dades del dicomdir
    m_dicomdirFileName = dicomdirFileInfo.fileName();

    // Comprovem si el sistema de fitxers treballa amb nom en minúscules o majúscules
    if (m_dicomdirFileName == m_dicomdirFileName.toUpper())
    {
        m_dicomFilesInLowerCase = false;
    }
    else
    {
        // Està montat en una vfat
        // indiquem que els fitxers estan en minúscules
        m_dicomFilesInLowerCase = true;
    }

    m_dicomdir = new DcmDicomDir(qPrintable(QDir::toNativeSeparators(dicomdirFilePath)));

    return state.setStatus(m_dicomdir->error());
}

// El dicomdir segueix una estructura d'abre on tenim n pacients, que tenen n estudis, que conté n series, i que conté n imatges, per llegir la informació
// hem d'accedir a través d'aquesta estructura d'arbre, primer llegim el primer pacient, amb el primer pacient, podem accedir el segon nivell de l'arbre, els
// estudis del pacient, i anar fent així fins arribar al nivell de baix de tot, les imatges,
Status DICOMDIRReader::readStudies(QList<Patient*> &outResultsStudyList, DicomMask studyMask)
{
    Status state;

    if (m_dicomdir == NULL)
    {
        // FER RETORNAR STATUS AMB ERROR
        return state.setStatus("Error: Not open dicomfile", false, 1302);
    }

    // Accedim a l'estructura d'arbres del dicomdir
    DcmDirectoryRecord *root = &(m_dicomdir->getRootRecord());
    // Accedim al primer pacient
    DcmDirectoryRecord *patientRecord = root->getSub(0);

    // En aquest primer while accedim al patient Record a nivell de dades de pacient
    while (patientRecord != NULL)
    {
        Patient *patient = fillPatient(patientRecord);

        // Si no compleix a nivelld de pacient ja no accedim als seus estudis
        if (matchPatientToDicomMask(patient, &studyMask))
        {
            // Indiquem que volem el primer estudi del pacient
            DcmDirectoryRecord *studyRecord = patientRecord->getSub(0);

            // En aquest while accedim a les dades de l'estudi
            while (studyRecord != NULL)
            {
                Study *study = fillStudy(studyRecord);

                // Comprovem si l'estudi compleix la màscara de cerca que ens han passat
                if (matchStudyToDicomMask(study, &studyMask))
                {
                    patient->addStudy(study);
                }
                else
                {
                    delete study;
                }

                // Accedim al següent estudi del pacient
                studyRecord = patientRecord->nextSub(studyRecord);
            }

            // Si cap estudi ha complert la màscara de cerca ja no afegim el pacient
            if (patient->getNumberOfStudies() > 0)
            {
                outResultsStudyList.append(patient);
            }
            else
            {
                delete patient;
            }
        }
        else
        {
            delete patient;
        }

        // Accedim al següent pacient del dicomdir
        patientRecord = root->nextSub(patientRecord);
    }

    return state.setStatus(m_dicomdir->error());
}

// Per trobar les sèries d'une estudi haurem de recorre tots els estudis dels pacients, que hi hagi en el dicomdir, fins que obtinguem l'estudi amb el UID
// sol·licitat una vegada found, podrem accedir a la seva informacio de la sèrie
Status DICOMDIRReader::readSeries(const QString &studyUID, const QString &seriesUID, QList<Series*> &outResultsSeriesList)
{
    Status state;

    if (m_dicomdir == NULL)
    {
        // FER
        return state.setStatus("Error: Not open dicomfile", false, 1302);
    }

    // Accedim a l'estructura d'abres del dicomdir
    DcmDirectoryRecord *root = &(m_dicomdir->getRootRecord());
    // Accedim al primer pacient
    DcmDirectoryRecord *patientRecord = root->getSub(0);
    DcmDirectoryRecord *studyRecord;
    OFString text;
    QString studyUIDRecord, seriesPath;
    bool found = false;

    // Accedim a nivell de pacient
    while (patientRecord != NULL && !found)
    {
        // Indiquem que volem el primer estudi del pacient
        studyRecord = patientRecord->getSub(0);

        while (studyRecord != NULL && !found)
        {
            text.clear();
            studyUIDRecord.clear();
            // Obtenim el UID de l'estudi al qual estem posicionats
            studyRecord->findAndGetOFStringArray(DCM_StudyInstanceUID, text);
            studyUIDRecord = text.c_str();
            // Busquem l'estudi que continguin el mateix UID
            if (studyUIDRecord == studyUID)
            {
                found = true;
            }
            else
            {
                // Si no trobem accedim al seguent estudi del pacient
                studyRecord = patientRecord->nextSub(studyRecord);
            }
        }

        if (!found)
        {
            // Accedim al següent pacient
            patientRecord = root->nextSub(patientRecord);
        }
    }

    // Si hem found l'estudi amb el UID que cercàvem
    if (found)
    {
        // Seleccionem la serie de l'estudi que conté el studyUID que cercàvem
        DcmDirectoryRecord *seriesRecord = studyRecord->getSub(0);

        while (seriesRecord != NULL)
        {
            Series *series = fillSeries(seriesRecord);

            if (seriesUID.length() == 0 || series->getInstanceUID() == seriesUID)
            {
                outResultsSeriesList.append(series);
            }
            else
            {
                delete series;
            }

            // Accedim a la següent sèrie de l'estudi
            seriesRecord = studyRecord->nextSub(seriesRecord);
        }
    }

    return state.setStatus(m_dicomdir->error());
}

Status DICOMDIRReader::readImages(const QString &seriesUID, const QString &sopInstanceUID, QList<Image*> &outResultsImageList)
{
    Status state;

    if (m_dicomdir == NULL)
    {
        // FER
        return state.setStatus("Error: Not open dicomfile", false, 1302);
    }

    // Accedim a l'estructura d'abres del dicomdir
    DcmDirectoryRecord *root = &(m_dicomdir->getRootRecord());
    // Accedim al primer pacient
    DcmDirectoryRecord *patientRecord = root->getSub(0);
    DcmDirectoryRecord *studyRecord, *seriesRecord;
    OFString text;
    QString studyUIDRecord, seriesUIDRecord, imagePath;
    bool found = false;

    // Accedim a nivell de pacient
    while (patientRecord != NULL && !found)
    {
        // Indiquem que volem el primer estudi del pacient
        studyRecord = patientRecord->getSub(0);

        // Accedim a nivell estudi
        while (studyRecord != NULL && !found)
        {
            // Seleccionem la serie de l'estudi que conté el studyUID que cercàvem
            seriesRecord = studyRecord->getSub(0);
            // Accedim a nivell
            while (seriesRecord != NULL && !found)
            {
                // UID Serie
                text.clear();
                seriesUIDRecord.clear();
                seriesRecord->findAndGetOFStringArray(DCM_SeriesInstanceUID, text);
                seriesUIDRecord.insert(0, text.c_str());
                // Busquem la sèrie amb les imatges
                if (seriesUIDRecord == seriesUID)
                {
                    found = true;
                }
                else
                {
                    // Accedim a la següent sèrie de l'estudi
                    seriesRecord = studyRecord->nextSub(seriesRecord);
                }
            }
            // Si no trobem accedim al seguent estudi del pacient
            studyRecord = patientRecord->nextSub(studyRecord);
        }

        if (!found)
        {
            // Accedim al següent pacient
            patientRecord = root->nextSub(patientRecord);
        }
    }

    // Si hem found la sèrie amb el UID que cercàvem
    if (found)
    {
        // Seleccionem la serie de l'estudi que conté el studyUID que cercàvem
        DcmDirectoryRecord *imageRecord = seriesRecord->getSub(0);

        while (imageRecord != NULL)
        {
            // SopUid Image
            Image *image = fillImage(imageRecord);

            if (sopInstanceUID.length() == 0 || sopInstanceUID == image->getSOPInstanceUID())
            {
                // Inserim a la llista la imatge
                outResultsImageList.append(image);
            }
            else
            {
                delete image;
            }

            // Accedim a la següent imatge de la sèrie
            imageRecord = seriesRecord->nextSub(imageRecord);
        }
    }

    return state.setStatus(m_dicomdir->error());
}

QString DICOMDIRReader::getDicomdirFilePath()
{
    return m_dicomdirAbsolutePath + "/" + m_dicomdirFileName;
}

// TODO s'haurai de mirar si es pot fer servir les funcions de readimage, readseries i readstudy, perquè aquest mètode, són els tres anteriors mètodes en un
QStringList DICOMDIRReader::getFiles(const QString &studyUID)
{
    QStringList files;
    Status state;

    if (m_dicomdir == NULL)
    {
        DEBUG_LOG("Error: Not open dicomfile");
        return files;
    }

    // Accedim a l'estructura d'abres del dicomdir
    DcmDirectoryRecord *root = &(m_dicomdir->getRootRecord());
    // Accedim al primer pacient
    DcmDirectoryRecord *patientRecord = root->getSub(0);
    DcmDirectoryRecord *studyRecord, *seriesRecord, *imageRecord;

    QString studyUIDRecord, seriesUIDRecord, imagePath;
    bool found = false;

    // Trobem primer l'estudi que volem
    while (patientRecord != NULL && !found)
    {
        // Indiquem que volem el primer estudi del pacient
        studyRecord = patientRecord->getSub(0);
        // Accedim a nivell estudi
        while (studyRecord != NULL && !found)
        {
            OFString text;
            studyUIDRecord.clear();
            studyRecord->findAndGetOFStringArray(DCM_StudyInstanceUID, text);
            studyUIDRecord = text.c_str();
            // És l'estudi que volem?
            if (studyUIDRecord == studyUID)
            {
                found = true;
            }
            else
            {
                // Si no trobem accedim al seguent estudi del pacient
                studyRecord = patientRecord->nextSub(studyRecord);
            }
        }
        if (!found)
        {
            // Accedim al següent pacient
            patientRecord = root->nextSub(patientRecord);
        }
    }

    // Si hem trobat l'uid que es demanava podem continuar amb la cerca dels arxius
    if (found)
    {
        found = false;
        // Seleccionem la serie de l'estudi que conté el studyUID que cercàvem
        seriesRecord = studyRecord->getSub(0);
        // Llegim totes les seves sèries
        while (seriesRecord != NULL && !found)
        {
            // Seleccionem cada imatge de la series
            imageRecord = seriesRecord->getSub(0);
            while (imageRecord != NULL)
            {
                OFString text;
                // Path de la imatge ens retorna el path relatiu respecte el dicomdir DirectoriEstudi/DirectoriSeries/NomImatge. Atencio retorna els directoris
                // separats per '\' (format windows)
                // Obtenim el path relatiu de la imatge
                imageRecord->findAndGetOFStringArray(DCM_ReferencedFileID, text);

                files << m_dicomdirAbsolutePath + "/" + buildImageRelativePath(text.c_str());
                // Accedim a la següent imatge de la sèrie
                imageRecord = seriesRecord->nextSub(imageRecord);
            }
            // Accedim a la següent sèrie de l'estudi
            seriesRecord = studyRecord->nextSub(seriesRecord);
        }
    }
    else
    {
        DEBUG_LOG("No s'ha trobat cap estudi amb aquest uid: " + studyUID + " al dicomdir");
    }
    return files;
}

Patient* DICOMDIRReader::retrieve(DicomMask maskToRetrieve)
{
    QStringList files = this->getFiles(maskToRetrieve.getStudyInstanceUID());

    PatientFiller patientFiller;

    QList<Patient*> patientsList = patientFiller.processFiles(files);
    if (patientsList.isEmpty())
    {
        return NULL;
    }
    else
    {
        return patientsList.first();
    }
}

// Per fer el match seguirem els criteris del PACS
bool DICOMDIRReader::matchPatientToDicomMask(Patient *patient, DicomMask *mask)
{
    if (!matchDicomMaskToPatientId(mask, patient))
    {
        return false;
    }

    if (!matchDicomMaskToPatientName(mask, patient))
    {
        return false;
    }

    return true;
}

// Per fer el match seguirem els criteris del PACS
bool DICOMDIRReader::matchStudyToDicomMask(Study *study, DicomMask *mask)
{
    if (!matchDicomMaskToStudyDate(mask, study))
    {
        return false;
    }

    if (!matchDicomMaskToStudyUID(mask, study))
    {
        return false;
    }

    return true;
}

bool DICOMDIRReader::matchDicomMaskToStudyUID(DicomMask *mask, Study *study)
{
    return mask->getStudyInstanceUID().length() == 0 || mask->getStudyInstanceUID() == study->getInstanceUID();
}

bool DICOMDIRReader::matchDicomMaskToPatientId(DicomMask *mask, Patient *patient)
{
    // Si la màscara és buida rebem  '', si té valor es rep *ID_PACIENT*
    if (mask->getPatientId().length() > 1)
    {
        QString clearedMaskPatientID = mask->getPatientId().remove("*");

        return patient->getID().contains(clearedMaskPatientID, Qt::CaseInsensitive);
    }
    else
    {
        return true;
    }
}

bool DICOMDIRReader::matchDicomMaskToStudyDate(DicomMask *mask, Study *study)
{
    QString maskStudyDate = mask->getStudyDate(), studyDate = study->getDate().toString("yyyyMMdd");

    if (maskStudyDate.length() > 0)
    {
        // Si hi ha màscara de data
        // la màscara de la data per DICOM segueix els formats :
        //  -  "YYYYMMDD-YYYYMMDD", per indicar un rang de dades
        //  - "-YYYYMMDD" per buscar estudis amb la data més petita o igual
        //  - "YYYYMMDD-" per buscar estudis amb la data més gran o igual
        //  - "YYYYMMDD" per buscar estudis d'aquella data
        // Hurem de mirar quin d'aquest formats és la nostre màscara

        if (maskStudyDate.length() == 8)
        {
            // Cas YYYYMMDDD
            return maskStudyDate == studyDate;
        }
        else if (maskStudyDate.length() == 9)
        {
            if (maskStudyDate.at(0) == '-')
            {
                // Cas -YYYYMMDD
                return maskStudyDate.mid(1, 8) >= studyDate;
            }
            else if (maskStudyDate.at(8) == '-')
            {
                // Cas YYYYMMDD-
                return maskStudyDate.mid(0, 8) <= studyDate;
            }
        }
        else if (maskStudyDate.length() == 17)
        {
            // Cas YYYYMMDD-YYYYMMDD
            return maskStudyDate.mid(0, 8) <= studyDate && maskStudyDate.mid(9, 8) >= studyDate;
        }
        else
        {
            return false;
        }
    }

    return true;
}

bool DICOMDIRReader::matchDicomMaskToPatientName(DicomMask *mask, Patient *patient)
{
    // Si la màscara és buida rebem  '' , si té valor és rep *NOM_A_CERCAR*
    if (mask->getPatientName().length() > 1)
    {
        QString clearedPatientNameMask = mask->getPatientName().remove("*");

        return patient->getFullName().contains(clearedPatientNameMask, Qt::CaseInsensitive);
    }
    else
    {
        return true;
    }
}

Patient* DICOMDIRReader::fillPatient(DcmDirectoryRecord *dcmDirectoryRecordPatient)
{
    OFString tagValue;
    Patient *patient = new Patient();

    // Nom pacient
    dcmDirectoryRecordPatient->findAndGetOFStringArray(DCM_PatientsName, tagValue);
    patient->setFullName(QString::fromLatin1(tagValue.c_str()));
    // Id pacient
    dcmDirectoryRecordPatient->findAndGetOFStringArray(DCM_PatientID, tagValue);
    patient->setID(tagValue.c_str());

    return patient;
}

Study* DICOMDIRReader::fillStudy(DcmDirectoryRecord *dcmDirectoryRecordStudy)
{
    OFString tagValue;

    Study *study = new Study();
    // Id estudi
    dcmDirectoryRecordStudy->findAndGetOFStringArray(DCM_StudyID, tagValue);
    study->setID(tagValue.c_str());

    // Hora estudi
    dcmDirectoryRecordStudy->findAndGetOFStringArray(DCM_StudyTime, tagValue);
    study->setTime(tagValue.c_str());

    // Data estudi
    dcmDirectoryRecordStudy->findAndGetOFStringArray(DCM_StudyDate, tagValue);
    study->setDate(tagValue.c_str());

    // Descripció estudi
    dcmDirectoryRecordStudy->findAndGetOFStringArray(DCM_StudyDescription, tagValue);
    study->setDescription(QString::fromLatin1(tagValue.c_str()));

    // Accession number
    dcmDirectoryRecordStudy->findAndGetOFStringArray(DCM_AccessionNumber, tagValue);
    study->setAccessionNumber(tagValue.c_str());

    // Obtenim el UID de l'estudi
    dcmDirectoryRecordStudy->findAndGetOFStringArray(DCM_StudyInstanceUID, tagValue);
    study->setInstanceUID(tagValue.c_str());

    return study;
}

Series* DICOMDIRReader::fillSeries(DcmDirectoryRecord *dcmDirectoryRecordSeries)
{
    OFString tagValue;
    Series *series = new Series;

    dcmDirectoryRecordSeries->findAndGetOFStringArray(DCM_SeriesInstanceUID, tagValue);
    series->setInstanceUID(tagValue.c_str());

    // Número de sèrie
    dcmDirectoryRecordSeries->findAndGetOFStringArray(DCM_SeriesNumber, tagValue);
    series->setSeriesNumber(tagValue.c_str());

    // Modalitat sèrie
    dcmDirectoryRecordSeries->findAndGetOFStringArray(DCM_Modality, tagValue);
    series->setModality(tagValue.c_str());

    // Protocol Name
    dcmDirectoryRecordSeries->findAndGetOFStringArray(DCM_ProtocolName, tagValue);
    series->setProtocolName(QString::fromLatin1(tagValue.c_str()));

    return series;
}

Image* DICOMDIRReader::fillImage(DcmDirectoryRecord *dcmDirectoryRecordImage)
{
    OFString tagValue;
    Image *image = new Image();

    // SopUid Image
    dcmDirectoryRecordImage->findAndGetOFStringArray(DCM_ReferencedSOPInstanceUIDInFile, tagValue);
    image->setSOPInstanceUID(tagValue.c_str());

    // Instance Number (Número d'imatge
    dcmDirectoryRecordImage->findAndGetOFStringArray(DCM_InstanceNumber, tagValue);
    image->setInstanceNumber(tagValue.c_str());

    // Path de la imatge ens retorna el path relatiu respecte el dicomdir DirectoriEstudi/DirectoriSeries/NomImatge. Atencio retorna els directoris separats
    // per '/', per linux s'ha de transformar a '\'
    // Obtenim el path relatiu de la imatge
    dcmDirectoryRecordImage->findAndGetOFStringArray(DCM_ReferencedFileID, tagValue);
    image->setPath(m_dicomdirAbsolutePath + "/" + buildImageRelativePath(tagValue.c_str()));

    return image;
}

QString DICOMDIRReader::backSlashToSlash(const QString &original)
{
    QString ret;

    ret = original;

    while (ret.indexOf("\\") != -1)
    {
        ret.replace(ret.indexOf("\\"), 1, "/");
    }

    return ret;
}

QString DICOMDIRReader::buildImageRelativePath(const QString &imageRelativePath)
{
    // Linux per defecte en les unitats vfat, mostra els noms de fitxer que són shortname (8 o menys caràcters) en
    // minúscules com que en el fitxer de dicomdir les rutes del fitxer es guarden en majúscules, m_dicomFilesInLowerCase
    // és true si s'ha troba tel fitxer dicomdir en minúscules, si és consistent el dicomdir els noms de les imatges i
    // rutes també serà en minúscules

    if (m_dicomFilesInLowerCase)
    {
        return backSlashToSlash(imageRelativePath).toLower();
    }
    else
    {
        return backSlashToSlash(imageRelativePath);
    }
}

}
