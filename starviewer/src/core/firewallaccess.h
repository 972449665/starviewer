#ifndef UDGFIREWALLACCESS_H
#define UDGFIREWALLACCESS_H

#include <QString>

namespace udg {

/**
    Interf�cie per comprovar si starviewer t� access a trav�s del firewall.
    El m�tode \sa newInstance �s l'encarregat de crear una inst�ncia de les classes d'implementaci� que es decideix
    segons el sistema operatiu, windows o altres. 
  */
class FirewallAccess {
public:
    enum FirewallStatus { FirewallIsAccessible, FirewallIsBlocking, FirewallError, FirewallUnknownStatus };

    virtual ~FirewallAccess();

    /// Crea una nova inst�ncia d'alguna de les classes que implementa la interf�cie
    static FirewallAccess* newInstance();

    virtual bool doesStarviewerHaveAccesThroughFirewall();

    QString getErrorString();
    FirewallStatus getStatus();

protected:
    /// Per instanciar nous objectes s'ha de fer �s del m�tode \sa newInstance
    FirewallAccess();

protected:
    /// Guarda l'estat del firewall
    FirewallStatus m_status;
    /// Si starviewer no t� acc�s a trav�s del firewall, m_errorString guarda la causa
    QString m_errorString;
};

} // End namespace udg

#endif
