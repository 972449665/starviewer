
#ifndef UDGUTILS_H
#define UDGUTILS_H

#include <QString>

namespace udg {

/**
    Classe que cont� utilitats v�ries utilitzades per la classe inputoutput
  */
class Utils {

public:
    /// Indica si el port passat per par�metre est� en �s
    static bool isPortInUse(int port);

    static QString generateUID(const QString &prefix = QString());

};

};  //  end  namespace udg

#endif
