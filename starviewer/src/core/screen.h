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
    
    /// Indica si aquesta pantalla est� sobre de l'altra. Per que sigui cert han de ser de la mateixa amplada
    /// i el top d'aquesta ha de ser igual al bottom de l'altra
    bool isTop(const Screen &screen) const;
    
    /// Indica si aquesta pantalla est� sota de l'altra. Per que sigui cert han de ser de la mateixa amplada
    /// i el bottom d'aquesta ha de ser igual al top de l'altra
    bool isBottom(const Screen &screen) const;
    
    /// Indica si aquesta pantalla est� a l'esquerra de l'altra. Per que sigui cert han de ser de la mateixa al�ada
    /// i el right d'aquesta ha de ser igual al left de l'altra
    bool isLeft(const Screen &screen) const;
    
    /// Indica si aquesta pantalla est� a la dreta de l'altra. Per que sigui cert han de ser de la mateixa al�ada
    /// i el left d'aquesta ha de ser igual al right de l'altra
    bool isRight(const Screen &screen) const;
    
    /// Indica si aquesta pantalla est� en diagonal a sobre a l'esquerra de l'altra. Per que sigui cert
    /// el punt bottomRight d'aquesta ha de ser igual al punt topLeft de l'altra
    bool isTopLeft(const Screen &screen) const;
    
    /// Indica si aquesta pantalla est� en diagonal a sobre a la dreta de l'altra. Per que sigui cert
    /// el punt bottomLeft d'aquesta ha de ser igual al punt topRight de l'altra
    bool isTopRight(const Screen &screen) const;
    
    /// Indica si aquesta pantalla est� en diagonal a sota a l'esquerra de l'altra. Per que sigui cert
    /// el punt topRight d'aquesta ha de ser igual al punt bottomLeft de l'altra
    bool isBottomLeft(const Screen &screen) const;
    
    /// Indica si aquesta pantalla est� en diagonal a sota a la dreta de l'altra. Per que sigui cert
    /// el punt topLeft d'aquesta ha de ser igual al punt bottomRight de l'altra
    bool isBottomRight(const Screen &screen) const;
    
    /// Operador igualtat
    bool operator==(const Screen &screen) const;
    
    /// Constant per definir l'ID de pantalla inexistent
    static const int NullScreenID;

private:
    /// Threshold amb el que definim la m�xima dist�ncia entre finestres per determinar que estan de cant�
    static const int MaximumDistanceInBetween;
    
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
