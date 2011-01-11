/***************************************************************************
 *   Copyright (C) 2005-2007 by Grup de Gràfics de Girona                  *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#include "hangingprotocolsloader.h"

#include "logging.h"
#include "hangingprotocol.h"
#include "hangingprotocollayout.h"
#include "hangingprotocolsrepository.h"
#include "hangingprotocolimageset.h"
#include "hangingprotocoldisplayset.h"
#include "hangingprotocolxmlreader.h"
#include "identifier.h"
#include "starviewerapplication.h"
#include "logging.h"
#include "coresettings.h"
// Qt's
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QApplication>
#include <QDir>

namespace udg {

HangingProtocolsLoader::HangingProtocolsLoader(QObject *parent)
 : QObject(parent)
{

}

HangingProtocolsLoader::~HangingProtocolsLoader()
{

}

void HangingProtocolsLoader::loadDefaults()
{
    /// Hanging protocols definits per defecte, agafa el directori de l'executable TODO això podria ser un setting més
    QString defaultPath = "/etc/xdg/" + OrganizationNameString + "/" + ApplicationNameString + "/hangingProtocols/"; // Path linux
    if (!QFile::exists(defaultPath))
    {
        defaultPath = qApp->applicationDirPath() + "/hangingProtocols/";
    }
    if (!QFile::exists(defaultPath))
    {
        // En entorn de desenvolupament Windows & Linux
        defaultPath = qApp->applicationDirPath() + "/../hangingprotocols/";
    }
    if (!QFile::exists(defaultPath))
    {
        // En entorn de desenvolupament Mac OS X
        defaultPath = qApp->applicationDirPath() + "/../../../../hangingprotocols/";
    }

    if (QFile::exists(defaultPath))
    {
        INFO_LOG(QString("Directori a on es van a buscar els hanging protocols per defecte: %1").arg(defaultPath));
        loadXMLFiles(defaultPath);
    }
    else
    {
        INFO_LOG(QString("El directori per defecte dels hanging protocols (%1) no existeix. No es carregaran.").arg(defaultPath));
    }

    /// Hanging protocols definits per l'usuari
    Settings systemSettings;
    QString userPath = systemSettings.getValue(CoreSettings::UserHangingProtocolsPath).toString();
    if (!userPath.isEmpty())
    {
        loadXMLFiles(userPath);
    }
}

bool HangingProtocolsLoader::loadXMLFiles(const QString &filePath)
{
	HangingProtocolXMLReader *xmlReader = new HangingProtocolXMLReader();
    QList<HangingProtocol *> listHangingProtocols = xmlReader->read(filePath);

    if (listHangingProtocols.size() > 0)
    {
        INFO_LOG(QString("Carreguem %1 hanging protocols de [%2].").arg(listHangingProtocols.size()).arg(filePath));
        QString hangingProtocolNamesLogList;
        foreach (HangingProtocol *hangingProtocol, listHangingProtocols)
        {
            Identifier id = HangingProtocolsRepository::getRepository()->addItem(hangingProtocol);
            hangingProtocol->setIdentifier(id.getValue());
            hangingProtocolNamesLogList.append(QString("%1, ").arg(hangingProtocol->getName()));
        }
        INFO_LOG(QString("Hanging protocols carregats: %1").arg(hangingProtocolNamesLogList));
    }

    delete xmlReader;
    return true;
}

}
