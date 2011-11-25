#ifndef UDGIMAGEOVERLAYREGIONFINDER_H
#define UDGIMAGEOVERLAYREGIONFINDER_H

#include <QList>

class QBitArray;
class QRect;

namespace udg {

class ImageOverlay;

/**
    Aquesta classe permet trobar regions en un ImageOverlay intentant minimitzar l'�rea buida total per� sense fer moltes regions molt petites.
 */
class ImageOverlayRegionFinder {

public:

    ImageOverlayRegionFinder(const ImageOverlay &overlay);

    /// Troba les regions de l'overlay que contenen objectes i les guarda a la llista de regions.
    /// Si optimizeForPowersOf2 �s cert, ajunta les regions que juntes ocupen menys mem�ria de textures que per separat, tenint en compte que les textures tenen
    /// mides que s�n pot�ncies de 2.
    void findRegions(bool optimizeForPowersOf2);
    /// Retorna la llista de regions de l'overlay.
    const QList<QRect>& regions() const;

protected:

    /// Retorna la dist�ncia entre les regions donades, calculada com una dist�ncia de Chebyshev (http://en.wikipedia.org/wiki/Chebyshev_distance).
    static int distanceBetweenRegions(const QRect &region1, const QRect &region2);

private:

    /// Retorna l'�ndex per accedir a les dades a partir de la fila i la columna.
    int getDataIndex(int row, int column) const;
    /// Retorna l'�ndex de la fila a partir de l'�ndex de les dades.
    int getRowIndex(int i) const;
    /// Retorna l'�ndex de la columna a partir de l'�ndex de les dades.
    int getColumnIndex(int i) const;

    /// Fa cr�ixer una regi� a partir del p�xel indicat. Emplena la m�scara i retorna la regi� trobada.
    QRect growRegion(int row, int column, QBitArray &mask);
    /// Posa a 1 tots els p�xels de la m�scara que pertanyen a la regi�.
    void fillMaskForRegion(QBitArray &mask, const QRect &region);
    /// Afegeix un padding d'un p�xel al voltant de la regi�.
    void addPadding(QRect &region);
    /// Treu el padding d'un p�xel al voltant de la regi�.
    void removePadding(QRect &region);
    /// Afegeix la regi� a la llista, fusionant-la amb altres si s�n molt properes.
    /// Si optimizeForPowersOf2 �s cert, tamb� les fusiona si juntes aprofiten millor la mem�ria de textures.
    void addRegion(QRect &region, bool optimizeForPowersOf2);

private:

    /// L'overlay que s'ha de separar.
    const ImageOverlay &m_overlay;
    /// Llista de regions trobades.
    QList<QRect> m_regions;

};

}

#endif // UDGIMAGEOVERLAYREGIONFINDER_H
