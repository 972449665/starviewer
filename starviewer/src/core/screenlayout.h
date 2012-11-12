#ifndef UDGSCREENLAYOUT_H
#define UDGSCREENLAYOUT_H

#include <QList>
#include <QRect>

#include "screen.h"

namespace udg {

/**
    Classe per representar la distribuci� de pantalles
 */
class ScreenLayout {
public:
    ScreenLayout();
    ~ScreenLayout();

    /// Afegeix una pantalla al layout. Si la pantalla no �s v�lida per afegir-se al layout actual, no s'afegir� i es retornar� fals, cert altrament.
    /// No s'afegiran pantalles amb ID's inv�lids (<0) o que tinguin el mateix ID que una pantalla existent al layout.
    /// Tampoc s'afegir� una pantalla si aquesta �s principal i ja n'existeix una al layout.
    bool addScreen(const Screen &screen);

    /// Retorna el nombre de pantalles al layout
    int getNumberOfScreens() const;
    
    /// Reseteja el layout eliminant totes les pantalles que contenia, deixant-lo buit
    void clear();
    
    /// Ens retorna la pantalla amb l'ID demanat. Si la pantalla no existeix, ens retornar� una Screen buida
    Screen getScreen(int screenID) const;

    /// Ens retorna l'id de la pantalla principal. En cas que no n'hi hagi cap de definida, ens retornar� -1
    int getPrimaryScreenID() const;
    
    /// Ens indica quina �s la pantalla que es troba a la dreta/esquerra de la pantalla amb ID indicada.
    /// Retorna -1 si no n'hi ha cap pantalla en el costat indicat
    int getScreenOnTheRightOf(int screenID);
    int getScreenOnTheLeftOf(int screenID);
    
    /// Indica si la primera pantalla est� per sobre de la segona. Comprova que el bottom de la primera estigui per sobre del top de la segona.
    bool isOver(int screen1, int screen2);
    
    /// Indica si la primera pantalla est� per sota de la segona. Comprova que el top de la primera estigui per sota del bottom de la segona.
    bool isUnder(int screen1, int screen2);
    
    /// Indica si la primera pantalla est� a l'esquerra de la segona. Comprova que el right de la primera sigui menor que el left de la segona.
    bool isOnLeft(int screen1, int screen2);
    
    /// Indica si la primera pantalla est� a la dreta de la segona. Comprova que el left de la primera sigui major que el right de la segona.
    bool isOnRight(int screen1, int screen2);

private:
    /// Ens retorna l'�ndex de la pantalla amb ID screenID. Si no existeix, l'�ndex ser� -1
    int getIndexOfScreen(int screenID) const;

private:
    /// Constant per determinar el llindar de proximitat entre finestres
    static const int SamePositionThreshold;
    
    /// Llista de pantalles
    QList<Screen> m_screens;
};

} // End namespace udg

#endif
