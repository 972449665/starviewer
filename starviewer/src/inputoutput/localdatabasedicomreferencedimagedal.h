/***************************************************************************
 *   Copyright (C) 2005-2006 by Grup de Gr�fics de Girona                  *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#ifndef UDGLOCALDATABASEDICOMREFERENCEDIMAGEDAL_H
#define UDGLOCALDATABASEDICOMREFERENCEDIMAGEDAL_H

#include <QString>

namespace udg {

class DICOMReferencedImage;
class DicomMask;
class DatabaseConnection;
class Series;

/** Classe que cont� els m�todes d'acc�s a la Taula DICOMReferencedImage
    @author Grup de Gr�fics de Girona  ( GGG ) <vismed@ima.udg.es>
*/
class LocalDatabaseDICOMReferencedImageDAL
{
public:

    LocalDatabaseDICOMReferencedImageDAL();
    ~LocalDatabaseDICOMReferencedImageDAL();

    /// Insereix un nou DICOMReferencedImage
    // TODO: La serie no s'hauria de passar com a par�metre, sino una nova classe generica DICOMObject d'on podriem obtenir tota la informaci� per guardar DICOMReferencedImage a la BD
    void insert(DICOMReferencedImage *newDICOMReferencedImage, Series *seriesOfParentObject);

    /// Esborra els DICOM Referenced Images que compleixin el filtre de la m�scara, nom�s es t� en compte el l'StudyUID, el SeriesUID i SOPInstanceUID del pare
    void del(const DicomMask &DICOMReferencedImageMaskToDelete);

    /// Cerca els DICOM Referenced Images que compleixen amb els criteris de la m�scara de cerca, nom�s t� en compte l'StudyUID, el SeriesUID i SOPInstanceUID del pare
    QList<DICOMReferencedImage*> query(const DicomMask &DICOMReferencedImageMaskToQuery);

    /// Retorna l'estat de la �ltima operaci� realitzada
    int getLastError() const;

    /// Connexi� de la base de dades a utilitzar
    void setDatabaseConnection(DatabaseConnection *dbConnection);

private:
    /// Construeix la sent�ncia sql per inserir un nou DICOMReferencedImage
    // TODO: La serie no s'hauria de passar com a par�metre, sino una nova classe generica DICOMObject d'on podriem obtenir tota la informaci� per guardar DICOMReferencedImage a la BD
    QString buildSqlInsert(DICOMReferencedImage *newDICOMReferencedImage, Series *seriesOfParentObject);

    /// Ens fa un ErrorLog d'una sent�ncia sql
    void logError(const QString &sqlSentence);

    /// Genera la sentencia Sql per esborrar DICOM Referenced Images, de la m�scara nom�s t� en compte per construir la sent�ncia el StudyUID, SeriesUID i SOPInstanceUID del pare
    QString buildSqlDelete(const DicomMask &DICOMReferencedImageMaskToDelete);

    /// Genera la sent�ncia del where a partir de la m�scara tenint en compte per construir la sent�ncia el SOPInstanceUID 
    QString buildWhereSentence(const DicomMask &DICOMReferencedImageMaskToDelete);

    /// Genera la sent�ncia sql per fer selectes de DICOM Referenced Images, de la m�scara nom�s t� en compte per construir la sent�ncia el StudyUID, SeriesUID i SOPInstanceUID del pare
    QString buildSqlSelect(const DicomMask &DICOMReferencedImageMaskToDelete);

    /// Emplena el DICOM Referenced Image de la fila passada per par�metre
    DICOMReferencedImage* fillDICOMReferencedImage(char **reply, int row, int columns);

private:
    /// Connexi� a la Base de Dades
    DatabaseConnection *m_dbConnection;

    /// Ultim error de Sqlite
    int m_lastSqliteError;
};

}

#endif