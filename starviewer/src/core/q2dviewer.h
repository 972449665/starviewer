/***************************************************************************
 *   Copyright (C) 2005 by Grup de Gràfics de Girona                       *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#ifndef UDGQ2DVIEWER_H
#define UDGQ2DVIEWER_H

#include "qviewer.h"
// Fordward declarations
// vtk
class vtkPropPicker;
class vtkTextActor;
class vtkCornerAnnotation;
class vtkCoordinate;
class vtkImageBlend;
class vtkImageActor;
class vtkImageData;
// Grayscale pipeline
class vtkImageMapToWindowLevelColors2; // Permet aplicar window/level amb imatges a color
// Thick Slab
class vtkProjectionImageFilter;

namespace udg {

// Fordward declarations
class Volume;
class Image;
class Drawer;
class ImagePlane;
class ImageOrientationOperationsMapper;

/**

    Classe base per als visualitzadors 2D.

    El mode d'operació habitual serà el de visualitar un sol volum.
    Normalment per poder visualitzar un volum farem el següent
    \code
    Q2DViewer* visor = new Q2DViewer();
    visor->setInput(volum);

    En el cas que desitjem solapar dos volums haurem d'indicar el volum solapat amb el mètode setOverlayInput().
    Quan solapem volums tenim 1 manera de solapar aquests volums, aplicant un blending,
    en aquest cas hauríem de fer servir el mètode setOverlapMethod() indicant una de les opcions (de moment únicament Blend)
    \TODO acabar la doc sobre solapament

    Per defecte el visualitzador mostra la primera imatge en Axial. Per les altres vistes (Sagital i Coronal) mostraria la imatge central

    Podem escollir quines annotacions textuals i de referència apareixeran en la vista 2D a través dels flags "AnnotationFlags" definits com enums.
    Aquests flags es poden passar en el constructor o els podem modificar a través dels mètodes \c addAnnotation() o \c removeAnnotation()
    que faran visible o invisible l'anotació indicada. Per defecte el flag és \c AllAnnotation i per tant es veuen totes les anotacions per defecte.

    @author Grup de Gràfics de Girona  (GGG )
*/
class Q2DViewer : public QViewer {
Q_OBJECT
public:
    /// Tipus de solapament dels models
    enum OverlapMethod { None, Blend };

    /// Alineament de la imatge (dreta, esquerre, centrat)
    enum AlignPosition { AlignCenter, AlignRight, AlignLeft };

    /// Aquests flags els farem servir per decidir quines anotacions seran visibles i quines no
    enum AnnotationFlag { NoAnnotation = 0x0, WindowInformationAnnotation = 0x1, PatientOrientationAnnotation = 0x2, SliceAnnotation = 0x8, PatientInformationAnnotation = 0x10, AcquisitionInformationAnnotation = 0x20, AllAnnotation = 0x7F };
    Q_DECLARE_FLAGS(AnnotationFlags, AnnotationFlag)

    Q2DViewer(QWidget *parent = 0);
    ~Q2DViewer();

    /// Ens retorna la vista que tenim en aquells moments del volum
    CameraOrientationType getView() const;

    /// Assigna/Retorna el volum solapat
    void setOverlayInput(Volume *volume);
    Volume* getOverlayInput();

    /// Indiquem que cal actualitzar l'Overlay actual
    void updateOverlay();

    /// Assignem l'opacitat del volum solapat. 
    /// Els valors podran anar de 0.0 a 1.0, on 0.0 és transparent i 1.0 és completament opac.
    void setOverlayOpacity(double opacity);

    /// Obté el window level actual de la imatge
    /// TODO els mètodes no es criden enlloc, mirar si són necessaris o no
    double getCurrentColorWindow();
    double getCurrentColorLevel();
    void getCurrentWindowLevel(double wl[2]);

    /// Retorna la llesca/fase actual
    int getCurrentSlice() const;
    int getCurrentPhase() const;

    /**
     * Ens retorna el drawer per poder pintar-hi primitives
     * @return Objecte drawer del viewer
     */
    Drawer* getDrawer() const;

