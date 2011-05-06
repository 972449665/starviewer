#ifndef UDGFILTER_H
#define UDGFILTER_H

#include <QObject>

namespace udg {

class Volume;

/*
    Classe base per a tots els filtres
*/

class Filter {
public:
    Filter();

    ~Filter();

    /* Tal i com fan els filtres de vtk i itk li assignem les dades que en aquest cas ser� un volum
    Aquest m�tode, segons quin tipus de filtre sigui extreur� les dades itk o vtk */
    void setInput(Volume *input);

    /* M�tode per realitzar els c�lculs amb la conseg�ent transformaci� del volum */
    void update();

    /* Metode per assignar un identificador */
    void setFilterName(QString name);

    QString getFilterName();

    /* Metode que retorna el volum transformat */
    Volume * getOutput();

protected:

    /* Volum d'entrada */
    Volume * m_inputVolume;

    /* Volum de sortida */
    Volume * m_outputVolume;

    /* Nom del filtre */
    QString m_filterName;

};

}

#endif
