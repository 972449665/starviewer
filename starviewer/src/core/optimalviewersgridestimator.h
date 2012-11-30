#ifndef UDGOPTIMALVIEWERSGRIDESTIMATOR_H
#define UDGOPTIMALVIEWERSGRIDESTIMATOR_H

#include <QPair>
#include <QMap>

namespace udg {

/**
    Classe per calcular quin �s el grid de visors �ptim segons els nombre de visors m�nims necessaris.
    Hi ha un nombre m�xim de visors que es poden crear fixat actualment en 48.
    // TODO La classe hauria d'evolucionar perqu� es puguin donar millors layouts �ptims, segons orientaci� de pantalla,
    n�mero de pantalles, etc.
  */
class OptimalViewersGridEstimator {
public:
    OptimalViewersGridEstimator();
    ~OptimalViewersGridEstimator();

    /// Donat un nombre m�nim de cel�les necess�ries, ens d�na la parella de files i columnes �ptima
    QPair<int, int> getOptimalGrid(int minimumCells);

private:
    /// Mapa que ens diu pel nombre de viewers, quantes files i columnes s�n les id�nies
    QMap<int, QPair<int, int> > m_gridsMap;

    /// Nombre m�xim de viewers que es poden crear. Vindr� determinat pel contingut de m_gridsMap;
    int m_maximumNumberOfViewers;
};

} // End namespace udg

#endif
