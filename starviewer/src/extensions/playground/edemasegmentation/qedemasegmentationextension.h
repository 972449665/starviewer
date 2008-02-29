/***************************************************************************
 *   Copyright (C) 2005-2006 by Grup de Gràfics de Girona                  *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#ifndef UDGQEDEMASEGMENTATION_H
#define UDGQEDEMASEGMENTATION_H

#include "ui_qedemasegmentationextensionbase.h"


#include <QString>

// FWD declarations
class QAction;
class QToolBar;
class vtkImageMask;
class vtkImageThreshold;
class vtkActor;

namespace udg {

// FWD declarations
class Volume;
class StrokeSegmentationMethod;
class ToolsActionFactory;

/**
Extensió que segmenta l'hematoma i l'edema

    @author Grup de Gràfics de Girona  ( GGG ) <vismed@ima.udg.es>
*/

class QEdemaSegmentationExtension : public QWidget , private ::Ui::QEdemaSegmentationExtensionBase {
Q_OBJECT
public:
    QEdemaSegmentationExtension( QWidget *parent = 0 );

    ~QEdemaSegmentationExtension();

    /// Obtenim la ToolBar d'eines de l'extensió \TODO 'pujar' al pare com a mètode com a Extensions?
    QToolBar *getToolsToolBar() const { return m_toolsToolBar; };

    /// Omple la ToolBar amb les eines de l'extensió \TODO 'pujar' al pare com a mètode com a Extensions?
    //void populateToolBar( QToolBar *toolbar );

public slots:

    /// Li assigna el volum principal
    /// És slot pq també ho fem servir en cas de canvi de sèrie
    void setInput( Volume *input );

    /// Executa l'algorisme de segmetnació
    void ApplyMethod();
    void ApplyCleanSkullMethod( );
    void ApplyEdemaMethod( );
    void ApplyVentriclesMethod( );
    void ApplyFilterMainImage( );

/*  void changeViewToSagital();
    void changeViewToCoronal();

    /// Canvia el layout de visor singular/doble
    void changeViewToSingle();
    void changeViewToDouble();

    /// Li assigna el volum secundari. Aquest mètode només és de conveniència i és temporal
    void setSecondInput( Volume *input );
    */
private:

    /// El volum principal
    Volume *m_mainVolume;

    /// El volum on hi guardem el resultat de la segmentació
    Volume *m_lesionMaskVolume;
    Volume *m_edemaMaskVolume;
    Volume *m_ventriclesMaskVolume;
    Volume *m_activedMaskVolume;
    vtkImageThreshold *m_imageThreshold;

    /// El volum on hi guardem la imatge principal filtrada
    Volume *m_filteredVolume;

    /// El volum on hi guardem la fusio de la imatge i la màscara
    //Volume *m_fusionVolume;
    //vtkImageMask *m_vtkFusionImage;

    ///Per pintar la llavor
    //vtkActor *pointActor;
    int m_seedSlice;

    /// Mètode de la segmentació
    StrokeSegmentationMethod *m_segMethod;

    /// Membres de classe
    bool m_isSeed;
    bool m_isMask;

    bool m_isLeftButtonPressed;
    vtkActor *squareActor;
    int m_cont;
    int m_edemaCont;
    int m_ventriclesCont;
    int* m_activedCont;
    double m_volume;
    double m_edemaVolume;
    double m_ventriclesVolume;
    double* m_activedVolume;

  /// tipus d'edició dels models
    enum EditorType{ NoEditor , Paint , Erase , EraseSlice , EraseRegion };
    int m_editorTool;

    int m_minValue, m_maxValue;
    int m_insideValue, m_outsideValue;
    int m_lowerVentriclesValue, m_upperVentriclesValue;

    /// La ToolBar de les eines de l'extensió \TODO 'pujar' al pare com a membre com a Extensions? [hauria de ser protected]
    QToolBar *m_toolsToolBar;

    /// Accions
    QAction *m_slicingAction;
    QAction *m_windowLevelAction;
    QAction *m_zoomAction;
    QAction *m_moveAction;
    QAction *m_seedAction;
    QAction *m_editorAction;
    QAction *m_voxelInformationAction;
    QAction *m_rotateClockWiseAction;
    ToolsActionFactory *m_actionFactory;

    QAction *m_lesionViewAction;
    QAction *m_edemaViewAction;
    QAction *m_ventriclesViewAction;

    QAction *m_paintEditorAction;
    QAction *m_eraseEditorAction;
    QAction *m_eraseSliceEditorAction;
    QAction *m_eraseRegionEditorAction;

    ///Directori on guardem les màscares
    QString m_savingMaskDirectory;

    /// Grup de botons en format exclusiu
    QActionGroup *m_toolsActionGroup;
    QActionGroup *m_viewOverlayActionGroup;
    QActionGroup* m_editorToolActionGroup;

    /// crea les accions \TODO 'pujar' al pare com a mètode virtual com a Extensions? [hauria de ser protected]
    void createActions();

    /// Crea la ToolBar d'eines i altres si n'hi ha \TODO 'pujar' al pare com a mètode virtual com a Extensions? [hauria de ser protected]
    void createToolBars();

    /// Crea les connexions entre signals i slots
    void createConnections();

    /// Guarda i llegeix les característiques
    void readSettings();
    void writeSettings();


private slots:
     /// gestiona els events del m_2DView
    void strokeEventHandler( unsigned long id );

    /// gestiona els events del botó esquerre
    void leftButtonEventHandler( );

    /// visualitza la informaci�de la llavor del m�ode de segmentaci�
    void setSeedPosition( );

     /// determina la llavor del m�ode de segmentaci�
    void setEditorPoint( );

    /// desactiva el boole�que ens diu si est�el bot�esquerra apretat
    void setLeftButtonOff( );

    /// actualitza el valor llindar baix
    void setLowerValue( int x );

     /// actualitza el valor llindar alt
    void setUpperValue( int x );

    /// Canvia la opacitat de la m�cara
    void setOpacity(int op);

    /// Canvia a la opci�esborrar
    void setErase();

    /// Canvia a la opci�pintar
    void setPaint();

    /// Canvia a la opci�esborrar llesca
    void setEraseSlice();

    /// Canvia a la opci�esborrar regió
    void setEraseRegion();

    /// Dibuixa el cursor en la forma del pinzell
    void setPaintCursor( );

    void eraseMask(int size);
    void paintMask(int size);
    void eraseSliceMask();
    void eraseRegionMask();
    void eraseRegionMaskRecursive(int a, int b, int c);

    /// Calcula el volum de la m�cara
    double calculateMaskVolume();

    /// Calcula el volum de la m�cara suposant que la variable m_cont cont�el nombre de v�els != 0 de la m�cara
    double updateMaskVolume();

    /// Refresca el resultat del volum
    void updateVolume();

    /// Visualitza la m�cara donats uns thresholds
    void viewThresholds();

    /// Visualitza els diferents overlays
    void viewLesionOverlay();
    void viewEdemaOverlay();
    void viewVentriclesOverlay();

    /// Desa la màscara que s'està visualitzant
    void saveActivedMaskVolume();

/*    /// sincronitza les llesques de les s?ies que es visualitzen
    void synchronizeSlices( bool ok );

    /// ens permet escollir una nova s?ie per a comparar
    void chooseNewSerie();

  signals:
    /// Aquest senyal s'emetr?quan es vulgui canviar de s?ie per comparar
    void newSerie();
*/
};

} // end namespace udg

#endif
