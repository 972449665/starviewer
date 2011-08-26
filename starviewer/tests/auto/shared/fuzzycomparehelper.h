#ifndef FUZZYCOMPAREHELPER_H
#define FUZZYCOMPAREHELPER_H

#include <QString>

namespace testing {

/// Classe que contindr� les funcions de comparaci� fuzzy que necessitem
class FuzzyCompareHelper {
public:
    /// Fa una comparaci� fuzzy de dos valors double. Tenim un tercer par�metre opcional epsilon que ens 
    /// permetr� fixar la precisi� que volem en la comparaci�.
    static bool fuzzyCompare(const double &v1, const double &v2, const double &epsilon = 0.000000001);
};

}

#endif // FUZZYCOMPAREHELPER_H
