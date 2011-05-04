#include "qstudytreewidget.h"

#include <QContextMenuEvent>

#include "logging.h"
#include "patient.h"
#include "study.h"
#include "series.h"
#include "image.h"
#include "dicommask.h"

namespace udg {

QStudyTreeWidget::QStudyTreeWidget(QWidget *parent)
 : QWidget(parent)
{
    setupUi(this);

    m_studyTreeView->setColumnHidden(Type, true);
    //Amaguem la columna Hora, ja que ara es mostra la data i hora en un mateix columna per poder ordenar per data i hora els estudis
    m_studyTreeView->setColumnHidden(Time, true);

    //carreguem les imatges que es mostren el QStudyTreeWidget
    m_openFolder = QIcon(":/images/folderopen.png");
    m_closeFolder = QIcon(":/images/folderclose.png");
    m_iconSeries = QIcon(":/images/series.png");

    createConnections();

    m_studyTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    //Indiquem que el nivell màxim que per defecte es pot expedir l'arbre Study/Series/Image és fins a nivell d'Image
    m_maximumExpandTreeItemsLevel = ImageLevel;
}

QStudyTreeWidget::~QStudyTreeWidget()
{
}

void QStudyTreeWidget::createConnections()
{
    connect(m_studyTreeView, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), SLOT(doubleClicked(QTreeWidgetItem *, int)));
    connect(m_studyTreeView, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), SLOT (currentItemChanged (QTreeWidgetItem *, QTreeWidgetItem *)));
    connect(m_studyTreeView, SIGNAL(itemExpanded(QTreeWidgetItem *)), SLOT (itemExpanded(QTreeWidgetItem *)));
    connect(m_studyTreeView, SIGNAL(itemCollapsed(QTreeWidgetItem *)), SLOT (itemCollapsed(QTreeWidgetItem *)));
}

QTreeWidget *QStudyTreeWidget::getQTreeWidget() const
{
    return m_studyTreeView;
}

QList<DicomMask> QStudyTreeWidget::getDicomMaskOfSelectedItems()
{
    QList<DicomMask> dicomMaskList;
    QList<QTreeWidgetItem *> selectedItems = m_studyTreeView->selectedItems();

    foreach (QTreeWidgetItem *item, selectedItems)
    {
        DicomMask dicomMask;
        if (isItemStudy(item)) //es un estudi
        {
            dicomMask.setStudyInstanceUID(item->text(UID));
        }
        else if (isItemSeries(item))
        {
            dicomMask.setStudyInstanceUID(item->parent()->text(UID));
            dicomMask.setSeriesInstanceUID(item->text(UID));
        }
        else if (isItemImage(item))
        {
            dicomMask.setStudyInstanceUID(item->parent()->parent()->text(UID));
            dicomMask.setSeriesInstanceUID(item->parent()->text(UID));
            dicomMask.setSOPInstanceUID(item->text(UID));
        }
        else
        {
            DEBUG_LOG("Texte no esperat: " + item->text(Type));
        }

        dicomMaskList.append(dicomMask);
    }

    return dicomMaskList;
}

void QStudyTreeWidget::insertPatientList(QList<Patient*> patientList)
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    foreach (Patient *patient, patientList)
    {
        if (patient->getNumberOfStudies() > 0)
        {
            m_studyTreeView->addTopLevelItems(fillPatient(patient));
        }
    }

    m_studyTreeView->clearSelection();

    QApplication::restoreOverrideCursor();
}

void QStudyTreeWidget::insertPatient(Patient* patient)
{
    if (patient->getNumberOfStudies() > 0)
    {
        if (getStudyQTreeWidgetItem(patient->getStudies().at(0)->getInstanceUID()) != NULL)
        {
            //si l'estudi ja hi existeix a StudyTreeView l'esborrem
            removeStudy(patient->getStudies().at(0)->getInstanceUID());
        }

        m_studyTreeView->addTopLevelItems(fillPatient(patient));
        m_studyTreeView->clearSelection();
    }
}

