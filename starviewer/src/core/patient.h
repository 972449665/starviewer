/***************************************************************************
 *   Copyright (C) 2005-2006 by Grup de Gràfics de Girona                  *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#ifndef UDGPATIENT_H
#define UDGPATIENT_H

#include <QObject>
#include <QString>
#include <QDate>
#include "study.h"

namespace udg {

/**
Classe que representa la unitat central de treball de l'aplicació: el Pacient. Guardarà tota la informació relacionada amb aquests ( Estudis, Sèries , etc )

    @author Grup de Gràfics de Girona  ( GGG ) <vismed@ima.udg.es>
*/
class Patient : public QObject
{
Q_OBJECT
public:
    ///Enumeració dels graus de similitud
    enum PatientsSimilarity { SamePatients, IndeterminableSimilarity, DifferentPatients };

    Patient( QObject *parent = 0 );

    /// constructor per còpia, necessari pels operadors +, +=, -, -=
    Patient( const Patient &patient, QObject *parent = 0 );

    ~Patient();

    /// Assigna/Obté el nom complet del pacient
    void setFullName( QString name );
    QString getFullName() const { return m_fullName; }

    /// Assigna/Obté l'ID del pacient
    void setID( QString id );
    QString getID() const { return m_patientID; }

    /// Assigna/Obté data de naixement
    void setBirthDate( int day , int month , int year );
    void setBirthDate( QString date );
    QString getBirthDateAsString();
    QDate getBirthDate();
    int getDayOfBirth();
    int getMonthOfBirth();
    int getYearOfBirth();

    /// Assigna/Obté sexe del pacient
    void setSex( QString sex );
    inline QString getSex() const { return m_sex; };

    /// Afegeix un nou estudi. A l'estudi se li assigna com a "parentPatient" aquest Patient.
    /// Retorna fals si existeix un estudi ja amb el mateix uid o l'uid és buit
    bool addStudy( Study *study );

    /// Li treu al pacient l'estudi amb l'UID donat
    void removeStudy( QString uid );

    /// Obté l'estudi amb l'UID donat. Si no n'hi ha cap amb aquest uid retorna nul
    Study *getStudy( QString uid );

    /// Retorna cert si aquest pacient té assignat l'estudi amb l'UID donat, fals altrament
    bool studyExists( QString uid );

    ///Mètode per obtenir el nombre d'estudis del pacient
    int getNumberOfStudies();

    /// Mètode per obtenir la llista d'estudis del pacient
    QList<Study *> getStudies() const;

    /// Mètode ràpid per trobar si hi ha una series amb el uid donat. Retorna nul si aquesta no existeix
    Series *getSeries( QString uid );

    /// Ens retorna una llista amb les sèries que estiguin seleccionades
    QList<Series *> getSelectedSeries();

    /// Ens diu si aquest arxiu pertany a alguna series del pacient
    bool hasFile( QString filename );

    /// Operador d'assignació
    Patient & operator =( const Patient &patient );

    /// Operador de fusió de pacient. Amb aquest operador podrem unificar en un sol objecte Patient la informació de dos Patient
    /// sempre que siguin identificats com a el mateix pacient. Això seria equivalent a un operador de Unió de conjunts
    // \TODO aquí caldria rumiar-se si es pot forçar la fusió encara que siguin pacients diferents aparentment.
    Patient operator+( const Patient &patient );
    Patient operator+=( const Patient &patient );

    /// Retorna si es considera que es pot identificar com al mateix pacient ( a partir de l'ID i el nom ).
    /// No compara ni els estudis ni les sèries que conté, únicament la identificació de pacient.
    PatientsSimilarity compareTo( const Patient *patient );

    /// \TODO Mètode que només serveix per debugar i que s'haurà de refer
    QString toString();

    /**
        Donada una llista de pacients d'entrada, ens retorna una nova llista on
        estaran fusionats els pacients que siguin considerats iguals. És a dir si d'un
        mateix pacient tenim dos estudis, però a la llista estan com dos pacients separats
        en la llista resultant seran un únic pacient amb els dos estudis
    */
    static QList<Patient *> mergePatients( QList<Patient *> patientsList );

