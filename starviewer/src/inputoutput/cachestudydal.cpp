/***************************************************************************
 *   Copyright (C) 2005-2006 by Grup de Gràfics de Girona                  *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#include <sqlite3.h>
#include <QString>

#include "cachestudydal.h"
#include "status.h"
#include "databaseconnection.h"
#include "dicomstudy.h"
#include "studylist.h"
#include "cachepool.h"
#include "logging.h"
#include "dicommask.h"

#include "study.h"

namespace udg {

CacheStudyDAL::CacheStudyDAL()
{
}

CacheStudyDAL::~CacheStudyDAL()
{
}

Status CacheStudyDAL::insertStudy( DICOMStudy *study, QString source )
{
    DatabaseConnection* databaseConnection = DatabaseConnection::getDatabaseConnection();
    QString sqlSentence;
    int databaseState;
    Status state;

    if ( !databaseConnection->connected() )
    {//el 50 es l'error de no connectat a la base de dades
        return databaseConnection->databaseStatus( 50 );
    }

    sqlSentence = QString("Insert into Patient ( PatId , PatNam , PatBirDat , PatSex ) values ( '%1' , '%2' , '%3' , '%4' )")
        .arg( study->getPatientId() )
        .arg( study->getPatientName() )
        .arg( study->getPatientBirthDate() )
        .arg( study->getPatientSex() );

    databaseConnection->getLock(); //s'insereix el pacient
    databaseState = sqlite3_exec( databaseConnection->getConnection() , qPrintable( sqlSentence ) , 0 , 0 , 0 ) ;
    databaseConnection->releaseLock();

    state = databaseConnection->databaseStatus( databaseState );

    //si el databaseState de l'operació és fals, però l'error és el 2019, significa que el pacient, ja existia a la bdd, per tant
    //continuem inserint l'estudi, si es provoca qualsevol altre error parem
    if ( !state.good() && state.code() != 2019 )
    {
        ERROR_LOG( QString("Error a la cache número %1 ").arg( state.code() ) );
        ERROR_LOG( sqlSentence );
        return state;
    }

    //creem  el sqlSentence per inserir l'estudi
    QString pacsAETitle;
    if( source == "PACS" )
    {
        // fem un select per assignar a l'estudi l'id del PACS al que pertany
        pacsAETitle = QString("( select PacsID from PacsList where AETitle = '%1' )").arg( study->getPacsAETitle() );
    }
    else if( source == "DICOMDIR" )
    {
        pacsAETitle = "99999";
    }
    else
    {
        DEBUG_LOG("Unrecognized source: " + source);
        return state.setStatus( "Unrecognized source: " + source, false, 0 );
    }

    sqlSentence = QString( "Insert into Study"
        "( PatId , StuInsUID , StuID , StuDat , StuTim , RefPhyNam , AccNum , StuDes , Modali , "
        " OpeNam , Locati , AccDat , AccTim , AbsPath , Status , PacsID , PatAge ) "
        "Values ( '%1' , '%2' , '%3' , '%4' , '%5' , '%6' , '%7' , '%8' , '%9' , '%10' , '%11' , %12 , %13 , '%14' , '%15' , "
        "%16 , '%17')" )
        .arg( study->getPatientId() ) // 1
        .arg( study->getStudyUID() ) // 2
        .arg( study->getStudyId() ) // 3
        .arg( study->getStudyDate() ) // 4
        .arg( study->getStudyTime() ) // 5
        .arg( QString() ) // 6 Referring Physician Name
        .arg( study->getAccessionNumber() ) // 7
        .arg( study->getStudyDescription() ) // 8
        .arg( study->getStudyModality() ) // 9
        .arg( QString() ) // 10 Operator Name
        .arg( QString() ) // 11 Location
        .arg( getDate() ) // 12
        .arg( getTime() ) // 13
        .arg( study->getAbsPath() ) // 14
        .arg( "PENDING" ) // 15 databaseState pendent perquè la descarrega de l'estudi encara no està completa
        .arg( pacsAETitle ) // 16
        .arg( study->getPatientAge()  ); // 17

    databaseConnection->getLock();
    databaseState = sqlite3_exec( databaseConnection->getConnection() , qPrintable(sqlSentence), 0 , 0 , 0 ) ;
    databaseConnection->releaseLock();

    state = databaseConnection->databaseStatus( databaseState );

    if ( !state.good() )
    {
        if ( state.code() != 2019 )
        {
            ERROR_LOG( QString("Error a la cache número %1 ").arg( state.code() ) );
            ERROR_LOG( sqlSentence );
        }
        else
            INFO_LOG( QString("L'estudi %1 ja existeix a la base de dades").arg(study->getStudyUID()) );
    }

    return state;
}

Status CacheStudyDAL::queryDatabase( QString query, QList<QStringList> &results )
{
    DatabaseConnection* databaseConnection = DatabaseConnection::getDatabaseConnection();
    int columns, rows, databaseState;
    char **reply = NULL , **error = NULL;
    Status state;

    if ( !databaseConnection->connected() )
    {//el 50 es l'error de no connectat a la base de dades
        return databaseConnection->databaseStatus( 50 );
    }

    databaseConnection->getLock();
    databaseState = sqlite3_get_table( databaseConnection->getConnection() , qPrintable( query ), &reply , &rows , &columns , error ); //connexio a la bdd,sentencia sqlSentence ,reply, numero de files,numero de cols.
    databaseConnection->releaseLock();
    state = databaseConnection->databaseStatus( databaseState );

    if ( !state.good() )
    {
        ERROR_LOG( QString("Error a la cache número %1").arg( state.code() ) );
        ERROR_LOG( query );
        return state;
    }

    int j;
    int i = 1;//ignorem les capçaleres
    while ( i <= rows )
    {
        QStringList rowResults;
        j = 0;
        while( j < columns )
        {
            rowResults << reply[j+i*columns];
            j++;
        }
        results << rowResults;
        i++;
    }

    return state;
}

Status CacheStudyDAL::queryDatabase( DicomMask maskQuery, QList<QStringList> &results )
{
    return this->queryDatabase( buildSqlQueryStudy(&maskQuery), results );
}

Status CacheStudyDAL::queryStudy( DicomMask studyMask , StudyList &ls )
{
    DatabaseConnection* databaseConnection = DatabaseConnection::getDatabaseConnection();
    int columns , rows , i = 0 , databaseState;
    DICOMStudy stu;
    char **reply = NULL , **error = NULL;
    Status state;

    if ( !databaseConnection->connected() )
    {//el 50 es l'error de no connectat a la base de dades
        return databaseConnection->databaseStatus( 50 );
    }

    databaseConnection->getLock();
    databaseState = sqlite3_get_table( databaseConnection->getConnection() , qPrintable( buildSqlQueryStudy( &studyMask ) ), &reply , &rows , &columns , error ); //connexio a la bdd,sentencia sqlSentence ,reply, numero de files,numero de cols.
    databaseConnection->releaseLock();
    state = databaseConnection->databaseStatus( databaseState );

    if ( !state.good() )
    {
        ERROR_LOG( QString("Error a la cache número %1").arg( state.code() ) );
        ERROR_LOG( buildSqlQueryStudy( & studyMask ) );
        return state;
    }

    i = 1;//ignorem les capçaleres
    while ( i <= rows )
    {
        stu.setPatientId( reply [ 0 + i * columns ] );
        stu.setPatientName( reply [ 1 + i * columns ] );
        stu.setPatientAge( reply [ 2+ i * columns ] );
        stu.setStudyId( reply [ 3+ i * columns ] );
        stu.setStudyDate( reply [ 4+ i * columns ] );
        stu.setStudyTime( reply [ 5+ i * columns ] );
        stu.setStudyDescription( reply [ 6+ i * columns ] );
        stu.setStudyUID( reply [ 7+ i * columns ] );
        //stu.setPacsAETitle( reply [ 8 + i * columns ] );
        stu.setAbsPath( reply [ 8 + i * columns ] );
        stu.setStudyModality( reply [ 9 + i * columns ] );
        stu.setAccessionNumber( reply [ 10 + i * columns ] );
        ls.insert( stu );
        i++;
    }

    return state;
}

Status CacheStudyDAL::queryOldStudies( QString OldStudiesDate , StudyList &ls )
{
    DatabaseConnection* databaseConnection = DatabaseConnection::getDatabaseConnection();
    int columns , rows , i = 0 , databaseState;
    DICOMStudy stu;
    QString sqlSentence;

    sqlSentence = QString( "select PatId, StuID, StuDat, StuTim, StuDes, StuInsUID, AbsPath, Modali "
            " from Study"
            " where AccDat < %1 "
            " order by StuDat,StuTim " )
            .arg( OldStudiesDate );

    char **reply = NULL , **error = NULL;
    Status state;

    if ( !databaseConnection->connected() )
    {//el 50 es l'error de no connectat a la base de dades
        return databaseConnection->databaseStatus( 50 );
    }

    databaseConnection->getLock();
    databaseState = sqlite3_get_table( databaseConnection->getConnection() , qPrintable( sqlSentence ) , &reply , &rows , &columns , error ); //connexio a la bdd,sentencia sqlSentence ,reply, numero de files,numero de cols.
    databaseConnection->releaseLock();
    state = databaseConnection->databaseStatus( databaseState );

    if ( !state.good() )
    {
        ERROR_LOG( QString("Error a la cache número %1").arg(state.code()) );
        ERROR_LOG( sqlSentence );
        return state;
    }

    i = 1;//ignorem les capçaleres
    while ( i <= rows )
    {
        stu.setPatientId( reply [ 0 + i * columns ] );
        stu.setStudyId( reply [ 1 + i * columns ] );
        stu.setStudyDate( reply [ 2 + i * columns ] );
        stu.setStudyTime( reply [ 3 + i * columns ] );
        stu.setStudyDescription( reply [ 4 + i * columns ] );
        stu.setStudyUID( reply [ 5 + i * columns ] );
        stu.setAbsPath( reply [ 6 + i * columns ] );
        stu.setStudyModality( reply [ 7 + i * columns ] );
        ls.insert( stu );
        i++;
    }

    return state;
}

Status CacheStudyDAL::queryStudy( QString studyUID , DICOMStudy &study )
{
    DatabaseConnection* databaseConnection = DatabaseConnection::getDatabaseConnection();
    int columns , rows , i = 0 , databaseState;
    char **reply = NULL , **error = NULL;
    Status state;
    QString sqlSentence;

    sqlSentence = QString( "select Study.PatId, PatNam, PatAge, StuID, StuDat, StuTim, StuDes, StuInsUID, AbsPath, Modali "
            " from Patient,Study,PacsList "
            " where Study.PatID=Patient.PatId "
            " and Status in ( 'RETRIEVED' , 'RETRIEVING' ) "
            " and StuInsUID = '%1'")
        .arg( studyUID );

    if ( !databaseConnection->connected() )
    {//el 50 es l'error de no connectat a la base de dades
        return databaseConnection->databaseStatus( 50 );
    }

    databaseConnection->getLock();
    databaseState = sqlite3_get_table( databaseConnection->getConnection() , qPrintable(sqlSentence), &reply , &rows , &columns , error ); //connexio a la bdd,sentencia sqlSentence ,reply, numero de files,numero de cols.
    databaseConnection->releaseLock();

    state = databaseConnection->databaseStatus( databaseState );

    if ( !state.good() )
    {
        ERROR_LOG( QString("Error a la cache número %1").arg(state.code()) );
        ERROR_LOG( sqlSentence );
        return state;
    }

    i = 1;//ignorem les capçaleres
    if ( rows > 0 )
    {
        study.setPatientId( reply [ 0 + i * columns ] );
        study.setPatientName( reply [ 1 + i * columns ] );
        study.setPatientAge( reply [ 2+ i * columns ] );
        study.setStudyId( reply [ 3+ i * columns ] );
        study.setStudyDate( reply [ 4+ i * columns ] );
        study.setStudyTime( reply [ 5+ i * columns ] );
        study.setStudyDescription( reply [ 6+ i * columns ] );
        study.setStudyUID( reply [ 7+ i * columns ] );
        //study.setPacsAETitle( reply [ 8 + i * columns ] );
        study.setAbsPath( reply [ 8 + i * columns ] );
        study.setStudyModality( reply [ 9 + i * columns ] );
    }
    else
    {
        databaseState = 99; //no trobat
        state = databaseConnection->databaseStatus( databaseState );
        if ( !state.good() )
        {
            ERROR_LOG( QString("Error a la cache número %1").arg(state.code()) );
            ERROR_LOG( sqlSentence );
        }
    }

    return state;
}

Status CacheStudyDAL::queryStudy( QString studyUID , Study *study )
{
    QString sqlSentence = QString( "select StuID, StuDat, StuTim, StuDes, StuInsUID "
            " from Study,PacsList "
            " where Status in ( 'RETRIEVED' , 'RETRIEVING' ) "
            " and StuInsUID = '%1'")
        .arg( studyUID );

    Status state;
    QList<QStringList> results;
    state = queryDatabase(sqlSentence,results);
    if( state.good() )
    {
        foreach( QStringList list, results )
        {
            study->setID( list.at(0) );
            study->setDate( list.at(1) );
            study->setTime( list.at(2) );
            study->setDescription( list.at(3) );
            study->setInstanceUID( list.at(4) );
        }
    }
    return state;
}

Status CacheStudyDAL::queryAllStudies( StudyList &ls )
{
    DatabaseConnection* databaseConnection = DatabaseConnection::getDatabaseConnection();
    int columns , rows , i = 0 , databaseState;
    DICOMStudy selectedStudy;
    char **reply = NULL , **error = NULL;
    Status state;
    QString sqlSentence;

    if ( !databaseConnection->connected() )
    {//el 50 es l'error de no connectat a la base de dades
        return databaseConnection->databaseStatus( 50 );
    }

    sqlSentence = "select Study.PatId, PatNam, PatAge, StuID, StuDat, StuTim, StuDes, StuInsUID, AbsPath, Modali, AccNum  from Study, Patient where Study.PatId = Patient.PatId ";

    databaseConnection->getLock();
    databaseState = sqlite3_get_table( databaseConnection->getConnection() , qPrintable(sqlSentence) , &reply , &rows , &columns , error ); //connexio a la bdd,sentencia sql ,reply, numero de files,numero de cols.
    databaseConnection->releaseLock();
    state = databaseConnection->databaseStatus( databaseState );

    if ( !state.good() )
    {
        ERROR_LOG( QString("Error a la cache número %1").arg( state.code() ) );
        ERROR_LOG( sqlSentence );
        return state;
    }

    i = 1;//ignorem les capçaleres
    while ( i <= rows )
    {
        selectedStudy.setPatientId( reply [ 0 + i * columns ] );
        selectedStudy.setPatientName( reply [ 1 + i * columns ] );
        selectedStudy.setPatientAge( reply [ 2+ i * columns ] );
        selectedStudy.setStudyId( reply [ 3+ i * columns ] );
        selectedStudy.setStudyDate( reply [ 4+ i * columns ] );
        selectedStudy.setStudyTime( reply [ 5+ i * columns ] );
        selectedStudy.setStudyDescription( reply [ 6+ i * columns ] );
        selectedStudy.setStudyUID( reply [ 7+ i * columns ] );
        selectedStudy.setAbsPath( reply [ 8 + i * columns ] );
        selectedStudy.setStudyModality( reply [ 9 + i * columns ] );
        selectedStudy.setAccessionNumber( reply [ 10 + i * columns ] );
        ls.insert( selectedStudy );
        i++;
    }

    return state;
}

Status CacheStudyDAL::delStudy( QString studyUID )
{
    DatabaseConnection* databaseConnection = DatabaseConnection::getDatabaseConnection();
    Status state;
    int databaseState;
    char **reply = NULL , **error = NULL;
    int columns , rows , studySize;
    QString absPathStudy, sqlSentence;
    CachePool cachePool;

    if ( !databaseConnection->connected() )
    {//el 50 es l'error de no connectat a la base de dades
        return databaseConnection->databaseStatus( 50 );
    }

    /* La part d'esborrar un estudi com que s'ha d'accedir a diverses taules, ho farem en un transaccio per si falla alguna sentencia sqlSentence fer un rollback, i així deixa la taula en estat estable, no deixem anar el candau fins al final */
    databaseConnection->getLock();
    databaseState = sqlite3_exec( databaseConnection->getConnection() , "BEGIN TRANSACTION ", 0 , 0 , 0 );
     //comencem la transacció

    state = databaseConnection->databaseStatus( databaseState );
    if ( !state.good() )
    {
        databaseState = sqlite3_exec( databaseConnection->getConnection() , "ROLLBACK TRANSACTION " , 0 , 0 , 0 );
        databaseConnection->releaseLock();

        ERROR_LOG( QString("Error a la cache número %1").arg(state.code()) );
        return state;
    }

    //sqlSentence per saber el directori on es guarda l'estudi
    sqlSentence = QString("select AbsPath from study where StuInsUID = '%1'").arg(studyUID);

    databaseState = sqlite3_get_table(databaseConnection->getConnection(), qPrintable( sqlSentence ), &reply , &rows , &columns , error ); //connexio a la bdd,sentencia sqlSentence ,reply, numero de files,numero de columnss.

    state = databaseConnection->databaseStatus( databaseState );

    if ( !state.good() )
    {
        databaseState = sqlite3_exec( databaseConnection->getConnection() , "ROLLBACK TRANSACTION " , 0 , 0 , 0 );
        databaseConnection->releaseLock();
        ERROR_LOG( QString("Error a la cache número %1").arg(state.code()) );
        ERROR_LOG( sqlSentence );
        return state;
    }
    else if (  rows == 0 )
    {
        databaseState = sqlite3_exec( databaseConnection->getConnection() , "ROLLBACK TRANSACTION " , 0 , 0 , 0 );
        databaseConnection->releaseLock();

        state = databaseConnection->databaseStatus( 99 );//error 99 registre no trobat
        ERROR_LOG( QString("Error a la cache número %1").arg(state.code()) );
        ERROR_LOG( sqlSentence );
        return state;
    }
    else
    {
        absPathStudy = reply[1];
    }

    //sqlSentence per saber quants estudis te el pacient
    sqlSentence = QString( "select count(*) from study where PatID in (select PatID from study where StuInsUID = '%1')").arg(studyUID);


    databaseState = sqlite3_get_table( databaseConnection->getConnection() , qPrintable( sqlSentence ) , &reply , &rows , &columns , error ); //connexio a la bdd,sentencia sqlSentence ,reply, numero de files,numero de columnss.

    state = databaseConnection->databaseStatus( databaseState );
    if( !state.good() )
    {
        databaseState = sqlite3_exec( databaseConnection->getConnection() , "ROLLBACK TRANSACTION " , 0 , 0 , 0 );
        databaseConnection->releaseLock();
        ERROR_LOG( QString("Error a la cache número %1").arg(state.code()) );
        ERROR_LOG( sqlSentence );
        return state;
    }
    else if( rows == 0 )
    {
        databaseState = sqlite3_exec( databaseConnection->getConnection() , "ROLLBACK TRANSACTION " , 0 , 0 , 0 );
        databaseConnection->releaseLock();
        state = databaseConnection->databaseStatus( 99 );//error 99 registre no trobat
        ERROR_LOG( QString("Error a la cache número %1").arg(state.code()) );
        ERROR_LOG( sqlSentence );
        return state;
    }

    //ignorem el reply [0], perque hi ha la capçalera
    if ( atoi( reply [1] ) == 1 )
    {//si aquell pacient nomes te un estudi l'esborrem de la taula Patient

        sqlSentence = QString("delete from Patient where PatID in (select PatID from study where StuInsUID = '%1')").arg(studyUID);

        databaseState = sqlite3_exec( databaseConnection->getConnection(), qPrintable(sqlSentence), 0, 0, 0);

        state = databaseConnection->databaseStatus( databaseState );
        if ( !state.good() )
        {
            databaseState = sqlite3_exec( databaseConnection->getConnection() , "ROLLBACK TRANSACTION " , 0 , 0 , 0 );
            databaseConnection->releaseLock();
            ERROR_LOG( QString("Error a la cache número %1").arg(state.code()) );
            ERROR_LOG( sqlSentence );
            return state;
        }
    }

    //esborrem de la taula estudi
    sqlSentence = QString("delete from study where StuInsUID= '%1'").arg( studyUID );

    databaseState = sqlite3_exec( databaseConnection->getConnection(), qPrintable(sqlSentence), 0, 0, 0);

    state = databaseConnection->databaseStatus( databaseState );
    if ( !state.good() )
    {
        databaseState = sqlite3_exec( databaseConnection->getConnection() , "ROLLBACK TRANSACTION " , 0 , 0 , 0 );
        databaseConnection->releaseLock();
        ERROR_LOG( QString("Error a la cache número %1").arg(state.code()) );
        ERROR_LOG( sqlSentence );
        return state;
    }

    sqlSentence = QString("delete from series where StuInsUID= '%1'").arg(studyUID);

    databaseState = sqlite3_exec( databaseConnection->getConnection(), qPrintable(sqlSentence), 0, 0, 0);

    state = databaseConnection->databaseStatus( databaseState );
    if ( !state.good() )
    {
        databaseState = sqlite3_exec( databaseConnection->getConnection() , "ROLLBACK TRANSACTION " , 0 , 0 , 0 );
        databaseConnection->releaseLock();
        ERROR_LOG( QString("Error a la cache número %1").arg(state.code()) );
        ERROR_LOG( sqlSentence );
        return state;
    }

    //calculem el que ocupava l'estudi per actualitzar l'espai actualitzat
    sqlSentence = QString("select sum(ImgSiz) from image where StuInsUID= '%1'").arg(studyUID);

    databaseState = sqlite3_get_table( databaseConnection->getConnection() , qPrintable( sqlSentence ) , &reply , &rows , &columns , error );

    state = databaseConnection->databaseStatus( databaseState );
    if ( !state.good() )
    {
        databaseState = sqlite3_exec( databaseConnection->getConnection() , "ROLLBACK TRANSACTION " , 0 , 0 , 0 );
        databaseConnection->releaseLock();
        ERROR_LOG( QString("Error a la cache número %1").arg(state.code()) );
        ERROR_LOG( sqlSentence );
        return state;
    }

    if ( reply[1] != NULL )
    {
        studySize = atoi( reply [1] );
    }
    else studySize = 0;

    //esborrem de la taula image
    sqlSentence = QString("delete from image where StuInsUID= '%1'").arg( studyUID );

    databaseState = sqlite3_exec( databaseConnection->getConnection(), qPrintable(sqlSentence), 0, 0, 0);

    state = databaseConnection->databaseStatus( databaseState );
    if ( !state.good() )
    {
        databaseState = sqlite3_exec( databaseConnection->getConnection() , "ROLLBACK TRANSACTION " , 0 , 0 , 0 );
        databaseConnection->releaseLock();
        ERROR_LOG( QString("Error a la cache número %1").arg(state.code()) );
        ERROR_LOG( sqlSentence );
        return state;
    }

    sqlSentence = QString("Update Pool Set Space = Space - %1 where Param = 'USED'").arg( studySize );

    databaseState = sqlite3_exec( databaseConnection->getConnection(), qPrintable(sqlSentence), 0, 0, 0);

    state = databaseConnection->databaseStatus( databaseState );

    if ( !state.good() )
    {
        databaseState = sqlite3_exec( databaseConnection->getConnection() , "ROLLBACK TRANSACTION " , 0 , 0 , 0 );
        databaseConnection->releaseLock();
        ERROR_LOG( QString("Error a la cache número %1").arg(state.code()) );
        ERROR_LOG( sqlSentence );
        return state;
    }

    databaseState = sqlite3_exec( databaseConnection->getConnection() , "COMMIT TRANSACTION " , 0 , 0 , 0 );
     //fem commit
    state = databaseConnection->databaseStatus( databaseState );
    if ( !state.good() )
    {
        ERROR_LOG( QString("Error a la cache número %1").arg(state.code()) );
        return state;
    }

    databaseConnection->releaseLock();

    //una vegada hem esborrat les dades de la bd, podem esborrar les imatges, això s'ha de fer al final, perqué si hi ha un error i esborrem les
    //imatges al principi, no les podrem recuperar i la informació a la base de dades hi continuarà estant
    cachePool.removeStudy( absPathStudy );

    return state;
}