QList<QTreeWidgetItem*> QStudyTreeWidget::fillPatient(Patient *patient)
{
    QList<QTreeWidgetItem*> qtreeWidgetItemList;

    foreach (Study *studyToInsert, patient->getStudies())
    {
        QTreeWidgetItem *item = new QTreeWidgetItem();

        item->setIcon(ObjectName, m_closeFolder);
        item->setText(ObjectName, patient->getFullName());
        item->setText(PatientID, patient->getID());
        item->setText(PatientAge, formatAge(studyToInsert->getPatientAge()));
        item->setText(Modality, studyToInsert->getModalitiesAsSingleString());
        item->setText(Description, studyToInsert->getDescription());
        //TODO:No hauria de ser l'estudi que tornés la data formatada? Problema necessitem que la data estigui en format yyyy/mm/dd per poder ordenar per data
        item->setText(Date, formatDateTime(studyToInsert->getDate(), studyToInsert->getTime()));
        item->setText(StudyID, tr("Study %1").arg(studyToInsert->getID()));
        item->setText(Institution, studyToInsert->getInstitutionName());
        item->setText(AccNumber, studyToInsert->getAccessionNumber());
        item->setText(UID, studyToInsert->getInstanceUID());
        item->setText(Type, "STUDY");//indiquem de que es tracta d'un estudi
        item->setText(RefPhysName, studyToInsert->getReferringPhysiciansName());

        //Comprovem si el TreeItem s'ha d'expandir en funció del nivell màxim que ens han indicat que ens podem expandir
        if (m_maximumExpandTreeItemsLevel > StudyLevel)
        {
            /* degut que per cada item estudi tenim items fills que són series, i que consultar les series per cada estudi és
               una operació costosa (per exemple quan es consulta al pacs) només inserirem les sèries per a que les pugui
               consultar l'usuari quan es facin un expand d'estudi, però per a que apareixi el botó "+" de desplegar l'estudi inserim un item en blanc
             */
            QTreeWidgetItem *expandableItem = new QTreeWidgetItem();

            expandableItem->setText(Type, "EXPANDABLE_ITEM");
            item->addChild(expandableItem);
        }
        qtreeWidgetItemList.append(item);

        //Inserim l'estudi a la llista d'estudis
        m_insertedStudyList.append(studyToInsert);
    }

    return qtreeWidgetItemList;
}

void QStudyTreeWidget::insertSeriesList(QString studyInstanceUID, QList<Series*> seriesList)
{
    QTreeWidgetItem *studyItem = getStudyQTreeWidgetItem(studyInstanceUID);
    QList<QTreeWidgetItem*> qTreeWidgetItemSeriesList;

    foreach (Series *series, seriesList)
    {
        qTreeWidgetItemSeriesList.append(fillSeries(series));
    }
    studyItem->addChildren(qTreeWidgetItemSeriesList);
}

QTreeWidgetItem* QStudyTreeWidget::fillSeries(Series *series)
{
    QTreeWidgetItem *seriesItem = new QTreeWidgetItem();

    seriesItem->setIcon(ObjectName, m_iconSeries);
    //Li fem un padding per poder ordenar la columna, ja que s'ordena per String
    seriesItem->setText(ObjectName, tr("Series %1").arg(paddingLeft(series->getSeriesNumber(), 4)));
    seriesItem->setText(Modality, series->getModality());

    seriesItem->setText(Description, series->getDescription().simplified());//treiem els espaics en blanc del davant i darrera

    seriesItem->setText(Date, formatDateTime(series->getDate(), series->getTime()));
    seriesItem->setText(UID, series->getInstanceUID());
    seriesItem->setText(Type, "SERIES"); //indiquem que es tracta d'una sèrie

    seriesItem->setText(ProtocolName, series->getProtocolName());
    seriesItem->setText(PPStartDate, series->getPerformedProcedureStepStartDate());
    seriesItem->setText(PPStartTime, series->getPerformedProcedureStepStartTime());
    seriesItem->setText(ReqProcID, series->getRequestedProcedureID());
    seriesItem->setText(SchedProcStep, series->getScheduledProcedureStepID());

    //Comprovem si el TreeItem s'ha d'expandir en funció del nivell màxim que ens han indicat que ens podem expandir
    if (m_maximumExpandTreeItemsLevel > SeriesLevel)
    {
        /* degut que per cada item serie tenim items fills que són imatges, i que consultar les imatges per cada sèrie és
        una operació costosa (per exemple quan es consulta al pacs) només inserirem les sèries per a que les pugui
        consultar l'usuari quan es facin un expand de la sèrie, però per a que apareixi el botó "+" de desplegar la sèrie inserim un item en blanc
        */
        QTreeWidgetItem *expandableItem = new QTreeWidgetItem();

        expandableItem->setText(Type, "EXPANDABLE_ITEM");
        seriesItem->addChild(expandableItem);
    }
    return seriesItem;
}

