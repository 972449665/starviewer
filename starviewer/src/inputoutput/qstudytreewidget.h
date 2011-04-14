#ifndef UDGQSTUDYTREEWIDGET_H
#define UDGQSTUDYTREEWIDGET_H

#include "ui_qstudytreewidgetbase.h"

#include <QMenu>
#include <QList>

// forward declarations
class QString;

namespace udg {

// forward declarations
class Study;
class Patient;
class Series;
class Image;
class DicomMask;

/** Aquesta classe  mostrar estudis i sèries d'una manera organitzada i fàcilment.
  * Aquesta classe és una modificació de la QTreeWidget que s'ha adaptat per poder visualitzar la informació 
  * de la cerca d'estudis, permetent consultar les series d'aquell estudi. 
  * Aquesta classe es sincronitza amb la informació mostrada a QSeriesListWidget
  */
class QStudyTreeWidget : public QWidget , private Ui::QStudyTreeWidgetBase
{
Q_OBJECT
public:
    
    enum ItemTreeLevels { StudyLevel = 0, SeriesLevel = 1, ImageLevel = 2 };

    //Object Name s'utilitza per guardar El NomPacient, Serie + Identificador Sèrie i Imatge + Identificador Image
    enum ColumnIndex{ ObjectName = 0, PatientID = 1, PatientAge = 2, Description = 3, Modality = 4, Date = 5, Time = 6,
    PACSId = 7, Institution = 8, UID = 9, StudyID = 10, ProtocolName = 11, AccNumber = 12, Type = 13,
    RefPhysName = 14, PPStartDate = 15, PPStartTime = 16, ReqProcID = 17, SchedProcStep = 18
    };

    QStudyTreeWidget( QWidget *parent = 0 );
    ~QStudyTreeWidget();

    ///Mostrar els estudis passats per paràmetres (Els pacients passats per paràmetre ha de contenir només un estudi)
    void insertPatientList(QList<Patient*> patientList);

    ///Insereix el pacient al QStudyTreeWiget
    void insertPatient(Patient* patient);

    /** Insereix un llista de sèries a l'estudi seleccionat actualment
     * @param seriesList series afegir
     */
    void insertSeriesList(QString studyIstanceUID, QList<Series*> seriesList);

    /** Insereix una llista d'imatges a la sèrie seleccionada actualment
     * @param imageList llista d'imatges afegir a la sèrie
     */
    void insertImageList(QString studyInstanceUID, QString seriesInstanceUID, QList<Image*> imageList);

    /** removes study from the list
     * @param esbora l'estudi amb StudyUID de la llista
     */
    void removeStudy( QString StudyInstanceUIDToRemove );

    /** Esborra la sèrie del QStudyTreeWidget, si és l'única sèrie de l'estudi també esborra l'estudi, no té sentit tenir una estudi sense
     *  series al TreeWidget
     */
    void removeSeries(const QString &studyInstanceUIDToRemove, const QString &seriesInstanceUIDToRemove);

    ///Indica/Retorna la columna i direcció per la que s'ordena llista 
    void setSortByColumn(QStudyTreeWidget::ColumnIndex sortColumn, Qt::SortOrder sortOrder);
    QStudyTreeWidget::ColumnIndex getSortColumn();
    Qt::SortOrder getSortOrderColumn();

    /** Retorna el UID Study de l'estudi seleccionat
     * @return UID de l'estudi seleccionat
     */
    QString getCurrentStudyUID();

    ///Retorna una llista amb l'UID del estudis seleccionats
    QStringList getSelectedStudiesUID();

    ///Retorna una llista amb els estudis seleccionats
    QList<Study*> getSelectedStudies();

    ///Retorna l'estudi que tingui el studyInstanceUID passat per paràmetre
    Study* getStudy(QString studyInstanceUID);

    /** Retorna el UID de la sèrie seleccionada, si en aquell moment no hi ha cap sèrie seleccionada, retorna un QString buit
     *  @return UID de la sèrie seleccionat
     */
    QString getCurrentSeriesUID();

        /** Retorna el UID de la imatge seleccionada, si en aquell moment no hi ha cap imatge seleccionada, retorna un QString buit
     *  @return UID de la imatge seleccionada
     */
    QString getCurrentImageUID();

    ///ordena descendentment per la columna seleccionada
    void sort();

    ///Estableix el menú contextual del Widget
    void setContextMenu( QMenu *contextMenu );

    /// Retorna el QTreeWidget que conté el widget
    QTreeWidget *getQTreeWidget() const;

    ///Per cada element selecciona Study/Series/Imatge retorna la seva DicomMask
    QList<DicomMask> getDicomMaskOfSelectedItems();

