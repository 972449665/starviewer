/***************************************************************************
 *   Copyright (C) 2005-2006 by Grup de Gràfics de Girona                  *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#include "series.h"

#include "study.h"
#include "image.h"
#include "logging.h"
#include "volumerepository.h"
#include "thumbnailcreator.h"

#include <QStringList>
#include <QPainter>

namespace udg {

Series::Series(QObject *parent)
 : QObject(parent), m_modality("OT"), m_selected(false), m_parentStudy(NULL), m_numberOfPhases(1), m_numberOfSlicesPerPhase(1), m_numberOfImages(0)
{
}

Series::~Series()
{
}

void Series::setInstanceUID( QString uid )
{
    m_seriesInstanceUID = uid;
}

QString Series::getInstanceUID() const
{
    return m_seriesInstanceUID;
}

void Series::setParentStudy( Study *study )
{
    m_parentStudy = study;
    this->setParent( m_parentStudy );
}

Study *Series::getParentStudy() const
{
    return m_parentStudy;
}

bool Series::addImage( Image *image )
{
    bool ok = true;
    QString uid = image->getSOPInstanceUID();
    if( uid.isEmpty() )
    {
        ok = false;
        DEBUG_LOG("L'uid de la imatge està buit! No la podem insertar per inconsistent");
    }
    else if( this->imageExists(uid) )
    {
        ok = false;
        DEBUG_LOG("Ja existeix una imatge amb aquest mateix SOPInstanceUID:: " + uid );
    }
    else
    {
        image->setParentSeries( this );
        // TODO això provocava que les imatges amb el mateix instance number s'inserissin allà mateix. Fem un add directament i avall. Això provocava el bug definit al ticket https://trueta.udg.edu/trac/ticket/239.
        // Cal replantejar.se si l'ordenació la fa el mètode insertImage o ho fa un fillerStep
//         this->insertImage( image );
        m_imageSet << image;
    }

    return ok;
}

Image *Series::getImage( QString SOPInstanceUID )
{
    int index = this->findImageIndex(SOPInstanceUID);
    if( index != -1 )
        return m_imageSet[index];
    else
        return NULL;
}

bool Series::imageExists( QString sopInstanceUID )
{
    if( this->findImageIndex(sopInstanceUID) != -1 )
        return true;
    else
        return false;
}

QList<Image *> Series::getImages() const
{
    return m_imageSet;
}

void Series::setImages( QList<Image *> imageSet )
{
    // Buidar la llista abans d'afegir-hi la nova
    m_imageSet.clear();

    m_imageSet = imageSet;
}

int Series::getNumberOfImages()
{
    return m_imageSet.isEmpty() ? m_numberOfImages : m_imageSet.size();
}

void Series::setNumberOfImages(int numberOfImages)
{
    m_numberOfImages = numberOfImages;
}

bool Series::hasImages() const
{
    return ! m_imageSet.isEmpty();
}

void Series::setSOPClassUID( QString sopClassUID )
{
    m_sopClassUID = sopClassUID;
}

QString Series::getSOPClassUID() const
{
    return m_sopClassUID;
}

void Series::setModality( QString modality )
{
    m_modality = modality;
}

QString Series::getModality() const
{
    return m_modality;
}

void Series::setSeriesNumber( QString number )
{
    m_seriesNumber = number;
}

QString Series::getSeriesNumber() const
{
    return m_seriesNumber;
}

void Series::setFrameOfReferenceUID( QString uid )
{
    m_frameOfReferenceUID = uid;
}

QString Series::getFrameOfReferenceUID() const
{
    return m_frameOfReferenceUID;
}

void Series::setPositionReferenceIndicator( QString position )
{
    m_positionReferenceIndicator = position;
}

QString Series::getPositionReferenceIndicator() const
{
    return m_positionReferenceIndicator;
}

void Series::setDescription( QString description )
{
    m_description = description;
}

QString Series::getDescription() const
{
    return m_description;
}

void Series::setPatientPosition( QString position )
{
    m_patientPosition = position;
}

QString Series::getPatientPosition() const
{
    return m_patientPosition;
}

void Series::setProtocolName( QString protocolName )
{
    m_protocolName = protocolName;
}

QString Series::getProtocolName() const
{
    return m_protocolName;
}

void Series::setImagesPath( QString imagesPath )
{
    m_imagesPath = imagesPath;
}

QString Series::getImagesPath() const
{
    return m_imagesPath;
}

bool Series::setDateTime( int day , int month , int year , int hour , int minute, int second )
{
    return this->setDate( day, month, year ) && this->setTime( hour, minute, second );
}

bool Series::setDateTime( QString date , QString time )
{
    return this->setDate( date ) && this->setTime( time );
}

bool Series::setDate( int day , int month , int year )
{
    return this->setDate( QDate( year , month , day ) );
}

bool Series::setDate( QString date )
{
    // Seguim la suggerència de la taula 6.2-1 de la Part 5 del DICOM standard de tenir en compte el format yyyy.MM.dd
    return this->setDate( QDate::fromString(date.remove("."), "yyyyMMdd") );
}

bool Series::setDate( QDate date )
{
    bool ok = true;
    if( date.isValid() )
    {
        m_date = date;
        ok = true;
    }
    else if( !date.isNull() )
    {
        DEBUG_LOG("La data està en un mal format: " + date.toString( Qt::LocaleDate ) );
        ok = false;
    }
    return ok;
}

bool Series::setTime( int hour , int minute, int second )
{
    return this->setTime( QTime(hour, minute, second) );
}

bool Series::setTime( QString time )
{
    // Seguim la suggerència de la taula 6.2-1 de la Part 5 del DICOM standard de tenir en compte el format hh:mm:ss.frac
    time = time.remove(":");

    QStringList split = time.split(".");
    QTime convertedTime = QTime::fromString(split[0], "hhmmss");

    if (split.size() == 2) //té fracció al final
    {
        // Trunquem a milisegons i no a milionèssimes de segons
        convertedTime = convertedTime.addMSecs( split[1].leftJustified(3,'0',true).toInt() );
    }

    return this->setTime( convertedTime );
}

bool Series::setTime(QTime time)
{
    bool ok = true;
    if (time.isValid())
    {
        m_time = time;
        ok = true;
    }
    else if( !time.isNull() )
    {
        DEBUG_LOG( "El time està en un mal format" );
        ok = false;
    }
    return ok;
}

QDate Series::getDate()
{
    return m_date;
}

QString Series::getDateAsString()
{
    return m_date.toString( Qt::LocaleDate );
}

QTime Series::getTime()
{
    return m_time;
}

QString Series::getTimeAsString()
{
    return m_time.toString( "HH:mm:ss" );
}

void Series::setInstitutionName( QString institutionName )
{
    m_institutionName = institutionName;
}

QString Series::getInstitutionName() const
{
    return m_institutionName;
}

void Series::setBodyPartExamined( QString bodyPart )
{
    m_bodyPartExamined = bodyPart;
}

QString Series::getBodyPartExamined() const
{
    return m_bodyPartExamined;
}

void Series::setViewPosition( QString viewPosition )
{
    m_viewPosition = viewPosition;
}

QString Series::getViewPosition() const
{
    return m_viewPosition;
}

void Series::setRequestedProcedureID( const QString &requestedProcedureID )
{
    m_requestedProcedureID = requestedProcedureID;
}

QString Series::getRequestedProcedureID() const
{
    return m_requestedProcedureID;
}

void Series::setScheduledProcedureStepID( const QString &scheduledProcedureID )
{
    m_scheduledProcedureStepID = scheduledProcedureID;
}

QString Series::getScheduledProcedureStepID() const
{
    return m_scheduledProcedureStepID;
}

void Series::setPerformedProcedureStepStartDate(const QString &startDate )
{
    m_performedProcedureStepStartDate = startDate;
}

QString Series::getPerformedProcedureStepStartDate() const
{
    return m_performedProcedureStepStartDate;
}

void Series::setPerformedProcedureStepStartTime( const QString &startTime )
{
    m_performedProcedureStepStartTime = startTime;
}

QString Series::getPerformedProcedureStepStartTime() const
{
    return m_performedProcedureStepStartTime;
}

int Series::getNumberOfVolumes()
{
    return m_volumesList.size();
}

Volume* Series::getVolume(Identifier id)
{
    int index = m_volumesList.indexOf(id);
    return index != -1 ? VolumeRepository::getRepository()->getVolume(m_volumesList[index]) : NULL;
}

Volume* Series::getFirstVolume()
{
    return m_volumesList.isEmpty() ? NULL : this->getVolume(m_volumesList[0]);
}

Identifier Series::addVolume(Volume *volume)
{
    Identifier volumeID = VolumeRepository::getRepository()->addVolume(volume);
    m_volumesList.append(volumeID);
    volume->setIdentifier(volumeID);
    return volumeID;
}

QList<Volume*> Series::getVolumesList()
{
    QList<Volume*> volumesList;
    foreach (Identifier id, m_volumesList)
    {
        volumesList << VolumeRepository::getRepository()->getVolume(id);
    }
    return volumesList;
}

QList<Identifier> Series::getVolumesIDList() const
{
    return m_volumesList;
}

void Series::addFilePath(QString filePath)
{
    m_filesPathList.insert(filePath);
}

void Series::removeFilePath(QString filePath)
{
    m_filesPathList.remove(filePath);
}

QStringList Series::getFilesPathList()
{
   QList<QString> list = m_filesPathList.toList();
   qSort(list);
   return list;
}

QStringList Series::getImagesPathList()
{
    QStringList pathList;
    foreach( Image *image, m_imageSet )
    {
        pathList << image->getPath();
    }

    return pathList;
}

bool Series::isSelected() const
{
    return m_selected;
}

QString Series::toString(bool verbose)
{
    QString result;

    result += "        - Series\n";
    result += "            SeriesNumber : " + getSeriesNumber() + "\n";
    result += "            Modality : " + getModality() + "\n";
    result += "            Description : " + getDescription() + "\n";
    result += "            ProtocolName : " + getProtocolName() + "\n";
    result += "            Num.Images : " + QString::number( getImages().size() ) + "\n";

    if (verbose)
    {
        foreach (Image *image, getImages())
        {
            result += "            - Image " + image->getPath() + "\n";
        }
    }
    // TODO Idem per PS, KIN....

    return result;
}

void Series::select()
{
    setSelectStatus( true );
}

void Series::unSelect()
{
    setSelectStatus( false );
}

void Series::setSelectStatus( bool select )
{
    m_selected = select;
}

QPixmap Series::getThumbnail()
{
    if (m_seriesThumbnail.isNull())
    {
        ThumbnailCreator thumbnailCreator;
        m_seriesThumbnail = QPixmap::fromImage(thumbnailCreator.getThumbnail(this));
    }

    return m_seriesThumbnail;
}


Image *Series::getImageByIndex( int index )
{
	Image *resultImage = 0;
	if( index >= 0 && index < m_imageSet.count() ) // està dins del rang
	{
		resultImage = m_imageSet.at( index );
	}

	return resultImage;
}

void Series::setThumbnail(QPixmap seriesThumbnail)
{
    m_seriesThumbnail = seriesThumbnail;
}

bool Series::isViewable() const
{
    if( m_modality == "KO" || m_modality == "PR" || m_modality == "SR" || !this->hasImages() )
        return false;
    else
        return true;
}

void Series::setNumberOfPhases( int phases )
{
    m_numberOfPhases = phases;
}

int Series::getNumberOfPhases() const
{
    return m_numberOfPhases;
}

void Series::setNumberOfSlicesPerPhase( int slices )
{
    m_numberOfSlicesPerPhase = slices;
}

int Series::getNumberOfSlicesPerPhase() const
{
    return m_numberOfSlicesPerPhase;
}

void Series::setManufacturer( QString manufacturer )
{
    m_manufacturer = manufacturer;
}

QString Series::getManufacturer() const
{
    return m_manufacturer;
}

void Series::setRetrievedDate(QDate retrievedDate)
{
    m_retrievedDate = retrievedDate;
}

void Series::setRetrievedTime(QTime retrievedTime)
{
    m_retrieveTime = retrievedTime;
}

QDate Series::getRetrievedDate()
{
    return m_retrievedDate;
}

QTime Series::getRetrievedTime()
{
    return m_retrieveTime;
}

void Series::insertImage( Image *image )
{
    int i = 0;
    while( i < m_imageSet.size() && m_imageSet.at(i)->getInstanceNumber().toInt() < image->getInstanceNumber().toInt() )
    {
        i++;
    }
    m_imageSet.insert( i, image );
}

int Series::findImageIndex( QString sopInstanceUID )
{
    int i = 0;
    bool found = false;
    while( i < m_imageSet.size() && !found )
    {
        if( m_imageSet.at(i)->getSOPInstanceUID() == sopInstanceUID )
            found = true;
        else
            i++;
    }
    if( !found )
        i = -1;

    return i;
}

}
