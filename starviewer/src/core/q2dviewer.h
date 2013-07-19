#ifndef UDGQ2DVIEWER_H
#define UDGQ2DVIEWER_H

#include "qviewer.h"

#include <QPointer>

// Fordward declarations
// Vtk
class vtkPropPicker;
class vtkTextActor;
class vtkCornerAnnotation;
class vtkCoordinate;
class vtkImageBlend;
class vtkImageActor;
class vtkImageData;
class vtkImageMask;
// Grayscale pipeline
// Permet aplicar window/level amb imatges a color
class vtkImageMapToWindowLevelColors2;
// Thick Slab
class vtkProjectionImageFilter;

namespace udg {

// Fordward declarations
class Volume;
class Image;
class ImageOverlay;
class Drawer;
class DrawerBitmap;
class ImagePlane;
class ImageOrientationOperationsMapper;
class VolumeReaderJob;
class QViewerCommand;
class PatientOrientation;

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
  */
class Q2DViewer : public QViewer {
Q_OBJECT
public:
    /// Tipus de solapament dels models
    enum OverlapMethod { None, Blend };

    /// Alineament de la imatge (dreta, esquerre, centrat)
    enum AlignPosition { AlignCenter, AlignRight, AlignLeft };

    /// Aquests flags els farem servir per decidir quines anotacions seran visibles i quines no
    enum AnnotationFlag { NoAnnotation = 0x0, WindowInformationAnnotation = 0x1, PatientOrientationAnnotation = 0x2, SliceAnnotation = 0x8,
                          PatientInformationAnnotation = 0x10, AcquisitionInformationAnnotation = 0x20, AllAnnotation = 0x7F };
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
    void getCurrentWindowLevel(double wl[2]);

    /// Retorna la llesca/fase actual
    int getCurrentSlice() const;
    int getCurrentPhase() const;

    /// Ens retorna el drawer per poder pintar-hi primitives
    /// @return Objecte drawer del viewer
    Drawer* getDrawer() const;

    /// Calcula la coordenada de la imatge que es troba per sota del cursor en coordenades de món
    /// En el cas el cursor estigui fora de la imatge, la coordenada no té cap validesa
    /// @param xyz[] La coordenada de la imatge, en sistema de coordenades de món
    /// @return Cert si el cursor es troba dins de la imatge, fals altrament
    bool getCurrentCursorImageCoordinate(double xyz[3]);

    /// Returns current displayed image.
    /// If some orthogonal reconstruction different from original acquisition is applied, returns null
    Image* getCurrentDisplayedImage() const;

    /// Ens dóna el pla d'imatge actual que estem visualitzant
    /// @param vtkReconstructionHack HACK variable booleana que ens fa un petit hack
    /// per casos en que el pla "real" no és el que volem i necessitem una petita modificació
    /// ATENCIÓ: Només es donarà aquest paràmetre (amb valor true) en casos que realment se sàpiga el que s'està fent!
    /// @return El pla imatge actual
    ImagePlane* getCurrentImagePlane(bool vtkReconstructionHack = false);

    /// Ens dóna el pla d'imatge de la llesca
    /// @param vtkReconstructionHack HACK variable booleana que ens fa un petit hack
    /// per casos en que el pla "real" no és el que volem i necessitem una petita modificació
    /// ATENCIÓ: Només es donarà aquest paràmetre (amb valor true) en casos que realment se sàpiga el que s'està fent!
    /// @return El pla imatge de la llesca
    ImagePlane* getImagePlane(int sliceNumber, int phaseNumber, bool vtkReconstructionHack = false);

    /// donat un punt 3D en espai de referència DICOM, ens dóna la projecció d'aquest punt sobre
    /// el pla actual, transformat a coordenades de món VTK
    /// @param pointToProject[]
    /// @param projectedPoint[]
    /// @param vtkReconstructionHack HACK variable booleana que ens fa un petit hack
    /// per casos en que el pla "real" no és el que volem i necessitem una petita modificació
    void projectDICOMPointToCurrentDisplayedImage(const double pointToProject[3], double projectedPoint[3], bool vtkReconstructionHack = false);

