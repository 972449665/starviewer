/***************************************************************************
 *   Copyright (C) 2005-2006 by Grup de Gr�fics de Girona                  *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#ifndef UDGLOCALDATABASEKEYIMAGENOTEDAL_H
#define UDGLOCALDATABASEKEYIMAGENOTEDAL_H

#include <QString>

namespace udg {

class KeyImageNote;
class DicomMask;
class DatabaseConnection;

/** Classe que cont� els m�todes d'acc�s a la Taula KeyImageNote
    @author Grup de Gr�fics de Girona  ( GGG ) <vismed@ima.udg.es>
*/
class LocalDatabaseKeyImageNoteDAL
{
public:
    LocalDatabaseKeyImageNoteDAL();
    ~LocalDatabaseKeyImageNoteDAL();
    
    /// Insereix un nou Key Image Note
    void insert(KeyImageNote *newKeyImageNote);

    /// Actualitza un Key Image Note
    void update(KeyImageNote *keyImageNoteToUpdate);

    /// Esborra els Key Image Notes que compleixin el filtre de la m�scara, nom�s es t� en compte l'StudyUID, SeriesUID i SOPInstanceUID
    void del(const DicomMask &keyImageNoteMaskToDelete);

    /// Cerca els Key Image Notes que compleixen amb els criteris de la m�scara de cerca, nom�s t� en compte l'StudyUID, el SeriesUID i SOPInstanceUID
    QList<KeyImageNote*> query(const DicomMask &keyImageNoteMaskToQuery);
    
    /// Retorna l'estat de la �ltima operaci� realitzada
    int getLastError() const;

    /// Connexi� de la base de dades a utilitzar
    void setDatabaseConnection(DatabaseConnection *dbConnection);

private:
    /// Construeix la sent�ncia sql per inserir el nou Key Image Note
    QString buildSqlInsert(KeyImageNote *newKeyImageNote);

    /// Construeix la sent�ncia sql per actualitzar un Key Image Note
    QString buildSqlUpdate(KeyImageNote *keyImageNoteToUpdate);

    /// Genera la sentencia Sql per esborrar Key Image Notes, de la m�scara nom�s t� en compte per construir la sent�ncia el StudyUID, SeriesUID i SOPInstanceUID 
    QString buildSqlDelete(const DicomMask &keyImageNoteMaskToDelete);

    /// Genera la sent�ncia sql per fer selectes de Key Image Notes, de la m�scara nom�s t� en compte per construir la sent�ncia el StudyUID, SeriesUID i SOPInstanceUID 
    QString buildSqlSelect(const DicomMask &keyImageNoteMaskToSelect);

    /// Genera la sent�ncia del where a partir de la m�scara tenint en compte per construir la sent�ncia el StudyUID, SeriesUID i SOPInstanceUID 
    QString buildWhereSentence(const DicomMask &keyImageNoteMask);

    /// Emplena el Key Image Note de la fila passada per par�metre
    KeyImageNote* fillKeyImageNote(char **reply, int row, int columns);

    /// Ens fa un ErrorLog d'una sent�ncia sql
    void logError(const QString &sqlSentence);

private:
    /// Connexi� a la Base de Dades
    DatabaseConnection *m_dbConnection;

    /// Ultim error de Sqlite
    int m_lastSqliteError;

};

}

#endif