void QStudyTreeWidget::insertImageList(QString studyInstanceUID, QString seriesInstanceUID, QList<Image*> imageList)
{
    QTreeWidgetItem *newImageItem, *seriesItem = getSeriesQTreeWidgetItem(studyInstanceUID, seriesInstanceUID);
    QList<QTreeWidgetItem*> qTreeWidgetItemImageList;

    if (seriesItem != NULL)
    {
        foreach (Image *image, imageList)
        {
            newImageItem = new QTreeWidgetItem();

            newImageItem->setIcon(ObjectName, m_iconSeries);
            newImageItem->setText(ObjectName, tr("File %1").arg(paddingLeft(image->getInstanceNumber(), 4)));//Li fem un padding per poder ordenar la columna, ja que s'ordena per String
            newImageItem->setText(UID, image->getSOPInstanceUID());
            newImageItem->setText(Type, "IMAGE"); //indiquem que es tracta d'una imatge
            qTreeWidgetItemImageList.append(newImageItem);
        }
        //Afegim la llista d'imatges
        seriesItem->addChildren(qTreeWidgetItemImageList);
    }
    else DEBUG_LOG("NO S'HA POGUT TROBAR LA SERIE A LA QUE S'HAVIA D'INSERIR LA IMATGE");
}

void QStudyTreeWidget::setMaximumExpandTreeItemsLevel(QStudyTreeWidget::ItemTreeLevels maximumExpandTreeItemsLevel)
{
    m_maximumExpandTreeItemsLevel = maximumExpandTreeItemsLevel;
}

QStudyTreeWidget::ItemTreeLevels QStudyTreeWidget::getMaximumExpandTreeItemsLevel()
{
    return m_maximumExpandTreeItemsLevel;
}

QString QStudyTreeWidget::formatAge(const QString age)
{
    QString text(age);

    if (text.length() > 0)
    {
        if (text.at(0) == '0')
        {//treiem el 0 de davant els anys, el PACS envia per ex: 047Y nosaltes tornem 47Y
            text.replace(0, 1, " ");
        }
    }

    return text;
}

QString QStudyTreeWidget::formatDateTime(const QDate &date, const QTime &time)
{
    if (!date.isNull() && !time.isNull())
    {
        return date.toString(Qt::ISODate) + "   " + time.toString(Qt::ISODate);
    }
    else if (!date.isNull())
    {
        return date.toString(Qt::ISODate);
    }
    else
    {
        return "";
    }
}

void QStudyTreeWidget::clear()
{
    m_studyTreeView->clear();
    //Reinicialitzem variables
    m_oldCurrentStudyUID = "";
    m_oldCurrentSeriesUID = "";

    m_insertedStudyList.clear();
}

void QStudyTreeWidget::setSortByColumn(QStudyTreeWidget::ColumnIndex col, Qt::SortOrder sortOrder)
{
    m_studyTreeView->sortItems(col, sortOrder);
    m_studyTreeView->clearSelection();
}

QStudyTreeWidget::ColumnIndex QStudyTreeWidget::getSortColumn()
{
    return (QStudyTreeWidget::ColumnIndex) m_studyTreeView->header()->sortIndicatorSection();;
}

Qt::SortOrder QStudyTreeWidget::getSortOrderColumn()
{
    return (Qt::SortOrder) m_studyTreeView->header()->sortIndicatorOrder();
}

