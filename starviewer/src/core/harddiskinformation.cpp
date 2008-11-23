/***************************************************************************
 *   Copyright (C) 2005 by Grup de Gràfics de Girona                       *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#include "harddiskinformation.h"

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/statvfs.h> // per statvfs
#endif // _WIN32

namespace udg{

HardDiskInformation::HardDiskInformation()
{
}


HardDiskInformation::~HardDiskInformation()
{
}

quint64 HardDiskInformation::getTotalNumberOfBytes(QString path)
{
    return getTotalBytesPlataformEspecific(path);
}

quint64 HardDiskInformation::getNumberOfFreeBytes(QString path)
{
    return getFreeBytesPlataformEspecific(path);
}

quint64 HardDiskInformation::getTotalNumberOfMBytes(QString path)
{
    return getTotalBytesPlataformEspecific(path) / 1048576;
}

quint64 HardDiskInformation::getNumberOfFreeMBytes(QString path)
{
    return getFreeBytesPlataformEspecific(path) / 1048576;
}

quint64 HardDiskInformation::getTotalBytesPlataformEspecific(QString path)
{
    quint64 total;
    bool existsError = true;

#ifdef _WIN32
    ULARGE_INTEGER totalBytes;

    if ( GetDiskFreeSpaceExA(path.toAscii(), NULL, &totalBytes, NULL) )
    {
        total = static_cast<quint64>(totalBytes.QuadPart);
        existsError = false;
    }
#else
    struct statvfs fsd;

    if ( statvfs(path.toAscii(), &fsd) == 0 )
    {
        total = static_cast<quint64>( fsd.f_blocks ) * static_cast<quint64>( fsd.f_frsize );
        existsError = false;
    }

#endif // _WIN32

    if (existsError)
    {
        total = 0;
    }

    return total;
}

quint64 HardDiskInformation::getFreeBytesPlataformEspecific(QString path)
{
    quint64 total;
    bool existsError = true;

#ifdef _WIN32
    ULARGE_INTEGER freeBytes;

    if ( GetDiskFreeSpaceExA(path.toAscii(), &freeBytes, NULL, NULL) )
    {
        total = static_cast<quint64>(freeBytes.QuadPart);
        existsError = false;
    }
#else
    struct statvfs fsd;

    if ( statvfs( qPrintable(path), &fsd) == 0 )
    {
        total = static_cast<quint64>( fsd.f_bavail ) * static_cast<quint64>( fsd.f_bsize );
        existsError = false;
    }

#endif // _WIN32

    if (existsError)
    {
        total = 0;
    }

    return total;
}

}; //end udg namespace