    ///Assigna/Obté el nivell màxim fins el que es poden expandir els items que es mostren a QStudyTreeWiget, per defecte s'expandeix fins a nivell d'Image
    void setMaximumExpandTreeItemsLevel(QStudyTreeWidget::ItemTreeLevels maximumExpandTreeItemsLevel);
    QStudyTreeWidget::ItemTreeLevels getMaximumExpandTreeItemsLevel();

protected:
    /** Mostra el menu contextual
     *  @param Dades de l'event sol·licitat
     */
    void contextMenuEvent(QContextMenuEvent *event);

signals :
    ///signal cada vegada que seleccionem un estudi diferent
    void currentStudyChanged();

    ///signal que s'emete quan canviem de sèrie seleccionada
    void currentSeriesChanged( const QString &seriesUID );

    ///signal que s'emet quan canviem d'imatge seleccionada
    void currentImageChanged();

    ///signal que s'emet quan es fa expandir un estudi
    void studyExpanded(QString studyUID);

    ///signal que s'emet qua es fa expandir una series
    void seriesExpanded(QString studyUID, QString seriesUID);

    ///signal que s'emet quan s'ha fet un doble click a un estudi
    void studyDoubleClicked();

    ///signal que s'emet quan s'ha fet un doble click a una sèrie
    void seriesDoubleClicked();

    ///signal que s'emet quan s'ha fet un doble click a una imatge
    void imageDoubleClicked();

    ///signal que s'emet quan es passa de tenir un item seleccionat a no tenir-ne cap de seleccionat
    void notCurrentItemSelected();

public slots:
    /** Indique que ens marqui la sèrie amb el uid passat per paràmetre com a seleccionada
     * @param SeriesUID Uid de la serie seleccionada en QSeriesListWidget
     */
    void setCurrentSeries( QString seriesUID );

    /// Neteja el TreeView
    void clear();

private slots:
    ///Emet signal quan es selecciona un estudi o serie diferent a l'anterior
    void currentItemChanged( QTreeWidgetItem * current, QTreeWidgetItem * previous );

    ///Emet signal quan s'expandeix un item, i no té items fills
    void itemExpanded( QTreeWidgetItem *itemExpanded );

    ///Emet signal quan es col·lapsa un item, i no té items fills
    void itemCollapsed( QTreeWidgetItem *itemCollapsed );

    ///Emet signal qua es fa doble click sobre un item
    void doubleClicked( QTreeWidgetItem * , int );

private:
    /// crea les connexions dels signals i slots
    void createConnections();

    /** formata l'edat per mostrar per pantalla
     * @param edat
     */
    QString formatAge( const QString );

    ///Formata la data i hora passada a ISO 8601 extended (YYYY-MM-DD HH:MM:SS) Amb aquest format de data es pot ordenar els estudis per data/hora
	///Si l'hora no té valor només retorna la data, i si ni Data i Hora tenen valor retorna string buit
    QString formatDateTime(const QDate &date, const QTime &time);

    /** Retorna l'objecte TreeWidgetItem, que pertany a un estudi cercem, per studUID i PACS, ja que
     * un mateix estudi pot estar a més d'un PACS
     * @param studyUID uid de l'estudi a cercar
     */
    QTreeWidgetItem* getStudyQTreeWidgetItem(QString studyUID);

    ///Retorna l'Objecte QTtreeWidgeItem que és de l'estudi i series
    QTreeWidgetItem* getSeriesQTreeWidgetItem(QString studyUID, QString seriesUID);

private:
    ///Ens indica si l'item passat és un estudi
    bool isItemStudy( QTreeWidgetItem * );

    ///Ens indica si l'item passat és una sèrie
    bool isItemSeries( QTreeWidgetItem * );

    ///Ens indica si l'item passat és una imatge
    bool isItemImage( QTreeWidgetItem * );

    ///Afegeix espais a l'esquerre del text fins arribar l'allargada passada per paràmetre
    QString paddingLeft( QString text , int length );

    /**Dona una sèrie emplena un QTreeWidgetItem en format sèrie
     *@param informació de la serie
     */
    QTreeWidgetItem *fillSeries(Series *serie);

    ///Retorna llista QTreeWidgetItem resultant dels estudis que té el pacient
    QList<QTreeWidgetItem*> fillPatient(Patient *);

private:
    /// Menu contextual
    QMenu *m_contextMenu;

    /// Strings per guardar valors de l'anterior element
    QString m_oldCurrentStudyUID, m_oldCurrentSeriesUID, m_OldInstitution;

    QString m_doubleClickedItemUID;

    /// Icones utilitzades com a root al TreeWidget
    QIcon m_openFolder , m_closeFolder , m_iconSeries;

    QList<Study*> m_insertedStudyList;

    QStudyTreeWidget::ItemTreeLevels m_maximumExpandTreeItemsLevel;
};

}; // end namespace

#endif