Status CacheStudyDAL::setStudyRetrieved( QString studyUID )
{
    DatabaseConnection* databaseConnection = DatabaseConnection::getDatabaseConnection();
    int databaseState;
    Status state;
    QString sqlSentence;

    if ( !databaseConnection->connected() )
    {//el 50 es l'error de no connectat a la base de dades
        return databaseConnection->databaseStatus( 50 );
    }

    sqlSentence = QString("update study set Status = 'RETRIEVED' where StuInsUID= '%1'").arg( studyUID );

    databaseConnection->getLock();
    databaseState = sqlite3_exec( databaseConnection->getConnection(), qPrintable(sqlSentence), 0, 0, 0);
    databaseConnection->releaseLock();

    state = databaseConnection->databaseStatus( databaseState );

    if ( !state.good() )
    {
        ERROR_LOG( QString("Error a la cache número %1").arg(state.code()) );
        ERROR_LOG( sqlSentence );
    }

    return state;
}

Status CacheStudyDAL::setStudyRetrieving( QString studyUID )
{
    DatabaseConnection* databaseConnection = DatabaseConnection::getDatabaseConnection();
    int databaseState;
    Status state;
    QString sqlSentence;

    if ( !databaseConnection->connected() )
    {//el 50 es l'error de no connectat a la base de dades
        return databaseConnection->databaseStatus( 50 );
    }

    sqlSentence = QString("update study set Status = 'RETRIEVING' where StuInsUID= '%1'").arg(studyUID);

    databaseConnection->getLock();
    databaseState = sqlite3_exec( databaseConnection->getConnection(), qPrintable(sqlSentence), 0, 0, 0);
    databaseConnection->releaseLock();

    state = databaseConnection->databaseStatus( databaseState );

    if ( !state.good() )
    {
        ERROR_LOG( QString("Error a la cache número %1").arg(state.code()) );
        ERROR_LOG( sqlSentence );
    }

    return state;
}

