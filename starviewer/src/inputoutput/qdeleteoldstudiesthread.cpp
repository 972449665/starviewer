#include "qdeleteoldstudiesthread.h"

#include "logging.h"

namespace udg {

QDeleteOldStudiesThread::QDeleteOldStudiesThread(QObject *parent)
 : QThread(parent)
{
    m_lastError = LocalDatabaseManager::Ok;
}

void QDeleteOldStudiesThread::deleteOldStudies()
{
    start();
}

LocalDatabaseManager::LastError QDeleteOldStudiesThread::getLastError()
{
    return m_lastError;
}

void QDeleteOldStudiesThread::run()
{
    LocalDatabaseManager localDatabaseManager;

    localDatabaseManager.deleteOldStudies();

    m_lastError = localDatabaseManager.getLastError();

    emit finished();
}

}
