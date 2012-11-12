#ifndef UDGSCREEN_H
#define UDGSCREEN_H

#include <QRect>

namespace udg {

/**
    Classe per definir una pantalla. Ens permet definir la seva geometria, el seu ID i si �s principal o no.
    Tamb� incorpora m�todes per determinar les posicions relatives entre la seva geometria
 */
class Screen {
public:
    Screen();
    Screen(const QRect &geometry, const QRect &availableGeometry);
    ~Screen();

    /// Assigna/Obt� la geometria de la pantalla
    void setGeometry(const QRect &geometry);
    QRect getGeometry() const;

    /// Assigna/Obt� la geometria disponible de la pantalla per les aplicacions, tenint en compte, per exemple,
    /// les barres de men� de sistema i altres elements de l'entorn d'escriptori
    void setAvailableGeometry(const QRect &geometry);
    QRect getAvailableGeometry() const;

    /// Indica/Obt� si la pantalla �s principal
    void setAsPrimary(bool isPrimary);
    bool isPrimary() const;

    /// Assigna/obt� l'ID de la pantalla
    void setID(int ID);
    int getID() const;

    /// Ens retorna les propietats de l'objecte en un string
    QString toString() const;
    
    /// Ens diu si aquesta pantalla est� m�s amunt/avall que l'altra. Nom�s comprova que el seu top estigui per sobre/sota del top de la pantalla a comparar,
    /// per tant retornar� cert tant si est� completa com parcialment per sobre/sota.
    bool isHigher(const Screen &screen);
    bool isLower(const Screen &screen);
    
    /// Ens diu si aquesta pantalla est� m�s a la esquerra/dreta que l'altra. Nom�s comprova que que l'esquerra/dreta de cadasc� ho estigui m�s o no,
    /// per tant retornar� cert tant si est� completa com parcialment m�s a l'esquerra/dreta.
    bool isMoreToTheLeft(const Screen &screen);
    bool isMoreToTheRight(const Screen &screen);

    /// Indica si aquesta pantalla est� completament per sobre de l'altra. Comprova que el bottom d'aquesta estigui per sobre del top de l'altra.
    bool isOver(const Screen &screen) const;
    
    /// Indica si aquesta pantalla est� completament per sota de l'altra. Comprova que el top d'aquesta estigui per sota del bottom de l'altra.
    bool isUnder(const Screen &screen) const;
    
    /// Indica si aquesta pantalla est� completament a l'esquerra de l'altra. Comprova que el right d'aquesta sigui menor que el left de l'altra.
    bool isOnLeft(const Screen &screen) const;
    
    /// Indica si aquesta pantalla est� completament a la dreta de l'altra. Comprova que el left d'aquesta sigui major que el right de l'altra.
    bool isOnRight(const Screen &screen) const;
    
    /// Operador igualtat
    bool operator==(const Screen &screen) const;
    
    /// Constant per definir l'ID de pantalla inexistent
    static const int NullScreenID;

private:
    /// Inicialitza valors per defecte
    void initializeValues();

private:
    /// Indica si �s una pantalla principal
    bool m_isPrimary;

    /// Geometria total i disponible de la pantalla
    QRect m_geometry;
    QRect m_availableGeometry;

    /// ID de la pantalla
    int m_ID;
};

} // End namespace udg

#endif
