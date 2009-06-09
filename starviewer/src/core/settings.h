#ifndef UDGSETTINGS_H
#define UDGSETTINGS_H

#include <QSettings>

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
    Settings();
    ~Settings();

    /// Retorna el valor per la clau demanada. Si el setting no existeix, retorna defaultValue.
    /// Si no s'especifica cap valor per defecte, es retorna una default QVariant.
    QVariant getValue( const QString &key, const QVariant &defaultValue = QVariant() ) const;

    /// Assigna el valor al setting amb la clau indicada. Si la clau ja existeix, els valor anterior queda sobre-escrit.
    void setValue( const QString &key, const QVariant &value );

    /// Retorna cert si existeix un setting anomenat key; returna fals altrament.
    bool contains( const QString &key ) const;

    /// Elimina els settings de key i totes les sub-settings que hi hagi per sota de la clau
    void remove( const QString &key );

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
    /// Objecte QSettings amb el que manipularem les configuracions
    QSettings m_settings;
};

} // end namespace udg

#endif

