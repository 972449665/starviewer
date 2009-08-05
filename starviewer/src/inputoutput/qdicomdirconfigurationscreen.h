/***************************************************************************
 *   Copyright (C) 2005 by Grup de Gr�fics de Girona                       *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#ifndef UDGQDICOMDIRCONFIGURATIONSCREEN_H
#define UDGQDICOMDIRCONFIGURATIONSCREEN_H

#include "ui_qdicomdirconfigurationscreenbase.h"

namespace udg {

class Status;

/** Widget en el que es configuren els par�metres del programa de grabaci�
*/
class QDICOMDIRConfigurationScreen : public QWidget, private ::Ui::QDICOMDIRConfigurationScreenBase
{
Q_OBJECT

public:
    /// Constructor de la classe
    QDICOMDIRConfigurationScreen( QWidget *parent = 0 );

    ///Destructor de classe
    ~QDICOMDIRConfigurationScreen();

public slots:
    /// Aplica els canvis de la configuraci�
    bool applyChanges();

//signals:
    /// Signal que s'emet cada vegada que hi ha un canvi a la configuraci� que pot afectar al queryscreen
    //void configurationChanged(const QString& configuration);

private slots:
    /// Mostra un QDialog per especificar on es troba el programa de grabaci� en el disc dur
    void examinateDICOMDIR();

    /// Mostra un QDialog per especificar on s'han de guardar les imatges descarregades
    //void examinateCacheImagePath();

    /// Esborra tota la cach�
    //void deleteStudies();

    /// Compacta la base de dades de la cache
    //void compactCache();

    /// Slot que s'utilitza quant es fa algun canvi a la configuraci�, per activar els buttons apply
    //void enableApplyButtons();

    /// Slot que s'utilitza quant es fa algun canvia el path de la base de dades, per activar els buttons apply
    //void configurationChangedDatabaseRoot();

    /// Afegeix la '/' al final del path del directori si l'usuari no l'ha escrit
    //void cacheImagePathEditingFinish();

    /// crear base de dades
    //void createDatabase();

private:
    ///crea els connects dels signals i slots
    void createConnections();

    /// Crea els input validators necessaris pels diferents camps d'edici�.
    //void configureInputValidator();

    /// Tracta els errors que s'han produ�t a la base de dades en general
    //void showDatabaseErrorMessage( const Status &state );

    /// Carrega les dades de configuraci� del programa de grabaci�
    void loadBurningDefaults();

    ///  Aplica els canvis fets a la configuraci� de la cache
    void applyChangesDICOMDIR();

    /** Valida que els canvis de la configuraci� siguin correctes
     *  Path de la base de dades i directori dicom's existeix
     *  @return indica si els canvis son correctes
     */
    bool validateChanges();

private:
    bool m_configurationChanged; ///Indica si la configuraci� ha canviat
    bool m_createDatabase; /// Indica si s'ha comprovat demanat que es cre� la base de dades indicada a m_textDatabaseRoot
};

};// end namespace udg

#endif
