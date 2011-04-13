#ifndef _LOGGING_
#define _LOGGING_

#include <QString>
#include <cstdlib> // per posar la variable d'entorn
/*!
    Aquest arxiu conté totes les macros per a fer logs en l'aplicació.
*/

// include log4cxx header files.
#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/helpers/exception.h>

/// Macro per a inicialitzar els logger
/// Definim la variable d'entorn que indica la localització
/// dels fitxers de log i llavors llegim la configuració dels logs
#define LOGGER_INIT( file ) \
    if (true) \
    { \
        QByteArray logFilePathByteArray = ("logFilePath=" + QDir::toNativeSeparators(udg::UserLogsFile)).toAscii(); \
        char *logFilePath = logFilePathByteArray.data(); \
        putenv(logFilePath); \
        log4cxx::PropertyConfigurator::configure( file ); \
    } else (void)0

/// Macro per a missatges de debug. \TODO de moment fem servir aquesta variable de qmake i funciona bé, però podria ser més adequat troba la forma d'afegir una variable pròpia, com per exemple DEBUG
#ifdef QT_NO_DEBUG
#define DEBUG_LOG( msg ) (void)0
#else
#define DEBUG_LOG( msg ) \
    if (true) \
    { \
        LOG4CXX_DEBUG( log4cxx::Logger::getLogger("development"), qPrintable( QString(msg) )) \
    } else (void)0


#endif

/// Macro per a missatges d'informació general
#define INFO_LOG( msg ) \
    if (true) \
    { \
        LOG4CXX_INFO( log4cxx::Logger::getLogger("info.release"), qPrintable( QString(msg) )) \
    } else (void)0

/// Macro per a missatges de warning
#define WARN_LOG( msg ) \
    if (true) \
    { \
        LOG4CXX_WARN( log4cxx::Logger::getLogger("info.release"), qPrintable( QString(msg) )) \
    } else (void)0

/// Macro per a missatges d'error
#define ERROR_LOG( msg ) \
    if (true) \
    { \
        LOG4CXX_ERROR( log4cxx::Logger::getLogger("errors.release"), qPrintable( QString(msg) )) \
    } else (void)0

/// Macro per a missatges d'error fatals/crítics
#define FATAL_LOG( msg ) \
    if (true) \
    { \
        LOG4CXX_FATAL( log4cxx::Logger::getLogger("errors.release"), qPrintable( QString(msg) )) \
    } else (void)0

#endif
