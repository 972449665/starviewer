#ifndef UDGFIREWALLTEST_H
#define UDGFIREWALLTEST_H

#include "diagnosistest.h"
#include "diagnosistestresult.h"
#include "diagnosistestfactoryregister.h"

namespace udg {

class FirewallAccess;

/**
    Test de diagnosis de l'aplicaci� que comprova si Starviewer t� acc�s a trav�s del firewall.
*/
class FirewallAccessTest : public DiagnosisTest {
Q_OBJECT
public:
    FirewallAccessTest(QObject *parent = 0);
    ~FirewallAccessTest();

    DiagnosisTestResult run();

    /// Retorna la descripci� del test
    QString getDescription();

protected:
    virtual FirewallAccess* createFirewall();
    virtual void checkIfStarviewerHaveAccessThroughFirewall(FirewallAccess *firewall);

};

static DiagnosisTestFactoryRegister<FirewallAccessTest> registerFirewallTest("FirewallAccessTest");

}

#endif
