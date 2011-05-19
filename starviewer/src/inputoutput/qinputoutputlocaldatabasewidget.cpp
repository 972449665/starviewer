#include "qinputoutputlocaldatabasewidget.h"

#include <QMessageBox>
#include <QShortcut>

#include "status.h"
#include "logging.h"
#include "starviewerapplication.h"
#include "dicommask.h"
#include "patient.h"
#include "statswatcher.h"
#include "qcreatedicomdir.h"
#include "inputoutputsettings.h"
#include "qwidgetselectpacstostoredicomimage.h"
#include "pacsmanager.h"
#include "senddicomfilestopacsjob.h"
#include "retrievedicomfilesfrompacsjob.h"
#include "shortcutmanager.h"

namespace udg {

QInputOutputLocalDatabaseWidget::QInputOutputLocalDatabaseWidget(QWidget *parent) : QWidget(parent)
{
    setupUi(this);

    //esborrem els estudis vells de la cache
    deleteOldStudies();
    createContextMenuQStudyTreeWidget();

    Settings settings;
    settings.restoreColumnsWidths(InputOutputSettings::LocalDatabaseStudyList, m_studyTreeWidget->getQTreeWidget());
    settings.restoreGeometry(InputOutputSettings::LocalDatabaseSplitterState, m_StudyTreeSeriesListQSplitter);

    QStudyTreeWidget::ColumnIndex sortByColumn = (QStudyTreeWidget::ColumnIndex) 
        settings.getValue(InputOutputSettings::LocalDatabaseStudyListSortByColumn).toInt();

    Qt::SortOrder sortOrderColumn = (Qt::SortOrder) settings.getValue(InputOutputSettings::LocalDatabaseStudyListSortOrder).toInt();
    m_studyTreeWidget->setSortByColumn (sortByColumn, sortOrderColumn);

    m_statsWatcher = new StatsWatcher("QueryInputOutputLocalDatabaseWidget", this);
    m_statsWatcher->addClicksCounter(m_viewButton);

    m_studyTreeWidget->setMaximumExpandTreeItemsLevel(QStudyTreeWidget::SeriesLevel);

    m_qwidgetSelectPacsToStoreDicomImage = new QWidgetSelectPacsToStoreDicomImage();

    createConnections();
}

QInputOutputLocalDatabaseWidget::~QInputOutputLocalDatabaseWidget()
{
    Settings settings;
    settings.saveColumnsWidths(InputOutputSettings::LocalDatabaseStudyList, m_studyTreeWidget->getQTreeWidget());

    //Guardem per quin columna està ordenada la llista d'estudis i en quin ordre
    settings.setValue(InputOutputSettings::LocalDatabaseStudyListSortByColumn, m_studyTreeWidget->getSortColumn());
    settings.setValue(InputOutputSettings::LocalDatabaseStudyListSortOrder, m_studyTreeWidget->getSortOrderColumn());

}

void QInputOutputLocalDatabaseWidget::createConnections()
{
    connect(m_studyTreeWidget, SIGNAL(studyExpanded(QString)), SLOT(expandSeriesOfStudy(QString)));
    connect(m_studyTreeWidget, SIGNAL(seriesExpanded(QString, QString)), SLOT(expandImagesOfSeries(QString, QString)));

    connect(m_studyTreeWidget, SIGNAL(studyDoubleClicked()), SLOT(viewFromQStudyTreeWidget()));
    connect(m_studyTreeWidget, SIGNAL(seriesDoubleClicked()), SLOT(viewFromQStudyTreeWidget()));
    connect(m_studyTreeWidget, SIGNAL(imageDoubleClicked()), SLOT(viewFromQStudyTreeWidget()));

    connect(m_viewButton, SIGNAL(clicked()), SLOT(viewFromQStudyTreeWidget()));

    connect(m_seriesListWidget, SIGNAL(selectedSeriesIcon(QString)), m_studyTreeWidget, SLOT(setCurrentSeries(QString)));
    connect(m_seriesListWidget, SIGNAL(viewSeriesIcon()), SLOT(viewFromQSeriesListWidget()));
    connect(m_studyTreeWidget, SIGNAL(currentStudyChanged()), SLOT(setSeriesToSeriesListWidget()));
    connect(m_studyTreeWidget, SIGNAL(currentSeriesChanged(QString)), m_seriesListWidget, SLOT(setCurrentSeries(QString)));
    //Si passem de tenir un element seleccionat a no tenir-ne li diem al seriesListWidget que no mostri cap previsualització
    connect(m_studyTreeWidget, SIGNAL(notCurrentItemSelected()), m_seriesListWidget, SLOT(clear()));

    //Connecta amb el signal que indica que ha finalitza el thread d'esborrar els estudis vells
    connect(&m_qdeleteOldStudiesThread, SIGNAL(finished()), SLOT(deleteOldStudiesThreadFinished()));

    ///Si movem el QSplitter capturem el signal per guardar la seva posició
    connect(m_StudyTreeSeriesListQSplitter, SIGNAL(splitterMoved (int, int)), SLOT(qSplitterPositionChanged()));
    connect(m_qwidgetSelectPacsToStoreDicomImage, SIGNAL(selectedPacsToStore()), SLOT(sendSelectedStudiesToSelectedPacs()));
}

void QInputOutputLocalDatabaseWidget::createContextMenuQStudyTreeWidget()
{
    QAction *action;

    action = m_contextMenuQStudyTreeWidget.addAction(QIcon(":/images/view.png"), tr("&View"), this, SLOT(viewFromQStudyTreeWidget()),
                                                     ShortcutManager::getShortcuts(Shortcuts::ViewSelectedStudies).first());
    (void) new QShortcut(action->shortcut(), this, SLOT(viewFromQStudyTreeWidget()));

    action = m_contextMenuQStudyTreeWidget.addAction(QIcon(":/images/databaseRemove.png"), tr("&Delete"), this, SLOT(deleteSelectedItemsFromLocalDatabase()),
                                                     ShortcutManager::getShortcuts(Shortcuts::DeleteSelectedLocalDatabaseStudies).first());
    (void) new QShortcut(action->shortcut(), this, SLOT(deleteSelectedItemsFromLocalDatabase()));

#ifndef STARVIEWER_LITE
    action = m_contextMenuQStudyTreeWidget.addAction(tr("Send to DICOMDIR List"), this, SLOT(addSelectedStudiesToCreateDicomdirList()),
                                                     ShortcutManager::getShortcuts(Shortcuts::SendSelectedStudiesToDICOMDIRList).first());
    (void) new QShortcut(action->shortcut(), this, SLOT(addSelectedStudiesToCreateDicomdirList()));

    action = m_contextMenuQStudyTreeWidget.addAction(QIcon(":/images/store.png"), tr("Send to PACS"), this, SLOT(selectedStudiesStoreToPacs()),
                                                     ShortcutManager::getShortcuts(Shortcuts::StoreSelectedStudiesToPACS).first());
    (void) new QShortcut(action->shortcut(), this, SLOT(selectedStudiesStoreToPacs()));
#endif
    // Especifiquem que és el menú per la cache
    m_studyTreeWidget->setContextMenu(&m_contextMenuQStudyTreeWidget);
}

//TODO s'hauria buscar una manera més elegant de comunicar les dos classes, fer un singletton de QCreateDicomdir ?
void QInputOutputLocalDatabaseWidget::setQCreateDicomdir(QCreateDicomdir *qcreateDicomdir)
{
    m_qcreateDicomdir = qcreateDicomdir;
}

void QInputOutputLocalDatabaseWidget::clear()
{
    m_studyTreeWidget->clear();
    m_seriesListWidget->clear();
}

void QInputOutputLocalDatabaseWidget::setPacsManager(PacsManager *pacsManager)
{
    m_pacsManager = pacsManager;
    connect(pacsManager, SIGNAL(newPACSJobEnqueued(PACSJob*)), SLOT(newPACSJobEnqueued(PACSJob*)));
}

void QInputOutputLocalDatabaseWidget::queryStudy(DicomMask queryMask)
{
    LocalDatabaseManager localDatabaseManager;
    QList<Patient*> patientStudyList;

    StatsWatcher::log("Cerca d'estudis a la base de dades local amb paràmetres: " + queryMask.getFilledMaskFields());
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    m_studyTreeWidget->clear();
    m_seriesListWidget->clear();

    patientStudyList = localDatabaseManager.queryPatientStudy(queryMask);

    if (showDatabaseManagerError(localDatabaseManager.getLastError()))
    {
        return;
    }

    // Aquest mètode a part de ser cridada quan l'usuari fa click al botó search, també es cridada al
    // constructor d'aquesta classe, per a que al engegar l'aplicació ja es mostri la llista d'estudis
    // que hi ha a la base de dades local. Si el mètode no troba cap estudi a la base de dades local
    // es llença el missatge que no s'han trobat estudis, però com que no és idonii, en el cas aquest que es
    // crida des del constructor que es mostri el missatge de que no s'han trobat estudis al engegar l'aplicació, el que
    // es fa és que per llançar el missatge es comprovi que la finestra estigui activa. Si la finestra no està activa
    // vol dir que el mètode ha estat invocat des del constructor
    if (patientStudyList.isEmpty() && isActiveWindow())
    {
        QApplication::restoreOverrideCursor();
        QMessageBox::information(this, ApplicationNameString, tr("No study match found."));
    }
    else
    {
        // Es mostra la llista d'estudis
        m_studyTreeWidget->insertPatientList(patientStudyList);
        QApplication::restoreOverrideCursor();
    }
}

void QInputOutputLocalDatabaseWidget::addStudyToQStudyTreeWidget(QString studyUID)
{
    LocalDatabaseManager localDatabaseManager;
    DicomMask studyMask;
    QList<Patient*> patientList;

    studyMask.setStudyInstanceUID(studyUID);
    patientList = localDatabaseManager.queryPatientStudy(studyMask);
    if (showDatabaseManagerError(localDatabaseManager.getLastError()))
    {
        return;
    }

    if (patientList.count() == 1)
    {
        m_studyTreeWidget->insertPatient(patientList.at(0));
        m_studyTreeWidget->sort();
    }
}

void QInputOutputLocalDatabaseWidget::removeStudyFromQStudyTreeWidget(QString studyInstanceUID)
{
    m_studyTreeWidget->removeStudy(studyInstanceUID);
}

void QInputOutputLocalDatabaseWidget::expandSeriesOfStudy(QString studyInstanceUID)
{
    QList<Series*> seriesList;
    LocalDatabaseManager localDatabaseManager;
    DicomMask mask;

    INFO_LOG("Cerca de sèries a la font cache de l'estudi " + studyInstanceUID);

    //preparem la mascara i cerquem les series a la cache
    mask.setStudyInstanceUID(studyInstanceUID);
    seriesList = localDatabaseManager.querySeries(mask);

    if (showDatabaseManagerError(localDatabaseManager.getLastError()))
    {
        return;
    }

    if (seriesList.isEmpty())
    {
        QMessageBox::information(this, ApplicationNameString, tr("No series match for this study.\n"));
        return;
    }
    else
    {
        // Inserim la informació de les sèries al estudi
        m_studyTreeWidget->insertSeriesList(studyInstanceUID, seriesList);
    }
}

void QInputOutputLocalDatabaseWidget::expandImagesOfSeries(QString studyInstanceUID, QString seriesInstanceUID)
{
    LocalDatabaseManager localDatabaseManager;
    DicomMask mask;
    QList<Image*> imageList;

    INFO_LOG("Cerca d'imatges a la font cache de l'estudi " + studyInstanceUID + " i serie " + seriesInstanceUID);

    mask.setStudyInstanceUID(studyInstanceUID);
    mask.setSeriesInstanceUID(seriesInstanceUID);
    imageList = localDatabaseManager.queryImage(mask);

    if (showDatabaseManagerError(localDatabaseManager.getLastError()))
    {
        return;
    }

    if (imageList.isEmpty())
    {
        QMessageBox::information(this, ApplicationNameString, tr("No images match for this study.\n"));
        return;
    }
    else
    {
        m_studyTreeWidget->insertImageList(studyInstanceUID, seriesInstanceUID, imageList);
    }
}

void QInputOutputLocalDatabaseWidget::setSeriesToSeriesListWidget()
{
    QList<Series*> seriesList;
    LocalDatabaseManager localDatabaseManager;
    DicomMask mask;
    QString studyInstanceUID = m_studyTreeWidget->getCurrentStudyUID();

    INFO_LOG("Cerca de sèries a la cache de l'estudi " + studyInstanceUID);

    //preparem la mascara i cerquem les series a la cache
    mask.setStudyInstanceUID(studyInstanceUID);

    seriesList = localDatabaseManager.querySeries(mask);
    if (showDatabaseManagerError(localDatabaseManager.getLastError()))
    {
        return;
    }

    m_seriesListWidget->clear();

    foreach (Series *series, seriesList)
    {
        m_seriesListWidget->insertSeries(studyInstanceUID, series);
    }
}

void QInputOutputLocalDatabaseWidget::deleteSelectedItemsFromLocalDatabase()
{
    QList<DicomMask> selectedDicomMaskToDelete = m_studyTreeWidget->getDicomMaskOfSelectedItems();

    if (!selectedDicomMaskToDelete.isEmpty())
    {
        QMessageBox::StandardButton response = QMessageBox::question(this, ApplicationNameString,
                                                                           tr("Are you sure you want to delete the selected Items?"),
                                                                           QMessageBox::Yes | QMessageBox::No,
                                                                           QMessageBox::No);
        if (response == QMessageBox::Yes)
        {
            QApplication::setOverrideCursor(Qt::BusyCursor);
            LocalDatabaseManager localDatabaseManager;

            foreach (DicomMask dicomMaskToDelete, selectedDicomMaskToDelete)
            {
                if (m_qcreateDicomdir->studyExistsInDICOMDIRList(dicomMaskToDelete.getStudyInstanceUID()))
                {
                    Study *studyToDelete = m_studyTreeWidget->getStudy(dicomMaskToDelete.getStudyInstanceUID());
                    QString warningMessage;

                    if (dicomMaskToDelete.getSeriesInstanceUID().isEmpty())
                    {
                        warningMessage = tr("The study %1 of patient %2 is in use by the DICOMDIR List. If you want to delete "
                                            "this study you should remove it from the DICOMDIR List first.")
                                         .arg(studyToDelete->getID(), studyToDelete->getParentPatient()->getFullName());
                    }
                    else
                    {   
                        // TODO:Hauriem de mostar el Series ID en lloc del Series UID
                        warningMessage = tr("The series with UID %1 of study %2 patient %3 is in use by the DICOMDIR List. If you want to delete "
                                            "this series you should remove the study from the DICOMDIR List first.")
                                         .arg(dicomMaskToDelete.getSeriesInstanceUID(), studyToDelete->getID(),
                                              studyToDelete->getParentPatient()->getFullName());
                    }

                    QMessageBox::warning(this, ApplicationNameString, warningMessage);
                }
                else
                {
                    if (!dicomMaskToDelete.getSeriesInstanceUID().isEmpty())
                    {
                        INFO_LOG(QString("L'usuari ha indicat que vol esborrar de la cache la serie %1 de l'estudi %2")
                                    .arg(dicomMaskToDelete.getSeriesInstanceUID(), dicomMaskToDelete.getStudyInstanceUID()));
                        localDatabaseManager.deleteSeries(dicomMaskToDelete.getStudyInstanceUID(), dicomMaskToDelete.getSeriesInstanceUID());

                        m_seriesListWidget->removeSeries(dicomMaskToDelete.getSeriesInstanceUID());
                        m_studyTreeWidget->removeSeries(dicomMaskToDelete.getStudyInstanceUID(), dicomMaskToDelete.getSeriesInstanceUID());
                    }
                    else
                    {
                        INFO_LOG(QString("L'usuari ha indicat que vol esborrar de la cache l'estudi %1").arg(dicomMaskToDelete.getStudyInstanceUID()));
                        localDatabaseManager.deleteStudy(dicomMaskToDelete.getStudyInstanceUID());

                        m_seriesListWidget->clear();
                        m_studyTreeWidget->removeStudy(dicomMaskToDelete.getStudyInstanceUID());
                    }

                    if (showDatabaseManagerError(localDatabaseManager.getLastError()))
                    {
                        break;
                    }
                }
            }
            QApplication::restoreOverrideCursor();
        }
    }
    else
    {
        QMessageBox::information(this, ApplicationNameString, tr("Please select at least one item to delete."));
    }
}

void QInputOutputLocalDatabaseWidget::view(QStringList selectedStudiesInstanceUID, QString selectedSeriesInstanceUID, bool loadOnly)
{
    DicomMask patientToProcessMask;
    Patient *patient;
    QList<Patient*> selectedPatientsList;

    if (selectedStudiesInstanceUID.isEmpty())
    {
        QMessageBox::warning(this, ApplicationNameString, tr("Select at least one study to view."));
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    // TODO: S'hauria de millorar el mètode ja que per la seva estructura lo d'obrir l'estudi per la sèrie que ens tinguin seleccionada només ho farà per un
    // estudi ja que aquest mètode només se li passa per paràmetre una sèrie per defecte
    foreach (QString studyInstanceUIDSelected, selectedStudiesInstanceUID)
    {
        LocalDatabaseManager localDatabaseManager;

        patientToProcessMask.setStudyInstanceUID(studyInstanceUIDSelected);

        patient = localDatabaseManager.retrieve(patientToProcessMask);

        if (showDatabaseManagerError(localDatabaseManager.getLastError()))
        {
            QApplication::restoreOverrideCursor();
            return;
        }

        if (patient)
        {
            // Marquem la sèrie per defecte
            // TODO ara sempre posem el mateix UID, per tant de moment només funciona bé del tot quan seleccionem un únic estudi
            patient->setSelectedSeries(selectedSeriesInstanceUID);
            selectedPatientsList << patient;
        }
        else
        {
            DEBUG_LOG("No s'ha pogut obtenir l'estudi amb UID " + studyInstanceUIDSelected);
        }
    }

    if (selectedPatientsList.count() > 0)
    {
        DEBUG_LOG("Llançat signal per visualitzar estudi del pacient " + patient->getFullName());
        emit viewPatients(Patient::mergePatients(selectedPatientsList), loadOnly);
    }

    QApplication::restoreOverrideCursor();
}

void QInputOutputLocalDatabaseWidget::viewFromQStudyTreeWidget()
{
    view(m_studyTreeWidget->getSelectedStudiesUID(), m_studyTreeWidget->getCurrentSeriesUID());
}

void QInputOutputLocalDatabaseWidget::viewFromQSeriesListWidget()
{
    QStringList selectedStudiesInstanceUID;

    // Agafem l'estudi uid de la sèrie seleccionada
    selectedStudiesInstanceUID << m_seriesListWidget->getCurrentStudyUID();
    view(selectedStudiesInstanceUID, m_seriesListWidget->getCurrentSeriesUID());

    StatsWatcher::log("Obrim estudi seleccionant sèrie desde thumbnail");
}

// TODO en comptes de fer un signal cap a la queryscreen, perquè aquesta indiqui a la QInputOutPacsWidget que guardi un estudi al PACS,
// no hauria de ser aquesta funció l'encarregada de guardar l'estudi directament al PACS, entenc que no és responsabilitat de
// QInputOutputPacsWidget
void QInputOutputLocalDatabaseWidget::selectedStudiesStoreToPacs()
{
    if (m_studyTreeWidget->getSelectedStudies().count() == 0)
    {
        QMessageBox::warning(this, ApplicationNameString, tr("Select at least one item to send to PACS."));
    }
    else
    {
        m_qwidgetSelectPacsToStoreDicomImage->show();
    }
}

void QInputOutputLocalDatabaseWidget::addSelectedStudiesToCreateDicomdirList()
{
    DicomMask studyMask;
    LocalDatabaseManager localDatabaseManager;
    QList<Patient*> patientList;
    QList<Study*> studies;

    foreach (QString studyUID, m_studyTreeWidget->getSelectedStudiesUID())
    {
        studyMask.setStudyInstanceUID(studyUID);
        patientList = localDatabaseManager.queryPatientStudy(studyMask);
        if (showDatabaseManagerError(localDatabaseManager.getLastError()))
        {
            return;
        }

        // \TODO Això s'ha de fer perquè queryPatientStudy retorna llista de Patients
        // Nosaltres, en realitat, volem llista d'study amb les dades de Patient omplertes.
        if (patientList.size() != 1 && patientList.first()->getNumberOfStudies() != 1)
        {
            showDatabaseManagerError(LocalDatabaseManager::DatabaseCorrupted);
            return;
        }
        studies << patientList.first()->getStudies().first();
    }
    m_qcreateDicomdir->addStudies(studies);
}

// TODO: Aquesta responsabilitat d'esborrar els estudis vells al iniciar-se l'aplicació s'hauria de traslladar a un altre lloc, no és responsabilitat
//       d'aquesta inferfície
void QInputOutputLocalDatabaseWidget::deleteOldStudies()
{
    Settings settings;
    // Mirem si està activada la opció de la configuració d'esborrar els estudis vells no visualitzats en un número de dies determinat
    // fem la comprovació, per evitar engegar el thread si no s'han d'esborrar els estudis vells
    if (settings.getValue(InputOutputSettings::DeleteLeastRecentlyUsedStudiesInDaysCriteria).toBool())
    {
        m_qdeleteOldStudiesThread.deleteOldStudies();
    }
}

QList<Image*> QInputOutputLocalDatabaseWidget::getAllImagesFromPatient(Patient *patient)
{
    QList<Image*> images;

    foreach (Study *study, patient->getStudies())
    {
        foreach (Series *series, study->getSeries())
        {
            images.append(series->getImages());
        }
    }

    return images;
}

void QInputOutputLocalDatabaseWidget::deleteOldStudiesThreadFinished()
{
    showDatabaseManagerError(m_qdeleteOldStudiesThread.getLastError(), tr("deleting old studies"));
}

void QInputOutputLocalDatabaseWidget::qSplitterPositionChanged()
{
    Settings().saveGeometry(InputOutputSettings::LocalDatabaseSplitterState, m_StudyTreeSeriesListQSplitter);
}

void QInputOutputLocalDatabaseWidget::sendSelectedStudiesToSelectedPacs()
{
    foreach (PacsDevice pacsDevice, m_qwidgetSelectPacsToStoreDicomImage->getSelectedPacsToStoreDicomImages())
    {
        foreach (DicomMask dicomMask, m_studyTreeWidget->getDicomMaskOfSelectedItems())
        {
            LocalDatabaseManager localDatabaseManager;
            Patient *patient = localDatabaseManager.retrieve(dicomMask);

            if (localDatabaseManager.getLastError() != LocalDatabaseManager::Ok)
            {
                ERROR_LOG(QString("Error a la base de dades intentar obtenir els estudis que s'han d'enviar al PACS, Error: %1; StudyUID: %2")
                                  .arg(localDatabaseManager.getLastError())
                                  .arg(dicomMask.getStudyInstanceUID()));

                QString message = tr("An error occurred with database, preparing the DICOM files to send to PACS %1. The DICOM files won't be sent.")
                    .arg(pacsDevice.getAETitle());
                message += tr("\nClose all %1 windows and try again."
                         "\n\nIf the problem persists contact with an administrator.").arg(ApplicationNameString);
                QMessageBox::critical(this, ApplicationNameString, message);
            }
            else
            {
                sendDICOMFilesToPACS(pacsDevice, getAllImagesFromPatient(patient));
            }
        }
    }
}

void QInputOutputLocalDatabaseWidget::sendDICOMFilesToPACS(PacsDevice pacsDevice, QList<Image*> images)
{
    SendDICOMFilesToPACSJob *sendDICOMFilesToPACSJob = new SendDICOMFilesToPACSJob(pacsDevice, images);
    connect(sendDICOMFilesToPACSJob, SIGNAL(PACSJobFinished(PACSJob*)), SLOT(sendDICOMFilesToPACSJobFinished(PACSJob*)));
    m_pacsManager->enqueuePACSJob(sendDICOMFilesToPACSJob);
}

void QInputOutputLocalDatabaseWidget::sendDICOMFilesToPACSJobFinished(PACSJob *pacsJob)
{
    SendDICOMFilesToPACSJob *sendDICOMFilesToPACSJob = dynamic_cast<SendDICOMFilesToPACSJob*> (pacsJob);

    if (sendDICOMFilesToPACSJob->getStatus() != PACSRequestStatus::SendOk)
    {
        if (sendDICOMFilesToPACSJob->getStatus() == PACSRequestStatus::SendWarningForSomeImages ||
            sendDICOMFilesToPACSJob->getStatus() == PACSRequestStatus::SendSomeDICOMFilesFailed)
        {
            QMessageBox::warning(this, ApplicationNameString, sendDICOMFilesToPACSJob->getStatusDescription());
        }
        else
        {
            QMessageBox::critical(this, ApplicationNameString, sendDICOMFilesToPACSJob->getStatusDescription());
        }
    }

    // Com que l'objecte és un punter altres classes poden haver capturat el Signal per això li fem un deleteLater() en comptes d'un delete, per evitar
    // que quan responguin al signal es trobin que l'objecte ja no existeix. L'objecte serà destruït per Qt quan es retorni el eventLoop
    pacsJob->deleteLater();
}

void QInputOutputLocalDatabaseWidget::newPACSJobEnqueued(PACSJob *pacsJob)
{
    // Connectem amb el signal RetrieveDICOMFilesFromPACSJob de que s'esborrarà un estudi de la caché per treure'ls de la QStudyTreeWidget quan se
    // n'esborrin
    // TODO: RetrieveDICOMFilesFromPACS no hauria d'emetre aquest signal, hauria de ser una CacheManager d'aquesta manera treuriem la responsabilitat
    //       de RetrieveDICOMFilesFromPACS de fer-ho, i a més no caldria connectar el signal cada vegada que fan un nou Job. Una vegada s'hagi implementar la
    //       CacheManager aquest mètode HA DE DESAPAREIXER, quan es tregui aquest mètode recordar a treure l'include a "retrievedicomfilesfrompacsjob.h"
    if (pacsJob->getPACSJobType() == PACSJob::RetrieveDICOMFilesFromPACSJobType)
    {
        connect(dynamic_cast<RetrieveDICOMFilesFromPACSJob*> (pacsJob), SIGNAL(studyFromCacheWillBeDeleted(QString)),
                SLOT(removeStudyFromQStudyTreeWidget(QString)));
    }
}

bool QInputOutputLocalDatabaseWidget::showDatabaseManagerError(LocalDatabaseManager::LastError error, const QString &doingWhat)
{
    QString message;

    if (!doingWhat.isEmpty())
    {
        message = tr("An error has occurred while ") + doingWhat + ":\n\n";
    }

    switch (error)
    {
        case LocalDatabaseManager::Ok:
            return false;
        case LocalDatabaseManager::DatabaseLocked:
            message += tr("The database is blocked by another process.");
            message += tr("\nClose all %1 windows and try again."
                         "\n\nIf the problem persists contact with an administrator.").arg(ApplicationNameString);
            break;
        case LocalDatabaseManager::DatabaseCorrupted:
            message += tr("%1 database is corrupted.").arg(ApplicationNameString);
            message += tr("\nClose all %1 windows and try again."
                         "\n\nIf the problem persists contact with an administrator.").arg(ApplicationNameString);
            break;
        case LocalDatabaseManager::SyntaxErrorSQL:
            message += tr("%1 database syntax error.").arg(ApplicationNameString);
            message += tr("\nClose all %1 windows and try again."
                         "\n\nIf the problem persists contact with an administrator.").arg(ApplicationNameString);
            break;
        case LocalDatabaseManager::DatabaseError:
            message += tr("An internal error occurs with %1 database.").arg(ApplicationNameString);
            message += tr("\nClose all %1 windows and try again."
                         "\n\nIf the problem persists contact with an administrator.").arg(ApplicationNameString);
            break;
        case LocalDatabaseManager::DeletingFilesError:
            message += tr("Some files cannot be deleted."
                         "\nThese files have to be deleted manually.");
            break;
        default:
            message = tr("Unknown error.");
            break;
    }

    QApplication::restoreOverrideCursor();

    QMessageBox::critical(this, ApplicationNameString, message);

    return true;
}

};
