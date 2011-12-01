#ifndef UDGQVIEWER_H
#define UDGQVIEWER_H

#include <QWidget>
// Llista de captures de pantalla
#include <QList>
#include <vtkImageData.h>

// Fordward declarations
class QStackedLayout;
class QVTKWidget;
class vtkCamera;
class vtkRenderer;
class vtkRenderWindow;
class vtkRenderWindowInteractor;
class vtkWindowToImageFilter;
class vtkEventQtSlotConnect;

namespace udg {

// Fordward declarations
class Volume;
class Series;
class ToolProxy;
class WindowLevelPresetsToolData;
class TransferFunction;
class PatientBrowserMenu;
class QViewerWorkInProgressWidget;
class AutomaticSynchronizationWidget;

/**
    Classe base per a totes les finestres de visualització
  */
class QViewer : public QWidget {
Q_OBJECT
public:
    QViewer(QWidget *parent = 0);
    ~QViewer();

    /// Per definir els diferents plans on podem operar
    enum PlaneType { AxialPlane = 2, SagitalPlane = 0, CoronalPlane = 1 };

    /// Definim les diverses orientacions que podem tenir: Axial: XY, Sagital: YZ, Coronal: XZ, Other: orientació arbitrària
    enum CameraOrientationType { Axial = AxialPlane, Sagital = SagitalPlane, Coronal = CoronalPlane, Other };

    /// Tipus de fitxer que pot desar
    enum FileType { PNG, JPEG, TIFF, DICOM, PNM, META, BMP };

    /// Tipus de format de gravació de vídeo suportats
    enum RecordFileFormatType { MPEG2 };

    /// Estat del viewer
    enum ViewerStatus { NoVolumeInput, DownloadingVolume, DownloadingError, LoadingVolume, LoadingError, VisualizingVolume, SynchronizationEdit };

    /// Retorna l'interactor renderer
    virtual vtkRenderWindowInteractor* getInteractor();

    /// Retorna el renderer
    virtual vtkRenderer* getRenderer();

    /// Ens retorna el volum d'entrada
    virtual Volume* getInput()
    {
        return m_mainVolume;
    }

    /// Retorna el vtkRenderWindow
    vtkRenderWindow* getRenderWindow();

    /// Retorna la mida en coordenades de pantalla (píxels?) de tot el viewport del viewer
    /// amaga la crida vtkRenderWindow::GetSize()
    int* getRenderWindowSize();

    /// Mètodes per obtenir l'actual i/o l'anterior coordenada del cursor
    /// del ratolí durant l'últim event
    /// Les coordenades retornades estan en sistema de coordenades de Display
    int* getEventPosition();
    int* getLastEventPosition();
    void getEventPosition(int position[2]);
    void getLastEventPosition(int position[2]);
    int getEventPositionX();
    int getEventPositionY();
    int getLastEventPositionX();
    int getLastEventPositionY();

    /// Ens diu si el viewer és actiu en aquell moment
    /// @return Cert si actiu, fals altrament
    bool isActive() const;

    /// Ens retorna el ToolProxy del viewer
    ToolProxy* getToolProxy() const;

    /// Passa coordenades de display a coordenades de món i viceversa
    void computeDisplayToWorld(double x, double y, double z, double worldPoint[4]);
    void computeWorldToDisplay(double x, double y, double z, double displayPoint[3]);

    /// Ens dóna la coordenada de món de l'últim (o previ a aquest) event capturat
    void getEventWorldCoordinate(double worldCoordinate[3]);
    void getLastEventWorldCoordinate(double worldCoordinate[3]);

    /// Fa una captura de la vista actual i la guarda en una estructura interna
    void grabCurrentView();

    /// Desa la llista de captures en un arxiu de diversos tipus amb el nom de fitxer base \c baseName i en format especificat per \c extension.
    /// Retorna TRUE si hi havia imatges per guardar, FALSE altrament
    bool saveGrabbedViews(const QString &baseName, FileType extension);

    /// Retorna el nombre de vistes capturades que estan desades
    int grabbedViewsCount()
    {
        return m_grabList.size();
    }

    /// Grava en format de vídeo els frames que s'hagin capturat amb grabCurrentView.
    /// Un cop gravat, esborra la llista de frames.
    /// TODO de moment només accepta format MPEG
    bool record(const QString &baseName, RecordFileFormatType format = MPEG2);

    /// Fa zoom sobre l'escena amb el factor donat
    /// @param factor Factor de zoom que volem aplicar a la càmera
    void zoom(double factor);

    /// Desplaça la càmera segons el vector de moviment que li passem
    /// @param motionVector[] Vector de moviment que determina cap on i quant es mou la càmera
    void pan(double motionVector[3]);