    /**
     * Calcula la coordenada de la imatge que es troba per sota del cursor en coordenades de món
     * En el cas el cursor estigui fora de la imatge, la coordenada no té cap validesa
     * @param xyz[] La coordenada de la imatge, en sistema de coordenades de món
     * @return Cert si el cursor es troba dins de la imatge, fals altrament
     */
    bool getCurrentCursorImageCoordinate(double xyz[3]);

    /**
     * Ens retorna la imatge que s'està visualitzant en aquell moment
     * TODO pot ser que aquest mètode sigui purament de conveniència per la tool de reference lines i desaparegui
     * en un futur o sigui substituit per un altre
     * @return Objecte Image corresponent al que es visualitza en aquell moment. NULL si no es veu cap imatge
     */
    Image* getCurrentDisplayedImage() const;

    /**
     * Ens dóna el pla d'imatge actual que estem visualitzant
     * @param vtkReconstructionHack HACK variable booleana que ens fa un petit hack
     * per casos en que el pla "real" no és el que volem i necessitem una petita modificació
     * ATENCIÓ: Només es donarà aquest paràmetre (amb valor true) en casos que realment se sàpiga el que s'està fent!
     * @return El pla imatge actual
     */
    ImagePlane* getCurrentImagePlane(bool vtkReconstructionHack = false);

    /**
     * Ens dóna el pla d'imatge de la llesca
     * @param vtkReconstructionHack HACK variable booleana que ens fa un petit hack
     * per casos en que el pla "real" no és el que volem i necessitem una petita modificació
     * ATENCIÓ: Només es donarà aquest paràmetre (amb valor true) en casos que realment se sàpiga el que s'està fent!
     * @return El pla imatge de la llesca
     */
    ImagePlane* getImagePlane(int sliceNumber, int phaseNumber, bool vtkReconstructionHack = false);

    /**
     * donat un punt 3D en espai de referència DICOM, ens dóna la projecció d'aquest punt sobre
     * el pla actual, transformat a coordenades de món VTK
     * @param pointToProject[]
     * @param projectedPoint[]
     * @param vtkReconstructionHack HACK variable booleana que ens fa un petit hack
     * per casos en que el pla "real" no és el que volem i necessitem una petita modificació
     */
    void projectDICOMPointToCurrentDisplayedImage(const double pointToProject[3], double projectedPoint[3], bool vtkReconstructionHack = false);

    /// Retorna el thickness
    /// TODO potser un nom més correcte seria getCurrentSliceThickness() ja que podríem tenir imatges amb diferents thickness
    double getThickness();

    /**
     * Ens retorna el rang actual de llesques, tenint en compte totes les imatges,
     * tant com si hi ha fases com si no
     * @param min valor mínim
     * @param max valor màxim
     */
    /// TODO podria ser protected o private, ningú necessita aquestes dades fora del visor
    void getSliceRange(int &min, int &max);
    int* getSliceRange();

    /**
     * Ens dóna la llesca mínima/màxima de llesques, tenint en compte totes les imatges,
     * tant com si hi ha fases com si no
     * @return valor de la llesca mínima/màxima
     */
    int getMinimumSlice();
    int getMaximumSlice();

    /// Ens indica si s'està aplicant o no thick slab
    bool isThickSlabActive() const;

    /// Obtenim el mode de projecció del thickslab.
    /// Si el thickslab no està actiu, el valor és indefinit
    int getSlabProjectionMode() const;

    /// Obtenim el gruix de l'slab
    /// Si el thickslab no està actiu, el valor és indefinit
    int getSlabThickness() const;

    /**
     * Donada una coordenada de món, l'ajustem perquè caigui dins dels límits de l'imatge actual
     * Això ens serveix per tools que agafen qualsevol punt de món, però necessiten que aquesta estigui 
     * dins dels límits de la imatge, com pot ser una ROI. Aquest mètode acaba d'ajustar la coordenada perquè 
     * estigui dins dels límits de la pròpia imatge
     * @param xyz[] Coordenada que volem ajustar. Serà un paràmetre d'entrada/sortida i el seu contingut
     * es modificarà perquè caigui dins dels límits de la imatge
     */
    void putCoordinateInCurrentImageBounds(double xyz[3]);

