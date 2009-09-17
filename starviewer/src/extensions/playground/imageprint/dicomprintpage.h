#ifndef UDGDICOMPRINTPAGE_H
#define UDGDICOMPRINTPAGE_H

#include <QString>

#include "printpage.h"

/**
* Implementaci� per la Interf�cie DicomPrintPage que defineix una p�gina amb imatges per imprimir en impressores Dicom en DCMTK.
*/

namespace udg
{

class Image;

class DicomPrintPage: public PrintPage
{
public:
    
    ///Especifica/Retorna la mida sobre de la placa sobre el que s'imprimir�
    void setFilmSize(const QString &filmSize);	
    QString getFilmSize() const;

    ///Especifica/Retorna layout de la placa sobre el que s'imprimir�
    void setFilmLayout(const QString &filmLayout);	
    QString getFilmLayout() const;

    ///Retorna el n�mero de columnes/files del layout
    int getFilmLayoutColumns();
    int getFilmLayoutRows();
       
    ///Especifica/Retorna l'orientaci� (Horitzontal/Vertical) de la placa en que s'imprimir�
    void setFilmOrientation(const QString &filmOrientation);	
    QString getFilmOrientation() const;

    ///Especifica/retorna el Magnification amb le que s'imprimir�
    void setMagnificationType(const QString &magnificationType);	
    QString	getMagnificationType() const;

    ///Especifica/retorna la densitat m�xima amb la que s'imprimir�
    void setMaxDensity(ushort maxDensity);
    ushort getMaxDensity();

    ///Especifica/retorna la densitat m�nima amb la que s'imprimir�
    void setMinDensity(ushort minDensity);
    ushort getMinDensity();
 
    ///Especifica si s'imprimir� les imatges amb trim o sense
    void setTrim(bool trim);
    bool getTrim();

    ///Especifica/retorna la densitat amb la que s'imprimir� la l�nia de separaci� entre les imatges
    void setBorderDensity(const QString &borderDensity);
    QString getBorderDensity() const;

    ///Especifica/retorna la densitat amb que s'imprimiran els image box sense imatge
    void setEmptyImageDensity(const QString &emptyImageDensity);
    QString getEmptyImageDensity() const;

    ///Especifica/Retorna el smoothing type per defecte amb la que s'imprimir�
    void setSmoothingType(const QString &smoothingType);
    QString getSmoothingType() const;

    ///Especifica/retorna la polaritat per defecte que s'utilitzar� per imprimir
    //Aquest par�metre a nivell dicom s'aplica per imatge no per p�gina
    void setPolarity(const QString &polarity);
    QString getPolarity() const;

private:

    QString m_filmSize;
    QString	m_filmLayout;
    QString m_filmOrientation;
    QString	m_magnificationType;
    bool m_trim;
    QString m_borderDensity;
    QString m_emptyImageDensity;
    QString m_smoothingType;
    QString m_polarity;
    ushort m_minDensity, m_maxDensity;

    /**M�tode que retorna el n�mero de columnes i files del layout, s'implementa en un sol m�tode per no tenir codi duplicat
       si s'implement�s un m�tode per obtenir el n�mero de columnes i una altre per obtenir el n�mero de files*/
    void getFilmLayoutColumnsRows(int &colums, int &rows);    
    
};
};
#endif