#include "qserieslistwidget.h"

#include <QString>
#include <QIcon>

#include "series.h"

namespace udg {

QSeriesListWidget::QSeriesListWidget(QWidget *parent)
 : QWidget(parent)
{
    setupUi(this);
    QSize size;

    //Definim la mida de la imatge que mostrem
    size.setHeight(scaledSeriesSizeY);
    size.setWidth(scaledSeriesSizeX);
    m_seriesListWidget->setIconSize(size);

    createConnections();

    m_nonDicomImageSeriesList << "KO" << "PR" << "SR";
    m_lastInsertedImageRow = -1;
}

void QSeriesListWidget::createConnections()
{
    connect(m_seriesListWidget, SIGNAL(itemClicked (QListWidgetItem *)), SLOT(clicked(QListWidgetItem *)));
    connect(m_seriesListWidget, SIGNAL(itemDoubleClicked (QListWidgetItem *)), SLOT(view(QListWidgetItem *)));
}

void QSeriesListWidget::insertSeries(QString studyInstanceUID, Series *series)
{
    QString text, num;
    QListWidgetItem *item = new QListWidgetItem();
    QString statusTip;

    text = tr(" Series ") + series->getSeriesNumber();
    if (series->getProtocolName().length() > 0)
    {//si hi ha descripció la inserim
        text += " " + series->getProtocolName();
    }
    text +="\n";

    if (series->getNumberOfImages() > 0)
    {
        text += QString::number(series->getNumberOfImages());
        QString modality = series->getModality();
        if (modality == "KO")
            text += tr(" Key Object Note");
        else if (modality == "PR")
            text += tr(" Presentation State");
        else if (modality == "SR")
            text += tr(" Structured Report");
        else
            text += tr(" Images");
    }

    QIcon icon(series->getThumbnail());

    item->setText(text);
    item->setIcon(icon);
    item->setToolTip(text);

    /* A l'status Tip de cada item es guarda la UID de la serie, ja que aquest camp no el vull mostrar i no tinc
   enlloc per amagar-lo, ho utilitzo per identificar la sèrie */
    item->setStatusTip(series->getInstanceUID());

    m_HashSeriesStudy[series->getInstanceUID()] = studyInstanceUID;//Guardem per la sèrie a quin estudi pertany

    //TODO s'hauria de millorar el sistema d'ordenació de les sèries
    //Comprovem la posició que hem d'inserir la sèrie, si és un DICOM Non-Image (no és una imatge) val final, sinó va després de la última imatge inserida
    if (m_nonDicomImageSeriesList.contains(series->getModality()))
    {
        m_seriesListWidget->addItem(item);
    }
    else
    {//és una imatge
        m_lastInsertedImageRow++;
        m_seriesListWidget->insertItem(m_lastInsertedImageRow, item);
    }
}

void QSeriesListWidget::setCurrentSeries(const QString &seriesUID)
{
    int index = 0;
    bool stop = false;
    QList<QListWidgetItem *> llistaSeries = m_seriesListWidget->findItems ("", Qt::MatchContains);

    while (!stop && index < llistaSeries.count())
    {
        if (llistaSeries.at(index)->statusTip() == seriesUID)
        {
            stop = true;
        }
        else index++;
    }
    if (stop) m_seriesListWidget->setCurrentItem(llistaSeries.at(index));
}

void QSeriesListWidget::removeSeries(const QString &seriesInstanceUID)
{
    int index = 0;
    bool stop = false;
    QList<QListWidgetItem *> llistaSeries = m_seriesListWidget->findItems("", Qt::MatchContains);

    while (!stop && index < llistaSeries.count())
    {
        if (llistaSeries.at(index)->statusTip() == seriesInstanceUID)
        {
            stop = true;
        }
        else index++;
    }

    if (stop) delete llistaSeries.at(index);
}

void QSeriesListWidget::clicked(QListWidgetItem *item)
{
    if (item != NULL) emit(selectedSeriesIcon(item->statusTip()));
}

void QSeriesListWidget::view(QListWidgetItem *item)
{
    if (item != NULL) emit(viewSeriesIcon());
}

QString QSeriesListWidget::getCurrentSeriesUID()
{
    return m_seriesListWidget->currentItem()->statusTip();
}

QString QSeriesListWidget::getCurrentStudyUID()
{
    return m_HashSeriesStudy[getCurrentSeriesUID()];
}

void QSeriesListWidget::clear()
{
    m_seriesListWidget->clear();
    m_HashSeriesStudy.clear();

    m_lastInsertedImageRow = -1;//Indiquem que la última imatge insertada està a la posició 0 perquè hem un clear
}

QSeriesListWidget::~QSeriesListWidget()
{
}

};
