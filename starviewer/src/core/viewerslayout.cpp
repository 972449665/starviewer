#include "viewerslayout.h"

#include "logging.h"

namespace udg {

ViewersLayout::ViewersLayout(QWidget *parent)
 : QWidget(parent), m_selectedViewer(0)
{
    initLayouts();
}

ViewersLayout::~ViewersLayout()
{
}

void ViewersLayout::initLayouts()
{
    m_visibleRows = 0;
    m_visibleColumns = 0;
    m_totalRows = 0;
    m_totalColumns = 0;

    m_regularViewersGridLayout = new QGridLayout();
    m_regularViewersGridLayout->setSpacing(0);
    m_regularViewersGridLayout->setMargin(0);

    this->setLayout(m_regularViewersGridLayout);

    m_isRegular = true;
}

Q2DViewerWidget* ViewersLayout::getSelectedViewer()
{
    return m_selectedViewer;
}

Q2DViewerWidget* ViewersLayout::getNewQ2DViewerWidget()
{
    Q2DViewerWidget *newViewer = new Q2DViewerWidget(this);
    connect(newViewer, SIGNAL(selected(Q2DViewerWidget*)), SLOT(setSelectedViewer(Q2DViewerWidget*)));
    connect(newViewer, SIGNAL(manualSynchronizationStateChanged(bool)), SIGNAL(manualSynchronizationStateChanged(bool)));
    // Per defecte no li posem cap annotació
    newViewer->getViewer()->removeAnnotation(AllAnnotation);

    emit viewerAdded(newViewer);
    return newViewer;
}

void ViewersLayout::deleteQ2DViewerWidget(Q2DViewerWidget *viewer)
{
    // TODO: Xapussilla que cal fer ja que si es fa un "delete viewer" directament peta al canviar d'HP i cal fer-li un deleteLater.
    // Si es fa un deleteLater i prou els viewer no s'esborren fins que s'han creat els nous. Això faria que hi hagi un moment en que
    // tindríem la memòria acumulada dels viewers vells i els nous (això inclou la memòria de textura que allotja el renderer). Això podria provar
    // que ens en quedessim sense si es tracten d'imatges grans. Per això fem un delete del viewer i després el deleteLater perquè no peti.
    emit viewerRemoved(viewer);
    delete viewer->getViewer();
    viewer->deleteLater();
}

void ViewersLayout::addColumns(int columns)
{
    while (columns > 0)
    {
        if (m_totalRows == 0)
        {
            m_totalRows = 1;
            m_visibleRows = 1;
        }

        int viewerPosition = m_visibleColumns;
        ++m_visibleColumns;
        ++m_totalColumns;
        for (int rows = 0; rows < m_totalRows; ++rows)
        {
            // Si tenim visors creats anteriorment amb geometries,
            // els aprofitem, sinó en creem de nous
            Q2DViewerWidget *newViewer;
            if (!m_freeLayoutViewersList.isEmpty())
            {
                newViewer = m_freeLayoutViewersList.takeFirst();
            }
            else
            {
                newViewer = getNewQ2DViewerWidget();
            }

            m_regularViewersGridLayout->addWidget(newViewer, rows, m_totalColumns - 1);
            showViewer(newViewer);
            m_regularViewersGridVector.insert(viewerPosition, newViewer);
            viewerPosition += m_visibleColumns;
            if (rows >= m_visibleRows)
            {
                hideViewer(newViewer);
            }
        }
        --columns;
    }
}

void ViewersLayout::addRows(int rows)
{
    while (rows > 0)
    {
        if (m_totalColumns == 0)
        {
            m_totalColumns = 1;
            m_visibleColumns = 1;
        }
        m_visibleRows += 1;
        m_totalRows += 1;
        // Afegim tants widgets com columnes
        for (int column = 0; column < m_totalColumns; ++column)
        {
            // Si tenim visors creats anteriorment amb geometries,
            // els aprofitem, sinó en creem de nous
            Q2DViewerWidget *newViewer;
            if (!m_freeLayoutViewersList.isEmpty())
            {
                newViewer = m_freeLayoutViewersList.takeFirst();
            }
            else
            {
                newViewer = getNewQ2DViewerWidget();
            }
            m_regularViewersGridLayout->addWidget(newViewer, m_visibleRows - 1, column);
            showViewer(newViewer);
            m_regularViewersGridVector.push_back(newViewer);
            if (column >= m_visibleColumns)
            {
                hideViewer(newViewer);
            }
        }
        --rows;
    }
}

void ViewersLayout::removeColumns(int columns)
{
    while (columns > 0 && m_visibleColumns > 1)
    {
        int viewerPosition = m_visibleColumns - 1;
        // Eliminem un widget de cada fila per tenir una columna menys
        for (int rows = 0; rows < m_regularViewersGridLayout->rowCount(); ++rows)
        {
            Q2DViewerWidget *oldViewer = getViewerWidget(viewerPosition);
            m_regularViewersGridLayout->removeWidget(oldViewer);
            m_regularViewersGridVector.remove(viewerPosition);
            if (m_selectedViewer == oldViewer)
            {
                setSelectedViewer(getViewerWidget(0));
            }
            this->deleteQ2DViewerWidget(oldViewer);
            viewerPosition += (m_visibleColumns - 1);
        }
        --m_visibleColumns;
        --columns;
    }
}

void ViewersLayout::removeRows(int rows)
{
    int viewerPosition = m_regularViewersGridVector.count() - 1;

    while (rows > 0 && m_visibleRows > 1)
    {
        // Eliminem tants widgets com columnes
        for (int i = 0; i < m_visibleColumns; ++i)
        {
            Q2DViewerWidget *oldViewer = getViewerWidget(viewerPosition);
            m_regularViewersGridVector.remove(viewerPosition);
            m_regularViewersGridLayout->removeWidget(oldViewer);
            // TODO eliminar els viewers que treiem del toolManager???
            if (m_selectedViewer == oldViewer)
            {
                setSelectedViewer(getViewerWidget(0));
            }
            this->deleteQ2DViewerWidget(oldViewer);
            --viewerPosition;
        }
        --m_visibleRows;
        --rows;
    }
}

void ViewersLayout::setGrid(int rows, int columns)
{
    if (!m_isRegular)
    {
        // Amaguem els viewers que tinguem assignats amb geometries
        for (int i = rows * columns; i < m_freeLayoutViewersList.size(); ++i)
        {
            hideViewer(m_freeLayoutViewersList.at(i));
        }
        m_totalRows = 0;
        m_totalColumns = 0;
        m_visibleColumns = 0;
        m_visibleRows = 0;
    }

    // Mirem si les tenim amagades i mostrem totes les necessaries
    int windowsToShow = 0;
    int windowsToCreate = 0;

    if (columns > m_visibleColumns)
    {
        int windowsToHide = m_totalColumns - m_visibleColumns;

        if (windowsToHide < (columns - m_visibleColumns))
        {
            windowsToShow = windowsToHide;
        }
        else
        {
            windowsToShow = columns - m_visibleColumns;
        }

        showColumns(windowsToShow);

        if (columns > m_totalColumns)
        {
            windowsToCreate = columns - m_totalColumns;
        }

        addColumns(windowsToCreate);
    }
    else if (columns < m_visibleColumns)
    {
        hideColumns(m_visibleColumns - columns);
    }

    windowsToShow = 0;
    windowsToCreate = 0;

    if (rows > m_visibleRows)
    {
        int windowsToHide = m_totalRows - m_visibleRows;

        if (windowsToHide < (rows - m_visibleRows))
        {
            windowsToShow = windowsToHide;
        }
        else
        {
            windowsToShow = rows - m_visibleRows;
        }

        showRows(windowsToShow);

        if (rows > m_totalRows)
        {
            windowsToCreate = rows - m_totalRows;
        }

        addRows(windowsToCreate);
    }
    else if (rows < m_visibleRows)
    {
        hideRows(m_visibleRows - rows);
    }

    m_isRegular = true;
}

Q2DViewerWidget* ViewersLayout::addViewer(const QString &geometry)
{
    if (m_isRegular)
    {
        // Si tenim un grid regular, hauríem de fer un  cleanUp() prèviament
        // per poder afegir els viewers amb geometries
        cleanUp();
    }

    Q2DViewerWidget *newViewer = getNewQ2DViewerWidget();
    m_freeLayoutViewersList << newViewer;

    setViewerGeometry(newViewer, geometry);
    newViewer->show();

    m_geometriesList << geometry;
    m_isRegular = false;

    return newViewer;
}

void ViewersLayout::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    if (!m_isRegular)
    {
        for (int i = 0; i < m_geometriesList.size(); ++i)
        {
            setViewerGeometry(m_freeLayoutViewersList.at(i), m_geometriesList.at(i));
        }
    }
}

