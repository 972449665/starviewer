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
    static const QString OpenFileLastPath;
    static const QString OpenDirectoryLastPath;
    static const QString OpenFileLastFileExtension;
    static const QString ApplicationMainWindowGeometry;
    // Indicar� si permetem tenir m�s d'una inst�ncia de cada extensi� (true) o �nicament una (false)
    static const QString AllowMultipleInstancesPerExtension;
    // Defineix quina �s l'extensi� que s'obrir� per defecte
    static const QString DefaultExtension;
};

} // end namespace udg 

#endif
