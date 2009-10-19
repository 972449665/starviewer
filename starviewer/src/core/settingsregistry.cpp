#include "settingsregistry.h"

#include "logging.h"
#include "settingsaccesslevelfilereader.h"

#include <QApplication>
#include <QFile>

namespace udg {

SettingsRegistry::SettingsRegistry()
{
    loadAccesLevelTable();
}

SettingsRegistry::~SettingsRegistry()
{
}

void SettingsRegistry::addSetting( const QString &key, const QVariant &defaultValue, Settings::Properties properties )
{
    m_keyDefaultValueAndPropertiesMap.insert( key, qMakePair(defaultValue,properties) );
}

QVariant SettingsRegistry::getDefaultValue( const QString &key )
{
    return m_keyDefaultValueAndPropertiesMap.value(key).first;
}

Settings::AccessLevel SettingsRegistry::getAccessLevel( const QString &key ) const
{
    Settings::AccessLevel accessLevel = Settings::UserLevel;

    if( m_accessLevelTable.contains(key) )
    {
        accessLevel = m_accessLevelTable.value( key );
    }

    return accessLevel;
}

Settings::Properties SettingsRegistry::getProperties( const QString &key )
{
    return m_keyDefaultValueAndPropertiesMap.value(key).second;
}

void SettingsRegistry::loadAccesLevelTable()
{
    // Al directori on s'instal�la l'aplicaci� tenim una carpeta "config" on tindrem
    // un .ini que definir� els nivells d'acc�s de cada settings
    // TODO aquesta ubicaci� encara no �s definitiva
    QString filePath = qApp->applicationDirPath() +  "/config/settingsAccessLevel.ini";
    QFile file(filePath);
    if( !file.exists() )
    {
        DEBUG_LOG( "L'arxiu [" + filePath + "] No existeix. No es poden carregar els nivells d'acc�s" );   
    }
    else
    {
        DEBUG_LOG("Llegim arxiu d'Access Level de settings: [" + filePath + "]");
        SettingsAccessLevelFileReader fileReader;
        if( fileReader.read(filePath) )
        {
            m_accessLevelTable = fileReader.getAccessLevelTable();
        }
    }
}

} // end namespace udg