void ViewersLayout::setSelectedViewer(Q2DViewerWidget *viewer)
{
    if (!viewer)
    {
        // Si "viewer" és nul, llavors entenem que no volem cap de seleccionat
        m_selectedViewer = 0;
        emit selectedViewerChanged(m_selectedViewer);
        return;
    }

    if (viewer != m_selectedViewer)
    {
        if (m_selectedViewer)
        {
            m_selectedViewer->setSelected(false);
        }
        m_selectedViewer = viewer;
        m_selectedViewer->setSelected(true);
        emit selectedViewerChanged(m_selectedViewer);
    }
}

void ViewersLayout::cleanUp()
{
    // No hi ha cap visor seleccionat
    setSelectedViewer(0);

    // Eliminem tots els widgets que contingui viewers layout
    // i els propis widgets
    for (int i = 0; i < getNumberOfViewers(); ++i)
    {
        Q2DViewerWidget *viewer = getViewerWidget(i);
        m_regularViewersGridLayout->removeWidget(viewer);
        this->deleteQ2DViewerWidget(viewer);
    }
    // Eliminem els visors i les geometries
    m_regularViewersGridVector.clear();
    m_freeLayoutViewersList.clear();
    m_geometriesList.clear();

    /// Reinicialitzem la resta de variables
    m_visibleRows = 0;
    m_visibleColumns = 0;
    m_totalRows = 0;
    m_totalColumns = 0;
    m_isRegular = false;
}

