/***************************************************************************
 *   Copyright (C) 2005 by Grup de Gràfics de Girona                       *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/

#include "pacslistdb.h"

#include <QString>
#include <QSettings>

#include "pacsparameters.h"
#include "logging.h"

namespace udg {

const QString PacsListDB::PacsListConfigurationSectionName = "PacsList";

PacsListDB::PacsListDB()
{
}

PacsListDB::~PacsListDB()
{
}

bool PacsListDB::insertPacs(const PacsParameters &pacs)
{
    if(!this->existPacs(pacs))
    {
        QList<PacsParameters> pacsList = getConfiguredPacsList();
        pacsList.append(pacs);

        this->saveConfiguredPacsListToDisk(pacsList);

        return true;
    }
    return false;
}

void PacsListDB::updatePacs(const PacsParameters &pacsToUpdate)
{
    QList<PacsParameters> pacsList = getConfiguredPacsList();

    pacsList[pacsToUpdate.getPacsID().toInt()] = pacsToUpdate;

    this->saveConfiguredPacsListToDisk(pacsList);
}

QList<PacsParameters> PacsListDB::queryPacsList()
{
    return getConfiguredPacsList();
}

PacsParameters PacsListDB::queryPacs(QString pacsIDString)
{
    QList<PacsParameters> pacsList = getConfiguredPacsList();
    bool ok = false;
    int pacsID = pacsIDString.toInt(&ok);
    PacsParameters pacs;

    if (ok)
    {
        if (pacsID < pacsList.count()) 
        {
            pacs = pacsList.at(pacsID);
        }
    }

    return pacs;
}

bool PacsListDB::existPacs(const PacsParameters &pacs)
{
    QList<PacsParameters> pacsList = getConfiguredPacsList();

    foreach(PacsParameters pacsParameters, pacsList)
    {
        if (pacsParameters.getAEPacs() == pacs.getAEPacs() && 
            pacsParameters.getPacsPort() == pacs.getPacsPort() &&
            pacsParameters.getPacsAddress() == pacs.getPacsAddress())
        {
            return true;
        }
    }

    return false;
}

bool PacsListDB::deletePacs(QString pacsIDString)
{
    QList<PacsParameters> pacsList = getConfiguredPacsList();
    bool ok = false;
    int pacsID = pacsIDString.toInt(&ok);

    if (!ok) return false;

    pacsList.removeAt(pacsID);
    this->saveConfiguredPacsListToDisk(pacsList);

    return true;
}

QList<PacsParameters> PacsListDB::getConfiguredPacsList()
{
    QList<PacsParameters> configuredPacsList;
    QSettings settings;
    int size = settings.beginReadArray(PacsListConfigurationSectionName);

    for(int i = 0; i < size; ++i)
    {
        settings.setArrayIndex(i);
        PacsParameters pacs = fillPacs(settings);
        configuredPacsList.append(pacs);
    }
    settings.endArray();

    return configuredPacsList;
}

void PacsListDB::saveConfiguredPacsListToDisk(const QList<PacsParameters> &pacsList)
{
    QSettings settings;

    settings.beginWriteArray(PacsListConfigurationSectionName);
    settings.remove(""); //Esborrem la llista de pacs abans de guardar la nova.
    for(int i = 0; i < pacsList.size(); ++i)
    {
        settings.setArrayIndex(i);

        settings.setValue("ID", i);
        settings.setValue("AETitle", pacsList.at(i).getAEPacs() );
        settings.setValue("PacsPort", pacsList.at(i).getPacsPort() );
        settings.setValue("Location", pacsList.at(i).getLocation() );
        settings.setValue("Institution", pacsList.at(i).getInstitution() );
        settings.setValue("Default", pacsList.at(i).getDefault() );
        settings.setValue("PacsHostname", pacsList.at(i).getPacsAddress() );
        settings.setValue("Description", pacsList.at(i).getDescription() );
    }
    settings.endArray();
}

PacsParameters PacsListDB::fillPacs(const QSettings &settings)
{
    PacsParameters pacsParameters;

    pacsParameters.setPacsID( settings.value("ID" ).toString() );
    pacsParameters.setAEPacs( settings.value("AETitle" ).toString() );
    pacsParameters.setPacsPort( settings.value("PacsPort" ).toString() );
    pacsParameters.setLocation( settings.value("Location" ).toString() );
    pacsParameters.setInstitution( settings.value("Institution" ).toString() );
    pacsParameters.setDefault( settings.value("Default" ).toString() );
    pacsParameters.setPacsAddress( settings.value("PacsHostname" ).toString() );
    pacsParameters.setDescription( settings.value("Description" ).toString() );

    return pacsParameters;
}

};
