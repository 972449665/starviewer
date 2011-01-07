/***************************************************************************
 *   Copyright (C) 2005-2007 by Grup de Gràfics de Girona                  *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#ifndef UDGHANGINGPROTOCOLSLOADER_H
#define UDGHANGINGPROTOCOLSLOADER_H

#include <QObject>

namespace udg {

/**

*/
class HangingProtocolsLoader: public QObject
{
Q_OBJECT
public:
    HangingProtocolsLoader(QObject *parent = 0);

    ~HangingProtocolsLoader();

    /// Càrrega de hanging protocols per defecte
    void loadDefaults();

    /// Càrrega des d'un directori de hanging protocols o un fitxer XML
    bool loadXMLFiles(const QString &filePath);
};

}

#endif
