#include "screenmanager.h"
#include "dynamicmatrix.h"

#include <QDesktopWidget>
#include <QApplication>
#include <QWidget>

namespace udg {

ScreenManager::ScreenManager()
: SamePosition(5)
{
    m_applicationDesktop = QApplication::desktop();
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
        int desktopIAm = m_applicationDesktop->screenNumber(window);
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
    int desktopIAm = getIdOfScreen(window);
    int desktopIllBe = -1;

    // Buscar una pantalla a l'esquerra i a la mateixa altura + o -
    for (int i = 0; i < getNumberOfScreens(); i++)
    {
        // Si està a l'esquerra, però no està completament per sobre ni per sota
        if (isOnLeft(i, desktopIAm) && !isOver(i, desktopIAm) && !isUnder(i, desktopIAm))
        {
            // Si encara no hem trobat cap pantalla
            if (desktopIllBe == -1)
            {
                desktopIllBe = i;
            }
            // De les pantalles de l'esquerra, volem la més pròxima
            // Si la pantalla que hem trobat està més a la dreta que la que tenim
            else if (isOnRight(i, desktopIllBe))
            {
                desktopIllBe = i;
            }
        }
    }

    // Si no hi ha cap pantalla a l'esquerra, llavors busquem la de més a la dreta que està per sobre d'aquesta
    if (desktopIllBe == -1)
    {
        for (int i = 0; i < getNumberOfScreens(); i++)
        {
            if (isHigher(i, desktopIAm))
            {
                // Si encara no hem trobat cap pantalla
                if (desktopIllBe == -1)
                {
                    desktopIllBe = i;
                }
                // De les pantalles de sobre, volem la més a la dreta
                else if (isOnRight(i, desktopIllBe))
                {
                    desktopIllBe = i;
                }
            }
        }
    }

    // Si no hi ha cap patalla per sobre de la actual, agafarem la de més avall a la dreta
    if (desktopIllBe == -1)
    {
        // Amb això assegurem que mai arribarà al moveToDesktop valent -1
        desktopIllBe = 0;
        for (int i = 1; i < getNumberOfScreens(); i++)
        {
            // Si està per sota de l'actual ens la quedem
            if (isUnder(i, desktopIllBe))
            {
                desktopIllBe = i;
            }
            // Si no, si no està per sobre, l'agafem si està més a la dreta que l'actual
            else if (!isOver(i, desktopIllBe) && isMoreOnRight(i, desktopIllBe))
            {
                desktopIllBe = i;
            }
        }
    }

    moveToDesktop(window, desktopIllBe);
}

void ScreenManager::moveToNextDesktop(QWidget *window)
{
    int desktopIAm = getIdOfScreen(window);
    int desktopIllBe = -1;

    // Buscar una pantalla a la dreta i a la mateixa altura + o -
    for (int i = 0; i < getNumberOfScreens(); i++)
    {
        // Si està a la dreta, però no està completament per sobre ni per sota
        if (isOnRight(i, desktopIAm) && !isOver(i, desktopIAm) && !isUnder(i, desktopIAm))
        {
            // Si encara no hem trobat cap pantalla
            if (desktopIllBe == -1)
            {
                desktopIllBe = i;
            }
            // De les pantalles de la dreta, volem la més pròxima
            // Si la pantalla que hem trobat està més a l'esquerra que la que tenim
            else if (isOnLeft(i, desktopIllBe))
            {
                desktopIllBe = i;
            }
        }
    }

    // Si no hi ha cap pantalla a la dreta, llavors busquem la de més a l'esquerra que està per sota d'aquesta
    if (desktopIllBe == -1)
    {
        for (int i = 0; i < getNumberOfScreens(); i++)
        {
            if (isLower(i, desktopIAm))
            {
                // Si encara no hem trobat cap pantalla
                if (desktopIllBe == -1)
                {
                    desktopIllBe = i;
                }
                // De les pantalles de sota, volem la més a l'esquerra
                else if (isOnLeft(i, desktopIllBe))
                {
                    desktopIllBe = i;
                }
            }
        }
    }

    // Si no hi ha cap patalla per sota de la actual, agafarem la de més amunt a l'esquerra
    if (desktopIllBe == -1)
    {
        // Amb això assegurem que mai arribarà al moveToDesktop valent -1
        desktopIllBe = 0;
        for (int i = 1; i < getNumberOfScreens(); i++)
        {
            // Si està per sobre de l'actual ens la quedem
            if (isOver(i, desktopIllBe))
            {
                desktopIllBe = i;
            }
            // Si no, si no està per sota, l'agafem si està més a l'esquerra que l'actual
            else if (!isUnder(i, desktopIllBe) && isMoreOnLeft(i, desktopIllBe))
            {
                desktopIllBe = i;
            }
        }
    }

    moveToDesktop(window, desktopIllBe);
}

int ScreenManager::getNumberOfScreens()
{
    return m_applicationDesktop->numScreens();
}

int ScreenManager::getIdOfScreen(QWidget *window)
{
    return m_applicationDesktop->screenNumber(window);
}

int ScreenManager::getScreenID(const QPoint &point) const
{
    return m_applicationDesktop->screenNumber(point);
}

QRect ScreenManager::getAvailableScreenGeometry(int screenID) const
{
    return m_applicationDesktop->availableGeometry(screenID);
}

DynamicMatrix ScreenManager::computeScreenMatrix(QWidget *window)
{
    int desktopIAm = m_applicationDesktop->screenNumber(window);

    // Primer de tot buscar les pantalles de la mateixa fila
    DynamicMatrix dynamicMatrix;
    int indexLeft = 0;
    int indexRight = 0;
    int indexTop = 0;
    int indexBottom = 0;
    dynamicMatrix.setValue(0, 0, desktopIAm);
    bool changes = true;
    while (changes)
    {
        changes = false;
        for (int i = 0; i < m_applicationDesktop->numScreens(); i++)
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
            while (j < m_applicationDesktop->numScreens())
            {
                if (isTop(j, dynamicMatrix.getValue(indexTop, index)))
                {
                    topRow.append(j);
                    found = true;
                    j = m_applicationDesktop->numScreens();
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
            while (j < m_applicationDesktop->numScreens())
            {
                if (isBottom(j, dynamicMatrix.getValue(indexBottom, index)))
                {
                    bottomRow.append(j);
                    found = true;
                    j = m_applicationDesktop->numScreens();
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

    int newDesktopWidth = m_applicationDesktop->availableGeometry(IdDesktop).width();
    int newDesktopHeight = m_applicationDesktop->availableGeometry(IdDesktop).height();

    // Si és massa ampla o massa alt, no hi cap.
    return !(newDesktopWidth < window->minimumWidth() || newDesktopHeight < window->minimumHeight());
}

void ScreenManager::fitInto(QWidget *window, int IdDesktop)
{
    int newDesktopWidth = m_applicationDesktop->availableGeometry(IdDesktop).width();
    int newDesktopHeight = m_applicationDesktop->availableGeometry(IdDesktop).height();

    int width = window->frameSize().width();
    int height = window->frameSize().height();
    int x = m_applicationDesktop->availableGeometry(IdDesktop).topLeft().x();
    int y = m_applicationDesktop->availableGeometry(IdDesktop).topLeft().y();

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
    // Esta posat a sobre
    if (abs(m_applicationDesktop->screenGeometry(desktop1).bottom() - m_applicationDesktop->screenGeometry(desktop2).top()) < SamePosition)
    {
        // Te la mateixa alçada
        int leftPart = abs(m_applicationDesktop->screenGeometry(desktop1).left() - m_applicationDesktop->screenGeometry(desktop2).left());
        int rightPart = abs(m_applicationDesktop->screenGeometry(desktop1).right() - m_applicationDesktop->screenGeometry(desktop2).right());
        if (leftPart + rightPart < SamePosition)
        {
            return true;
        }
    }

    return false;
}

bool ScreenManager::isBottom(int desktop1, int desktop2)
{
    // Esta posat a sota
    if (abs(m_applicationDesktop->screenGeometry(desktop1).top() - m_applicationDesktop->screenGeometry(desktop2).bottom()) < SamePosition)
    {
        // Te la mateixa alçada
        int leftPart = abs(m_applicationDesktop->screenGeometry(desktop1).left() - m_applicationDesktop->screenGeometry(desktop2).left());
        int rightPart = abs(m_applicationDesktop->screenGeometry(desktop1).right() - m_applicationDesktop->screenGeometry(desktop2).right());
        if (leftPart + rightPart < SamePosition)
        {
            return true;
        }
    }

    return false;
}

bool ScreenManager::isLeft(int desktop1, int desktop2)
{
    // Esta posat a l'esquerra
    if (abs(m_applicationDesktop->screenGeometry(desktop1).right() - m_applicationDesktop->screenGeometry(desktop2).left()) < SamePosition)
    {
        // Te la mateixa alçada
        int topPart = abs(m_applicationDesktop->screenGeometry(desktop1).top() - m_applicationDesktop->screenGeometry(desktop2).top());
        int bottomPart = abs(m_applicationDesktop->screenGeometry(desktop1).bottom() - m_applicationDesktop->screenGeometry(desktop2).bottom());
        if (topPart + bottomPart < SamePosition)
        {
            return true;
        }
    }
    return false;
}

bool ScreenManager::isRight(int desktop1, int desktop2)
{
    // Esta posat a l'esquerra
    if (abs(m_applicationDesktop->screenGeometry(desktop1).left() - m_applicationDesktop->screenGeometry(desktop2).right()) < SamePosition)
    {
        // Te la mateixa alçada
        int topPart = abs(m_applicationDesktop->screenGeometry(desktop1).top() - m_applicationDesktop->screenGeometry(desktop2).top());
        int bottomPart = abs(m_applicationDesktop->screenGeometry(desktop1).bottom() - m_applicationDesktop->screenGeometry(desktop2).bottom());
        if (topPart + bottomPart < SamePosition)
        {
            return true;
        }
    }
    return false;
}

bool ScreenManager::isTopLeft(int desktop1, int desktop2)
{
    int x = abs(m_applicationDesktop->screenGeometry(desktop1).bottomRight().x() - m_applicationDesktop->screenGeometry(desktop2).topLeft().x());
    int y = abs(m_applicationDesktop->screenGeometry(desktop1).bottomRight().y() - m_applicationDesktop->screenGeometry(desktop2).topLeft().y());
    if (abs(x * x - y * y) < SamePosition)
    {
        return true;
    }
    return false;
}

bool ScreenManager::isTopRight(int desktop1, int desktop2)
{
    int x = abs(m_applicationDesktop->screenGeometry(desktop1).bottomLeft().x() - m_applicationDesktop->screenGeometry(desktop2).topRight().x());
    int y = abs(m_applicationDesktop->screenGeometry(desktop1).bottomLeft().y() - m_applicationDesktop->screenGeometry(desktop2).topRight().y());
    if (abs(x * x - y * y) < SamePosition)
    {
        return true;
    }
    return false;
}

bool ScreenManager::isBottomLeft(int desktop1, int desktop2)
{
    int x = abs(m_applicationDesktop->screenGeometry(desktop1).topRight().x() - m_applicationDesktop->screenGeometry(desktop2).bottomLeft().x());
    int y = abs(m_applicationDesktop->screenGeometry(desktop1).topRight().y() - m_applicationDesktop->screenGeometry(desktop2).bottomLeft().y());
    if (abs(x * x - y * y) < SamePosition)
    {
        return true;
    }
    return false;
}

bool ScreenManager::isBottomRight(int desktop1, int desktop2)
{

    int x = abs(m_applicationDesktop->screenGeometry(desktop1).topLeft().x() - m_applicationDesktop->screenGeometry(desktop2).bottomRight().x());
    int y = abs(m_applicationDesktop->screenGeometry(desktop1).topLeft().y() - m_applicationDesktop->screenGeometry(desktop2).bottomRight().y());
    if (abs(x * x - y * y) < SamePosition)
    {
        return true;
    }
    return false;
}

bool ScreenManager::isOver(int desktop1, int desktop2)
{
    if (m_applicationDesktop->screenGeometry(desktop1).bottom() <= m_applicationDesktop->screenGeometry(desktop2).top())
    {
        return true;
    }
    return false;
}

bool ScreenManager::isUnder(int desktop1, int desktop2)
{
    if (m_applicationDesktop->screenGeometry(desktop1).top() >= m_applicationDesktop->screenGeometry(desktop2).bottom())
    {
        return true;
    }
    return false;
}

bool ScreenManager::isOnLeft(int desktop1, int desktop2)
{
    if (m_applicationDesktop->screenGeometry(desktop1).right() <= m_applicationDesktop->screenGeometry(desktop2).left())
    {
        return true;
    }
    return false;
}

bool ScreenManager::isOnRight(int desktop1, int desktop2)
{
    if (m_applicationDesktop->screenGeometry(desktop1).left() >= m_applicationDesktop->screenGeometry(desktop2).right())
    {
        return true;
    }
    return false;
}

bool ScreenManager::isHigher(int desktop1, int desktop2)
{
    if (m_applicationDesktop->screenGeometry(desktop1).top() < m_applicationDesktop->screenGeometry(desktop2).top())
    {
        return true;
    }
    return false;
}

bool ScreenManager::isLower(int desktop1, int desktop2)
{
    if (m_applicationDesktop->screenGeometry(desktop1).top() > m_applicationDesktop->screenGeometry(desktop2).top())
    {
        return true;
    }
    return false;
}

bool ScreenManager::isMoreOnLeft(int desktop1, int desktop2)
{
    if (m_applicationDesktop->screenGeometry(desktop1).left() < m_applicationDesktop->screenGeometry(desktop2).left())
    {
        return true;
    }
    return false;
}

bool ScreenManager::isMoreOnRight(int desktop1, int desktop2)
{
    if (m_applicationDesktop->screenGeometry(desktop1).right() > m_applicationDesktop->screenGeometry(desktop2).right())
    {
        return true;
    }
    return false;
}

int ScreenManager::whoIsLeft(int desktopIAm)
{
    for (int i = 0; i < m_applicationDesktop->numScreens(); i++)
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
    for (int i = 0; i < m_applicationDesktop->numScreens(); i++)
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
    for (int i = 0; i < m_applicationDesktop->numScreens(); i++)
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

    for (int i = 0; i < m_applicationDesktop->numScreens(); i++)
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
    int x = m_applicationDesktop->availableGeometry(screens[0]).left();
    for (int i = 1; i < screens.count(); i++)
    {
        x = std::max(x, m_applicationDesktop->availableGeometry(screens[i]).left());
    }
    // El mateix per la part superior
    screens = dynamicMatrix.getTopRow();
    int y = m_applicationDesktop->availableGeometry(screens[0]).top();
    for (int i = 1; i < screens.count(); i++)
    {
        y = std::max(y, m_applicationDesktop->availableGeometry(screens[i]).top());
    }

    return QPoint(x, y);
}

QPoint ScreenManager::getBottomRight(const DynamicMatrix &dynamicMatrix) const
{

    // Primer de tot buscar la cantonada dreta, a partir de la llista de monitors a lla dreta,
    // agafar el mínim, per si la barra de windows esta a la dreta en algun d'ells
    QList<int> screens = dynamicMatrix.getRightColumn();
    int x = m_applicationDesktop->availableGeometry(screens[0]).right();
    for (int i = 1; i < screens.count(); i++)
    {
        x = std::min(x, m_applicationDesktop->availableGeometry(screens[i]).right());
    }
    // El mateix per la part de baix
    screens = dynamicMatrix.getBottomRow();
    int y = m_applicationDesktop->availableGeometry(screens[0]).bottom();
    for (int i = 1; i < screens.count(); i++)
    {
        y = std::min(y, m_applicationDesktop->availableGeometry(screens[i]).bottom());
    }

    return QPoint(x, y);
}

} // End namespace udg
