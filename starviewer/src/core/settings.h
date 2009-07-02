#ifndef UDGSETTINGS_H
#define UDGSETTINGS_H

#include <QSettings>
#include <QMap>

// Forward declarations
class QString;
class QTreeWidget;
class QSplitter;

namespace udg {

/**
    Interf�cie per manipular settings.
    La manera correcta de fer servir la classe �s declarant-la localment en el m�tode en
    el que volem accedir o escriure al setting, mai es declarar� com a variable membre d'una classe.
    L'escriptura d'un setting no es far� efectiva a disc fins que no es destrueixi la inst�ncia.
    Aquest mecanisme funciona d'aquesta manera per raons d'efici�ncia, aix� podem assignar diversos
    settings alhora sense penalitzar un acc�s a disc fins que no es destrueixi l'objecte de Settings.
*/
class Settings
{
public:
    enum AccessLevel{ UserLevel, SystemLevel };
    enum PropertiesFlag{ None = 0x0, Parseable = 0x1 };
    Q_DECLARE_FLAGS(Properties, PropertiesFlag)

    Settings();
    ~Settings();

    /// Retorna el valor per la clau demanada. Si el setting no existeix, retorna el valor 
    /// per defecte que aquesta clau tingui registrat
    QVariant getValue( const QString &key ) const;

    /// Assigna el valor al setting amb la clau indicada. Si la clau ja existeix, els valor anterior queda sobre-escrit.
    void setValue( const QString &key, const QVariant &value );

    /// Retorna cert si existeix un setting anomenat key; returna fals altrament.
    bool contains( const QString &key ) const;

    /// Elimina els settings de key i totes les sub-settings que hi hagi per sota de la clau
    void remove( const QString &key );

    /**
     * M�todes per la manipulaci� de llistes de Settings
     */

    // Tamb� existeix el tipus QSettings::SettingsMap que �s QMap<QString, QVariant>
    // TODO podr�em fer typedef QSettings::SettingsMap en comptes d'aix�
    typedef QMap<QString, QVariant> KeyValueMapType;
    typedef QList< KeyValueMapType > SettingListType;

    /**
     * Obtenci� d'informaci� de llistes de settings
     */
    /// Ens retorna l'i-�ssim (index) conjunt de valors de la llista amb clau "key"
    KeyValueMapType getListItem( const QString &key, int index );

    /// Ens retorna tota la llista de settings que hi hagi sota key
    SettingListType getList( const QString &key );

    /**
     * Modificaci� de llistes de conjunts de valors
     */
    /// Afegeix a la llista amb clau "key" un conjunt de valors
    void addListItem( const QString &key, const KeyValueMapType &item );

    /// Actualitza les dades del conjunt de valors "item" a l'�ndex index de la llista amb clau "key" 
    void setListItem( int index, const QString &key, const KeyValueMapType &item );

    /// Elimina de la llista amb clau "key" l'element i-�ssim (index)
    void removeListItem( const QString &key, int index );

    /// Afegeix una llista sencera de conjunts de valors amb clau "key". 
    /// Si existia una llista anteriorment amb aquesta clau, els valors queden
    /// sobre-escrits per la llista proporcionada
    void setList( const QString &key, const SettingListType &list );

    /**
     * M�todes per facilitar el guardar i/o restaurar la geometria de certs widgets
     */

    /// Guarda/Restaura els amples de columna del widget dins de la clau donada. 
    /// Sota la clau donada es guardaran els amples de cada columna amb nom columnWidthX on X ser� el nombre de columna
    /// L'unica implementaci� de moment �s per QTreeWidget (i classes que n'hereden). 
    /// Es sobrecarregar� el m�tode per tants widgets com calgui.
    void saveColumnsWidths( const QString &key, QTreeWidget *treeWidget );
    void restoreColumnsWidths( const QString &key, QTreeWidget *treeWidget );

    /// Guarda/Restaura la geometria d'un widget/splitter dins de la clau donada.
    void saveGeometry( const QString &key, QWidget *widget );
    void restoreGeometry( const QString &key, QWidget *widget );
    void saveGeometry( const QString &key, QSplitter *splitter );
    void restoreGeometry( const QString &key, QSplitter *splitter );

private:
    /// A partir d'una llista de claus, omplim un conjunt clau-valor. 
    /// �s necessari que li passem l'objecte qsettings (user/system) amb el que obtindr� els valors
    KeyValueMapType fillKeyValueMapFromKeyList( const QStringList &keysList, QSettings *qsettings );

    /// Traspassa el contingut del conjunt clau-valor a m_settings
    /// �s necessari que li passem l'objecte qsettings (user/system) en el que volcar� els valors
    void dumpKeyValueMap( const KeyValueMapType &item, QSettings *qsettings  );

    /// Ens retorna l'objecte adient de settings (usuari o sistema) 
    /// segons com estigui configurada la clau en q�esti�
    QSettings *getSettingsObject( const QString &key );

private:
    /// Objectes QSettings amb el que manipularem les configuracions
    QMap<int, QSettings *> m_settings;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(Settings::Properties)
} // end namespace udg

#endif

