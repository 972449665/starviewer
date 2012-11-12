#include "screenmanager.h"
#include "dynamicmatrix.h"

#include <QDesktopWidget>
#include <QApplication>
#include <QWidget>

namespace udg {

ScreenManager::ScreenManager()
: SamePosition(5)
{
    setupCurrentScreenLayout();
}

void ScreenManager::setupCurrentScreenLayout()
{
    m_screenLayout.clear();
    m_applicationDesktop = QApplication::desktop();

    Screen screen;
    for (int i = 0; i < m_applicationDesktop->screenCount(); ++i)
    {
        screen.setID(i);
        screen.setGeometry(m_applicationDesktop->screenGeometry(i));
        screen.setAvailableGeometry(m_applicationDesktop->availableGeometry(i));
        screen.setAsPrimary(m_applicationDesktop->primaryScreen() == i);
        
        m_screenLayout.addScreen(screen);
    }
}

ScreenLayout ScreenManager::getScreenLayout() const
{
    return m_screenLayout;
}

void ScreenManager::maximize(QWidget *window)
{
    DynamicMatrix dynamicMatrix = computeScreenMatrix(window);

    // Agafa el top left i el bottomright per determinar les dimensions de la finestra
    QPoint topLeft = getTopLeft(dynamicMatrix);
    QPoint bottomRight = getBottomRight(dynamicMatrix);

    if (dynamicMatrix.isMaximizable())
    {
        if (window->isMaximized())
        {
            window->showNormal();
        }

        // Buscar la mida del frame i de la finestra
        QRect frameSize = window->frameGeometry();
        QRect windowSize = window->geometry();

        // Trobar el tamany real de les cantonades i la title bar
        int topBorder = windowSize.top() - frameSize.top();
        int bottomBorder = frameSize.bottom() - windowSize.bottom();
        int leftBorder = windowSize.left() - frameSize.left();
        int rightBorder = frameSize.right() - windowSize.right();

        // Calcular quin és el tamany que ha de tenir.
        // Se li ha de passar la geometria de la finestra, sense cantonades.
        int x = topLeft.x() + leftBorder;
        int y = topLeft.y() + topBorder;
        // x val x + leftBorder
        int width = bottomRight.x() - x - rightBorder;
        // y val y + topBorder
        int height = bottomRight.y() - y - bottomBorder;

        window->setGeometry(x, y, width, height);
    }
    else
    {
        window->showMaximized();
    }
}

void ScreenManager::moveToDesktop(QWidget *window, int idDesktop)
{
    // Aquesta variable s'utilitza per saber si abans de fer el fit la finestra hi cabia.
    bool fits = doesItFitInto(window, idDesktop);

    if (window->isMaximized())
    {
        // La finestra està maximitzada i es mou a una altra pantalla
        // per tant es desmaximitza, es mou i es maximitza de nou
        // Si es mou a la mateixa pantalla, no es fa res
        int desktopIAm = getScreenID(window);
        if (idDesktop != desktopIAm)
        {
            window->showNormal();
            fitInto(window, idDesktop);
            window->showMaximized();
        }
    }
    else
    {
        fitInto(window, idDesktop);
        if (!fits)
        {
            window->showMaximized();
        }
    }
}

void ScreenManager::moveToPreviousDesktop(QWidget *window)
{
    int desktopIAm = getScreenID(window);
    int desktopIllBe = m_screenLayout.getPreviousScreenOf(desktopIAm);

    moveToDesktop(window, desktopIllBe);
}

void ScreenManager::moveToNextDesktop(QWidget *window)
{
    int desktopIAm = getScreenID(window);
    int desktopIllBe = m_screenLayout.getNextScreenOf(desktopIAm);

    moveToDesktop(window, desktopIllBe);
}

void ScreenManager::restoreFromMinimized(QWidget *window)
{
    //WindowState accepta una combinació dels Enums Qt::WindowState, el que fem amb aquest mètode es treure l'estat de minimitzat i li indiquem torna a està activa
    //Imaginem que teníem la finestra maximitzada, si la minimitzem WindowState tindrà el valor Qt::WindowMinimized | QtWindowMaximized, i aplicant aquest codi treurem l'estat Qt:WindowMinimized
    //i li afegeix el WindowActive restaurant la finestra al seu estat original
    window->setWindowState(window->windowState() & ~Qt::WindowMinimized | Qt::WindowActive);
}

int ScreenManager::getScreenID(QWidget *window) const
{
    return m_applicationDesktop->screenNumber(window);
}

int ScreenManager::getScreenID(const QPoint &point) const
{
    return m_applicationDesktop->screenNumber(point);
}

DynamicMatrix ScreenManager::computeScreenMatrix(QWidget *window)
{
    int desktopIAm = getScreenID(window);

    // Primer de tot buscar les pantalles de la mateixa fila
    DynamicMatrix dynamicMatrix;
    int indexLeft = 0;
    int indexRight = 0;
    int indexTop = 0;
    int indexBottom = 0;
    dynamicMatrix.setValue(0, 0, desktopIAm);
    int numberOfScreens = m_screenLayout.getNumberOfScreens();
    bool changes = true;
    while (changes)
    {
        changes = false;
        for (int i = 0; i < numberOfScreens; i++)
        {
            if (isLeft(i, dynamicMatrix.getValue(0, indexLeft)))
            {
                dynamicMatrix.setValue(0, --indexLeft, i);
                changes = true;
            }
            else if (isRight(i, dynamicMatrix.getValue(0, indexRight)))
            {
                dynamicMatrix.setValue(0, ++indexRight, i);
                changes = true;
            }
        }
    }
    // Ara anar mirant si tota la fila te pantalles a sobre o a sota
    changes = true;
    bool keepLookingUp = true;
    bool keepLookingDown = true;
    while (changes)
    {
        changes = false;
        // Mirar si es pot afegir la fila de sobre
        QList<int> topRow;
        int index = indexLeft;
        // Si hem de mirar la fila de sobre true altrament false
        bool rowFound = keepLookingUp;
        while (keepLookingUp && index <= indexRight)
        {
            bool found = false;
            int j = 0;
            while (j < numberOfScreens)
            {
                if (isTop(j, dynamicMatrix.getValue(indexTop, index)))
                {
                    topRow.append(j);
                    found = true;
                    j = numberOfScreens;
                }
                j++;
            }
            if (found)
            {
                index++;
            }
            else
            {
                rowFound = false;
                keepLookingUp = false;
                index = indexRight + 1;
            }
        }
        // Si s'ha pogut afegir TOTA la fila de sobre
        if (rowFound)
        {
            indexTop++;
            for (int i = indexLeft, j = 0; i <= indexRight; i++, j++)
            {
                dynamicMatrix.setValue(indexTop, i, topRow[j]);
            }
            changes = true;
        }

        // Mirar si es pot afegir la fila de sota
        QList<int> bottomRow;
        index = indexLeft;
        // Si em de mirar la fila de sota true, altrament false
        rowFound = keepLookingDown;
        while (keepLookingDown && index <= indexRight)
        {
            bool found = false;
            int j = 0;
            while (j < numberOfScreens)
            {
                if (isBottom(j, dynamicMatrix.getValue(indexBottom, index)))
                {
                    bottomRow.append(j);
                    found = true;
                    j = numberOfScreens;
                }
                j++;
            }
            if (found)
            {
                index++;
            }
            else
            {
                rowFound = false;
                keepLookingDown = false;
                index = indexRight + 1;
            }
        }
        // Si s'ha pogut afegir TOTA la fila de sobre
        if (rowFound)
        {
            indexBottom--;
            for (int i = indexLeft, j = 0; i <= indexRight; i++, j++)
            {
                dynamicMatrix.setValue(indexBottom, i, bottomRow[j]);
            }
            changes = true;
        }
    }

    return dynamicMatrix;
}

bool ScreenManager::doesItFitInto(QWidget *window, int IdDesktop)
{
    QRect newDesktopAvailableGeometry = m_screenLayout.getScreen(IdDesktop).getAvailableGeometry();

    // Si és massa ampla o massa alt, no hi cap.
    return !(newDesktopAvailableGeometry.width() < window->minimumWidth() || newDesktopAvailableGeometry.height() < window->minimumHeight());
}

void ScreenManager::fitInto(QWidget *window, int IdDesktop)
{
    QRect newDesktopAvailableGeometry = m_screenLayout.getScreen(IdDesktop).getAvailableGeometry();
    int newDesktopWidth = newDesktopAvailableGeometry.width();
    int newDesktopHeight = newDesktopAvailableGeometry.height();

    int width = window->frameSize().width();
    int height = window->frameSize().height();
    int x = newDesktopAvailableGeometry.topLeft().x();
    int y = newDesktopAvailableGeometry.topLeft().y();

    // Buscar la mida del frame i de la finestra
    QRect frameSize = window->frameGeometry();
    QRect windowSize = window->geometry();

    // Trobar el tamany real de les cantonades i la title bar
    int topBorder = windowSize.top() - frameSize.top();
    int bottomBorder = frameSize.bottom() - windowSize.bottom();
    int leftBorder = windowSize.left() - frameSize.left();
    int rightBorder = frameSize.right() - windowSize.right();

    // Si la finestra és més ample que la pantalla
    if (width > newDesktopWidth)
    {
        width = newDesktopWidth;
        x += leftBorder;
    }
    // Altrament centrar
    else
    {
        x = x + newDesktopWidth / 2 - width / 2 + leftBorder;
    }

    // Si la finestra és més alta que la pantalla
    if (height > newDesktopHeight)
    {
        height = newDesktopHeight;
        y += topBorder;
    }
    // Altrament centrar
    else
    {
        y = y + newDesktopHeight / 2 - height / 2 + topBorder;
    }

    // La mida de la finestra l'hem agafat del frame, per tant li hem de treure
    // les cantonades per que la mida sigui la mateixa
    window->setGeometry(x,
                        y,
                        width - leftBorder - rightBorder,
                        height - topBorder - bottomBorder);
}

bool ScreenManager::isTop(int desktop1, int desktop2)
{
    QRect screen1Geometry = m_screenLayout.getScreen(desktop1).getGeometry();
    QRect screen2Geometry = m_screenLayout.getScreen(desktop2).getGeometry();
    // Esta posat a sobre
    if (abs(screen1Geometry.bottom() - screen2Geometry.top()) < SamePosition)
    {
        // Te la mateixa alçada
        int leftPart = abs(screen1Geometry.left() - screen2Geometry.left());
        int rightPart = abs(screen1Geometry.right() - screen2Geometry.right());
        if (leftPart + rightPart < SamePosition)
        {
            return true;
        }
    }

    return false;
}

bool ScreenManager::isBottom(int desktop1, int desktop2)
{
    QRect screen1Geometry = m_screenLayout.getScreen(desktop1).getGeometry();
    QRect screen2Geometry = m_screenLayout.getScreen(desktop2).getGeometry();
    // Esta posat a sota
    if (abs(screen1Geometry.top() - screen2Geometry.bottom()) < SamePosition)
    {
        // Te la mateixa alçada
        int leftPart = abs(screen1Geometry.left() - screen2Geometry.left());
        int rightPart = abs(screen1Geometry.right() - screen2Geometry.right());
        if (leftPart + rightPart < SamePosition)
        {
            return true;
        }
    }

    return false;
}

bool ScreenManager::isLeft(int desktop1, int desktop2)
{
    QRect screen1Geometry = m_screenLayout.getScreen(desktop1).getGeometry();
    QRect screen2Geometry = m_screenLayout.getScreen(desktop2).getGeometry();
    // Esta posat a l'esquerra
    if (abs(screen1Geometry.right() - screen2Geometry.left()) < SamePosition)
    {
        // Te la mateixa alçada
        int topPart = abs(screen1Geometry.top() - screen2Geometry.top());
        int bottomPart = abs(screen1Geometry.bottom() - screen2Geometry.bottom());
        if (topPart + bottomPart < SamePosition)
        {
            return true;
        }
    }
    return false;
}

bool ScreenManager::isRight(int desktop1, int desktop2)
{
    QRect screen1Geometry = m_screenLayout.getScreen(desktop1).getGeometry();
    QRect screen2Geometry = m_screenLayout.getScreen(desktop2).getGeometry();
    // Esta posat a l'esquerra
    if (abs(screen1Geometry.left() - screen2Geometry.right()) < SamePosition)
    {
        // Te la mateixa alçada
        int topPart = abs(screen1Geometry.top() - screen2Geometry.top());
        int bottomPart = abs(screen1Geometry.bottom() - screen2Geometry.bottom());
        if (topPart + bottomPart < SamePosition)
        {
            return true;
        }
    }
    return false;
}

bool ScreenManager::isTopLeft(int desktop1, int desktop2)
{
    QRect screen1Geometry = m_screenLayout.getScreen(desktop1).getGeometry();
    QRect screen2Geometry = m_screenLayout.getScreen(desktop2).getGeometry();
    int x = abs(screen1Geometry.bottomRight().x() - screen2Geometry.topLeft().x());
    int y = abs(screen1Geometry.bottomRight().y() - screen2Geometry.topLeft().y());
    if (abs(x * x - y * y) < SamePosition)
    {
        return true;
    }
    return false;
}

bool ScreenManager::isTopRight(int desktop1, int desktop2)
{
    QRect screen1Geometry = m_screenLayout.getScreen(desktop1).getGeometry();
    QRect screen2Geometry = m_screenLayout.getScreen(desktop2).getGeometry();
    int x = abs(screen1Geometry.bottomLeft().x() - screen2Geometry.topRight().x());
    int y = abs(screen1Geometry.bottomLeft().y() - screen2Geometry.topRight().y());
    if (abs(x * x - y * y) < SamePosition)
    {
        return true;
    }
    return false;
}

bool ScreenManager::isBottomLeft(int desktop1, int desktop2)
{
    QRect screen1Geometry = m_screenLayout.getScreen(desktop1).getGeometry();
    QRect screen2Geometry = m_screenLayout.getScreen(desktop2).getGeometry();
    int x = abs(screen1Geometry.topRight().x() - screen2Geometry.bottomLeft().x());
    int y = abs(screen1Geometry.topRight().y() - screen2Geometry.bottomLeft().y());
    if (abs(x * x - y * y) < SamePosition)
    {
        return true;
    }
    return false;
}

bool ScreenManager::isBottomRight(int desktop1, int desktop2)
{
    QRect screen1Geometry = m_screenLayout.getScreen(desktop1).getGeometry();
    QRect screen2Geometry = m_screenLayout.getScreen(desktop2).getGeometry();
    int x = abs(screen1Geometry.topLeft().x() - screen2Geometry.bottomRight().x());
    int y = abs(screen1Geometry.topLeft().y() - screen2Geometry.bottomRight().y());
    if (abs(x * x - y * y) < SamePosition)
    {
        return true;
    }
    return false;
}

int ScreenManager::whoIsLeft(int desktopIAm)
{
    for (int i = 0; i < m_screenLayout.getNumberOfScreens(); i++)
    {
        if (isLeft(i, desktopIAm))
        {
            return i;
        }
    }

    return -1;
}

int ScreenManager::whoIsRight(int desktopIAm)
{
    for (int i = 0; i < m_screenLayout.getNumberOfScreens(); i++)
    {
        if (isRight(i, desktopIAm))
        {
            return i;
        }
    }

    return -1;
}

int ScreenManager::whoIsTop(int desktopIAm)
{
    for (int i = 0; i < m_screenLayout.getNumberOfScreens(); i++)
    {
        if (isTop(i, desktopIAm))
        {
            return i;
        }
    }

    return -1;
}

int ScreenManager::whoIsBottom(int desktopIAm)
{

    for (int i = 0; i < m_screenLayout.getNumberOfScreens(); i++)
    {
        if (isBottom(i, desktopIAm))
        {
            return i;
        }
    }

    return -1;
}

QPoint ScreenManager::getTopLeft(const DynamicMatrix &dynamicMatrix) const
{
    // Primer de tot buscar la cantonada esquerra, a partir de la llista de monitors a l'esquerra,
    // agafar el màxim, per si la barra de windows esta a l'esquerra en algun d'ells
    QList<int> screens = dynamicMatrix.getLeftColumn();
    int x = m_screenLayout.getScreen(screens[0]).getAvailableGeometry().left();
    for (int i = 1; i < screens.count(); i++)
    {
        x = std::max(x, m_screenLayout.getScreen(screens[i]).getAvailableGeometry().left());
    }
    // El mateix per la part superior
    screens = dynamicMatrix.getTopRow();
    int y = m_screenLayout.getScreen(screens[0]).getAvailableGeometry().top();
    for (int i = 1; i < screens.count(); i++)
    {
        y = std::max(y, m_screenLayout.getScreen(screens[i]).getAvailableGeometry().top());
    }

    return QPoint(x, y);
}

QPoint ScreenManager::getBottomRight(const DynamicMatrix &dynamicMatrix) const
{

    // Primer de tot buscar la cantonada dreta, a partir de la llista de monitors a lla dreta,
    // agafar el mínim, per si la barra de windows esta a la dreta en algun d'ells
    QList<int> screens = dynamicMatrix.getRightColumn();
    int x = m_screenLayout.getScreen(screens[0]).getAvailableGeometry().right();
    for (int i = 1; i < screens.count(); i++)
    {
        x = std::min(x, m_screenLayout.getScreen(screens[i]).getAvailableGeometry().right());
    }
    // El mateix per la part de baix
    screens = dynamicMatrix.getBottomRow();
    int y = m_screenLayout.getScreen(screens[0]).getAvailableGeometry().bottom();
    for (int i = 1; i < screens.count(); i++)
    {
        y = std::min(y, m_screenLayout.getScreen(screens[i]).getAvailableGeometry().bottom());
    }

    return QPoint(x, y);
}

} // End namespace udg