    /// Retorna la informació de la llesca actual del visualitzador
    vtkImageData* getCurrentSlabProjection();

    /// Retorna un vector de 4 strings en el que tenim quatre elements que representen les etiquetes
    /// indicant on està la dreta/esquerra, cap/peu, davant/darrere del pacient
    /// El primer element correspon a la esquerra de la imatge, el segon el damunt, el tercer a la dreta i el quart a sota
    /// Si tenim una imatge axial pura la llista seria R,H,L,F (Right, Head, Left, Feet )
    QVector<QString> getCurrentDisplayedImageOrientationLabels() const;

    /// Ens diu quin és el pla de projecció de la imatge que es veu en aquell moment
    /// Valors: AXIAL, SAGITAL, CORONAL, OBLIQUE o N/A
    QString getCurrentPlaneProjectionLabel() const;

    /// Obté els índexs dels eixos de coordenades corresponents a la vista 2D indicada
    /// Aquests índexs indiquen com hem d'indexar una coordenada 3D per obtenir 
    /// la corresponent projecció sobre la vista 2D sobre la que estem operant
    static void getXYZIndexesForView(int &x, int &y, int &z, int view);
    static int getXIndexForView(int view);
    static int getYIndexForView(int view);
    static int getZIndexForView(int view);

public slots:
    virtual void setInput(Volume *volume);

    void resetView(CameraOrientationType view);
    void resetViewToAxial();
    void resetViewToCoronal();
    void resetViewToSagital();

    /// Restaura el visualitzador a l'estat inicial
    void restore();

    /// Esborra totes les primitives del visor
    void clearViewer();

    /// Canvia el WW del visualitzador, per tal de canviar els blancs per negres, i el negres per blancs
    void invertWindowLevel();

    /// Canvia la llesca que veiem de la vista actual
    void setSlice(int value);

    /// Canvia la fase en que es veuen les llesques si n'hi ha
    void setPhase(int value);

    /// Indica el tipu de solapament dels volums, per defecte blending
    void setOverlapMethod(OverlapMethod method);
    void setOverlapMethodToNone();
    void setOverlapMethodToBlend();

    /// Afegir o treure la visibilitat d'una anotació textual/gràfica
    void enableAnnotation(AnnotationFlags annotation, bool enable = true);
    void removeAnnotation(AnnotationFlags annotation);

    void setWindowLevel(double window, double level);
    void setTransferFunction(TransferFunction *transferFunction);

    /// L'únic que fa és emetre el senyal seedPositionChanged, per poder-ho cridar desde la seedTool
    /// TODO Aquest mètode hauria de quedar obsolet
    void setSeedPosition(double pos[3]);

    /// Aplica una rotació de 90 graus en el sentit de les agulles del rellotge
    /// tantes "times" com li indiquem, per defecte sempre serà 1 "time"
    void rotateClockWise(int times = 1);

    /// Aplica una rotació de 90 graus en el sentit contrari a les agulles del rellotge
    /// tantes "times" com li indiquem, per defecte sempre serà 1 "time"
    void rotateCounterClockWise(int times = 1);

    /// Aplica un flip horitzontal/vertical sobre la imatge. El flip vertical es farà com una rotació de 180º seguida d'un flip horitzontal
    void horizontalFlip();
    void verticalFlip();

    // TODO aquests mètodes també haurien d'estar en versió QString!
    
    /// Li indiquem quin mode de projecció volem aplicar sobre l'slab
    /// @param projectionMode Valor que identifica quina projecció apliquem
    void setSlabProjectionMode(int projectionMode);

    /// Indiquem el gruix de l'slab
    /// @param thickness Nombre de llesques que formen l'slab
    void setSlabThickness(int thickness);

    /// Activem a desactivem l'aplicació del thick slab sobre la imatge
    /// @param enable
    void enableThickSlab(bool enable = true);

    /// Alineament de la imatge dins del visualitzador
    void alignLeft();
    void alignRight();

