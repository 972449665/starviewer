/***************************************************************************
 *   Copyright (C) 2005 by Grup de Gràfics de Girona                       *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#include "databaseinstallation.h"

#include <QDir>
#include <QFile>
#include <QString>
#include <QMessageBox>
#include <sqlite3.h>

#include "databaseconnection.h"
#include "logging.h"
#include "deletedirectory.h"
#include "localdatabasemanager.h"
#include "starviewerapplication.h"

namespace udg {

DatabaseInstallation::DatabaseInstallation()
 : m_qprogressDialog(0)
{
}

DatabaseInstallation::~DatabaseInstallation()
{
}

bool DatabaseInstallation::checkStarviewerDatabase()
{
    LocalDatabaseManager localDatabaseManager;
    m_errorMessage = "";
    bool isCorrect;

    //Comprovem que existeix el path on s'importen les imatges, sinó existeix l'intentarà crear
    isCorrect = checkLocalImagePath();

    if (!existsDatabaseFile())
    {
        if (!createDatabaseFile())
        {
            ERROR_LOG("Error no s'ha pogut crear la base de dades a " + LocalDatabaseManager::getDatabaseFilePath());
			m_errorMessage.append( tr("\nCan't create database, be sure you have write permissions on database directory.") );
            isCorrect = false;
        }
    }
    else
    {
        // comprovar que tenim permisos d'escriptura a la BDD
        if(!isDatabaseFileWritable())
        {
            // TODO què fem? cal retornar fals? Avisar a l'usuari?
            ERROR_LOG("L'arxiu de base de dades [" + LocalDatabaseManager::getDatabaseFilePath() + "] no es pot obrir amb permisos d'escriptura, no podrem guardar estudis nous ni modificar els ja existents.");
            m_errorMessage.append( tr("\nYou don't have write permissions on %1 database, you couldn't retrieve or import new studies.").arg(ApplicationNameString) );
        }

        isCorrect = checkDatabaseRevision();
        if (!isCorrect)
        {
            m_errorMessage.append( tr("\nCan't upgrade database file. Be sure you have write permissions on database directory.").arg(ApplicationNameString) );    
        }
    }

    if( !isCorrect )
    {
        return false;
    }

    INFO_LOG("Estat de la base de dades correcte ");
    INFO_LOG("Base de dades utilitzada : " + LocalDatabaseManager::getDatabaseFilePath() + " revisio " +  QString().setNum(localDatabaseManager.getDatabaseRevision()));
    return true;
}

bool DatabaseInstallation::checkLocalImagePath()
{
    if (!existsLocalImagePath())
    {
        if (!createLocalImageDir())
        {
            ERROR_LOG("Error el path de la cache d'imatges no s'ha pogut crear " + LocalDatabaseManager::getCachePath());
            m_errorMessage.append( tr("\nCan't create the cache image directory. Please check users permissions.") );
            return false;
        }
    }
    else
    {
        // comprovar que tenim permisos d'escriptura al directori local d'imatges
        QFileInfo imagePathInfo( LocalDatabaseManager::getCachePath() );
        if( !imagePathInfo.isWritable() )
        {
            ERROR_LOG("El directori de la cache d'imatges no te permisos d'escriptura: " + LocalDatabaseManager::getCachePath() );
            m_errorMessage.append( tr("\nYou don't have write permissions on cache image directory. You couldn't retrieve or import new studies.") );
            return false;
        }
    }

    INFO_LOG("Estat de la cache d'imatges correcte ");
    INFO_LOG("Cache d'imatges utilitzada : " + LocalDatabaseManager::getCachePath());

    return true;
}

bool DatabaseInstallation::checkDatabasePath()
{
    if (!existsDatabasePath())
    {
        if (!createDatabaseDirectory())
        {
            ERROR_LOG("Error el path de la base de dades no s'ha pogut crear " + LocalDatabaseManager::getDatabaseFilePath());
            return false;
        }
    }

    return true;
}

bool DatabaseInstallation::checkDatabaseRevision()
{
    LocalDatabaseManager localDatabaseManager;

    if (localDatabaseManager.getDatabaseRevision() != StarviewerDatabaseRevisionRequired)
    {
        INFO_LOG("La revisio actual de la base de dades es " + QString().setNum(localDatabaseManager.getDatabaseRevision()) + " per aquesta versio d'" + ApplicationNameString + " és necessaria la " + QString().setNum(StarviewerDatabaseRevisionRequired) + ", es procedira a actualitzar la base de dades");

        return updateDatabaseRevision();
    }
    else return true;
}

bool DatabaseInstallation::isDatabaseFileWritable()
{
    QFileInfo databaseFilePath(LocalDatabaseManager::getDatabaseFilePath());
    
    return databaseFilePath.isWritable();
}

bool DatabaseInstallation::reinstallDatabase()
{
    //si existeix l'esborrem la base de dades
    if (existsDatabaseFile())
    {
        if (!QFile().remove(LocalDatabaseManager::getDatabaseFilePath()))
        {
            return false;
        }
    }

    if (!createDatabaseFile())
    {
        return false;
    }
    else
    {
        return existsDatabaseFile();
    }
}

bool DatabaseInstallation::removeCacheAndReinstallDatabase()
{
    DeleteDirectory *deleteDirectory = new DeleteDirectory();

    if (m_qprogressDialog == NULL)
    {
        //Si nó existeix creem barra de progrés per donar feedback
        m_qprogressDialog = new QProgressDialog(tr ("Reinstalling database"), "", 0, 0);
        m_qprogressDialog->setCancelButton(0);
        m_qprogressDialog->setValue(1);
        m_qprogressDialog->setModal(true);
    }

    //Esborrem les imatges que tenim a la base de dades local, al reinstal·lar la bd ja no té sentit mantenir-les, i per cada directori esborrat movem la barra de progrés
    connect(deleteDirectory, SIGNAL(directoryDeleted()), this, SLOT(setValueProgressBar()));
    deleteDirectory->deleteDirectory(LocalDatabaseManager::getCachePath(), false);
    delete deleteDirectory;

    m_qprogressDialog->close();

    return reinstallDatabase();
}

bool DatabaseInstallation::updateDatabaseRevision()
{
    bool status;

    //Creem barra de progrés per donar feedback
    m_qprogressDialog = new QProgressDialog(tr ("Updating database"), "", 0, 0);
    m_qprogressDialog->setCancelButton(0);
    m_qprogressDialog->setValue(1);

    /*Per aquesta versió degut a que s'ha tornat a reimplementar i a reestructurar tota la base de dades fent importants 
     *canvis, no s'ha fet cap codi per transformar la bd antiga amb la nova, per això es reinstal·la la BD*/
    status = removeCacheAndReinstallDatabase();

    if (!status)
    {
        ERROR_LOG("HA FALLAT L'ACTUALITZACIO DE LA BASE DE DADES");
    }

    return status;
}

