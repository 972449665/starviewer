#include "harddiskinformation.h"
#include "logging.h"
#include <QFileInfo>
#include <QDir>

#ifdef _WIN32
    #include <windows.h>
#else
    // Per statvfs
    #include <sys/statvfs.h>
#endif // _WIN32

namespace udg {

HardDiskInformation::HardDiskInformation()
{
}

HardDiskInformation::~HardDiskInformation()
{
}

quint64 HardDiskInformation::getTotalNumberOfBytes(const QString &path)
{
    return getTotalBytesPlataformEspecific(path);
}

quint64 HardDiskInformation::getNumberOfFreeBytes(const QString &path)
{
    return getFreeBytesPlataformEspecific(path);
}

quint64 HardDiskInformation::getTotalNumberOfMBytes(const QString &path)
{
    return getTotalBytesPlataformEspecific(path) / 1048576;
}

quint64 HardDiskInformation::getNumberOfFreeMBytes(const QString &path)
{
    return getFreeBytesPlataformEspecific(path) / 1048576;
}

qint64 HardDiskInformation::getDirectorySizeInBytes(const QString &directoryPath)
{
    QDir directory(directoryPath);
    QFileInfoList fileInfoList;
    QStringList directoryList;
    qint64 directorySize = 0;

    // Llista de fitxers del directori
    fileInfoList = directory.entryInfoList(QDir::Files);

    foreach (QFileInfo fileInfo, fileInfoList)
    {
        directorySize += fileInfo.size();
    }

    // Obtenim llistat de subdirectoris
    directoryList = directory.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);

    // Per cada subdirectori
    foreach (const QString &subdirectory, directoryList)
    {
        directorySize += getDirectorySizeInBytes(directoryPath + "/" + subdirectory);
    }

    return directorySize;
}

quint64 HardDiskInformation::getTotalBytesPlataformEspecific(const QString &path)
{
    quint64 total;
    bool existsError = true;

#ifdef _WIN32
    ULARGE_INTEGER totalBytes;

    if (GetDiskFreeSpaceExA(path.toAscii(), NULL, &totalBytes, NULL))
    {
        total = static_cast<quint64>(totalBytes.QuadPart);
        existsError = false;
    }
#else
    struct statvfs fsd;

    if (statvfs(path.toAscii(), &fsd) == 0)
    {
        total = static_cast<quint64>(fsd.f_blocks) * static_cast<quint64>(fsd.f_frsize);
        existsError = false;
    }
#endif // _WIN32

    if (existsError)
    {
        logLastError("getTotalBytesPlataformEspecific: " + path);
        total = 0;
    }

    return total;
}

quint64 HardDiskInformation::getFreeBytesPlataformEspecific(const QString &path)
{
    quint64 total;
    bool existsError = true;

#ifdef _WIN32
    ULARGE_INTEGER freeBytes;

    if (GetDiskFreeSpaceExA(path.toAscii(), &freeBytes, NULL, NULL))
    {
        total = static_cast<quint64>(freeBytes.QuadPart);
        existsError = false;
    }
#else
    struct statvfs fsd;

    if (statvfs(qPrintable(path), &fsd) == 0)
    {
        total = static_cast<quint64>(fsd.f_bavail) * static_cast<quint64>(fsd.f_bsize);
        existsError = false;
    }
#endif // _WIN32

    if (existsError)
    {
        logLastError("getFreeBytesPlataformEspecific: " + path);
        total = 0;
    }

    return total;
}

void HardDiskInformation::logLastError(const QString &additionalInformation)
{
    QString qtErrorMessage;
#ifdef Q_OS_WIN32
    // Obtenim el missatge d'error
    TCHAR errorMessage[512];
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, errorMessage, 1024, NULL);
    // Formatem a QString
#ifdef UNICODE
    qtErrorMessage = QString::fromUtf16((ushort *)errorMessage);
#else
    qtErrorMessage = QString::fromLocal8Bit(errorMessage);
#endif

#else
    // TODO implementar per altres sistemes (MAC, LINUX)
    qtErrorMessage = "TODO! No tenim implementat l'obtenció del missatge d'error en aquest sistema operatiu";
#endif // Q_OS_WIN32
    ERROR_LOG("Error: " + qtErrorMessage + ". " + additionalInformation);
}

}; // End udg namespace
