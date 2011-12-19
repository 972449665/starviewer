#ifndef UDGAPPLICATIONVERSIONTEST_H
#define UDGAPPLICATIONVERSIONTEST_H

#include "diagnosistest.h"
#include "diagnosistestresult.h"
#include "diagnosistestfactoryregister.h"

#include <QObject>

namespace udg {

class ApplicationVersionTest : public DiagnosisTest {
Q_OBJECT
public:
    /// Constructor per defecte
    ApplicationVersionTest(QObject *parent = 0);
    /// Destructor
    ~ApplicationVersionTest();
    /// M�tode implementat de la interf�cie DiagnosisTest.
    /// DiagnosisTestResult valdr� OK quan l'aplicaci� estigui actualitzada, Warning si hi ha alguna actualitzaci� disponible,
    /// i Error si hi ha hagut algun error.
    DiagnosisTestResult run();
    /// Retorna la descripci� del test
    QString getDescription();

signals:
    void startCheckForUpdates();

protected:
    virtual void onlineCheck();

protected:
    bool m_onlineCheckOk;
    bool m_newVersionAvailable;
    QString m_olineCheckerErrorDescription;
};

static DiagnosisTestFactoryRegister<ApplicationVersionTest> registerApplicationVersionTest("ApplicationVersionTest");

}

#endif
