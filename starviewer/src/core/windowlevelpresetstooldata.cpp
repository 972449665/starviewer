#include "windowlevelpresetstooldata.h"
#include "customwindowlevelsrepository.h"
#include "logging.h"
#include <QStringList>

namespace udg {

WindowLevelPresetsToolData::WindowLevelPresetsToolData(QObject *parent)
 : ToolData(parent)
{
    // Per defecte afegirem els window levels predeterminats estàndar.
    // TODO Més endavant aquests valors podrien estar continguts en algun determinat fitxer amb un format definit
    addPreset(tr("CT Head"), 80, 40, StandardPresets);
    addPreset(tr("CT Bone"), 2000, 500, StandardPresets);
    addPreset(tr("CT Lung"), 1500, -650, StandardPresets);
    addPreset(tr("CT Soft Tissues"), 400, 40, StandardPresets);
    addPreset(tr("CT Soft Tissues+Contrast"), 400, 70, StandardPresets);

    // 60-100
    addPreset(tr("CT Liver+Contrast"), 300, 60, StandardPresets);
    addPreset(tr("CT Liver"), 200, 40, StandardPresets);
    addPreset(tr("CT Neck+Contrast"), 300, 50, StandardPresets);

    // 100-200
    addPreset(tr("Angiography"), 500, 100, StandardPresets);

    // 100-1500:window!
    addPreset(tr("Osteoporosis"), 1000, 300, StandardPresets);
    addPreset(tr("Emphysema"), 800, -800, StandardPresets);
    addPreset(tr("Petrous Bone"), 4000, 700, StandardPresets);
    addPreset(tr("Custom"), 0, 0, CustomPreset);
    // TODO ara caldria afegir els presets que tinguem guardats en QSettins, o altres tipus d'arxius tipus XML o ".ini"
    loadCustomWindowLevelPresets();
    connect(CustomWindowLevelsRepository::getRepository(), SIGNAL(changed()), this, SLOT(updateCustomWindowLevels()));
}

WindowLevelPresetsToolData::~WindowLevelPresetsToolData()
{
}

void WindowLevelPresetsToolData::addPreset(const QString &description, double window, double level, int group)
{
    if (!m_presets.contains(description))
    {
        WindowLevelStruct data = { window, level, group };
        m_presets.insert(description, data);
        if (group == WindowLevelPresetsToolData::FileDefined)
        {
            m_fileDefinedPresets << description;
        }
        emit presetAdded(description);
    }
}

void WindowLevelPresetsToolData::removePreset(const QString &description)
{
    if (m_presets.contains(description))
    {
        int group;
        if (getGroup(description, group))
        {
            if (group == FileDefined)
            {
                m_fileDefinedPresets.removeAt(m_fileDefinedPresets.indexOf(description));
            }
        }
        m_presets.remove(description);
        emit presetRemoved(description);
    }
}

void WindowLevelPresetsToolData::removePresetsFromGroup(int group)
{
    QMutableMapIterator<QString, WindowLevelStruct> iterator(m_presets);
    while (iterator.hasNext())
    {
        iterator.next();
        if (iterator.value().m_group == group)
        {
            emit presetRemoved(iterator.key());
            iterator.remove();
        }
    }

    if (group == FileDefined)
    {
        m_fileDefinedPresets.clear();
    }
}

bool WindowLevelPresetsToolData::getWindowLevelFromDescription(const QString &description, double &window, double &level)
{
    bool ok = true;
    if (m_presets.contains(description))
    {
        WindowLevelStruct data = m_presets.value(description);
        window = data.m_window;
        level = data.m_level;
    }
    else
    {
        ok = false;
        window = 0.0;
        level = 0.0;
    }
    return ok;
}

bool WindowLevelPresetsToolData::getGroup(const QString &description, int &group)
{
    bool ok = true;
    if (m_presets.contains(description))
    {
        WindowLevelStruct data = m_presets.value(description);
        group = data.m_group;
    }
    else
    {
        ok = false;
        group = Other;
    }
    return ok;
}

QStringList WindowLevelPresetsToolData::getDescriptionsFromGroup(int group)
{
    QStringList descriptionList;
    QMapIterator<QString, WindowLevelStruct> iterator(m_presets);
    while (iterator.hasNext())
    {
        iterator.next();
        if (iterator.value().m_group == group)
        {
            descriptionList << iterator.key();
        }
    }
    return descriptionList;
}

QString WindowLevelPresetsToolData::getCurrentPreset() const
{
    return m_currentPreset;
}

int WindowLevelPresetsToolData::getFileDefinedPresetIndex(const QString &preset) const
{
    return m_fileDefinedPresets.indexOf(preset);
}

void WindowLevelPresetsToolData::updateCurrentFileDefinedPreset(double window, double level)
{
    int group;
    if (getGroup(m_currentPreset, group))
    {
        if (group == FileDefined)
        {
            WindowLevelStruct data = { window, level, FileDefined };
            m_presets.insert(m_currentPreset, data);
            emit currentWindowLevel(window, level);
            // En aquest cas no fem emit presetChanged(preset) perquè sinó provocaria un bucle infinit
            // Només cal notificar el valor perquè el diàleg de custom WW/WL actualitzi el seu valor
        }
        else
        {
            DEBUG_LOG("El preset actual no és de tipus FileDefined. No s'actualitza cap valor de window/level");
        }
    }
    else
    {
        DEBUG_LOG("El preset actual no existeix al contenidor de presets. No s'actualitza cap valor de window/level");
    }
}

void WindowLevelPresetsToolData::setCustomWindowLevel(double window, double level)
{
    WindowLevelStruct data = { window, level, CustomPreset };
    m_presets.insert(tr("Custom"), data);
    emit currentWindowLevel(window, level);
    emit presetChanged(tr("Custom"));
    m_currentPreset = tr("Custom");
}

void WindowLevelPresetsToolData::activatePreset(const QString &preset)
{
    double window, level;
    if (this->getWindowLevelFromDescription(preset, window, level))
    {
        m_currentPreset = preset;
        emit currentWindowLevel(window, level);
        emit presetChanged(preset);
    }
}

void WindowLevelPresetsToolData::loadCustomWindowLevelPresets()
{
    foreach (CustomWindowLevel *customWindowLevel, CustomWindowLevelsRepository::getRepository()->getItems())
    {
        addPreset(customWindowLevel->getName(), customWindowLevel->getWidth(), customWindowLevel->getLevel(), UserDefined);
    }
}

void WindowLevelPresetsToolData::updateCustomWindowLevels()
{
    this->removePresetsFromGroup(UserDefined);
    this->loadCustomWindowLevelPresets();
}

}
