/***************************************************************************
 *   Copyright (C) 2010 by Grup de Gr�fics de Girona                       *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#ifndef COPYDIRECTORY_H
#define COPYDIRECTORY_H

#include <QString>

namespace udg {

/** Classe que ens permet copiar el contingut d'un directori

	@author Grup de Gr�fics de Girona  ( GGG ) <vismed@ima.udg.es>
*/
class CopyDirectory{
public:

    /**Copia el directori origen al directori dest�. Si la c�pia falla al copiar algun fitxer o directori esborra del directori dest�
       tot el que s'havia copiat fins el moment*/
    static bool copyDirectory(const QString &sourceDirectory, const QString &sourceDestination);

private:
    
    ///M�tode recursiu per copiar directoris
    static bool copyDirectoryRecursive(const QString &sourceDirectory, const QString &sourceDestination);

};

};  //  end  namespace udg

#endif
