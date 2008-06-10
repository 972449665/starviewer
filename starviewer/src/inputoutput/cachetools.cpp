/***************************************************************************
 *   Copyright (C) 2005-2006 by Grup de Gràfics de Girona                  *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/

#include <sqlite3.h>

#include "cachetools.h"
#include "databaseconnection.h"
#include "status.h"
#include "logging.h"

namespace udg {

CacheTools::CacheTools()
{
}

Status CacheTools::compactCachePacs()
{
    int stateDatabase;
    Status state;
    QString sql;
    DatabaseConnection *databaseConnection = DatabaseConnection::getDatabaseConnection();

    if ( !databaseConnection->connected() )
    {//el 50 es l'error de no connectat a la base de dades
        return databaseConnection->databaseStatus( 50 );
    }

    sql.insert( 0 , "vacuum" );//amb l'acció vacuum es compacta la base de dades

    databaseConnection->getLock();
    stateDatabase = sqlite3_exec( databaseConnection->getConnection() , qPrintable(sql) , 0 , 0 , 0 );
    databaseConnection->releaseLock();

    state = databaseConnection->databaseStatus ( stateDatabase );
    INFO_LOG( "S'HA COMPACTAT LA BASE DE DADES" );

    return state;
}


CacheTools::~CacheTools()
{
}


}
