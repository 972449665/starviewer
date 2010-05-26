/***************************************************************************
 *   Copyright (C) 2005-2006 by Grup de Gràfics de Girona                  *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#ifndef UDGQ2DVIEWERWIDGET_H
#define UDGQ2DVIEWERWIDGET_H

#include "ui_q2dviewerwidgetbase.h"

// FWD declarations
class QAction;

namespace udg {

class Volume;
class StatsWatcher;

/**
Aquesta classe és la interfície genèrica d'un Q2DViewer juntament amb el seu slider i spinbox corresponent. Conté el mètode per posar un input així com els mètodes per modificar el valor de l'slider i l'spinbox.

	@author Grup de Gràfics de Girona  ( GGG ) <vismed@ima.udg.es>
*/
class Q2DViewerWidget : public QFrame, private Ui::Q2DViewerWidgetBase {
Q_OBJECT
public:
    Q2DViewerWidget(QWidget *parent = 0);

    ~Q2DViewerWidget();

    /// Marca el widget com a seleccionat
    void setSelected( bool option );

    /// Retorna el visualitzador
    Q2DViewer *getViewer();

    /// Retorna cert si el model té fases, fals altrament.
    bool hasPhases();

    /// Acció pel botó de sincronitzacio
    void setDefaultAction( QAction *synchronizeAction );

    void setInput( Volume *input );

    /// Activa/Desactiva l'estat de descarrega.
    void enableDownloadingState();
    void disableDownloadingState();

public slots:
    /// Habilita o deshabilita l'eina de sincronització en el visor, si aquest la té registrada
    /// Aquest mètode es podrà invocar al clicar sobré el botó de sincronització o bé cridant-lo directament
    void enableSynchronization(bool enable);

signals:
    /// Aquest senyal s'emetrà quan el mouse entri al widget
    void selected(Q2DViewerWidget * viewer);

protected:
    /// Sobrecàrrega de l'event que s'emet quan el mouse fa un clic dins l'àmbit del widget
    void mousePressEvent ( QMouseEvent * event );

private:
    /// Crea les connexions entre signals i slots
    void createConnections();

    /// Crea el widget que es mostra quan s'ha activat l'estat de descarrega
    void createDownloadingWidget();

private slots:
    /// Aquest slot es cridarà quan es faci alguna acció sobre l'slider
    /// i segons l'acció rebuda actualitzarà el valor de la llesca al visor
    /// Així doncs, quan l'usuari mogui l'slider, ja sigui amb la rodeta del
    /// mouse o pitjant a sobre del widget, li donarem el valor correcte al viewer
    void updateViewerSliceAccordingToSliderAction( int action );

    void updateInput(Volume *input);

    /// Quan el visualitzador s'ha seleccionat, emet el senyal amb aquest widget
    void emitSelectedViewer();

    /// actualitza correctament el valor de l'slider quan hi ha thick slab
    void updateSlider();

    /// Assigna el rang i el valor corresponent de l'slider segons l'input del viewer
    void resetSliderRangeAndValue();
    
    /// actualitza el valor de l'etiqueta que ens indica quina 
    /// projecció del pla estem veient
	void updateProjectionLabel();

private:
    /// El volum principal
    Volume *m_mainVolume;

    /// Acció del boto de sincronitzar
    QAction * m_synchronizeButtonAction;

    /// Per fer estadístiques
    StatsWatcher *m_statsWatcher;

    /// Widget utilitzat per mostrar que s'està descarregant la info que s'ha de mostrar
    QWidget * m_downloadingWidget;
};

};

#endif
