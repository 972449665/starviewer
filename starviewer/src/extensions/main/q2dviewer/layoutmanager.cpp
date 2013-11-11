#include "layoutmanager.h"

#include "patient.h"
#include "study.h"
#include "hangingprotocolmanager.h"
#include "viewerslayout.h"
#include "coresettings.h"
#include "studylayoutconfigsloader.h"
#include "studylayoutmapper.h"
#include "studylayoutconfig.h"
#include "studylayoutconfigsettingsmanager.h"

namespace udg {

LayoutManager::LayoutManager(Patient *patient, ViewersLayout *layout, QObject *parent)
 : QObject(parent)
{
    m_patient = patient;
    m_layout = layout;
    m_hangingProtocolManager = new HangingProtocolManager(this);
    
    connect(m_patient, SIGNAL(studyAdded(Study*)), SLOT(onStudyAdded(Study*)));
    connect(m_hangingProtocolManager, SIGNAL(discardedStudy(QString)), SLOT(addStudyToIgnore(QString)));
}

LayoutManager::~LayoutManager()
{
}

void LayoutManager::initialize()
{
    searchHangingProtocols();
    applyProperLayoutChoice();
}

bool LayoutManager::hasCurrentPatientAnyModalityWithHangingProtocolPriority()
{
    Settings settings;

    QStringList modalitiesWithHPPriority = settings.getValueAsQStringList(CoreSettings::ModalitiesToApplyHangingProtocolsAsFirstOption);

    QSet<QString> matchingModalities = modalitiesWithHPPriority.toSet().intersect(m_patient->getModalities().toSet());

    return !matchingModalities.isEmpty();
}

void LayoutManager::applyProperLayoutChoice()
{
    bool layoutApplied = false;
    if (hasCurrentPatientAnyModalityWithHangingProtocolPriority())
    {
        layoutApplied = applyBestHangingProtocol();
    }

    if (!layoutApplied)
    {
        applyLayoutCandidates(getLayoutCandidates(m_patient), m_patient);
    }
}

void LayoutManager::searchHangingProtocols()
{
    m_hangingProtocolCandidates = m_hangingProtocolManager->searchHangingProtocols(m_patient);
    emit hangingProtocolCandidatesFound(m_hangingProtocolCandidates);
}

bool LayoutManager::applyBestHangingProtocol()
{
    bool hangingProtocolApplied = false;
    
    if (m_hangingProtocolCandidates.size() > 0)
    {
        m_hangingProtocolManager->setBestHangingProtocol(m_patient, m_hangingProtocolCandidates, m_layout);
        hangingProtocolApplied = true;
    }

    return hangingProtocolApplied;
}

void LayoutManager::cancelOngoingOperations()
{
    m_hangingProtocolManager->cancelHangingProtocolDownloading();
}

QList<StudyLayoutConfig> LayoutManager::getLayoutCandidates(Patient *patient)
{
    QList<StudyLayoutConfig> configurationCandidates;
    
    if (!patient)
    {
        return configurationCandidates;
    }
    
    QStringList patientModalities = patient->getModalities();
    StudyLayoutConfigSettingsManager settingsManager;
    
    foreach (const StudyLayoutConfig &currentConfig, settingsManager.getConfigList())
    {
        if (patientModalities.contains(currentConfig.getModality(), Qt::CaseInsensitive))
        {
            configurationCandidates << currentConfig;
        }
    }

    return configurationCandidates;
}

void LayoutManager::applyLayoutCandidates(const QList<StudyLayoutConfig> &candidates, Patient *patient)
{
    StudyLayoutConfig layoutToApply = getBestLayoutCandidate(candidates, patient);

    StudyLayoutMapper mapper;
    mapper.applyConfig(layoutToApply, m_layout, patient);
}

StudyLayoutConfig LayoutManager::getBestLayoutCandidate(const QList<StudyLayoutConfig> &candidates, Patient *patient)
{
    StudyLayoutConfig bestLayout;

    if (!candidates.isEmpty())
    {
        if (candidates.count() > 1)
        {
            // If we different modalities/candidates we take the simple choice of apply a default generic layout
            // TODO This could be enhanced making smarter decisions, which can include decisions based on user settings i.e.
            bestLayout = StudyLayoutConfigsLoader::getDefaultConfigForModality("");
        }
        else
        {
            bestLayout = candidates.first();
        }
    }
    else
    {
        // If no candidate found, we choose a default configuration.
        // This default configuration is not yet configurable through settings, could be done in a future enhancement.
        bestLayout = StudyLayoutConfigsLoader::getDefaultConfigForModality(patient->getModalities().first());
    }

    return bestLayout;
}

void LayoutManager::setHangingProtocol(int hangingProtocolNumber)
{
    m_hangingProtocolManager->applyHangingProtocol(hangingProtocolNumber, m_layout, m_patient);
}

void LayoutManager::addHangingProtocolsWithPrevious(QList<Study*> studies)
{
    m_hangingProtocolCandidates = m_hangingProtocolManager->searchHangingProtocols(m_patient, studies);
    emit hangingProtocolCandidatesFound(m_hangingProtocolCandidates);
    // HACK To notify we ended searching related studies and thus we have all the hanging protocols available
    emit previousStudiesSearchEnded();
}

void LayoutManager::onStudyAdded(Study *study)
{
    addHangingProtocolsWithPrevious(QList<Study*>() << study);

    // If the study added was discarded by the hanging protocol manager we don't have to update the layout
    if (m_studiesToIgnoreWhenAdded.contains(study->getInstanceUID()))
    {
        // Remove from the list, it could be added later by other reason
        m_studiesToIgnoreWhenAdded.remove(study->getInstanceUID());
        return;
    }
    
    // Only update the layout again if the study does not correspond to a previously demanded study by a hanging protocol with previous
    if (!m_hangingProtocolManager->isPreviousStudyForHangingProtocol(study))
    {
        applyProperLayoutChoice();
    }
}

void LayoutManager::addStudyToIgnore(const QString &uid)
{
    m_studiesToIgnoreWhenAdded << uid;
}

} // end namespace udg
