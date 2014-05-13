// Starviewer
#include "systemrequeriments.h"

namespace udg {

SystemRequeriments::SystemRequeriments()
{
    m_minimumNumberOfCores = 4;
    m_minimumCoreSpeed = 2457; //2.4GHz
    m_minimumGPURAM = 256; // Mbytes
    m_minimumGPUOpenGLVersion = "2.1";
    m_minimumOSVersion = "5.0"; // XP
    m_minimum32bitServicePackVersion = 3; // XP service pack 3
    m_minimum64bitServicePackVersion = 2; // XP service pack 2
    m_minimumRAM = 4000; // 4Gb
    m_minimumScreenWidth = 1185; // La m�nima amplada que pot tenir starviewer (si s'afageixen controls a la pantalla, s'ha de modificar)

    // Quan s'estableixin quines s�n les extensions d'openGL que es necessiten per cada cosa, es poden afegir aqu�
    //m_minimumOpenGLExtensions << "GL_ARB_flux_capacitor";
    
    m_minimumDiskSpace = 5120; // 5 GB (en principi, el m�nim que es necessita per la cache est� en un setting)

    m_doesOpticalDriveNeedsToWrite = true;
}

SystemRequeriments::~SystemRequeriments()
{
}

unsigned int SystemRequeriments::getMinimumCPUNumberOfCores()
{
    return m_minimumNumberOfCores;
}

unsigned int SystemRequeriments::getMinimumCPUFrequency()
{
    return m_minimumCoreSpeed;
}

QStringList SystemRequeriments::getMinimumGPUOpenGLCompatibilities()
{
    return m_minimumOpenGLExtensions;
}

QString SystemRequeriments::getMinimumGPUOpenGLVersion()
{
    return m_minimumGPUOpenGLVersion;
}

unsigned int SystemRequeriments::getMinimumGPURAM()
{
    return m_minimumGPURAM;
}

unsigned int SystemRequeriments::getMinimumHardDiskFreeSpace()
{
    return m_minimumDiskSpace;
}

QString SystemRequeriments::getMinimumOperatingSystemVersion()
{
    return m_minimumOSVersion;
}

unsigned int SystemRequeriments::getMinimum32bitOperatingSystemServicePackVersion()
{
    return m_minimum32bitServicePackVersion;
}

unsigned int SystemRequeriments::getMinimum64bitOperatingSystemServicePackVersion()
{
    return m_minimum64bitServicePackVersion;
}

bool SystemRequeriments::doesOperatingSystemNeedToBe64BitArchitecutre()
{
    return m_doesOperatingSystemNeedToBe64BitArchitecutre;
}

unsigned int SystemRequeriments::getMinimumRAMTotalAmount()
{
    return m_minimumRAM;
}

unsigned int SystemRequeriments::getMinimumScreenWidth()
{
    return m_minimumScreenWidth;
}

bool SystemRequeriments::doesOpticalDriveNeedWriteCapabilities()
{
    return m_doesOpticalDriveNeedsToWrite;
}

}
