#ifndef UDGIMAGEPLANEPROJECTIONTOOLDATA_H
#define UDGIMAGEPLANEPROJECTIONTOOLDATA_H

#include "tooldata.h"
#include <QMap>

namespace udg {

class ImagePlane;
class Volume;
        
/**
 Classe que guarda les dades compartides per la tool ImagePlaneProjectionTool
*/
class ImagePlaneProjectionToolData : public ToolData
{
Q_OBJECT
public:
    
    ImagePlaneProjectionToolData(QObject *parent = 0);

    ~ImagePlaneProjectionToolData();

    /// Assigna el volum amb el que s'ha de treballar als visors que tenen activada la tool
    void setVolume( Volume *volume );

    /// Retorna el volum amb el que s'ha de treballar als visors que tenen activada la tool
    Volume * getVolume( );

    /// Assigna o actualitza el pla que projecta la l�nia identificada amb el nom passat per par�metre
    void setProjectedLineImagePlane( QString projectedLineName, ImagePlane *imagePlane );

    /// Retorna el pla projectat per la l�nia identificada amb el nom passat per par�metre
    ImagePlane *getProjectedLineImagePlane( QString projectedLineName );

    /// Fa que es llenci la senyal indicant que s'han inicialitzat les dades de treball
    void emitDataInitialized();

    /// Assigna o actualitza el gruix associat a la l�nia de projecci� identificada amb el nom passat per par�metre
    void setProjectedLineThickness( QString projectedLineName, int thickness );

    /// Retorn el gruix associat a la l�nia de projecci� identificada amb el nom passat per par�metre
    int getProjectedLineThickness( QString projectedLineName );

signals:
    /// Senyal que indica que s'han inicialitzat les dades de treball
    void dataInitialized();

    /// Senyal que indica que s'ha modificat el pla projectat per la l�nia identificada amb el nom passat per par�metre
    /// Els viewers consumer associats a la l�nia amb aquest nom canviaran el pla projectat quan rebin aquesta senyal
    void imagePlaneUpdated( QString projectedLineName );

    /// Senyal que indica que s'ha modificat el gruix associat a la l�nia identificada amb el nom passat per par�metre
    /// Els viewers producer associats a la l�nia amb aquest nom comprovaran si han de mostrar o amagar les l�nies de 
    /// gruix mostrades
    void thicknessUpdated( QString projectedLineName );

private:
    
    /// Volum amb el que es treballa als visors que tenen activada la tool
    Volume *m_volume;

    /// Plans projectats per cadascuna de les l�nies incloses al viewer producer
    /// Per cada l�nia existir� un nom identificatiu i a aquest se li associar� el pla que projecta
    QMap< QString, ImagePlane *> m_imagePlanes;

    /// Gruix associat a cadascuna de les l�nies incloses al viewer producer
    /// Per cada l�nia existir� un nom identificatiu i a aquest se li associar� el gruix
    QMap< QString, int > m_projectedLinesThickness;

};

}

#endif