    /// Retorna el thickness. En cas que no disposem del thickness, el valor retornat serà 0.0
    double getCurrentSliceThickness();

    /// Ens retorna el rang actual de llesques, tenint en compte totes les imatges,
    /// tant com si hi ha fases com si no
    /// @param min valor mínim
    /// @param max valor màxim
    /// TODO podria ser protected o private, ningú necessita aquestes dades fora del visor
    void getSliceRange(int &min, int &max);
    int* getSliceRange();

    /// Ens dóna la llesca mínima/màxima de llesques, tenint en compte totes les imatges,
    /// tant com si hi ha fases com si no
    /// @return valor de la llesca mínima/màxima
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

    /// Donada una coordenada de món, l'ajustem perquè caigui dins dels límits de l'imatge actual
    /// Això ens serveix per tools que agafen qualsevol punt de món, però necessiten que aquesta estigui
    /// dins dels límits de la imatge, com pot ser una ROI. Aquest mètode acaba d'ajustar la coordenada perquè
    /// estigui dins dels límits de la pròpia imatge
    /// @param xyz[] Coordenada que volem ajustar. Serà un paràmetre d'entrada/sortida i el seu contingut
    /// es modificarà perquè caigui dins dels límits de la imatge
    void putCoordinateInCurrentImageBounds(double xyz[3]);

    /// Retorna la informació de la llesca actual del visualitzador
    vtkImageData* getCurrentSlabProjection();

    /// Retorna la orientació de pacient corresponent a la imatge que s'està visualitzant en aquell moment,
    /// és a dir, tenint en compte rotacions, flips, reconstruccions, etc.
    PatientOrientation getCurrentDisplayedImagePatientOrientation() const;

    /// Ens diu quin és el pla de projecció de la imatge que es veu en aquell moment
    /// Valors: AXIAL, SAGITAL, CORONAL, OBLIQUE o N/A
    QString getCurrentAnatomicalPlaneLabel() const;

    /// Obté els índexs dels eixos de coordenades corresponents a la vista 2D indicada
    /// Aquests índexs indiquen com hem d'indexar una coordenada 3D per obtenir
    /// la corresponent projecció sobre la vista 2D sobre la que estem operant
    /// En cas que el valor de view no sigui un dels esperats, el valor de l'índex serà -1
    static void getXYZIndexesForView(int &x, int &y, int &z, CameraOrientationType view);
    static int getXIndexForView(CameraOrientationType view);
    static int getYIndexForView(CameraOrientationType view);
    static int getZIndexForView(CameraOrientationType view);

    /// Retorna l'espai que hi ha entre les llesques segons la vista actual i si hi ha el thickness activat
    double getCurrentSpacingBetweenSlices();

    /// retorna la llesca més propera al punt projectedPosition, i posa a distance la distancia de la llesca al punt
    int getNearestSlice(double projectedPosition[3], double &distance);

    /// Ens retorna l'acttor vtk de la imatge
    vtkImageActor* getVtkImageActor() const;

    /// Returns true if this Q2DViewer can show a display shutter in its current state, i.e. if there is a display shutter for the current image and there isn't
    /// any restriction to show display shutters.
    bool canShowDisplayShutter() const;

public slots:
    virtual void setInput(Volume *volume);

    /// Especifica el volum d'entrada de forma asíncrona.
    /// Es pot indicar un command que s'executarà un cop el volum s'ha carregat i està a punt de ser visualitzat.
    /// Útil per poder especificar canvis al viewer (canvi de llesca, w/l, etc.) sense preocupar-se de quan s'ha carregat el volume.
    void setInputAsynchronously(Volume *volume, QViewerCommand *inputFinishedCommand = 0);

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

