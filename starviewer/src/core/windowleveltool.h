#ifndef UDGWINDOWLEVELTOOL_H
#define UDGWINDOWLEVELTOOL_H

#include "tool.h"

namespace udg {

class QViewer;

class WindowLevelTool : public Tool {
Q_OBJECT
public:
    /// Estats de la tool
    enum { None, WindowLevelling };

    WindowLevelTool(QViewer *viewer, QObject *parent = 0);
    ~WindowLevelTool();

    void handleEvent(unsigned long eventID);

private slots:
    /// Comença el window level
    void startWindowLevel();

    /// Calcula el nou window level
    void doWindowLevel();

    /// Atura l'estat de window level
    void endWindowLevel();

private:
    /// Valors per controlar el mapeig del window level
    double m_initialWindow, m_initialLevel, m_currentWindow, m_currentLevel;
    int m_windowLevelStartPosition[2], m_windowLevelCurrentPosition[2];

    /// Estats d'execució de la tool
    int m_state;
};

}

#endif
