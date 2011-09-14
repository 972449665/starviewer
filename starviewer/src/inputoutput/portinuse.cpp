#include "portinuse.h"

#include <QTcpServer>

#include "logging.h"

namespace udg {

PortInUse::PortInUse()
{
    m_status = PortInUse::PortUnknownStatus;
    m_errorString = "No port checked yet";
}

bool PortInUse::isPortInUse(int port)
{
    bool portInUse;
    QAbstractSocket::SocketError serverError;
    QString errorString;

    portInUse = !isPortAvailable(port, serverError, errorString);

    if (!portInUse)
    {
        m_status = PortInUse::PortIsAvailable;
    }
    else if (serverError == QAbstractSocket::AddressInUseError)
    {
        m_status = PortInUse::PortIsInUse;
    }
    else
    {
        // No s'hauria de donar un error diferent a AddressInUseError, de totes maneres per seguretat el loggagem
        ERROR_LOG("No s'ha pogut comprovat correctament si el port " + QString().setNum(port) + " est� en �s, per error: " + errorString);
        m_errorString = errorString;
        m_status = PortInUse::PortCheckError;
    }

    return portInUse;
}

bool PortInUse::isPortAvailable(int port, QAbstractSocket::SocketError &serverError, QString &errorString)
{
    QTcpServer tcpServer;
    bool result;

    /// Result ser� cert si el port est� lliure, pertant s'ha de retorna l'oposat
    result = tcpServer.listen(QHostAddress::Any, port);
    serverError = tcpServer.serverError();
    errorString = tcpServer.errorString();

    tcpServer.close();

    return result;
}

PortInUse::PortInUseStatus PortInUse::getStatus()
{
    return m_status;
}

QString PortInUse::getErrorString()
{
    return m_errorString;
}

} // End udg namespace
