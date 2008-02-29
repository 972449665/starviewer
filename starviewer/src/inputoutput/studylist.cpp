#include <list>

#include "studylist.h"

namespace udg
{
/*Tenint en compte que es podran realitzar multiples cerques en diferents pacs alhora, això implica que tindrem diferents threads executant-se
   que hauran d'inserir studis a la llista d'estudis, això fa necessari que la part d'inserir un nou estudi a la llista s'hagi de fer en execlusió
   mutua, ja que com es pot consultatr a
 */

StudyList::StudyList()
{
    int init_value = 1;//Només un thread alhora pot gravar a la llista

    m_semafor = (sem_t*) malloc( sizeof( sem_t ) );
    sem_init( m_semafor , 0 , init_value );

    m_iterator = m_listStudy.begin();
}

void StudyList::insert( DICOMStudy study )
{
    sem_wait( m_semafor );
    m_listStudy.push_back( study );
    sem_post( m_semafor );
}

void StudyList::firstStudy()
{
    m_listStudy.sort();
    m_iterator = m_listStudy.begin();
}

void StudyList:: nextStudy()
{
    m_iterator++;
}

bool StudyList::end()
{
    if ( m_listStudy.size()== 0 )
    {
        return true;
    }
    else return m_iterator == m_listStudy.end();
}

bool StudyList::exists( QString UID , QString AETitlePacs )
{
    sem_wait( m_semafor );
    m_iterator = m_listStudy.begin();

    if ( AETitlePacs.isEmpty() ) // si no ens passen un AETitle, al hora de mirar si un estudi existeix un estudi a la llista, no discriminem per AETitle, només per UID
    {
        while ( m_iterator != m_listStudy.end() )
        {

            if ( (*m_iterator).getStudyUID() == UID ) break;
            else m_iterator++;
        }
    }
    else // si ens passen AETitle, al hora de comprovar si un estudi està a la llista discriminem per UID i AETitle
    {
        while ( m_iterator != m_listStudy.end() )
        {

            if ( (*m_iterator).getStudyUID() == UID && (*m_iterator).getPacsAETitle() == AETitlePacs ) break;
            else m_iterator++;
        }
    }

    sem_post( m_semafor );
    return ( m_iterator != m_listStudy.end() );
}

DICOMStudy StudyList::getStudy()
{
    return ( *m_iterator );
}

int StudyList::count()
{
    return m_listStudy.size();
}

void StudyList::clear()
{
    sem_wait( m_semafor );
    m_listStudy.clear();
    sem_post( m_semafor );
}

}

