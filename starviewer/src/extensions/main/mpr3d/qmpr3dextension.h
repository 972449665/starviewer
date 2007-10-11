/***************************************************************************
 *   Copyright (C) 2005 by Grup de Gràfics de Girona                       *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#ifndef UDGQMPR3DEXTENSION_H
#define UDGQMPR3DEXTENSION_H

#include "ui_qmpr3dextensionbase.h"

namespace udg {

// FWD declarations
class Volume;
class ToolsActionFactory;

/**
Extensió de l'MPR 3D

@author Grup de Gràfics de Girona  ( GGG )
*/
class QMPR3DExtension : public QWidget , private ::Ui::QMPR3DExtensionBase{
Q_OBJECT
public:
    QMPR3DExtension( QWidget *parent = 0 );

    ~QMPR3DExtension();

    /**
        Li assigna el volum amb el que s'aplica l'MPR.
    */
    void setInput( Volume *input );

private:
    /// El volum d'entrada
    Volume* m_volume;

    /// factoria d'accions per les tools
    ToolsActionFactory *m_actionFactory;

    /// Accions de les tools
    QAction *m_zoomAction;
    QAction *m_moveAction;
    QAction *m_rotate3DAction;
    QAction *m_screenShotAction;
    QActionGroup *m_toolsActionGroup;

    /// posa a disposició les tools
    void createTools();

    /// Estableix les connexions de signals i slots
    void createConnections();

};

};  //  end  namespace udg

#endif
