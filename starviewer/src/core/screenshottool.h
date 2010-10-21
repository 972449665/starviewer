/***************************************************************************
 *   Copyright (C) 2005-2006 by Grup de Gràfics de Girona                  *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#ifndef UDGSCREENSHOTTOOL_H
#define UDGSCREENSHOTTOOL_H

#include "tool.h"

#include <QStringList>

class QString;

namespace udg {

class QViewer;

/**
    Fa captures de pantalla dels visors. 
    Ctrl+S fa la captura simple del que s'està veient en aquell moment
    Ctrl+A fa totes les captures possibles. En el cas que el viewer sigui el 2D, fara les captures de totes
    les imatges de la sèrie, altrament farà una captura simple

    @author Grup de Gràfics de Girona  (GGG) <vismed@ima.udg.es>
*/
class ScreenShotTool: public Tool {
Q_OBJECT
public:
    ScreenShotTool(QViewer *viewer, QObject *parent = 0);
    ~ScreenShotTool();

    void handleEvent(unsigned long eventID);

public slots:
    /// Només fa captura de la imatge actual
    void singleCapture();

    /// Fa captura de totes les imatges que pot mostrar el visualitzador
    void completeCapture();

private slots:
    /// Fa la captura d'imatge del viewer
    /// Si single és true fa la captura del que s'està veient, sinó ho fa per totes en cas que sigui un 2DViewer
    void screenShot(bool singleShot = true);
    
    /// Llegeix les configuracions que han estat guardades
    void readSettings();

    /// Guarda les configuracions
    void writeSettings();
    
    /// Composa un nom de fitxer per defecte
    QString compoundSelectedName();
    
private:
    /// Constants
    static const QString PngFileFilter;
    static const QString JpegFileFilter;
    static const QString BmpFileFilter;
    static const QString TiffFileFilter;

    /// Path de l'última imatge guardada
    QString m_lastScreenShotPath;
    
    /// Filtre de l'extensió d'arxiu de l'última imatge guardada
    QString m_lastScreenShotExtensionFilter;
    
    /// Nom de l'última imatge guardada
    QString m_lastScreenShotFileName;
    
    /// Llista amb els filtres d'extensió de fitxers que podem escollir
    QString m_fileExtensionFilters;
};

}

#endif