void ViewersLayout::showRows(int rows)
{
    while (rows > 0)
    {
        for (int columnNumber = 0; columnNumber < m_visibleColumns; ++columnNumber)
        {
            Q2DViewerWidget *viewer = getViewerWidget((m_totalColumns * m_visibleRows) + columnNumber);
            showViewer(viewer);
        }
        ++m_visibleRows;
        --rows;
    }
}

void ViewersLayout::hideRows(int rows)
{
    while (rows > 0)
    {
        --m_visibleRows;
        for (int columnNumber = 0; columnNumber < m_visibleColumns; ++columnNumber)
        {
            Q2DViewerWidget *viewer = getViewerWidget(((m_totalColumns * m_visibleRows) + columnNumber));
            hideViewer(viewer);
            if (m_selectedViewer == viewer)
            {
                setSelectedViewer(getViewerWidget(0));
            }
        }
        --rows;
    }
}

void ViewersLayout::showColumns(int columns)
{
    while (columns > 0)
    {
        for (int rowNumber = 0; rowNumber < m_visibleRows; ++rowNumber)
        {
            Q2DViewerWidget *viewer = getViewerWidget((m_totalColumns * rowNumber) + m_visibleColumns);
            showViewer(viewer);
        }
        ++m_visibleColumns;
        --columns;
    }
}

void ViewersLayout::hideColumns(int columns)
{
    while (columns > 0)
    {
        --m_visibleColumns;
        for (int rowNumber = 0; rowNumber < m_visibleRows; ++rowNumber)
        {
            Q2DViewerWidget *viewer = getViewerWidget((m_totalColumns * rowNumber) + m_visibleColumns);
            hideViewer(viewer);
            if (m_selectedViewer == viewer)
            {
                setSelectedViewer(getViewerWidget(0));
            }
        }
        --columns;
    }
}

int ViewersLayout::getNumberOfViewers() const
{
    if (m_isRegular)
    {
        return m_regularViewersGridVector.size() + m_freeLayoutViewersList.size();
    }
    else
    {
        return m_freeLayoutViewersList.size();
    }
}

Q2DViewerWidget* ViewersLayout::getViewerWidget(int number)
{
    Q2DViewerWidget *viewerWidget = 0;

    // Comprovem que el viewer demanat estigui dins del rang
    if (number < getNumberOfViewers() && number >= 0)
    {
        if (m_isRegular)
        {
            if (number < m_regularViewersGridVector.size())
            {
                viewerWidget = m_regularViewersGridVector.at(number);
            }
            else
            {
                viewerWidget = m_freeLayoutViewersList.at(number - m_regularViewersGridVector.size());
            }
        }
        else
        {
            viewerWidget = m_freeLayoutViewersList.at(number);
        }
    }

    return viewerWidget;
}

bool ViewersLayout::isRegular() const
{
    return m_isRegular;
}

int ViewersLayout::getVisibleColumns() const
{
    return m_visibleColumns;
}

int ViewersLayout::getVisibleRows() const
{
    return m_visibleRows;
}

void ViewersLayout::setViewerGeometry(Q2DViewerWidget *viewer, const QString &geometry)
{
    if (!viewer)
    {
        DEBUG_LOG("El viewer proporcionat és nul! No s'aplicarà cap geometria.");
        return;
    }

    QStringList splittedGeometryList = geometry.split("\\");
    if (splittedGeometryList.count() < 4)
    {
        DEBUG_LOG("La geometria proporcionada no conté el nombre d'elements necessaris o està mal formada. Geometry dump: [" +
                  geometry + "]. No s'aplicarà cap geometria al viewer proporcinat.");
        WARN_LOG("La geometria proporcionada no conté el nombre d'elements necessaris o està mal formada. Geometry dump: [" +
                 geometry + "]. No s'aplicarà cap geometria al viewer proporcinat.");
        return;
    }

    double x1;
    double y1;
    double x2;
    double y2;
    x1 = splittedGeometryList.at(0).toDouble();
    y1 = splittedGeometryList.at(1).toDouble();
    x2 = splittedGeometryList.at(2).toDouble();
    y2 = splittedGeometryList.at(3).toDouble();

    int screenX = this->width();
    int screenY = this->height();
    viewer->setGeometry(x1 * screenX, (1 - y1) * screenY, (x2 - x1) * screenX, (y1 - y2) * screenY);
}

void ViewersLayout::hideViewer(Q2DViewerWidget *viewer)
{
    if (viewer)
    {
        viewer->hide();
        emit viewerHidden(viewer);
    }
}

void ViewersLayout::showViewer(Q2DViewerWidget *viewer)
{
    if (viewer)
    {
        viewer->show();
        emit viewerShown(viewer);
    }
}

}
