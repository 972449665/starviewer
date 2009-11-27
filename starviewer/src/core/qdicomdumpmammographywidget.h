/***************************************************************************
 *   Copyright (C) 2005-2007 by Grup de Gr�fics de Girona                  *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#ifndef UDGQDICOMDUMPMAMMOGRAPHYWIDGET_H
#define UDGQDICOMDUMPMAMMOGRAPHYWIDGET_H

#include <ui_qdicomdumpmammographywidgetbase.h>

namespace udg {

class Image;

/** 
 *  Classe encaragada de fer el dicom dump per a imatges de mammografia (MG)
 *  @author Grup de Gr�fics de Girona  ( GGG ) <vismed@ima.udg.es>
 */
class QDicomDumpMammographyWidget : public QWidget , private Ui::QDicomDumpMammographyWidgetBase{
Q_OBJECT
public:
    QDicomDumpMammographyWidget( QWidget *parent = 0 );
    ~QDicomDumpMammographyWidget();

    /** 
     * Li assigna la imatge a partir de la qual s'ha d'obtenir la informaci�
     * @param image Imatge de la que s'ha de fer el dicomdump
     */
    void setCurrentDisplayedImage( Image *image );

private:
    /// Inicalitza els continguts de les etiquetes per quan no disposem encar d'informaci�
    void initializeLabels();

    /** 
     * Obt� la informaci� DICOM i la volca a les corresponents etiquetes
     * @param image Objecte del que obtenim la informaci�
     */
    void dumpDICOMInformation( Image *image );
};

}

#endif
