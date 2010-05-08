#ifndef UDGIMAGEPLANEPROJECTIONTOOLDATA_H
#define UDGIMAGEPLANEPROJECTIONTOOLDATA_H

#include "tooldata.h"
#include <QMap>

namespace udg {

class ImagePlane;
class Volume;
        
/**
 Classe que guarda les dades compartides per la tool ImagePlaneProjectionTool

	@author Grup de Gr�fics de Girona  ( GGG ) <vismed@ima.udg.es>
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

signals:
    /// Senyal que indica que s'ha modificat el volum de treball
    void volumeChanged( Volume * );

    /// Senyal que indica que s'ha modificat el pla projectat per la l�nia identificada amb el nom passat per par�metre
    /// Els viewers consumer associats a la l�nia amb aquest nom canviaran el pla projectat quan rebin aquesta senyal
    void imagePlaneUpdated( QString projectedLineName );

private:
    
    /// Volum amb el que es treballa als visors que tenen activada la tool
    Volume *m_volume;

    /// Plans projectats per cadascuna de les l�nies incloses al viewer producer
    /// Per cada l�nia existir� un nom identificatiu i a aquest se li associar� el pla que projecta
    QMap< QString, ImagePlane *> m_imagePlanes;

};

}

#endif