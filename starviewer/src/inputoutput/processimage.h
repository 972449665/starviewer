/***************************************************************************
 *   Copyright (C) 2005 by Grup de Gràfics de Girona                       *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#ifndef UDGPROCESSIMAGE_H
#define UDGPROCESSIMAGE_H

namespace udg {

class DICOMImage;
class DICOMTagReader;

/** Classe que s'encarrega de dur a terme unes accions determinades per cada descarrega d'una imatge. Aquesta classe és una classe genèrica, per ser reimplementada
@author marc
*/
class ProcessImage
{

public:

	///Constructor de la classe
    ProcessImage();

	/** Processa la imatge
 	 * @param imatge a processar
 	 */
    virtual void process(DICOMImage* image) = 0;

    ///Processa el DICOMTagReader
    virtual void process(DICOMTagReader *dicomTagReader) = 0;

	///Indica que s'ha produit algun error descarregant alguna de les imatges de l'estudi
    virtual void setError() = 0;

	///Indica si s'ha produit algun error descarregant alguna de les imatges de l'estudi
    virtual bool getError() = 0;

	///Destructor de la classe
    virtual ~ProcessImage() {};

};

};

#endif