Status CacheStudyDAL::updateStudyAccTime( QString studyUID )
{
    DatabaseConnection* databaseConnection = DatabaseConnection::getDatabaseConnection();
    int databaseState;
    Status state;
    QString sqlSentence;

    if ( !databaseConnection->connected() )
    {//el 50 es l'error de no connectat a la base de dades
        return databaseConnection->databaseStatus( 50 );
    }

    sqlSentence = QString("Update Study Set AccDat = %1, AccTim = %2 where StuInsUID = '%3'")
        .arg( getDate() )
        .arg( getTime() )
        .arg( studyUID );

    databaseConnection->getLock();
    databaseState = sqlite3_exec( databaseConnection->getConnection(), qPrintable(sqlSentence), 0, 0, 0);
    databaseConnection->releaseLock();

    state = databaseConnection->databaseStatus( databaseState );
    if ( !state.good() )
    {
        ERROR_LOG( QString("Error a la cache número %1").arg(state.code()) );
        ERROR_LOG( sqlSentence );
    }

    return state;
}

Status CacheStudyDAL::updateStudy( DICOMStudy updateStudy )
{
    DatabaseConnection* databaseConnection = DatabaseConnection::getDatabaseConnection();
    int databaseState;
    Status state;
    QString sqlSentence;

    if ( !databaseConnection->connected() )
    {//el 50 es l'error de no connectat a la base de dades
        return databaseConnection->databaseStatus( 50 );
    }

    //Actualizem estudi

    sqlSentence = QString( "Update Study set StuID = '%1', StuDat = '%2', StuTim = '%3', RefPhyNam = '%4', AccNum = '%5', StuDes = '%6',  Modali = '%7', OpeNam = '%8', Locati = '%9', AccDat = %10, AccTim = %11, Status = 'PENDING', PatID = '%12' where StuInsUID = '%13'"
    )
    .arg( updateStudy.getStudyId() )
    .arg( updateStudy.getStudyDate() )
    .arg( updateStudy.getStudyTime() )
    .arg( QString() ) //Referring Physician Name
    .arg( updateStudy.getAccessionNumber() )
    .arg( updateStudy.getStudyDescription() )
    .arg( updateStudy.getStudyModality() )
    .arg( QString() ) //Operator Name
    .arg( QString() ) //Location
    .arg( getDate() )
    .arg( getTime() )
    .arg( updateStudy.getPatientId() )
    .arg( updateStudy.getStudyUID() );

    databaseConnection->getLock();
    databaseState = sqlite3_exec( databaseConnection->getConnection(), qPrintable(sqlSentence), 0, 0, 0);
    databaseConnection->releaseLock();

    state = databaseConnection->databaseStatus( databaseState );
    if ( !state.good() )
    {
        ERROR_LOG( QString("Error a la cache número %1").arg(state.code()) );
        ERROR_LOG( sqlSentence );
        return state;
    }

    //ACTUALITZEM DADES PACIENT

    sqlSentence = QString("Update Patient set PatNam = '%1', PatBirDat = '%2', PatSex = '%3' where PatID = '%4'")
        .arg( updateStudy.getPatientName() )
        .arg( updateStudy.getPatientBirthDate() )
        .arg( updateStudy.getPatientSex() )
        .arg( updateStudy.getPatientId() );

    databaseConnection->getLock();
    databaseState = sqlite3_exec( databaseConnection->getConnection(), qPrintable(sqlSentence), 0, 0, 0);
    databaseConnection->releaseLock();

    state = databaseConnection->databaseStatus( databaseState );
    if ( !state.good() )
    {
        ERROR_LOG( QString("Error a la cache número %1").arg(state.code()) );
        ERROR_LOG( sqlSentence );
    }

    return state;
}