    /// Posa la posició d'alineament de la imatge (dreta, esquerre, centrat )
    void setAlignPosition(AlignPosition alignPosition);

    /// Aplica les transformacions 2D necessàries sobre la imatge actual perquè aquesta tingui la orientació indicada
    /// La orientació indicada ha de ser possible de conseguir mitjançant operacions de rotació i flip. En cas que no 
    /// existeixin combinacions possibles, no es canviarà la orientació de la imatge
    void setImageOrientation(const QString &orientation);

signals:
    /// Envia la nova llesca en la que ens trobem
    void sliceChanged(int);

    /// Envia la nova fase en la que ens trobem
    void phaseChanged(int);

    /// Envia la nova vista en la que ens trobem
    void viewChanged(int);

    /// Indica el nou window level
    void windowLevelChanged(double window, double level);

    /// Senyal que s'envia quan la llavor s'ha canviat
    /// TODO Mirar de treure-ho i posar-ho en la tool SeedTool
    void seedPositionChanged(double x, double y, double z);    

    /// S'emet quan canvia l'slab thickness
    /// @param thickness Nou valor de thickness
    void slabThicknessChanged(int thickness);

    /// Senyal que s'envia quan ha canviat l'overlay
    void overlayChanged();
    void overlayModified();

protected:
    /// Processem l'event de resize de la finestra Qt
    virtual void resizeEvent(QResizeEvent *resize);

private:
    /// Refresca la visibilitat de les annotacions en funció dels flags que tenim
    void refreshAnnotations();

    /// Actualitzem les dades de les annotacions, per defecte totes, sinó, només les especificades
    void updateAnnotationsInformation(AnnotationFlags annotation = Q2DViewer::AllAnnotation);

    /// Desglossem les actualitzacions de les diferents informacions que es mostren per pantalla
	void updatePatientAnnotationInformation();
    void updateSliceAnnotationInformation();

    /// Refresca els valors de les annotacions de llesca. Si els valors referents
    /// a les fases són < 2 no es printarà informació de fases
    /// Si hi ha thick slab, mostrarà el rang d'aquest
    void updateSliceAnnotation(int currentSlice, int maxSlice, int currentPhase = 0, int maxPhase = 0);

    /// Crea i inicialitza totes les anotacions que apareixeran per pantalla
    void createAnnotations();

    /// Crea les anotacions de l'orientació del pacient
    void createOrientationAnnotations();

    /// Afegeix tots els actors a l'escena
    void addActors();

    /// Helper method, donada una etiqueta d'orientació, ens retorna aquesta etiqueta però amb els valors oposats.
    /// Per exemple, si l'etiqueta que ens donen és RPI (Right-Posterior,Inferior), el valor retornat seria LAS (Left-Anterior-Superior)
    /// Les etiquetes vàlides i els seus oposats són les següents:
    /// R:L (Right-Left), A:P (Anterior-Posterior), S:I (Superior-Inferior), H:F(Head-Feet)
    static QString getOppositeOrientationLabel(const QString &label);

    /// A partir de l'string d'orientació del pacient mapeja les anotacions correctes segons com estem mirant el model. 
    /// A això li afecta també si la vista és axial, sagital o coronal
    void mapOrientationStringToAnnotation();

    /// Actualitza les característiques dels actors dels viewports
    void updateDisplayExtent();

    /// Aplica el pipeline d'escala de grisos segons la modality, voi i presentation lut's que s'hagin calculat.
    /// Això permet que el càlcul s'hagi fet en un presentation state, per exemple.
    /// TODO És públic únicament perquè el fa servir el presentation state attacher. Podria ser protected o private.
    void applyGrayscalePipeline();

    /// Re-inicia la càmera en la vista actual. Posa els paràmetres de rotació, zoom, desplaçament, flip, etc. als seus valors inicials
    void resetCamera();
    
    /// Thick slab
    void computeRangeAndSlice(int newSlabThickness);

    ///  Valida el valor d'slice donat i actualitza les variables membres pertinents, com m_currentSlice o m_firstSlabSlice
    void checkAndUpdateSliceValue(int value);