    /**
     * Marca com a seleccionada la sèrie amb l'UID indicat
     * En cas que no existeixi tal UID, se selecciona la primera 
     * sèrie del primer estudi
     */
    void setSelectedSeries( const QString &selectedSeriesUID );

private:
    /// Donat un pacient, copia únicament la informació del pacient. No fa res respecte els estudis,series o imatges.
    /// La informació es matxaca i no es fa cap mena de comprovació.
    void copyPatientInformation( const Patient *patient );

    /**
     * Inserta un estudi a la llista d'estudis ordenat per data.
     * Pre: se presuposa que s'ha comprovat anteriorment que l'estudi no existeix a la llista
     * @param study
     */
    void insertStudy( Study *study );

    /**
     * Troba l'índex de l'estudi amb l'uid donat a la llista d'estudis
     * @param uid L'uid d'estudi que volem trobar
     * @return L'índex d'aquell estudi dins de la llista, -1 si no existeix l'estudi amb aquell uid.
     */
    int findStudyIndex( QString uid );

    ///mètodes per a comparar la simulitud entre noms de pacients
    double levenshteinDistance( const QString &s, const QString &t );
    double needlemanWunch2Distance( const QString &s, const QString &t );
    double needlemanWunchDistance( const QString &s, const QString &t, int gap );

    /// A partir del resultat d'una metrica aplicada a dos pacients ens diu la similaritat entre aquests
    PatientsSimilarity metricToSimilarity(double measure);

    /**
     * Crea un missatge de log per saber quins pacients estem fusionant
     * @param patient Les dades del pacient que es fusiona amb aquest
     */
    void patientFusionLogMessage( const Patient &patient );

    /// Fa tractament del nom del pacient, traient caràcters extranys, espais inicials, finals i passant a majúscules.
    /// També elimina els números que contingui en nom del pacient.
    QString clearPatientName( const QString &patientName );

    /// Fa tractament del nom del pacient, traient caràcters extranys, espais inicials, finals i passant a majúscules, 
    /// però sense eliminar els números que contingui el nom, ja que poden haver pacients anonimitzats i diferenciats 
    /// per un número, com per exemple VOLUNTARI1, VOLUNTARI2 i no són el mateix pacient, en canvi si treiem els números
    ///els detectarà com a l mateix pacient.
    QString clearStrangeSymbols( const QString &patientName );

    /// Ens diu si el nom del pacient conté números identificadors. Considerem números entre 0-9999, és un rang 
    /// suficientment gran.
    bool containtsNumericalSymbols( const QString &patientName );

private:
    /// Informació comuna de pacient per a totes les imatges que fan referència a aquest pacient. Apartat C.7.1.1 PS 3.3 DICOM.
    /// Aquests són atributs del pacient necessaris per a interpretació diagnòstica de les imatges del pacient i són comunes per
    /// a tots els estudis realitzats en el pacient. \TODO de moment no incloem cap atribut opcional

    /// Nom complet del pacient. (0010,0010) Tipus 2.
    QString m_fullName;

    /// Identificador primari donada al pacient per l'hospital. (0010,0020) Tipus 2.
    QString m_patientID;

    /// Data de naixement. (0010,0030) Tipus 2.
    QDate m_birthDate;

    /// Sexe. (0010,0040) Tipus 2. M = male, F = female, O = other
    QString m_sex;

    /// Indica si la identitat real del pacient s'ha tret dels atributs i de les dades. (0012,0062) Tipus 3. Considerem
    /// aquest paràmetre per si és necessari a l'hora d'anonimatitzar. Això ens obligaria a fer servir els tags (0012,0063) i (0012,0064)
    // \TODO aquest atribut encara no es farà servir, però és per tenir-ho en compte per si calgués.
    bool m_identityIsRemoved;

    /// Llista que conté els estudis del pacient ordenats per data
    QList<Study *> m_studiesSet;
};

}

#endif
