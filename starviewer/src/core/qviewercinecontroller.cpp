#include "qviewercinecontroller.h"

// TODO això és així perquè de moment només suportem Q2DViewer
// a la llarga amb l'interfície de QViewer n'hi hauria d'haver prou
#include "q2dviewer.h"

#include "volume.h"
#include "logging.h"

// Qt
#include <QAction>
#include <QBasicTimer>
#include <QDir>
#include <QTimerEvent>

namespace udg {

QViewerCINEController::QViewerCINEController(QObject *parent)
  : QObject(parent), m_firstSliceInterval(0), m_lastSliceInterval(0), m_nextStep(1), m_velocity(1), m_2DViewer(0), m_playing(false), m_cineDimension(TemporalDimension), m_loopEnabled(false), m_boomerangEnabled(false), m_recordFilename(QDir::homePath() + "/cineMovie")
{
    m_timer = new QBasicTimer();

    m_playAction = new QAction(this);
//     m_playAction->setShortcut(tr("Space"));
    m_playAction->setIcon(QIcon(":/images/play.png"));
    m_playAction->setText(tr("Play"));
    connect(m_playAction, SIGNAL(triggered()), SLOT(play()));

    m_recordAction = new QAction(this);
    m_recordAction->setIcon(QIcon(":/images/record.png"));
    connect(m_recordAction, SIGNAL(triggered()), SLOT(record()));

    m_boomerangAction = new QAction(this);
    m_boomerangAction->setIcon(QIcon(":/images/boomerang.png"));
    m_boomerangAction->setCheckable(true);
    connect(m_boomerangAction, SIGNAL(triggered(bool)), SLOT(enableBoomerang(bool)));

    m_loopAction = new QAction(this);
    m_loopAction->setIcon(QIcon(":/images/repeat.png"));
    m_loopAction->setCheckable(true);
    connect(m_loopAction, SIGNAL(triggered(bool)), SLOT(enableLoop(bool)));
}

QViewerCINEController::~QViewerCINEController()
{
    delete m_timer;
}

void QViewerCINEController::setInputViewer(QViewer *viewer)
{
    if (m_2DViewer)
    {
        disconnect(m_2DViewer, 0, this, 0);
    }

    m_2DViewer = qobject_cast<Q2DViewer*>(viewer);
    if (!m_2DViewer)
    {
        DEBUG_LOG("S'HA PROPORCIONAT UN VIEWER NO SUPORTAT. ARA NOMÉS SUPORTEM Q2DVIEWER!!!!!!!!!!!!");
        return;
    }

    connect(m_2DViewer, SIGNAL(volumeChanged(Volume*)), SLOT(resetCINEInformation(Volume*)));
    connect(m_2DViewer, SIGNAL(slabThicknessChanged(int)), SLOT(updateThickness(int)));

    resetCINEInformation(m_2DViewer->getInput());
}

void QViewerCINEController::setCINEDimension(int dimension)
{
    // Checkejem per si el que ens passen és vàlid o no
    if (dimension != SpatialDimension && dimension != TemporalDimension)
    {
        DEBUG_LOG("Paràmetre ilegal!");
    }
    else
    {
        m_cineDimension = dimension;
    }
}

QAction *QViewerCINEController::getPlayAction() const
{
    return m_playAction;
}

QAction *QViewerCINEController::getRecordAction() const
{
    return m_recordAction;
}

QAction *QViewerCINEController::getLoopAction() const
{
    return m_loopAction;
}

QAction *QViewerCINEController::getBoomerangAction() const
{
    return m_boomerangAction;
}

void QViewerCINEController::setRecordFilename(const QString filename)
{
    m_recordFilename = filename;
}

void QViewerCINEController::play()
{
    if (!m_playing)
    {
        m_playing = true;
        m_playAction->setIcon(QIcon(":/images/pause.png"));
        m_playAction->setText(tr("Pause"));
        emit playing();
        m_timer->start(1000 / m_velocity, this);
    }
    else
    {
        pause();
    }
}

void QViewerCINEController::pause()
{
    m_timer->stop();
    m_playing = false;
    m_playAction->setIcon(QIcon(":/images/play.png"));
    m_playAction->setText(tr("Play"));
    emit paused();
}

void QViewerCINEController::record()
{
    // Ens curem en salut
    if (!m_2DViewer)
    {
        return;
    }
    if (!m_2DViewer->getInput())
    {
        return;
    }

    int phases = m_2DViewer->getInput()->getNumberOfPhases();
    int currentSlice = m_2DViewer->getCurrentSlice();

    // Guardar els fotogrames
    for (int i = 0; i < phases; i++)
    {
        m_2DViewer->setPhase(i);
        m_2DViewer->grabCurrentView();
    }
    m_2DViewer->setSlice(currentSlice);

    // Fer la gravació
    m_2DViewer->record(m_recordFilename, QViewer::MPEG2);
}

void QViewerCINEController::setVelocity(int imagesPerSecond)
{
    m_velocity = imagesPerSecond;
    emit velocityChanged(m_velocity);
    if (m_playing)
    {
        m_timer->start(1000 / m_velocity, this);
    }
}

void QViewerCINEController::enableLoop(bool enable)
{
    m_loopEnabled = enable;
    if (!m_loopEnabled)
    {
        pause();
    }
}

void QViewerCINEController::enableBoomerang(bool enable)
{
    m_boomerangEnabled = enable;
}

void QViewerCINEController::setPlayInterval(int firstImage, int lastImage)
{
    m_firstSliceInterval = firstImage;
    m_lastSliceInterval = lastImage;
}

void QViewerCINEController::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_timer->timerId())
    {
        handleCINETimerEvent();
    }
}

