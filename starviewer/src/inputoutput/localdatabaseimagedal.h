#ifndef UDGLOCALDATABASEIMAGEDAL_H
#define UDGLOCALDATABASEIMAGEDAL_H

#include <QString>
#include <QList>

#include "localdatabasebasedal.h"
#include "image.h"

namespace udg {

class DicomMask;

/**
    Aquesta classe conté els mètodes per operar amb l'objecte image en la caché de l'aplicació
  */
class LocalDatabaseImageDAL : public LocalDatabaseBaseDAL {
public:
    LocalDatabaseImageDAL(DatabaseConnection *dbConnection);

    /// Insereix la informació d'una imatge a la caché, actualitzamt l'espai ocupat de la pool, com s'ha de fer un insert i un update aquests dos operacion
    /// es fan dins el marc d'una transaccio, per mantenir coherent l'espai de la pool ocupat. Per això tot i que accedim a dos taules, al haver-se de fer
    /// dins el marc d'una transacció, necessitem fer-les dins el mateix mètode. Ja que sinó ens podríem trobar que altres operacions entressin entre insertar
    /// la imatge i updatar la pool i quedessin incloses dins la tx
    /// @param dades de la imatge
    /// @return retorna estat del mètode
    void insert(Image *newImage);

    /// Esborra les imatges que compleixin el filtre de la màscara, només es té en compte l'StudyUID, SeriesUID i SOPInstanceUID
    void del(const DicomMask &imageMaskToDelete);

    /// Actualitza la imatge passada per paràmetre
    void update(Image *imageToUpdate);

    /// Selecciona les imatges que compleixen la màscara, només es té en compte de la màscara el StudyUID, SeriesUID i SOPInstanceUID
    QList<Image*> query(const DicomMask &imageMaskToSelect);

    /// Compta les imatges que compleixin el filtre de la màscara, només es té en compte l'StudyUID, SeriesUID i SOPInstanceUID
    int count(const DicomMask &imageMaskToCount);

private:
    double m_imageOrientationPatient[6];
    double m_pixelSpacing[2];
    double m_patientPosition[3];

    /// Emplena un l'objecte imatge de la fila passada per paràmetre
    Image* fillImage(char **reply, int row, int columns);

    /// Genera la sentència sql per fer selectes d'imatges, de la màscara només té en compte per construir la sentència el StudyUID, SeriesUID i SOPInstanceUID
    QString buildSqlSelect(const DicomMask &imageMaskToSelect);

    /// Genera la sentència sql per comptar número d'imatges, de la màscara només té en compte per construir la sentència el StudyUID, SeriesUID i
    /// SOPInstanceUID
    QString buildSqlSelectCountImages(const DicomMask &imageMaskToSelect);

    /// Genera la sentència sql per inserir la nova imatge a la base de dades
    QString buildSqlInsert(Image *newImage);

    /// Genera la sentència sql per updatar la imatge a la base de dades
    QString buildSqlUpdate(Image *imageToUpdate);

    /// Genera la sentencia Sql per esborrar Imatges, de la màscara només té en compte per construir la sentència el StudyUID, SeriesUID i SOPInstanceUID
    QString buildSqlDelete(const DicomMask &imageMaskToDelete);

    /// Genera la sentència del where a partir de la màscara tenint en compte per construir la sentència el StudyUID, SeriesUID i SOPInstanceUID
    QString buildWhereSentence(const DicomMask &imageMask);

    /// Retorna el Pixel Spacing en format d'string separat per "\\"
    QString getPixelSpacingAsQString(Image *newImage);
    /// Retorna el Pixel spacing en format de double
    double* getPixelSpacingAsDouble(const QString &pixelSpacing);

    /// Retorna l'ImageOrientationPatient en format d'string separat per "\\";
    QString getImageOrientationPatientAsQString(Image *newImage);
    /// Retorna l'ImageOrientationPatient  en format de double
    double* getImageOrientationPatientAsDouble(const QString &ImageOrientationPatient);

    /// Retorna l'imagePatientPosition en format d'string separat per "\\";
    QString getPatientPositionAsQString(Image *newImage);
    /// Retorna el Patient Position en format double
    double* getPatientPositionAsDouble(const QString &patientPosition);

    /// Retorna el WindowWidth en format d'string separats per "\\";
    QString getWindowWidthAsQString(Image *newImage);

    /// Retorna el WindowWidth en format d'string separats per "\\";
    QString getWindowCenterAsQString(Image *newImage);

    /// Retorna el WindowLevelExplanation en format d'string separats per "\\";
    QString getWindowLevelExplanationAsQString(Image *newImage);

    /// Retorna el WindowLevelExplanation en una llista d'strings
    QStringList getWindowLevelExplanationAsQStringList(const QString &explanationList);

    /// Emplena el windowlevel de la imatge
    void setWindowLevel(Image *selectedImage, const QString &windowLevelWidth, const QString &windowLevelCenter);
};

}

#endif
