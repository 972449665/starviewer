/*************************************************************************************
  Copyright (C) 2014 Laboratori de Gràfics i Imatge, Universitat de Girona &
  Institut de Diagnòstic per la Imatge.
  Girona 2014. All rights reserved.
  http://starviewer.udg.edu

  This file is part of the Starviewer (Medical Imaging Software) open source project.
  It is subject to the license terms in the LICENSE file found in the top-level
  directory of this distribution and at http://starviewer.udg.edu/license. No part of
  the Starviewer (Medical Imaging Software) open source project, including this file,
  may be copied, modified, propagated, or distributed except according to the
  terms contained in the LICENSE file.
 *************************************************************************************/

#include "asynchronousvolumereader.h"

#include <threadweaver/ThreadWeaver.h>
#include <threadweaver/DebuggingAids.h>

#include "volumereaderjob.h"
#include "volume.h"
#include "logging.h"
#include "coresettings.h"
#include "volumerepository.h"
#include "image.h"
#include "series.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace udg {

QHash<int, VolumeReaderJob*> AsynchronousVolumeReader::m_volumesLoading;
ThreadWeaver::ResourceRestrictionPolicy AsynchronousVolumeReader::m_resourceRestrictionPolicy;

AsynchronousVolumeReader::AsynchronousVolumeReader(QObject *parent)
 : QObject(parent)
{
}

bool is32BitWindows()
{
#if defined(_WIN64)
    return false;  // 64-bit programs run only on Win64
#elif defined(_WIN32)
    // 32-bit programs run on both 32-bit and 64-bit Windows
    // so must sniff
    BOOL f64 = false;
    return !(IsWow64Process(GetCurrentProcess(), &f64) && f64);
#else
    return false; // Win64 does not support Win16
#endif
}

VolumeReaderJob* AsynchronousVolumeReader::read(Volume *volume)
{
    DEBUG_LOG(QString("AsynchronousVolumeReader::read Begin volume: %1").arg(volume->getIdentifier().getValue()));

    if (this->isVolumeLoading(volume))
    {
        DEBUG_LOG(QString("AsynchronousVolumeReader::read Volume already loading: %1").arg(volume->getIdentifier().getValue()));

        return this->getVolumeReaderJob(volume);
    }

    VolumeReaderJob *volumeReaderJob = new VolumeReaderJob(volume);
    assignResourceRestrictionPolicy(volumeReaderJob);

    connect(volumeReaderJob, SIGNAL(done(ThreadWeaver::Job*)), SLOT(unmarkVolumeFromJobAsLoading(ThreadWeaver::Job*)));

    this->markVolumeAsLoadingByJob(volume, volumeReaderJob);

    // TODO Permetre escollir quants jobs alhora volem
    ThreadWeaver::Weaver *weaver = this->getWeaverInstance();
    weaver->enqueue(volumeReaderJob);

    return volumeReaderJob;
}

void AsynchronousVolumeReader::assignResourceRestrictionPolicy(VolumeReaderJob *volumeReaderJob)
{
    QSettings settings;
    if (settings.contains(CoreSettings::MaximumNumberOfVolumesLoadingConcurrently))
    {
        int maximumNumberOfVolumesLoadingConcurrently = Settings().getValue(CoreSettings::MaximumNumberOfVolumesLoadingConcurrently).toInt();
        if (maximumNumberOfVolumesLoadingConcurrently > 0)
        {
            m_resourceRestrictionPolicy.setCap(maximumNumberOfVolumesLoadingConcurrently);
            volumeReaderJob->assignQueuePolicy(&m_resourceRestrictionPolicy);
            INFO_LOG(QString("Limitem a %1 la quantitat de volums carregant-se simultàniament.").arg(m_resourceRestrictionPolicy.cap()));
        }
        else
        {
            ERROR_LOG("El valor per limitar la quantitat de volums carregant-se simultàniament ha de ser més gran de 0.");
        }
    }
    else 
    {
        QStringList allowedModalities;
        if (is32BitWindows())
        {
            // Si és 32 bits a més de limitar la concurrència si tenim volums multiframe ho fem també amb els que no siguin CT o MR
            allowedModalities << QString("CT") << QString("MR");
        }
        // Si és 64 bits, només limitarem la concurrència si tenim volums multiframe, ja que les gdcm necessiten molta memòria per carregar-les
        // i es poden produir pics de memòria considerables que impedeixen obrir les imatges
        
        bool foundRestrictions = checkForResourceRestrictions(true, allowedModalities);
        
        int numberOfVolumesLoadingConcurrently;
        if (foundRestrictions)
        {
            numberOfVolumesLoadingConcurrently = 1;
            if (is32BitWindows())
            {
                INFO_LOG(QString("Windows 32 bits amb volums que poden requerir molta memòria. Limitem a %1 la quantitat de volums carregant-se simultàniament.")
                    .arg(numberOfVolumesLoadingConcurrently));
            }
            else
            {
                INFO_LOG(QString("Windows 64 bits amb volums multiframe que poden requerir molta memòria. Limitem a %1 la quantitat de volums carregant-se "
                    "simultàniament.").arg(numberOfVolumesLoadingConcurrently));
            }
        }
        else
        {
            numberOfVolumesLoadingConcurrently = getWeaverInstance()->maximumNumberOfThreads();
        }
        m_resourceRestrictionPolicy.setCap(numberOfVolumesLoadingConcurrently);
        volumeReaderJob->assignQueuePolicy(&m_resourceRestrictionPolicy);
    }
}

