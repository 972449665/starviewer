#ifndef UDGDEFAULTSETTINGS_H
#define UDGDEFAULTSETTINGS_H

// Incloem Settings perqu� sempre que fem servir una de les seves classes
// derivades voldrem accedir a settings, per tant estalviem l'include
#include "settings.h"

namespace udg {

/**
 * Classe base per definir i registrar els settings per defecte de cada m�dul d'starviewer.
 * Cada implementaci� haur� d'implementar el m�tode init() que s'encarrega de registrar els
 * settings amb els corresponents valors per defecte.
 * Per una part, cada subclasse definir� amb constants al .h totes les claus dels settings perqu� aquestes
 * siguin accessibles per a tothom, independentment de si tenen un valor definit per defecte o no.
 * D'altra banda els settings que tinguin un valor per defecte es registraran en el m�tode init()
 */
class DefaultSettings {
public:
    DefaultSettings();
    ~DefaultSettings();

    /// M�tode virtual pur. Cada subclasse el reimplentar� registrant els settings
    virtual void init() = 0;

};

} // End namespace udg

#endif
