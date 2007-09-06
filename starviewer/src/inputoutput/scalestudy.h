/***************************************************************************
 *   Copyright (C) 2005 by Grup de Gràfics de Girona                       *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#ifndef UDGSCALESTUDY_H
#define UDGSCALESTUDY_H

class QString;

namespace udg {

class SeriesList;
class Status;
class DicomMask;
class DICOMSeries;

/** Escala les imatges del mig de cada sèrie d'un estudi, per poder ser previsualitzades quant es consulta la caché
@author Grup de Gràfics de Girona  ( GGG )

*/
class ScaleStudy
{

public:

    ///Constructor de la classe
    ScaleStudy();

    ///Destructor de la classe
    ~ScaleStudy();

    /** Escala una imatge de cada sèrie per poder fer la previsualització a la caché local del studyUID
     * @param Uid de l'estudi a escakar
     */
    void scale( QString studyUID );

    static QString getScaledImagePath(DICOMSeries* series);

private:
    /** Cerca les series de l'estudi
     * @param El UID de l'estudi a cercar
     * @param [out] retorna les sèries de l'estudi
     * @return retorna l'estat del mètode
     */
    Status getSeriesOfStudy( QString studyUID , SeriesList &seriesList );

    /** Compta el número d'imatges de l'estudi
     * @param màscara de les imatges a comptar
     * @param número d'imatges
     * @return retorna l'estat del mètode
     */
    Status countImageNumber( DicomMask , int &number );

    /** busca el path retatiu de la imatge que se li passa per paràmetre
     * @param màscara de la imatge a buscar el path relatiu
     * @param path relatiu, respecte el directori de l'estudi
     * @return retorna l'estat del mètode
     */
    Status imageRelativePath( DicomMask , QString &relPath );
};

};

#endif