QString QStudyTreeWidget::getCurrentStudyUID()
{
    if (m_studyTreeView->currentItem() != NULL)
    {
        if (isItemStudy(m_studyTreeView->currentItem())) //es un estudi
        {
            return m_studyTreeView->currentItem()->text(UID);
        }
        else if (isItemSeries(m_studyTreeView->currentItem()))
        {
            return m_studyTreeView->currentItem()->parent()->text(UID);
        }
        else if (isItemImage(m_studyTreeView->currentItem()))
        {
            return m_studyTreeView->currentItem()->parent()->parent()->text(UID);
        }
        else return "";
    }
    else return "";
}

QStringList QStudyTreeWidget::getSelectedStudiesUID()
{
    QStringList result;
    QList<QTreeWidgetItem *> selectedItems = m_studyTreeView->selectedItems();
    foreach (QTreeWidgetItem *item, selectedItems)
    {
        if (isItemStudy(item)) //es un estudi
        {
            if (!result.contains(item->text(UID)))
                result << item->text(UID);
        }
        else if (isItemSeries(item))
        {
            if (!result.contains(item->parent()->text(UID)))
                result << item->parent()->text(UID);
        }
        else if (isItemImage(item))
        {
            if (!result.contains(item->parent()->parent()->text(UID)))
                result << item->parent()->parent()->text(UID);
        }
        else
        {
            DEBUG_LOG("Texte no esperat: " + item->text(Type));
        }
    }
    return result;
}

QList<Study*> QStudyTreeWidget::getSelectedStudies()
{
    QStringList selectedStudiesUID = getSelectedStudiesUID();
    QList<Study*> selectedStudies;

   //Busquem pels estudis UID seleccionats
    foreach (Study *studyInserted, m_insertedStudyList)
    {
        if (selectedStudiesUID.contains(studyInserted->getInstanceUID()))
        {
            selectedStudies.append(studyInserted);
        }
    }

    return selectedStudies;
}

Study* QStudyTreeWidget::getStudy(QString studyInstanceUID)
{
    Study *study = NULL;

   //Busquem pels estudis UID seleccionats
    foreach (Study *studyInserted, m_insertedStudyList)
    {
        if (studyInserted->getInstanceUID() == studyInstanceUID)
        {
            study = studyInserted;
            break;
        }
    }

    return study;
}

QString QStudyTreeWidget::getCurrentSeriesUID()
{
    if (m_studyTreeView->currentItem() != NULL)
    {
        if (isItemSeries(m_studyTreeView->currentItem()))
        {
            return m_studyTreeView->currentItem()->text(UID);
        }
        else if (isItemImage(m_studyTreeView->currentItem()))
        {
            return m_studyTreeView->currentItem()->parent()->text(UID);
        }
        else return "";
    }
    else return "";
}

QTreeWidgetItem* QStudyTreeWidget::getStudyQTreeWidgetItem(QString studyUID)
{
    QList<QTreeWidgetItem*> qStudyList(m_studyTreeView->findItems(studyUID, Qt::MatchExactly, UID));//busquem l'estudi a la que pertany la sèrie

    //TODO d'aquesta manera podríem tenir problemes si tenim un StudyUID duplicat
    if (qStudyList.count() > 0)
    {
        return qStudyList.at(0);
    }
    else return NULL;
}

QTreeWidgetItem* QStudyTreeWidget::getSeriesQTreeWidgetItem(QString studyInstanceUID, QString seriesInstanceUID)
{
    QTreeWidgetItem* studyItem, *item = NULL;
    int index = 0;

    studyItem = getStudyQTreeWidgetItem(studyInstanceUID);

    while (item == NULL && index < studyItem->childCount())//cerquem la sèrie de la que depen la imatge
    {
        if (studyItem->child(index)->text(UID) == seriesInstanceUID)
        {
            item = studyItem->child(index);
        }
        else index++;
    }

    return item;
}

QString QStudyTreeWidget::getCurrentImageUID()
{
    QString result;

    if (m_studyTreeView->currentItem() != NULL)
    {
        if (isItemImage(m_studyTreeView->currentItem()))
        {
            result = m_studyTreeView->currentItem()->text(UID);
        }
    }

    return result;
}