    /// Ajusta el window/level a partir del preset.
    /// Si és de tipus FileDefined i en la vista original, per cada imatge aplicarà el seu WW/WL. Altrament aplicar el WW/WL que indica el preset.
    virtual void setWindowLevelPreset(const WindowLevel &preset);

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

    /// Posa la posició d'alineament de la imatge (dreta, esquerre, centrat)
    void setAlignPosition(AlignPosition alignPosition);

    /// Aplica les transformacions 2D necessàries sobre la imatge actual perquè aquesta tingui la orientació indicada
    /// La orientació indicada ha de ser possible de conseguir mitjançant operacions de rotació i flip. En cas que no
    /// existeixin combinacions possibles, no es canviarà la orientació de la imatge
    void setImageOrientation(const PatientOrientation &desiredPatientOrientation);

    /// Fa que els ImageOverlays siguin visibles o no
    void showImageOverlays(bool enable);

    /// Fa que els shutters siguin visibles o no
    void showDisplayShutters(bool enable);

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
    /// Inicialitza el filtre de màscara per als shutters
    void initializeShutterFilter();
    
    /// Updates image orientation according to the preferred presentation depending on its attributes, like modality.
    /// At this moment it is only applying to mammography (MG) images
    void updatePreferredImageOrientation();
    
    /// Translates View Code Meaning from image to the corresponding mammography projection label.
    QString getMammographyProjectionLabel(Image *image);
    
    /// Returns true if we can treat the image as an standard mamography, false otherwise
    bool isStandardMammographyImage(Image *image);
    
    /// Refresca la visibilitat de les annotacions en funció dels flags que tenim
    void refreshAnnotations();

    /// Actualitzem les dades de les annotacions, per defecte totes, sinó, només les especificades
    void updateAnnotationsInformation(AnnotationFlags annotation = Q2DViewer::AllAnnotation);

    /// Desglossem les actualitzacions de les diferents informacions que es mostren per pantalla
    void updatePatientAnnotationInformation();
    void updateSliceAnnotationInformation();
    void updateLateralityAnnotationInformation();
    void updatePatientInformationAnnotation();

    /// Returns the laterality corresponding to the current displayed image.
    /// If image is not reconstructed, image laterality is returned, or series laterality if not present
    /// If image is reconstructed, series laterality is returned
    /// If no laterality is found, en empty character will be returned
    QChar getCurrentDisplayedImageLaterality() const;
    
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

    /// Actualitza les etiquetes d'orientació del pacient segons la vista i orientació actuals de la càmera
    void updatePatientOrientationAnnotation();

    /// Actualitza les característiques dels actors dels viewports
    void updateDisplayExtent();

    /// Construeix el pipeline pel tractament de window/level de la imatge
    void buildWindowLevelPipeline();

    /// Actualitza el pipeline del filtre de shutter segons si està habilitat o no
    void updateShutterPipeline();

    /// Updates the mask used as display shutter if display shutters should and can be shown.
    void updateDisplayShutterMask();

    /// Decides and sets the input image for the image actor, choosing between the display shutter mask output and the window level output.
    void setImageActorInput();

    /// Re-inicia els paràmetres de la càmera segons la vista actual.
    void resetCamera();

    /// Thick slab
    void computeRangeAndSlice(int newSlabThickness);

    ///  Valida el valor d'slice donat i actualitza les variables membres pertinents, com m_currentSlice o m_firstSlabSlice
    void checkAndUpdateSliceValue(int value);

    /// Inicialitza els paràmetres del thick slab i del seu filtre
    void initializeThickSlab();
    
    /// Aplica el factor de rotació adient segons els girs que li indiquem. No actualitza la càmera ni l'escena, simplement
    /// es fa servir per posar els valors correctes a les variables internes que controlen com està girada la imatge.
    void rotate(int times);

    /// S'encarrega d'encaixar la imatge actual a la mida del viewport
    void fitImageIntoViewport();

    /// Carrega un volum asíncronament
    void loadVolumeAsynchronously(Volume *volume);

