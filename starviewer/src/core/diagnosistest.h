#ifndef UDGDIAGNOSISTEST_H
#define UDGDIAGNOSISTEST_H

#include "diagnosistestresult.h"

namespace udg {

class DiagnosisTest {
public:
    /// M�tode principal de la interf�cie. No est� implementat, i es for�a a les subclasses a fer-ho.
    virtual DiagnosisTestResult run() = 0;
protected:
    /// Destructor per defecte. �s virtual per que les subclasses puguin reimplementar-lo
    virtual ~DiagnosisTest(){};
};

}

#endif