QString CacheStudyDAL::buildSqlQueryStudy(DicomMask* studyMask)
{
    QString sqlSentence ,patientName,patID,stuDatMin,stuDatMax,stuID,accNum,stuInsUID,stuMod,studyDate;

    sqlSentence = "select Study.PatId, PatNam, PatAge, StuID, StuDat, StuTim, StuDes, StuInsUID, AbsPath, Modali, AccNum from Patient,Study where Study.PatID=Patient.PatId and Status = 'RETRIEVED' ";

    //llegim la informació de la màscara
    patientName = replaceAsterisk( studyMask->getPatientName() );
    patID = studyMask->getPatientId();
    studyDate = studyMask->getStudyDate();
    stuID = studyMask->getStudyId();
    accNum = studyMask->getAccessionNumber();
    stuInsUID = studyMask->getStudyUID();

    //cognoms del pacient
    if ( !patientName.isEmpty() )
    {
        sqlSentence += QString(" and PatNam like '%1'").arg( patientName );
    }

    //Id del pacient
    if ( patID != "*" && !patID.isEmpty() )
    {
        sqlSentence += QString(" and Study.PatID like '%1'").arg( replaceAsterisk( patID ) );
    }

    //data
    if ( studyDate.length() == 8 )
    {
        sqlSentence += QString(" and StuDat = '%1'" ).arg( studyDate );
    }
    else if ( studyDate.length() == 9 )
    {
        if ( studyDate.at( 0 ) == '-' )
        {
            sqlSentence += QString(" and StuDat <= '%1'").arg( studyDate.mid( 1 , 8 ) );
        }
        else if ( studyDate.at( 8 ) == '-' )
        {
            sqlSentence += QString(" and StuDat >= '%1'").arg( studyDate.mid( 0 , 8 ) );
        }
    }
    else if ( studyDate.length() == 17 )
    {
        sqlSentence += QString(" and StuDat between '%1' and '%2'").arg( studyDate.mid( 0 , 8 ) ).arg( studyDate.mid( 9 , 8 ) );
    }

    //id estudi
    if ( stuID != "*" && !stuID.isEmpty() )
    {
        sqlSentence += QString(" and StuID like '%1'").arg( replaceAsterisk( stuID ) );
    }

    //Accession Number
    if ( accNum != "*" && !accNum.isEmpty() )
    {
        sqlSentence += QString(" and AccNum = '%1'").arg( accNum );
    }

    if ( stuInsUID != "*" && !stuInsUID.isEmpty() )
    {
        sqlSentence += QString(" and StuInsUID = '%1'").arg( stuInsUID );
    }

    if ( studyMask->getStudyModality() != "*" && !studyMask->getStudyModality().isEmpty() )
    {
        sqlSentence += buildSqlStudyModality( studyMask );
    }

    return sqlSentence;
}

