#ifndef UDGWINDOWSSYSTEMINFORMATION_H
#define UDGWINDOWSSYSTEMINFORMATION_H

// Starviewer
#include "systeminformation.h"
// Qt
#include <QStringList>
// Windows
#include <Wbemidl.h>

namespace udg {

class WindowsSystemInformation : public SystemInformation {
public:
    /// Constructor per defecte
    WindowsSystemInformation();
    /// Destructor
    ~WindowsSystemInformation();
    
    OperatingSystem getOperatingSystem();
    bool isOperatingSystem64BitArchitecture();
    QString getOperatingSystemVersion();
    QString getOperatingSystemServicePackVersion();

    /// Retorna la quantitat total de mem�ria RAM en MegaBytes
    unsigned int getRAMTotalAmount();
    QList<unsigned int> getRAMModulesCapacity();
    QList<unsigned int> getRAMModulesFrequency();

    unsigned int getCPUNumberOfCores();
    /// Retorna una llista amb la freq��ncia de cada processador 
    QList<unsigned int> getCPUFrequencies();
    unsigned int getCPUL2CacheSize();

    QList<QString> getGPUBrand();
    QList<QString> getGPUModel();
    QList<unsigned int> getGPURAM();
    QList<QString> getGPUOpenGLCompatibilities();
    QString getGPUOpenGLVersion();
    QList<QString> getGPUDriverVersion();

    //Screen, Display, Monitor, Desktop, ...
    QList<QString> getScreenVendors();

    QList<QString> getHardDiskDevices();
    unsigned int getHardDiskCapacity(const QString &device); // Del disc dur que cont� la carpeta de la cache de Starviewer
    unsigned int getHardDiskFreeSpace(const QString &device);
    bool doesOpticalDriveHaveWriteCapabilities();

    unsigned int getNetworkAdapterSpeed();

protected:
    /// M�tode alternatiu per si no podem obtenir el nombre de nuclis via WMI
    unsigned int getCPUNumberOfCoresFromEnvironmentVar();

protected:
    virtual IWbemClassObject* getNextObject(IEnumWbemClassObject *enumerator);
    virtual IEnumWbemClassObject* executeQuery(QString query);
    virtual bool getProperty(IWbemClassObject *object, QString propertyName, VARIANT *propertyVariant);
    IWbemServices* initializeAPI();
    virtual void uninitializeAPI(IWbemServices *services);
    virtual QString createOpenGLContextAndGetExtensions();
    virtual QString createOpenGLContextAndGetVersion();

protected:
    IWbemServices* m_api;
};

}

#endif
