/***************************************************************************
 *   Copyright (C) 2005 by Grup de Gràfics de Girona                       *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/

#include "processimagesingleton.h"

#include <QSemaphore>

#include "processimage.h"
#include "dicomtagreader.h"

namespace udg {

ProcessImageSingleton::ProcessImageSingleton()
{
    m_semaphore = new QSemaphore( 1 );
}

ProcessImageSingleton *ProcessImageSingleton::pInstance = 0;

ProcessImageSingleton *ProcessImageSingleton::getProcessImageSingleton()
{
    if ( pInstance == 0 )
    {
        pInstance = new ProcessImageSingleton;
    }

    return pInstance;
}

void ProcessImageSingleton::addNewProcessImage( QString UID , ProcessImage *pi )
{
    SingletonProcess sp;

    sp.studyUID = UID;
    sp.imgProcess = pi;

    m_semaphore->acquire();
    m_listProcess << sp;
    m_semaphore->release();
}

void ProcessImageSingleton::process(const QString &studyUID, Image *image)
{
    QList<SingletonProcess>::const_iterator j;

    j = m_listProcess.begin();
    while ( j != m_listProcess.end() )
    {
        if ( (*j).studyUID != studyUID )
        {
            j++;
        }
        else 
            break;
    }

    if ( j != m_listProcess.end() )
    {
        (*j).imgProcess->process( image );
    }
}

void ProcessImageSingleton::process(const QString &studyUID, DICOMTagReader *dicomTagReader)
{
    QList<SingletonProcess>::const_iterator j;

    j = m_listProcess.begin();
    while ( j != m_listProcess.end() )
    {
        if ( (*j).studyUID != studyUID )
        {
            j++;
        }
        else 
            break;
    }

    if ( j != m_listProcess.end() )
    {
        (*j).imgProcess->process(dicomTagReader);
    }
}

void ProcessImageSingleton::setError( QString studyUID )
{
    QList<SingletonProcess>::const_iterator j;
 
    j = m_listProcess.begin();
    while ( j != m_listProcess.end() )
    {
        if ( (*j).studyUID != studyUID )
        {
            j++;
        }
        else 
            break;
    }

    if ( j != m_listProcess.end() )
    {
        (*j).imgProcess->setError();
    }
}

bool ProcessImageSingleton::delProcessImage( QString UID )
{
    QList<SingletonProcess>::iterator j;

    j = m_listProcess.begin();
    while ( j != m_listProcess.end() )
    {
        if ( (*j).studyUID != UID )
        {
            j++;
        }
        else 
            break;
    }

    if ( j != m_listProcess.end() )
    {
        m_semaphore->acquire();
        m_listProcess.erase( j );
        m_semaphore->release();
        return true;
    }
    else 
        return false;
}

void ProcessImageSingleton::setPath( QString path )
{
    m_imagePath = path;
}

QString ProcessImageSingleton::getPath()
{
    return m_imagePath;
}

ProcessImageSingleton::~ProcessImageSingleton()
{
}

};
