#ifndef UDGPORTINUSEBYANOTHERAPPLICATION_H
#define UDGPORTINUSEBYANOTHERAPPLICATION_H

namespace udg {

/**
    Classe que s'encarrega de comprovar si un port est� en �s per una aplicaci� que no sigui starviewer.
  */
class PortInUseByAnotherApplication {
public:
    virtual ~PortInUseByAnotherApplication();

    /// Crea una nova inst�ncia d'alguna de les subclasses que implementa la interf�cie
    static PortInUseByAnotherApplication* newInstance();

    /// Comprova si el port est� en �s per una aplicaci� que no sigui Starviewer
    virtual bool isPortInUseByAnotherApplication(int port, bool &error);

protected:
    // Constructor privat per obligar a utilitzar el m�tode newInstance
    PortInUseByAnotherApplication();
};

}  // end namespace udg

#endif