void QStudyTreeWidget::removeStudy(QString studyInstanceUIDToRemove)
{
    QList<QTreeWidgetItem *> qStudyList(m_studyTreeView->findItems(studyInstanceUIDToRemove, Qt::MatchExactly, UID));
    QTreeWidgetItem *item;
    int indexOfStudy = 0;

    for (int i = 0; i < qStudyList.count(); i++)
    {
        item = qStudyList.at(i);
        delete item;
    }

    while (indexOfStudy < m_insertedStudyList.count())
    {
        if (m_insertedStudyList.at(indexOfStudy)->getInstanceUID() == studyInstanceUIDToRemove)
        {
            m_insertedStudyList.removeAt(indexOfStudy);
        }
        indexOfStudy++;
    }

    m_studyTreeView->clearSelection();
}

void QStudyTreeWidget::removeSeries(const QString &studyInstanceUID, const QString &seriesInstanceUID)
{
    foreach (QTreeWidgetItem *studyItem, m_studyTreeView->findItems(studyInstanceUID, Qt::MatchExactly, UID))
    {
        bool seriesDeleted = false;

        for (int index = 0; index < studyItem->childCount(); index++)
        {
            if (studyItem->child(index)->text(UID) == seriesInstanceUID)
            {
                delete studyItem->takeChild(index);
                seriesDeleted = true;
            }
        }

        if (studyItem->childCount() == 0 && seriesDeleted)
        {
            /*Si després d'esborrar la sèrie l'estudi no té cap fill esborrem el QTreeWidgeItem estudi, no té sentit tenir un estudi sol
              si no té series*/
            delete studyItem;
        }
    }

    m_studyTreeView->clearSelection();
}

void QStudyTreeWidget::setCurrentSeries(QString seriesUID)
{
    //busquem l'estudi a la que pertany la sèrie
    QList<QTreeWidgetItem*> qStudyList(m_studyTreeView->findItems(seriesUID, Qt::MatchRecursive, UID));

    //Només hauria de trobar una sèrie amb el mateix UID, sinó tindrem una inconsistència de DICOM, el series UID ha de ser únic
    if (qStudyList.count() > 0) m_studyTreeView->setCurrentItem (qStudyList.at(0));
}

void QStudyTreeWidget::sort()
{
    //Ordenem per la columna seleccionada
    m_studyTreeView->sortItems(m_studyTreeView->sortColumn(), Qt::AscendingOrder);
}

void QStudyTreeWidget::setContextMenu(QMenu * contextMenu)
{
    m_contextMenu = contextMenu;
}

void QStudyTreeWidget::contextMenuEvent(QContextMenuEvent *event)
{
    if (!m_studyTreeView->selectedItems().isEmpty())
        m_contextMenu->exec(event->globalPos());
}

void QStudyTreeWidget::currentItemChanged(QTreeWidgetItem * current, QTreeWidgetItem *)
{
    if (current != NULL)
    {
        if (getCurrentStudyUID() != m_oldCurrentStudyUID)
        {
            emit(currentStudyChanged());
            m_oldCurrentStudyUID = getCurrentStudyUID();
        }

        if (getCurrentSeriesUID() != m_oldCurrentSeriesUID)
        {
            m_oldCurrentSeriesUID = getCurrentSeriesUID();
            emit(currentSeriesChanged(m_oldCurrentSeriesUID));
        }

        emit(currentImageChanged()); //sempre que canviem d'element segur que canviem d'imatge
    }
    else emit notCurrentItemSelected(); //cas en que no hi cap item seleccionat
}