    /// Aplica el factor de rotació adient segons els girs que li indiquem. No actualitza la càmera ni l'escena, simplement
    /// es fa servir per posar els valors correctes a les variables internes que controlen com està girada la imatge.
    void rotate(int times);

    /// S'encarrega d'encaixar la imatge actual a la mida del viewport
    void fitImageIntoViewport();

private slots:
    /// Actualitza les transformacions de càmera (de moment rotació i flip )
    void updateCamera();

protected:
    /// Actor d'imatge
    vtkImageActor *m_imageActor;

    /// Conserva la vista actual
    CameraOrientationType m_lastView;

    /// La llesca actual que estem visualitzant
    int m_currentSlice;

    /// La fase (instant de temps) de les llesques
    int m_currentPhase;

    /// Aquest és el segon volum afegit a solapar
    Volume *m_overlayVolume;

    /// Aquest és el blender per veure imatges fusionades
    vtkImageBlend* m_blender;

    /// Opacitat del volum solapat
    double m_overlayOpacity;

    /// El picker per agafar punts de la imatge
    vtkPropPicker *m_imagePointPicker;

    /// Annotacions de texte referents a informació de la sèrie
    /// (nom de pacient, protocol,descripció de sèrie, data de l'estudi, etc)
    /// i altre informació rellevant (nº imatge, ww/wl, etc )
    vtkCornerAnnotation *m_cornerAnnotations;

private:
    /// Flag que ens indica quines anotacions es veuran per la finestra
    AnnotationFlags m_enabledAnnotations;

    /// Tipus de solapament dels volums en cas que en tinguem més d'un
    OverlapMethod m_overlapMethod;

    /// Els strings amb els textes de cada part de la imatge
    QString m_lowerLeftText, m_lowerRightText, m_upperLeftText, m_upperRightText;

    /// Aquest string indica les anotacions que ens donen les referències del pacient (Right,Left,Posterior,Anterior,Inferior,Superior) 
    QString m_patientOrientationText[4];

    /// Textes adicionals d'anotoació
    vtkTextActor *m_patientOrientationTextActor[4];

    /// Mides (x,y) de la imatge que mostrarem com informació adicional
    int m_imageSizeInformation[2];

    /// Factor de rotació. En sentit de les agulles del rellotge 0: 0º, 1: 90º, 2: 180º, 3: 270º.
    int m_rotateFactor;

    /// Ampliació tractament dinàmic
    /// Nombre de fases
    int m_numberOfPhases;

    /// Valor màxim al que pot arribar m_currentSlice
    int m_maxSliceValue;

    /// Indica si cal aplicar un flip horitzontal o no sobre la càmera
    bool m_applyFlip;

    /// Aquesta variable controla si la imatge està flipada respecte la seva orientació original. Útil per controlar annotacions.
    bool m_isImageFlipped;

    /// Especialista en dibuixar primitives
    Drawer *m_drawer;

    /// Objectes per a les transformacions en el pipeline d'escala de grisos
    vtkImageMapToWindowLevelColors2 *m_windowLevelLUTMapper;

    // Secció "ThickSlab"
    /// Nombre de llesques que composen el thickSlab
    int m_slabThickness;

    /// Filtre per composar les imatges del thick slab
    vtkProjectionImageFilter *m_thickSlabProjectionFilter;

    /// Variables per controlar el rang de llesques en el que es troba l'slab
    int m_firstSlabSlice, m_lastSlabSlice;

    /// Variable per controlar si el thickSlab s'està aplicant o no.
    /// Perquè sigui true, m_slabThickness > 1, altrament false
    bool m_thickSlabActive;

    /// Indica quin tipus de projecció apliquem sobre l'slab
    int m_slabProjectionMode;

    /// Conté el mapeig d'operacions a fer quan voelm passar d'una orientació a un altre
    ImageOrientationOperationsMapper *m_imageOrientationOperationsMapper;

    /// Posició a on s'ha d'alinear la imatge (dreta, esquerre o centrat )
    AlignPosition m_alignPosition;

};
Q_DECLARE_OPERATORS_FOR_FLAGS(Q2DViewer::AnnotationFlags)
};  //  End namespace udg

#endif
