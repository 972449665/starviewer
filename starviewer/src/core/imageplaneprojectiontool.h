/***************************************************************************
 *   Copyright (C) 2010 by Grup de Gr�fics de Girona                       *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#ifndef UDGIMAGEPLANEPROJECTIONTOOL_H
#define UDGIMAGEPLANEPROJECTIONTOOL_H

#include "tool.h"

// Qt's
#include <QPointer>
#include <QMap>

class QStringList;
class vtkImageReslice;

namespace udg {

class ToolConfiguration;
class ToolData;
class ImagePlaneProjectionToolData;
class Q2DViewer;
class ImagePlane;
class DrawerLine;
class Volume;

class ImagePlaneProjectionTool : public Tool
{
Q_OBJECT
public:
    ImagePlaneProjectionTool( QViewer *viewer, QObject *parent = 0 );
    ~ImagePlaneProjectionTool();

    void handleEvent( long unsigned eventID );

    /// Assignem una configuracio
    void setConfiguration( ToolConfiguration *configuration );

    /// Sobrescriu la funci� del pare per assignar-li noves connexions
    void setToolData( ToolData *data );

    /// Indica si la tool est� habilitada o no
    void setEnabled( bool enabled );

    /// Indica el n�mero d'imatges a generar al volum de reconstrucci�
    /// assignat a un visor amb la tool configurada com a consumidor
    void setNumImagesReconstruction( int numImages );

private slots:
    /// Inicialitza el pla projectat per cada l�nia de l'actual viewer amb la tool configurada com a productor
    /// Actualitza el volum compartit amb la resta de visors amb la tool activa
    /// Es crida cada cop que el viewer canv�i d'input 
    void initializeImagePlanesUpdteVolume( Volume *volume );
    
    /// Inicialitza el pla projectat per cada l�nia de l'actual viewer amb la tool configurada com a productor&consumidor
    /// Es crida cada cop que es modifica el volum de les dades compartides
    void initializeImagePlanesCheckingData();
    
    /// Actualitza el pla projectat de cada l�nia de l'actual viewer amb la tool configurada com a productor 
    /// i ho modifica a les dades compartides
    /// Es crida cada cop que al viewer canv�i d'input, l'imatge o l'slab
    void updateProjections();

    /// Comprova si s'ha modificat el pla projectat per la l�nia associada a l'actual viewer amb la tool configurada com a consumidor 
    void checkProjectedLineBindUpdated( QString nameProjectedLine );

    /// Comprova si cal actualitzar el pla projectat per alguna l�nia associada perqu� un altre viewer amb la tool configurada com
    /// a productor ha manipulat una l�nia amb la mateixa orientaci�
    void checkImagePlaneBindUpdated( QString nameProjectedLine );

    /// Comprova si s'ha modificat el gruix per alguna l�nia associada perqu� l'usuari ha modifcat el n�mero d'imatges a mostrar
    /// a la reconstrucci� al visor consumidor.
    void checkThicknessProjectedLineUpdated( QString nameProjectedLine );

private:
    /// Activa les funcionalitats de la tool necess�ries segons el tipus de configuraci�
    /// (connect(signals - slots))
    void enableConnections();

    /// Desactiva les funcionalitats de la tool segons el tipus de configuraci�
    /// (disconnect(signals - slots))
    void disableConnections();

    /// Comprova que els valors assignats a la configuraci� siguin correctes
    void checkValuesConfiguration( ToolConfiguration *configuration );
    void checkConfigurationProducer( ToolConfiguration *configuration );
    void checkConfigurationConsumer( ToolConfiguration *configuration );

    /// Aplica els valors de configuraci� a la tool
    void applyConfiguration();

    /// Inicialitza el reslice del viewer amb la tool configurada com a consumidor
    void initReslice( Volume *volume );

    /// Inicialitza les linies projectades al viewer quan la tool est� configurada com a productor
    void initProjectedLines();

    // Inicialitza el pla projectat per una linia de projecci� de la tool
    // S'assigna un espaiat, dimensions i l�mits en funci� del tipus d'orientaci�
    void initializeImagePlane( DrawerLine *projectedLine, Volume *volume );

    /// Actualitza la projecci� de la l�nia indicada amb el pla indicat
    /// Tamb� actualitza el pla projectat per la l�nia a les dades compartides de la tool
    void updateProjection( DrawerLine *projectedLine, ImagePlane *imagePlane );
    void updateProjection( DrawerLine *projectedLine, ImagePlane *imagePlane, bool projectedLineDrawed, bool updateToolData );

    /// Modifica el reslice de l'actual viewer amb la tool configurada com a consumidor perqu�
    /// mostri el pla que toca
    void updateReslice( Volume *volume );

    /// Actualitza el viewer amb la tool configurada com a consumidor perqu� mostri el pla projectat per la seva l�nia associada
    void showImagePlaneProjectedLineBind();

    /// Determina si s'est� manipulant la l�nia projectada o no
    void detectManipulationProjectedLine();

    /// Rota la l�nia de projecci�
    void rotateProjectedLine();

    /// Obt� l'eix de rotaci� del pla projectat sobre el viewer
    void getRotationAxisImagePlane( ImagePlane *imagePlane, double axis[3] );

    /// Rota el pla projectat sobre el viewer pel seu centre
    void rotateMiddleImagePlane( ImagePlane *imagePlane, double degrees , double rotationAxis[3] );

    /// Despla�a la l�nia de projecci�
    void pushProjectedLine();

    void releaseProjectedLine();

    /// Mostra o amaga les l�nies per indicar el gruix del n�mero d'imatges que formen el volum
    /// reconstru�t pel visor consumidor
    void applyThicknessProjectedLine( QString nameProjectedLine, DrawerLine *projectedLine );

private:
    /// El volum al que se li practica l'MPR
    Volume *m_volume;
    
    /// Dades espec�fiques de la tool
    ImagePlaneProjectionToolData *m_myData;
    
    /// Viewer 2D sobre el qual treballem
    Q2DViewer *m_2DViewer;

    /// Indica l'estat de la tool habilitada o deshabilitada
    bool m_enabled;

    /// Linies projectades: cadascuna indica la intersecci� entre un pla projectat que representen i el pla actual del Viewer
    /// Per cada l�nia es guarda el nom identificador i la seva orientaci� inicial
    QMap< DrawerLine *, QStringList > m_projectedLines;

    /// Plans projectats per cadascuna de les l�nies incloses al viewer producer
    /// Per cada l�nia existir� un nom identificatiu i a aquest se li associar� el pla que projecta
    QMap< QString, ImagePlane *> m_imagePlanes;

    /// L�nia de projecci� que est� manipulant l'usuari
    DrawerLine *m_pickedProjectedLine;

    /// Nom de la l�nia de projecci� que est� manipulant l'usuari
    QString m_pickedProjectedLineName;

    /// Nom identificatiu de la linia de projecci� que fa modificar el pla mostrat al viewer amb la tool configurada 
    /// com a consumidor. (La l�nia estar� mostrada a un viewer amb la tool configurada com a productor)
    QString m_nameProjectedLineBind;

    /// Reslice del viewer amb la tool configurada com a consumidor
    vtkImageReslice *m_reslice;

    /// Estat en el que es troba la manipulaci� de la l�nia projectada que es manipula
    enum { NONE , ROTATING , PUSHING };
    int m_state;

    /// Per controlar el moviment de la l�nia projectada a partir de l'interacci� de l'usuari
    double m_initialPickX , m_initialPickY;

    /// Indica el gruix a tenir en compte al generar la reconstrucci�
    int m_thickness;

    /// L�nia que ens marca el l�mit superior del thickness indicat per l'usuari.
    QPointer<DrawerLine> m_upLineThickness;

    /// L�nia que ens marca el l�mit inferior del thickness indicat per l'usuari.
    QPointer<DrawerLine> m_downLineThickness;
};
}  // end namespace udg

#endif
