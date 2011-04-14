#ifndef UDGQDICOMDUMPCTHELIXWIDGET_H
#define UDGQDICOMDUMPCTHELIXWIDGET_H

#include <ui_qdicomdumpcthelixwidgetbase.h>

namespace udg {

class Series;
class Image;

/** Classe encaragada de fer el dicom dump per a imatges CT helicoïdals
*/
class QDicomDumpCTHelixWidget : public QWidget , private Ui::QDicomDumpCTHelixWidgetBase{
Q_OBJECT
public:
    QDicomDumpCTHelixWidget( QWidget *parent = 0 );

    ~QDicomDumpCTHelixWidget();

    /** Fa el DicomDump dels tags dicom de la imatge 
     * @param image imatge de la que s'ha de fer el dicomdump
     */
    void setCurrentDisplayedImage( Image *image );

private:

    ///Inicalitza tots els labelsValue a "-"
    void initialize();

    /** Estableix el valors dels tags de la imatge
     * @param imageInformation imatge
     */
    void setImageDicomTagsValue( Image *imageInformation );
    
    /** Estableix els valors dels tags de la seire
     * @param seriesInformation  serie
     */
    void setSeriesDicomTagsValue( Series *seriesInformation );
};

}

#endif
