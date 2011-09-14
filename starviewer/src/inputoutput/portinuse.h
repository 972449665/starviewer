#ifndef UDGPORTINUSE_H
#define UDGPORTINUSE_H

#include <QString>
#include <QAbstractSocket>

namespace udg {

/**
    Classe que s'encarrega de comprovar si un port est� en �s.
  */
class PortInUse {
public:
    enum PortInUseStatus { PortIsAvailable, PortIsInUse, PortCheckError, PortUnknownStatus };

    PortInUse();

    /// Indica si el port passat per par�metre est� en �s (cert) o lliure (fals)
    bool isPortInUse(int port);

    /// Retorna l'estat de l'�ltim port que s'ha comprovat
    PortInUse::PortInUseStatus getStatus();

    /// Retorna l'string corresponent a l'error. El seu valor nom�s ser� v�lid quan m_status valgui PortCheckError.
    QString getErrorString();

protected:
    /// Retorna si el port passat per par�metre est� lliure (cert) o en �s (fals)
    /// @param serverError: indica l'error del servidor
    /// @param errorString: descripcio de l'error.
    virtual bool isPortAvailable(int port, QAbstractSocket::SocketError &serverError, QString &errorString);

private:
    PortInUse::PortInUseStatus m_status;
    QString m_errorString;

};

}  // end namespace udg

#endif