    /// Fem un zoom del requadre definit pels paràmetres topCorner i bottomCorner en coordenades de món
    /// perquè s'ajusti a la mida de la finestra.
    /// La determinació sobre quina coordenada es considera "superior" o "inferior" és relativa segons el punt de vista,
    /// el realment important és que aquestes coordenades siguin dos vèrtexs oposats dins del requadre que es vol definir
    /// @param topCorner Coordenada superior
    /// @param bottomCorner Coordenada inferior oposada a la superior
    /// @param marginRate valor en percentatge del marge que es vol deixar
    void scaleToFit3D(double topCorner[3], double bottomCorner[3], double marginRate = 0.0);

    /// Ens retorna l'objecte que conté tota la informació referent al window level
    /// que es pot aplicar sobre aquest visor
    /// @return L'objecte WindowLevelPresetsToolData
    WindowLevelPresetsToolData* getWindowLevelData() const;

    /// Li assignem el window level data externament
    /// @param windowLevelData
    void setWindowLevelData(WindowLevelPresetsToolData *windowLevelData);

    /// Habilita/deshabilita que els renderings es facin efectius
    /// Útil en els casos en que necessitem fer diverses operacions de
    /// visualització però no volem que aquestes es facin efectives fins que no ho indiquem
    void enableRendering(bool enable);

    /// Ens retorna el menú de pacient amb el que s'escull l'input
    PatientBrowserMenu* getPatientBrowserMenu() const;

    /// Permet especificar si al seleccionar un input del menú de pacient aquest es carregarà automàticament
    /// o, pel contrari, no es farà res. Per defecte està activat i es carregarà l'input al visor.
    void setAutomaticallyLoadPatientBrowserMenuSelectedInput(bool load);

    /// Retorna l'status del viewer. Útil per saber si el visor està visualitzant dades
    /// o està carregant...
    ViewerStatus getViewerStatus() const;
    ViewerStatus getPreviousViewerStatus() const;

    /// Canvia l'status del viewer
    void setViewerStatus(ViewerStatus status);

    /// Obté el widget d'interfície de la sincronització automàtica
    AutomaticSynchronizationWidget * getAutomaticSynchronizationWidget();

public slots:
    /// Indiquem les dades d'entrada
    virtual void setInput(Volume *volume) = 0;

    /// Gestiona els events que rep de la finestra
    void eventHandler(vtkObject *object, unsigned long vtkEvent, void *clientData, void *callData, vtkCommand *command);

    /// Força l'execució de la visualització
    void render();

    /// Assignem si aquest visualitzador és actiu, és a dir, amb el que s'està interactuant
    /// @param active
    void setActive(bool active);

    /// Elimina totes les captures de pantalla
    void clearGrabbedViews();

    /// Obté el window/level per defecte. Si no se n'especifica cap amb setDefaultWindowLevel
    /// agafa un window/level que permeti veure correctament el Volum.
    void getDefaultWindowLevel(double windowLevel[2]);

    /// Obté el window level actual de la imatge
    virtual void getCurrentWindowLevel(double wl[2]) = 0;

    /// Li indiquem quina vista volem del volum: Axial, Coronal o Sagital
    virtual void resetView(CameraOrientationType view) = 0;
    virtual void resetViewToAxial() = 0;
    virtual void resetViewToSagital() = 0;
    virtual void resetViewToCoronal() = 0;

    /// Activa o desactiva el menú de contexte
    void enableContextMenu();
    void disableContextMenu();

    /// Ajusta ÚNICAMENT els valors de window i level per defecte. No els aplica sobre la imatge.
    /// Mètode de conveniència pels presentation states
    void setDefaultWindowLevel(double window, double level);

    /// Ajusta el window/level
    virtual void setWindowLevel(double window, double level) = 0;
    /// Ajusta el window/level a partir del preset. La implementació per defecte acaba cridant setWindowLevel sempre.
    virtual void setWindowLevelPreset(const QString &preset);

    /// Assigna/Obté la funció de transferència actual
    virtual void setTransferFunction(TransferFunction *transferFunction) = 0;
    TransferFunction* getTransferFunction() const;

    /// Reseteja el window level al que tingui per defecte el volum
    virtual void resetWindowLevelToDefault();

signals:
    /// Informem de l'event rebut. \TODO ara enviem el codi en vtkCommand, però podria (o hauria de) canviar per un mapeig nostre
    void eventReceived(unsigned long eventID);

    /// Signal que s'emet quan s'escull una altra serie per l'input
    void volumeChanged(Volume *volume);

    /// S'emet quan els paràmetres de la càmera han canviat
    void cameraChanged();

    /// Informa que el visualitzador ha rebut un event que es considera com que aquest s'ha seleccionat
    void selected(void);

    /// Informa que s'ha canviat el zoom
    void zoomFactorChanged(double factor);

    /// Informa que s'ha mogut la imatge
    void panChanged(double *translation);