    /// Retorna un volum "dummy"
    Volume* getDummyVolumeFromVolume(Volume *volume);

    /// Especifica quin command s'ha d'executar després d'especificar un volum com a input
    void setInputFinishedCommand(QViewerCommand *command);

    /// Elimina el command que s'hauria d'executar després de que s'especifiqui un input
    void deleteInputFinishedCommand();

    /// Si està definit, executa el command definit per després d'especificar un input al viewer
    void executeInputFinishedCommand();

    /// Cancela el job actual de carregar un volum en el cas que existeixi
    /// TODO: De moment només desconnecta el job del viewer, no cancel·la el job en sí
    void cancelCurrentVolumeReaderJob();

    /// En cas que estem en la vista original en la que s'han adquirit les imatges i tinguem un preset
    /// per defecte activat, actualitzarem el WW/WL perquè per cada imatge es mostri el seu.
    void updateDefaultPreset();

    /// Indica si el volum del visor s'està carregant assíncronament
    bool isVolumeLoadingAsynchronously();

    /// Crida els mètodes setNewVolume i executa el command. A més a més es fa un bloc try/catch per capturar
    /// qualsevol excepció possible durant el procés de rendering per evitar que peti
    void setNewVolumeAndExecuteCommand(Volume *volume);

    /// Elimina els bitmaps que s'hagin creat per aquest viewer
    void removeViewerBitmaps();
    
    /// Carrega en memòria els ImageOverlays del volum passat per paràmetre (sempre que no sigui un dummy) i els afegeix al Drawer
    void loadOverlays(Volume *volume);

private slots:
    /// Actualitza les transformacions de càmera (de moment rotació i flip)
    void updateCamera();

    /// Reimplementem per tal de que faci un setInputAsynchronously
    /// TODO: De moment es fa així de manera xapussa fins que no es traspassin els mètode de càrrega
    /// asíncrona a QViewer.
    virtual void setInputAndRender(Volume *volume);

    /// Canvia el volum que s'està visualitzant per un de nou que es passa per paràmetre.
    /// Sempre es canviarà, independentment de si el volume passat és el mateix o no al que ja hi havia.
    /// Es pressuposa que el volume està carregat en memòria.
    /// Ens permet escollir si al canviar el volum volem que es mostri o no canviant l'status del viewer.
    void setNewVolume(Volume *volume, bool setViewerStatusToVisualizingVolume = true);

    void volumeReaderJobFinished();

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
    /// i altre informació rellevant (nº imatge, ww/wl, etc)
    vtkCornerAnnotation *m_cornerAnnotations;

private:
    /// Nom del grups dins del drawer per als Overlays
    static const QString OverlaysDrawerGroup;

    /// Constant per a definir el nom d'objecte dels volums "dummy"
    static const QString DummyVolumeObjectName;
    
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

    /// Posició a on s'ha d'alinear la imatge (dreta, esquerre o centrat)
    AlignPosition m_alignPosition;

    /// Llista d'excepcions per les quals no s'ha d'aplicar una orientació concreta per estudis de mamografia.
    QStringList m_mammographyAutoOrientationExceptions;

    /// El job no ens pertany, per això ho guardem en un QPointer
    QPointer<VolumeReaderJob> m_volumeReaderJob;

    QViewerCommand *m_inputFinishedCommand;

    /// Indica el preset per defecte que s'ha d'aplicar.
    int m_defaultPresetToApply;

    /// Llistat d'overlays
    QList<DrawerBitmap*> m_viewerBitmaps;

    /// Controla si els overlays estan habilitats o no
    bool m_overlaysAreEnabled;
    
    /// If true, display shutters are visible when they are available and it's possible to show them.
    bool m_showDisplayShutters;

    /// Filtre de màscara per aplicar els shutters
    vtkImageMask *m_shutterMaskFilter;

};
Q_DECLARE_OPERATORS_FOR_FLAGS(Q2DViewer::AnnotationFlags)
};  //  End namespace udg

#endif
