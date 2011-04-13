#ifndef UDGQWINDOWLEVELCOMBOBOX_H
#define UDGQWINDOWLEVELCOMBOBOX_H

#include <QComboBox>

namespace udg {

// FWD declarations
class QCustomWindowLevelDialog;
class WindowLevelPresetsToolData;

/**
    Combo box personalitzat per a escollir valors de window level predeterminats i personalitzats

    @author Grup de Gràfics de Girona  (GGG) <vismed@ima.udg.es>
*/
class QWindowLevelComboBox : public QComboBox {
Q_OBJECT
public:
    QWindowLevelComboBox(QWidget *parent = 0);
    ~QWindowLevelComboBox();

    /// Li assignem la font de dades a partir de la qual obté els valors de window level
    /// @param windowLevelData Les dades en sí
    void setPresetsData(WindowLevelPresetsToolData *windowLevelData);

    /// Neteja el contingut del combo box i elimina qualsevol referència a un WindowLevelPresetsToolData
    void clearPresets();

public slots:
    /// Selecciona el preset indicat en el combo, però no l'activa
    /// @param preset Descripció del preset
    void selectPreset(const QString &preset);

private slots:    
    /// Afegeix o elimina un preset de la llista
    /// @param preset Preset a afegir o eliminar
    void addPreset(const QString &preset);
    void removePreset(const QString &preset);

    /// Seleccionem el window level que volem aplicar com a actiu
    void setActiveWindowLevel(const QString &text);

private:
    /// Omple el combo a partir de les dades de presets. Neteja les dades que hi pugui haver anteriorment
    void populateFromPresetsData();

    /// Desconnecta totes les connexions
    void disconnectPresetsData();

private:
    /// Diàleg per escollir un window level personalitzat
    QCustomWindowLevelDialog *m_customWindowLevelDialog;

    /// Les dades dels valors pre-establerts
    WindowLevelPresetsToolData *m_presetsData;

    /// Ens guardem el preset actual per reestablir-lo si l'usuari clica l'editor de WW/WL ja que sinó quedaria la fila de l'editor seleccionada.
    QString m_currentSelectedPreset;
};

}

#endif
