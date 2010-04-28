/***************************************************************************
 *   Copyright (C) 2005-2007 by Grup de Gràfics de Girona                  *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#ifndef UDGTRANSLATETOOL_H
#define UDGTRANSLATETOOL_H

#include "tool.h"

namespace udg {

/**
    Eina per moure la posició de la càmera en escena d'un viewer

    @author Grup de Gràfics de Girona  ( GGG ) <vismed@ima.udg.es>
*/
class TranslateTool : public Tool {
Q_OBJECT
public:
    enum { None , Translating };
    TranslateTool( QViewer *viewer, QObject *parent = 0 );
    ~TranslateTool();

    void handleEvent( unsigned long eventID );

private:
    /// Realitza la feina de desplaçament
    void pan();

private slots:
    /// Comença el translate
    void startTranslate();

    /// Calcula el nou translate
    void doTranslate();

    /// Atura l'estat de translate
    void endTranslate();

private:
    /// Estat de la tool
    int m_state;
};

}

#endif