QString CacheStudyDAL::replaceAsterisk( QString original )
{
    QString ret;

    ret = original;

    while( ret.indexOf("*") >= 0 )
        ret.replace( ret.indexOf("*") , 1 , "%" );
    return ret;
}

int CacheStudyDAL::getTime()
{
    //\TODO cad pot ser un QString i retornar l'enter amb el mètode ::toInt()
    time_t hora;
    char cad[5];
    struct tm *tmPtr;

    hora = time( NULL );
    tmPtr = localtime( &hora );
    strftime( cad , 5 , "%H%M" , tmPtr );

    return atoi( cad );
}

int CacheStudyDAL::getDate()
{
    //\TODO cad pot ser un QString i retornar l'enter amb el mètode ::toInt()
    time_t hora;
    char cad[9];
    struct tm *tmPtr;

    hora = time( NULL );
    tmPtr = localtime( &hora );
    strftime( cad , 9 , "%Y%m%d" , tmPtr );

    return atoi( cad );
}


/** Sabem que la màscara de modalitat té format "MR,CT,NM" passem a sentència sqlSentence fent and ( Modali = 'MR' or Modali='CT' .... )
 */
QString CacheStudyDAL::buildSqlStudyModality( DicomMask *mask )
{
    QString sqlSentence;
    int index = 0;

    if ( !mask->getStudyModality().isEmpty() )
    {
        //inserim la primera modalitat
        //\TODO assumeixes que totes les modalitats són de 2 cràcters i no és veritat!!!!! hi ha modalitats com REG o SEG que en tenen 3!!! hauries de separar per les comes o el caràcter delimitador!
        sqlSentence = QString(" and ( Modali = '%1'").arg( mask->getStudyModality().mid( 0 , 2 ) );

        index = 3; //=3 perqué ignorem la coma per exemple "CT,MR,NM"
        while ( mask->getStudyModality().length() > index )
        {
            sqlSentence += QString(" or Modali = '%1'" ).arg( mask->getStudyModality().mid( index , 2 ) );
            index += 3;
        }

        sqlSentence += ")"; // tanquem el primer parentesis
    }

    return sqlSentence;
}


}
