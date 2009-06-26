#ifndef UDG_SETTINGSACCESSLEVELXMLREADER_H
#define UDG_SETTINGSACCESSLEVELXMLREADER_H

#include "logging.h" // TODO eliminar quan no necessitem l'#if QT_VERSION i estem actualitzem a 4.5
#include "settings.h"

#if QT_VERSION >= 0x040300

#include <QXmlStreamReader>

namespace udg {

/**
    Lector de fitxers XML en els que es defineixen els nivells d'acc�s
    que li volem configurar als settings
 */
class SettingsAccessLevelXMLReader : public QXmlStreamReader
{
public:
    SettingsAccessLevelXMLReader();
    ~SettingsAccessLevelXMLReader();

    /// Llegeix el fitxer amb les dades de nivell d'acc�s
    bool read(QIODevice *device);

    /// Ens retorna la �ltima taula de nivells d'acc�s llegida
    QMap<QString, Settings::AccessLevel> getAccessLevelTable() const;
    
private:
    /// Lectura dels diferents tags de l'arxiu XML
    void readSettingsFile();
    void readSection( QString sectionName = QString() );
    void readKey( const QString &sectionName );
    void readAccessLevel( const QString &key );
    void readUnknownElement();

private:
    /// Taula de nivells d'acc�s
    QMap<QString, Settings::AccessLevel> m_accessLevelTable;
};

} // end namespace udg

#else

class QIODevice;
namespace udg {

class SettingsAccessLevelXMLReader
{
public:
    SettingsAccessLevelXMLReader(){};
    ~SettingsAccessLevelXMLReader(){};

    bool read(QIODevice *device)
    {
        DEBUG_LOG( "No llegim arxiu XML de nivell d'acc�s de settings ja que la versi� de Qt �s inferior a 4.3" );
        INFO_LOG( "No llegim arxiu XML de nivell d'acc�s de settings ja que la versi� de Qt �s inferior a 4.3" );
    }
    QMap<QString, Settings::AccessLevel> getAccessLevelTable() const
    {
        DEBUG_LOG( "No llegim arxiu XML de nivell d'acc�s de settings ja que la versi� de Qt �s inferior a 4.3" );
        INFO_LOG( "No llegim arxiu XML de nivell d'acc�s de settings ja que la versi� de Qt �s inferior a 4.3" );
    }
};
} // end namespace udg
#endif

#endif
