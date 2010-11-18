/***************************************************************************
 *   Copyright (C) 2005 by Grup de Gràfics de Girona                       *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/

#include "qpopuprisrequestsscreen.h"

#include <QTimer>
#include <QDesktopWidget>
#include <QRect>
#include <QApplication>
#include <QMovie>

#include "qpopuprisrequestsscreen.h"
#include "operation.h"
#include "logging.h"
#include "starviewerapplication.h"

namespace udg {

const int QPopUpRisRequestsScreen::msTimeOutToHidePopUp = 5000;

QPopUpRisRequestsScreen::QPopUpRisRequestsScreen( QWidget *parent ): QDialog( parent )
{
    setupUi(this);
    this->setWindowFlags(Qt::SubWindow | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);

    m_qTimer = new QTimer();
    connect(m_qTimer,SIGNAL(timeout()),SLOT(timeoutTimer()));

    QMovie *operationAnimation = new QMovie(this);
    operationAnimation->setFileName(":/images/loader.gif");
    m_operationAnimation->setMovie(operationAnimation);
    operationAnimation->start();
}

void QPopUpRisRequestsScreen::queryStudiesByAccessionNumberStarted(QString accessionNumber)
{
    QString popUpText = tr("%1 has received a request from a RIS to retrieve the study with accession number").arg(ApplicationNameString);
    m_labelRisRequestDescription->setText(popUpText + " " + accessionNumber + ".");
    m_operationDescription->setText(tr("Querying PACS..."));
    m_operationAnimation->show();

    m_studiesRetrievingCounter->setText("");

    m_studiesInstanceUIDToRetrieve.clear();
    m_studiesInstanceUIDRetrieved.clear();
}

void QPopUpRisRequestsScreen::addStudyToRetrieveByAccessionNumber(QString studyInstanceUID)
{
    if (m_studiesInstanceUIDToRetrieve.count() == 0)
    {
        m_operationDescription->setText(tr("Retrieving study"));
    }

    m_studiesInstanceUIDToRetrieve.append(studyInstanceUID);
    refreshLabelStudyCounter();
}

void QPopUpRisRequestsScreen::retrieveStudyByAccessionNumberFinished(QString studyInstanceUID)
{
    m_studiesInstanceUIDRetrieved.append(studyInstanceUID);
    
    if (m_studiesInstanceUIDRetrieved.count() < m_studiesInstanceUIDToRetrieve.count())
    {
        refreshLabelStudyCounter();
    }
    else
    {
        showRetrieveFinished();
    }
}

void QPopUpRisRequestsScreen::retrieveStudyByAccessionNumberFailed(QString studyInstanceUID)
{
    //Si ha fallat l'estudi el treiem de la llista d'estudis a descarregar
    m_studiesInstanceUIDToRetrieve.removeOne(studyInstanceUID);
    
    if (m_studiesInstanceUIDRetrieved.count() < m_studiesInstanceUIDToRetrieve.count())
    {
        refreshLabelStudyCounter();
    }
    else
    {
        showRetrieveFinished();
    }
}

void QPopUpRisRequestsScreen::showNotStudiesFoundMessage()
{
    m_operationDescription->setText(tr("No studies found."));
    m_operationAnimation->hide();
    m_qTimer->start(msTimeOutToHidePopUp);
}

void QPopUpRisRequestsScreen::refreshLabelStudyCounter()
{
    m_studiesRetrievingCounter->setText(QString(tr("%1 of %2.")).arg(m_studiesInstanceUIDRetrieved.count() + 1).arg(m_studiesInstanceUIDToRetrieve.count()));
}

void QPopUpRisRequestsScreen::showRetrieveFinished()
{
    m_operationAnimation->hide();
    m_operationDescription->setText(tr("%1 studies has been retrieved.").arg(m_studiesInstanceUIDRetrieved.count()));
    m_studiesRetrievingCounter->setText("");
    m_qTimer->start(msTimeOutToHidePopUp);
}

void QPopUpRisRequestsScreen::showEvent(QShowEvent *)
{
    //Es situa el PopUp a baix a l'esquerre de la pantalla on està la interfície activa del Starviewer
    QDesktopWidget desktopWidget;

    QRect screenGeometryActiveWindow = desktopWidget.availableGeometry(QApplication::activeWindow()); //Agafem les dimensions de la pantalla on està la finestra activa de l'starviewer

    this->move(screenGeometryActiveWindow.x() + screenGeometryActiveWindow.width() - this->width() - 10, screenGeometryActiveWindow.y() + screenGeometryActiveWindow.height() - this->height() -10);
}

void QPopUpRisRequestsScreen::timeoutTimer()
{
    this->hide();
    m_qTimer->stop();
}

QPopUpRisRequestsScreen::~QPopUpRisRequestsScreen()
{
    m_qTimer->stop();
    delete m_qTimer;
}

};