void QStudyTreeWidget::itemExpanded(QTreeWidgetItem *itemExpanded)
{
    // En el cas de que ens arribi l'item amb el text buit, no fem res
    // Això passa en situacions molt puntuals quan s'utilitza la tecla '*' per expandir l'item
    if (itemExpanded->text(UID).isEmpty())
    {
        return;
    }
    /* El QTreeWidget després de fer doble click expandeix o col·lapsa l'item en funció del seu estat, a nosaltres no ens interessa
     * que es faci això, per aquest motiu en cas d'un signal de collapse o expand, el que fem és comprovar si per aquell item s'acaba
     *de fer doble click, si és així anul·lem l'acció de col·lapsar o expandir
     */
    if (m_doubleClickedItemUID != itemExpanded->text(UID))
    {
        /* Com que inserim un item en blanc per simular fills dels estudis i de les sèries cada vegada que ens fan un expand hem d'eliminar l'item en blanc i
        * emetem un signal per a que qui el reculli s'encarregui de fer els passos corresponents per expandir l'estudi o imatge amb el seus fills pertinents
        */
        foreach (QTreeWidgetItem * childItem, itemExpanded->takeChildren())
        {
            delete childItem;
        }

        if (isItemStudy(itemExpanded))
        {
            itemExpanded->setIcon(ObjectName, m_openFolder);//canviem la icona per la de carpeta oberta quan l'item està expanded
            emit (studyExpanded(itemExpanded->text(UID)));
        }
        else if (isItemSeries(itemExpanded))
        {
            emit(seriesExpanded(itemExpanded->parent()->text(UID), itemExpanded->text(UID)));
        }

        m_doubleClickedItemUID = "";
    }
    else
    {//si s'ha fet un doble click a l'item hem d'anul·lar l'acció que ha fet qt de col·lapsar l'item per tan nosaltres el tornem a expandir
        m_doubleClickedItemUID = "";//Molt important fer-lo abans de fer collapseItem, sinó entrariem en bucle pq s'emetria signal de collapseItem
        m_studyTreeView->collapseItem(itemExpanded);
    }
}

void QStudyTreeWidget::itemCollapsed(QTreeWidgetItem *itemCollapsed)
{
    /* El QTreeWidget després de fer doble click expandeix o col·lapsa l'item en funció del seu estat, a nosaltres no ens interessa
     * que es faci això, per aquest motiu en cas d'un signal de collapse o expand, el que fem és comprovar si per aquell item s'acaba
     *de fer doble click, si és així anul·lem l'acció de col·lapsar o expandir
     */
    if (m_doubleClickedItemUID != itemCollapsed->text(UID)) // si l'item col·lapsat no se li acaba de fer un doble click
    {
        //Si és una estudi està collapsed, canviem la icona per la carpeta tancada
        if (isItemStudy(itemCollapsed)) itemCollapsed->setIcon(ObjectName, m_closeFolder);

        m_doubleClickedItemUID = "";
    }
    else
    {//si s'ha fet un doble click a l'item hem d'anul·lar l'acció que ha fet qt de col·lapsar l'item per tan nosaltres el tornem a expandir
        m_doubleClickedItemUID = "";//Molt important fer-lo abans de fer expandItem, sinó entrariem en bucle, pq s'emetria signal d'expandItem
        m_studyTreeView->expandItem (itemCollapsed);
    }

}

void QStudyTreeWidget::doubleClicked(QTreeWidgetItem *item, int)
{
    //al fer doblec click al QTreeWidget ja expandeix o amaga automàticament l'objecte
    if (item == NULL) return;

    if (isItemStudy(item)) emit(studyDoubleClicked());
    else if (isItemSeries(item)) emit(seriesDoubleClicked());
    else if (isItemImage(item)) emit(imageDoubleClicked());

    /*Pel comportament del tree widget quan es fa un un doble click es col·lapsa o expandeix l'item en funció del seu estat, com que
     *nosaltres pel doble click no volem que s'expendeixi o es col·lapsi, guardem per quin element s'ha fet el doble click, per anul·laro quan es detecti un signal d'expand o collapse item
     */
    m_doubleClickedItemUID = item->text(UID);
}

bool QStudyTreeWidget::isItemStudy(QTreeWidgetItem *item)
{
    return item->text(Type) == "STUDY";
}

bool QStudyTreeWidget::isItemSeries(QTreeWidgetItem *item)
{
    return item->text(Type) == "SERIES";
}

bool QStudyTreeWidget::isItemImage(QTreeWidgetItem *item)
{
    return item->text(Type) == "IMAGE";
}

QString QStudyTreeWidget::paddingLeft(QString text, int length)
{
    QString paddedText;

    if (text.length() < length)
    {
        for (int index = text.length(); index < length; index++)
        {
            paddedText += " ";
        }
        paddedText += text;
    }
    else paddedText = text;

    return paddedText;
}

};