bool AsynchronousVolumeReader::checkForResourceRestrictions(bool checkMultiframeImages, const QStringList &modalitiesWithoutRestriction)
{
    if (!checkMultiframeImages && modalitiesWithoutRestriction.isEmpty())
    {
        return false;
    }
    
    bool foundRestriction = false;
    QListIterator<Volume*> iterator(VolumeRepository::getRepository()->getItems());
    while (iterator.hasNext() && !foundRestriction)
    {
        Volume *currentVolume = iterator.next();
        // Mirem si és multiframe
        if (checkMultiframeImages)
        {
            if (currentVolume->isMultiframe())
            {
                foundRestriction = true;
            }
        }

        // Mirem si és una modalitat a la que cal aplicar restricció o no
        if (!modalitiesWithoutRestriction.isEmpty())
        {
            QString modality = currentVolume->getModality();
            if (!modalitiesWithoutRestriction.contains(modality))
            {
                foundRestriction = true;
            }
        }
    }

    return foundRestriction;
}

void AsynchronousVolumeReader::unmarkVolumeFromJobAsLoading(ThreadWeaver::Job *job)
{
    // TODO Aquí és el lloc més correcte per desmarcar el volume?? Així tenim el problema de que no podem destruïr aquest objecte
    // fins que s'ha finalitzat el job, si no, no es marcaria mai com a carregat. Si no es fa aquí, hem de tenir en compte
    // problemes de concurrència.
    VolumeReaderJob *volumeReaderJob = dynamic_cast<VolumeReaderJob*>(job);
    if (volumeReaderJob)
    {
        this->unmarkVolumeAsLoading(volumeReaderJob->getVolume());
    }
}

bool AsynchronousVolumeReader::isVolumeLoading(Volume *volume) const
{
    if (!volume)
    {
        DEBUG_LOG("El volum és nul. No pot estar carregant-se.");
        return false;
    }

    return m_volumesLoading.contains(volume->getIdentifier().getValue());
}

void AsynchronousVolumeReader::cancelLoadingAndDeleteVolume(Volume *volume)
{
    if (!volume)
    {
        DEBUG_LOG("El volum és nul. No es pot fer cancel de la càrrega ni esborrar.");
        return;
    }

    if (this->isVolumeLoading(volume))
    {
        DEBUG_LOG(QString("Volume %1 isLoading, trying dequeue").arg(volume->getIdentifier().getValue()));
        VolumeReaderJob *job = this->getVolumeReaderJob(volume);
        ThreadWeaver::Weaver *weaver = this->getWeaverInstance();
        if (weaver->dequeue(job))
        {
            delete volume;
        }
        else
        {
            DEBUG_LOG(QString("Volume %1 cannot be dequeued, requesting abort and delete").arg(volume->getIdentifier().getValue()));
            connect(job, SIGNAL(done(ThreadWeaver::Job*)), volume, SLOT(deleteLater()));
            job->requestAbort();
        }
    }
    else
    {
        delete volume;
    }
}

void AsynchronousVolumeReader::markVolumeAsLoadingByJob(Volume *volume, VolumeReaderJob *volumeReaderJob)
{
    DEBUG_LOG(QString("markVolumeAsLoading: Volume %1").arg(volume->getIdentifier().getValue()));
    m_volumesLoading.insert(volume->getIdentifier().getValue(), volumeReaderJob);
}

void AsynchronousVolumeReader::unmarkVolumeAsLoading(Volume *volume)
{
    DEBUG_LOG(QString("unmarkVolumeAsLoading: Volume %1").arg(volume->getIdentifier().getValue()));
    m_volumesLoading.remove(volume->getIdentifier().getValue());
}

ThreadWeaver::Weaver* AsynchronousVolumeReader::getWeaverInstance() const
{
    // TODO De moment es retorna la instància global, caldria permetre passar-la com a paràmetre.
    return ThreadWeaver::Weaver::instance();
}

VolumeReaderJob* AsynchronousVolumeReader::getVolumeReaderJob(Volume *volume) const
{
    if (this->isVolumeLoading(volume))
    {
        return m_volumesLoading.value(volume->getIdentifier().getValue());
    }
    else
    {
        return NULL;
    }
}

} // End namespace udg
