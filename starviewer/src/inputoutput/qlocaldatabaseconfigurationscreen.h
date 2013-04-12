#ifndef UDGQLOCALDATABASECONFIGURATIONSCREEN_H
#define UDGQLOCALDATABASECONFIGURATIONSCREEN_H

#include "ui_qlocaldatabaseconfigurationscreenbase.h"

namespace udg {

/**
    Widget en el que es configuren els paràmetres de la Base de Dades local
  */
class QLocalDatabaseConfigurationScreen : public QWidget, private Ui::QLocalDatabaseConfigurationScreenBase {
Q_OBJECT

public:
    /// Constructor de la classe
    QLocalDatabaseConfigurationScreen(QWidget *parent = 0);

    /// Destructor de classe
    ~QLocalDatabaseConfigurationScreen();

public slots:
    /// Aplica els canvis de la configuració
    bool applyChanges();

signals:
    /// Signal que s'emet cada vegada que hi ha un canvi a la configuració que pot afectar al queryscreen
    void configurationChanged(const QString &configuration);

private slots:
    /// Mostra un QDialog per especificar on es troba la base de dades de la caché
    void examinateDataBaseRoot();

    /// Mostra un QDialog per especificar on s'han de guardar les imatges descarregades
    void examinateCacheImagePath();

    /// Esborra tota la caché
    void deleteStudies();

    /// Neteja la cache. Esborra tots els estudis de la caché i reinstal·la la base de dades
    void clearCache();

    /// Compacta la base de dades de la cache
    void compactCache();

    /// Slot que s'utilitza quant es fa algun canvi a la configuració, per activar els buttons apply
    void enableApplyButtons();

    /// Slot que s'utilitza quant es fa algun canvia el path de la base de dades, per activar els buttons apply
    void configurationChangedDatabaseRoot();

    /// Afegeix la '/' al final del path del directori si l'usuari no l'ha escrit
    void cacheImagePathEditingFinish();

    /// Crear base de dades
    void createDatabase();

private:
    /// Crea els connects dels signals i slots
    void createConnections();

    /// Crea els input validators necessaris pels diferents camps d'edició.
    void configureInputValidator();

    /// Carrega les dades de configuració de la cache
    void loadCacheDefaults();

    ///  Aplica els canvis fets a la configuració de la cache
    void applyChangesCache();

    /// Valida que els canvis de la configuració siguin correctes
    ///  Path de la base de dades i directori dicom's existeix
    ///  @return indica si els canvis son correctes
    bool validateChanges();

private:
    /// Indica si s'ha comprovat demanat que es creï la base de dades indicada a m_textDatabaseRoot
    bool m_createDatabase;
};

};// end namespace udg

#endif