void QViewerCINEController::handleCINETimerEvent()
{
    if (!m_2DViewer)
    {
        return;
    }

    int currentImageIndex;
    int nextImageIndex;

    if (m_cineDimension == TemporalDimension)
    {
        currentImageIndex = m_2DViewer->getCurrentPhase();
    }
    else
    {
        currentImageIndex = m_2DViewer->getCurrentSlice();
    }

    // Si estem al final de l'interval
    if (currentImageIndex == m_lastSliceInterval)
    {
        if (m_loopEnabled)
        {
            if (m_boomerangEnabled)
            {
                m_nextStep = -1;
                nextImageIndex = currentImageIndex + m_nextStep;
            }
            else
            {
                nextImageIndex = m_firstSliceInterval;
            }
        }
        else if (m_boomerangEnabled)
        {
            // Pot ser que hagim desactivat el repeat, però no el boomerang!
            m_nextStep = 1;
        }
        else
        {
            // Tornem a l'inici TODO potser no hauria de ser així... i deixar en la última imatge de la seqüència
            nextImageIndex = m_firstSliceInterval;
            pause();
        }
    }
    // Si estem a l'inici de l'interval
    else if ((currentImageIndex == m_firstSliceInterval))
    {
        // Si tenim algun tipus de repeat activat
        if (m_loopEnabled /*|| m_boomerangEnabled*/)
        {
            m_nextStep = 1;
            nextImageIndex = currentImageIndex + m_nextStep;
        }
        else
        {
            // Fins ara reproduia endavant-endarrera
            if (m_nextStep == -1)
            {
                m_nextStep = 1;
                pause();
            }
            // Inici de la reproduccio
            else
            {
                nextImageIndex = currentImageIndex + m_nextStep;
            }
        }
    }
    else
    {
        nextImageIndex = currentImageIndex + m_nextStep;
    }

    if (m_cineDimension == TemporalDimension)
    {
        m_2DViewer->setPhase(nextImageIndex);
    }
    else
    {
        m_2DViewer->setSlice(nextImageIndex);
    }
}

void QViewerCINEController::resetCINEInformation(Volume *input)
{
    if (input)
    {
        int phases = input->getNumberOfPhases();
        if (phases > 1)
        {
            // Si tenim fases, per defecte treballem sota la temporal
            setCINEDimension(TemporalDimension); 
            setVelocity(input->getNumberOfPhases());
        }
        else
        {
            // TODO potser seria més correcte si s'interrogués a partir d'input!
            // Si no tenim fases, només podem treballar sobre la dim espaial
            setCINEDimension(SpatialDimension);
            // Li donarem una velocitat de 10 img/sec
            setVelocity(10);
        }
        m_firstSliceInterval = 0;
        this->updateThickness(m_2DViewer->getSlabThickness());
    }
}

void QViewerCINEController::updateThickness(int thickness)
{
    if (m_cineDimension == SpatialDimension)
    {
        if (m_2DViewer->isThickSlabActive())
        {
            m_lastSliceInterval = m_2DViewer->getMaximumSlice() - thickness + 1;
        }
        else
        {
            m_lastSliceInterval = m_2DViewer->getMaximumSlice();
        }
    }
    else
    {
        m_lastSliceInterval = m_2DViewer->getInput()->getNumberOfPhases() - 1;
    }
}

}