    /// Indica que l'estat del visor ha canviat
    void viewerStatusChanged();

protected:
    /// Definició de les diferents vistes amb les que pot operar la càmera
    enum CameraViewPlaneType { XYViewPlane = 2, YZViewPlane = 0, XZViewPlane = 1 };

    virtual void contextMenuEvent(QContextMenuEvent *menuEvent);

    void contextMenuRelease();

    /// Actualitza les dades contingudes a m_windowLevelData
    void updateWindowLevelData();

    /// Fixem la orientació de la càmera del renderer principal
    /// Si el paràmetre donat no és un dels valors enumerats vàlids, no farà res
    /// @param orientation Orientació, valors enumerats que podran ser Axial, Sagital o Coronal
    void setCameraOrientation(int orientation);

    /// Configura la orientació de la càmera donada
    void setCameraViewPlane(CameraViewPlaneType viewPlane);

    /// Ens retorna la càmera activa pel renderer principal, si n'hi ha, NUL altrament.
    vtkCamera* getActiveCamera();

    /// Ens dóna la coordenada de món de l'últim (o previ a aquest) event capturat
    /// @param worldCoordinate Variable on es retornarà la coordenada
    /// @param current Si true, ens dóna la coordenada de l'event més recent,
    /// si fals, ens dóna la coordenada anterior a l'event més recent
    void getRecentEventWorldCoordinate(double worldCoordinate[3], bool current);

    virtual void setupInteraction();

    /// Calcula un window level automàtic segons l'input actual.
    /// Si no tenim input el resultat és indefinit
    void computeAutomaticWindowLevel(double &windowWidth, double &windowLevel);

private slots:
    /// Slot que s'utilitza quan s'ha seleccionat una sèrie amb el PatientBrowserMenu
    /// Mètode que especifica un input seguit d'una crida al mètode render()
    /// TODO: Convertit en virtual per tal de poder ser reimplementat per Q2DViewer per càrrega asíncrona
    virtual void setInputAndRender(Volume *volume);

private:
    /// Actualitza quin és el widget actual que es mostra per pantalla a partir de l'estat del viewer
    void setCurrentWidgetByViewerStatus(ViewerStatus status);

    /// Inicialitza el widget QWorkInProgress a partir de l'status
    void initializeWorkInProgressByViewerStatus(ViewerStatus status);

protected:
    /// El volum a visualitzar
    Volume *m_mainVolume;

    /// El widget per poder mostrar una finestra vtk amb qt
    QVTKWidget *m_vtkWidget;

    /// La llista de captures de pantalla
    QList<vtkImageData*> m_grabList;

    /// Filtre per connectar el que es visualitza pel renderer en un pipeline, epr guardar les imatges en un arxiu, per exemple
    vtkWindowToImageFilter *m_windowToImageFilter;

    /// Connector d'events vtk i slots qt
    vtkEventQtSlotConnect *m_vtkQtConnections;

    /// Renderer per defecte
    vtkRenderer *m_renderer;

    /// Indica si el menú de contexte està actiu o no. Per defecte sempre està actiu
    bool m_contextMenuActive;

    /// ToolProxy
    ToolProxy *m_toolProxy;

    /// Ens servirà per controlar si entre event o event s'ha mogut el mouse
    bool m_mouseHasMoved;

    /// Dades de valors predeterminats de window level i dels valors actuals que s'apliquen
    WindowLevelPresetsToolData *m_windowLevelData;

    /// Valors dels window level per defecte. Pot venir donat pel DICOM o assignat per nosaltres a un valor estàndar de constrast
    double m_defaultWindow, m_defaultLevel;

    /// Funció de transferència
    TransferFunction *m_transferFunction;

    /// Indica si hem de fer l'acció de renderitzar o no
    bool m_isRenderingEnabled;

    /// Menú de pacient a través del qual podem escollir l'input del viewer
    PatientBrowserMenu *m_patientBrowserMenu;

    /// Widget que es mostra quan s'està realitzant algun treball asíncron
    QViewerWorkInProgressWidget *m_workInProgressWidget;

    /// Widget que es mostra quan s'activa l'edició de la sincronització automàtica
    AutomaticSynchronizationWidget *m_automaticSynchronizationWidget;
    
    /// Vista que enfoca la càmera
    CameraViewPlaneType m_currentViewPlane;

private:
    /// Indica si el viewer és actiu o no
    bool m_isActive;
    /// Indica si s'ha definit o no un window level per defecte
    bool m_hasDefaultWindowLevelDefined;

    /// Estats actual i previ del visor actual
    ViewerStatus m_viewerStatus;
    ViewerStatus m_previousViewerStatus;

    /// Layout que ens permet crear widgets diferents per els estats diferents del visor.
    QStackedLayout *m_stackedLayout;
};

};  // End namespace udg

#endif
