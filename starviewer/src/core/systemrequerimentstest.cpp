// Starviewer
#include "systemrequerimentstest.h"
#include "systemrequeriments.h"

// Qt
#include <QString>
#include <QList>
#include <QStringList>
#include <QSize>

namespace udg {

SystemRequerimentsTest::SystemRequerimentsTest(QObject *parent)
 : DiagnosisTest(parent)
{   
}

SystemRequerimentsTest::~SystemRequerimentsTest()
{
}

DiagnosisTestResult SystemRequerimentsTest::run()
{
    /// Requeriments m�nims
    SystemRequeriments *requeriments = getSystemRequeriments();
    const unsigned int MinimumNumberOfCores = requeriments->getMinimumCPUNumberOfCores();
    const unsigned int MinimumCoreSpeed = requeriments->getMinimumCPUFrequency();
    const unsigned int MinimumL2CacheSize = requeriments->getMinimumCPUL2CacheSize();
    const unsigned int MinimumGPURAM = requeriments->getMinimumGPURAM();
    const QList<QString> MinimumOpenGLExtensions = requeriments->getMinimumGPUOpenGLCompatibilities();
    const QString MinimumOpenGLVersion = requeriments->getMinimumGPUOpenGLVersion();
    const QString MinimumOSVersion = requeriments->getMinimumOperatingSystemVersion();
    const unsigned int MinimumServicePackVersion = requeriments->getMinimumOperatingSystemServicePackVersion();
    const unsigned int MinimumRAM = requeriments->getMinimumRAMTotalAmount();
    const unsigned int MinimumScreenWidth = requeriments->getMinimumScreenWidth();
    const unsigned int MinimumDiskSpace = requeriments->getMinimumHardDiskFreeSpace();

    // TODO Temporal, s'ha de treure i veure com obtenir la unitat on est� starviewer
    const QString whichHardDisk = "C:";


    SystemInformation *system = SystemInformation::newInstance();
    QString description = "";
    DiagnosisTestResult::DiagnosisTestResultState state = DiagnosisTestResult::Ok;
    
    /// Requeriments m�nims del sistema:
    // Per exemple: Dual core 1.5Ghz
    unsigned int numberOfCores = getCPUNumberOfCores(system);
    if (numberOfCores < MinimumNumberOfCores)
    {
        state = DiagnosisTestResult::Error;
        description += QString("The machine currently has %1 cores, and the minimum required is %2\n").arg(numberOfCores).arg(MinimumNumberOfCores);
    }
    else
    {
        // Una m�quina pot tenir m�s d'una CPU.
        // Buscar si alguna t� una velocitat superior al m�nim
        QList<unsigned int> cpuFrequencies = getCPUFrequencies(system);
        unsigned int maximumCPUFrequency = 0;
        for (int i = 0; i < cpuFrequencies.count(); i++)
        {
            if (cpuFrequencies.at(i) > maximumCPUFrequency)
            {
                maximumCPUFrequency = cpuFrequencies.at(i);
            }
        }
        
        if (maximumCPUFrequency < MinimumCoreSpeed)
        {
            state = DiagnosisTestResult::Error;
            description += QString("The fastest CPU runs at %1 and the minimum required is %2\n").arg(maximumCPUFrequency).arg(MinimumCoreSpeed);
        }
        // Cache de nivell 2
        unsigned int cacheSize = getCPUL2CacheSize(system);
        if (cacheSize < MinimumL2CacheSize)
        {
            state = DiagnosisTestResult::Error;
            description += QString("The cache size of the CPU is %1 and the minimum required is %2\n").arg(cacheSize).arg(MinimumL2CacheSize);
        }
    }

    // Comprovar si la versi� d'openGL del sistema �s suficient
    QString openGLVersion = getGPUOpenGLVersion(system);
    if (compareVersions(openGLVersion, MinimumOpenGLVersion) == SystemRequerimentsTest::Older)
    {
        state = DiagnosisTestResult::Error;
        description += QString("Current openGL version is %1 and the minimum required is %2").arg(openGLVersion).arg(MinimumOpenGLVersion);
    }

    // Tenir en una llista les compatibilitats openGL que starviewer utilitza i anar-les buscant una a una al retorn del m�tode
    QList<QString> openGLExtensions = getGPUOpenGLCompatibilities(system);
    for (int i = 0; i < MinimumOpenGLExtensions.count(); i++)
    {
        if (!openGLExtensions.contains(MinimumOpenGLExtensions.at(i)))
        {
            state = DiagnosisTestResult::Error;
            description += QString("Current openGL version does not support %1 extension\n").arg(MinimumOpenGLExtensions.at(i));
        }
    }
    // Mem�ria RAM de la GPU
    unsigned int gpuRAM = getGPURAM(system);
    if (gpuRAM < MinimumGPURAM)
    {
        state = DiagnosisTestResult::Error;
        description += QString("The graphics card has %1Mb of RAM and the minimum required id %2Mb\n").arg(gpuRAM).arg(MinimumGPURAM);
    }
    
    // TODO Disc dur. S'ha de fer tamb� del que cont� el directori de la cache????????
    if (getHardDiskFreeSpace(system, whichHardDisk) < MinimumDiskSpace)
    {
        state = DiagnosisTestResult::Error;
        description += "There is not enough disk space to run starviewer properly.\n";
    }

    // Arquitectura de la m�quina (32 o 64 bits)
    if (requeriments->doesOperatingSystemNeedToBe64BitArchitecutre() && !isOperatingSystem64BitArchitecture(system))
    {
        state = DiagnosisTestResult::Error;
        description += "Operating system is not 64 bit architecture.\n";
    }

    // Versi� del sistema operatiu
    QString version;
    QString servicePack;
    switch (getOperatingSystem(system))
    {
        case SystemInformation::OSWindows:
            // Si el SO �s windows, quina ha de ser la m�nima versi�??
            version = getOperatingSystemVersion(system);
            if (compareVersions(version, MinimumOSVersion) == SystemRequerimentsTest::Older)
            {
                state = DiagnosisTestResult::Error;
                description += QString("Current Operative System version is %1 and the minimum required is %2\n").arg(version).arg(MinimumOSVersion);
            }
            // Si �s windows XP (versi� 5.xx), s'ha de comprovar el service pack
            if (version.split(".").at(0).toInt() == 5)
            {
                servicePack = getOperatingSystemServicePackVersion(system);
                if (servicePack.right(servicePack.count() - 13).toUInt() < MinimumServicePackVersion)
                {
                    state = DiagnosisTestResult::Error;
                    description += QString("Current Service Pack version is %1 and the minimum required is Service Pack %2\n").arg(servicePack).arg(MinimumServicePackVersion);
                }
            }
            break;
        case SystemInformation::OSLinux:
            version = getOperatingSystemVersion(system);
            break;
        case SystemInformation::OSMac:
            version = getOperatingSystemVersion(system);
            break;
        default:
            break;
    }

    // Mem�ria RAM
    unsigned int RAMTotalAmount = getRAMTotalAmount(system);
    if (RAMTotalAmount < MinimumRAM)
    {
        state = DiagnosisTestResult::Error;
        description += QString("The total amount of RAM memory is %1 and the minimum required is %2\n").arg(RAMTotalAmount).arg(MinimumRAM);
    }

    // Si alguna de les pantalles �s menor de 1185 pixels d'amplada, poder retornar un warning, ja que starviewer no hi cap.
    QList<QSize> resolutions = getScreenResolutions(system);
    if (resolutions.count() == 1)
    {
        if (resolutions.at(0).width() < (int)MinimumScreenWidth)
        {
            state = DiagnosisTestResult::Error;
            description += "The screens is too small to fit Starviewer application.\n";
        }
    }
    else
    {
        // Nom�s retornarem aquest warning, si s'escau, en el cas de qu� la resta de requeriments es compleixin.
        if (state == DiagnosisTestResult::Ok)
        {
            int index = 0;
            bool found = false;
            while (!found && index < resolutions.count())
            {
                if (resolutions.at(index).width() < (int)MinimumScreenWidth)
                {
                    state = DiagnosisTestResult::Warning;
                    description = "One of the screens is too small. Keep in mind that Starviewer won't fit in that screen.\n";
                }
                index++;
            }
        }
    }

    // Que la unitat de CD/DVD no pugui grabar, ser� un warning si la resta de requeriments �s correcte
    if (state != DiagnosisTestResult::Error &&
        requeriments->doesOpticalDriveNeedWriteCapabilities() &&
        !doesOpticalDriveHaveWriteCapabilities(system))
    {
        state = DiagnosisTestResult::Warning;
        description += "The optical drive is not capable of writing.\n";
    }

    // Si la descripci� acaba en \n, es treu
    if (description.endsWith('\n'))
    {  
        description = description.left(description.count() - 1);
    }
    DiagnosisTestResult result;
    result.setState(state);
    result.setDescription(description);
    delete system;
    return result;
}

SystemRequerimentsTest::VersionComparison SystemRequerimentsTest::compareVersions(QString version1, QString version2)
{
    // TODO de moment nom�s estar pensat per windows. S'ha d'estendre a m�s S.O.
    
    //5.1.2600
    QStringList version1List = version1.split(".");
    QStringList version2List = version2.split(".");

    int index = 0;
    // Mentre les parts siguin iguals, anem comparant
    while (index < version1List.count() && index < version2List.count())
    {
        if (version1List.at(index).toInt() < version2List.at(index).toInt())
        {
            return SystemRequerimentsTest::Older;
        }
        else if (version1List.at(index).toInt() > version2List.at(index).toInt())
        {
            return SystemRequerimentsTest::Newer;
        }
        index++;
    }

    // Si totes les parts s�n iguals, la versi� que en tingui m�s ser� la major, ja que suposarem que la altra �s .0
    if (version1List.count() < version2List.count())
    {
        return SystemRequerimentsTest::Older;
    }
    else if (version1List.count() > version2List.count())
    {
        return SystemRequerimentsTest::Newer;
    }

    return SystemRequerimentsTest::Same;
}

unsigned int SystemRequerimentsTest::getCPUNumberOfCores(SystemInformation *system)
{
    return system->getCPUNumberOfCores();
}

QList<unsigned int> SystemRequerimentsTest::getCPUFrequencies(SystemInformation *system)
{
    return system->getCPUFrequencies();
}

unsigned int SystemRequerimentsTest::getCPUL2CacheSize(SystemInformation *system)
{
    return system->getCPUL2CacheSize();
}

QList<QString> SystemRequerimentsTest::getGPUOpenGLCompatibilities(SystemInformation *system)
{
    return system->getGPUOpenGLCompatibilities();
}

QString SystemRequerimentsTest::getGPUOpenGLVersion(SystemInformation *system)
{
    return system->getGPUOpenGLVersion();
}

unsigned int SystemRequerimentsTest::getGPURAM(SystemInformation *system)
{
    return system->getGPURAM();
}

unsigned int SystemRequerimentsTest::getHardDiskFreeSpace(SystemInformation *system, const QString &device)
{
    return system->getHardDiskFreeSpace(device);
}

SystemInformation::OperatingSystem SystemRequerimentsTest::getOperatingSystem(SystemInformation *system)
{
    return system->getOperatingSystem();
}

QString SystemRequerimentsTest::getOperatingSystemVersion(SystemInformation *system)
{
    return system->getOperatingSystemVersion();
}

QString SystemRequerimentsTest::getOperatingSystemServicePackVersion(SystemInformation *system)
{
    return system->getOperatingSystemServicePackVersion();
}

bool SystemRequerimentsTest::isOperatingSystem64BitArchitecture(SystemInformation *system)
{
    return system->isOperatingSystem64BitArchitecture();
}

unsigned int SystemRequerimentsTest::getRAMTotalAmount(SystemInformation *system)
{
    return system->getRAMTotalAmount();
}

QList<QSize> SystemRequerimentsTest::getScreenResolutions(SystemInformation *system)
{
    return system->getScreenResolutions();
}

bool SystemRequerimentsTest::doesOpticalDriveHaveWriteCapabilities(SystemInformation *system)
{
    return system->doesOpticalDriveHaveWriteCapabilities();
}

SystemRequeriments* SystemRequerimentsTest::getSystemRequeriments()
{
    return new SystemRequeriments();
}

}