bool DatabaseInstallation::existsLocalImagePath()
{
    QDir cacheImageDir;

    return cacheImageDir.exists(LocalDatabaseManager::getCachePath());
}

bool DatabaseInstallation::existsDatabasePath()
{
    QDir databaseDir;

    return databaseDir.exists(LocalDatabaseManager::getDatabaseFilePath());
}

bool DatabaseInstallation::existsDatabaseFile()
{
    QFile databaseFile;

    return databaseFile.exists(LocalDatabaseManager::getDatabaseFilePath());
}

bool DatabaseInstallation::createLocalImageDir()
{
    QDir cacheImageDir;

    if (cacheImageDir.mkpath(LocalDatabaseManager::getCachePath()))
    {
        INFO_LOG("S'ha creat el directori de la cache d'imatges " + LocalDatabaseManager::getCachePath());
        return true;
    }
    else
    {
        ERROR_LOG("No s'ha pogut crear el directori de la cache d'imatges " + LocalDatabaseManager::getCachePath());
        return false;
    }
}

bool DatabaseInstallation::createDatabaseDirectory()
{
    QDir databaseDir;

    QFileInfo databaseFilePath(LocalDatabaseManager::getDatabaseFilePath());
    QString databasePath = databaseFilePath.path();

    if (databaseDir.mkpath(databasePath))
    {
        INFO_LOG("S'ha creat el directori de la base de dades " + databasePath);
        return true;
    }
    else
    {
        ERROR_LOG("No s'ha pogut crear el directori de la base de dades " + databasePath);
        return false;
    }
}

bool DatabaseInstallation::createDatabaseFile()
{
    QFile sqlTablesScriptFile(":cache/database.sql");
    QByteArray sqlTablesScript;
    DatabaseConnection DBConnect;//obrim la bdd
    int status;

    //Comprovem que existeixi el path on s'ha de crear la base de dades, sinó el crea
    if (!checkDatabasePath()) return false;

    sqlTablesScriptFile.open(QIODevice::ReadOnly); //obrim el fitxer

    sqlTablesScript = sqlTablesScriptFile.read(sqlTablesScriptFile.size()); //el llegim

    status = sqlite3_exec(DBConnect.getConnection(), sqlTablesScript.constData(), 0, 0, 0); //creem les taules i els registres

    sqlTablesScriptFile.close(); //tanquem el fitxer

    if (status == 0)
    {
        INFO_LOG("S'ha creat correctament la base de dades");
        return true;
    }
    else
    {
        ERROR_LOG("No s'ha pogut crear la base de dades");
        return false;
    }
}

void DatabaseInstallation::setValueProgressBar()
{
    m_qprogressDialog->setValue(m_qprogressDialog->value() + 1);
}

QString DatabaseInstallation::getErrorMessage()
{
	return m_errorMessage;
}

}
