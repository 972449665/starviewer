#ifndef UDGPRINTPAGE_H
#define UDGPRINTPAGE_H

#include <QString>
#include <QList>

/**
* Implementaci� per la Interf�cie PrintPage, que defineix una p�gina d'impressi� amb imatges DICOM.
*/

namespace udg
{

class Image;

class PrintPage 
{
public:
    
    ///Especifica/retorna el n�mero de p�gina
    void setPageNumber(int pageNumber);
    int getPageNumber();

    ///Especifica/retorna les imatges que s'han d'imprimir en aquesta p�gina. Les imatges s'imprimiren segons l'ordre d'inserci� a la llista
    void setImagesToPrint(QList<Image*> imagesToPrint);
    QList<Image*> getImagesToPrint();

private:

    int m_pageNumber;
    QList<Image*> m_imagesToPrint;
    
};
};
#endif