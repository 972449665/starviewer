#ifndef UDGINTERFACESETTINGS_H
#define UDGINTERFACESETTINGS_H

#include "defaultsettings.h"

namespace udg {

class InterfaceSettings : public DefaultSettings
{
public:
    InterfaceSettings();
    ~InterfaceSettings();

    void init();

    /// Declaraci� de claus
    static const QString openFileLastPathKey;
    static const QString openDirectoryLastPathKey;
    static const QString openFileLastFileExtensionKey;
    static const QString applicationMainWindowGeometryKey;
    // Indicar� si permetem tenir m�s d'una inst�ncia de cada extensi� (true) o �nicament una (false)
    static const QString allowMultipleInstancesPerExtensionKey;
    // Defineix quina �s l'extensi� que s'obrir� per defecte
    static const QString defaultExtensionKey;
};

} // end namespace udg 

#endif
