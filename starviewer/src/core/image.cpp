/***************************************************************************
 *   Copyright (C) 2005-2006 by Grup de Gràfics de Girona                  *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#include "image.h"

#include "series.h"
#include "logging.h"
#include "thumbnailcreator.h"
#include "dicomtagreader.h"
#include "dicomdictionary.h"

#include <QStringList>
#include <QPainter>
#include <QBuffer>

#include <vtkMath.h> // pel ::Cross()

namespace udg {

Image::Image(QObject *parent)
 : QObject(parent), m_sliceThickness(0.0), m_samplesPerPixel(1), m_photometricInterpretation("MONOCHROME2"), m_rows(0), m_columns(0), m_bitsAllocated(16), m_bitsStored(16), m_pixelRepresentation(0), m_rescaleSlope(1), m_rescaleIntercept(0), m_parentSeries(NULL), m_frameNumber(0)
{
    m_pixelSpacing[0] = 0.;
    m_pixelSpacing[1] = 0.;
    memset( m_imageOrientationPatient, 0, 9*sizeof(double) );
    memset( m_imagePositionPatient, 0, 3*sizeof(double) );
}

Image::~Image()
{
}

void Image::setSOPInstanceUID( QString uid )
{
    m_SOPInstanceUID = uid;
}

QString Image::getSOPInstanceUID() const
{
    return m_SOPInstanceUID;
}

void Image::setInstanceNumber( QString number )
{
    m_instanceNumber = number;
}

QString Image::getInstanceNumber() const
{
    return m_instanceNumber;
}

void Image::setImageOrientationPatient( double orientation[6] )
{
    memcpy( m_imageOrientationPatient, orientation, 6*sizeof(double) );

    double normal[3];
    // calculem la Z
    vtkMath::Cross( &orientation[0] , &orientation[3] , normal );

    memcpy( &m_imageOrientationPatient[6], normal, 3*sizeof(double) );
}

void Image::setImageOrientationPatient( double xVector[3], double yVector[3] )
{
    memcpy( m_imageOrientationPatient, xVector, 3*sizeof(double) );
    memcpy( &m_imageOrientationPatient[3], yVector, 3*sizeof(double) );
    double normal[3];
    // calculem la Z
    vtkMath::Cross( xVector , yVector , normal );

    memcpy( &m_imageOrientationPatient[6], normal, 3*sizeof(double) );
}

const double* Image::getImageOrientationPatient() const
{
    return m_imageOrientationPatient;
}

void Image::setPatientOrientation( QString orientation )
{
    m_patientOrientation = orientation;
}

QString Image::getPatientOrientation() const
{
    return m_patientOrientation;
}

QString Image::getOrientationLabel()
{
    return getProjectionLabelFromPlaneOrientation( m_patientOrientation );
}

void Image::setPixelSpacing( double x, double y )
{
    m_pixelSpacing[0] = x;
    m_pixelSpacing[1] = y;
}

const double *Image::getPixelSpacing() const
{
    return m_pixelSpacing;
}

void Image::setSliceThickness( double z )
{
    m_sliceThickness = z;
}

double Image::getSliceThickness() const
{
    return m_sliceThickness;
}

void Image::setImagePositionPatient( double position[3] )
{
    memcpy( m_imagePositionPatient, position, 3*sizeof(double) );
}

const double *Image::getImagePositionPatient() const
{
    return m_imagePositionPatient;
}

void Image::getImagePlaneNormal( double normal[3] )
{
	normal[0] = m_imageOrientationPatient[6];
	normal[1] = m_imageOrientationPatient[7];
	normal[2] = m_imageOrientationPatient[8];
}

void Image::setSamplesPerPixel( int samples )
{
    m_samplesPerPixel = samples;
}

int Image::getSamplesPerPixel() const
{
    return m_samplesPerPixel;
}

void Image::setPhotometricInterpretation( QString value )
{
    m_photometricInterpretation = value;
}

QString Image::getPhotometricInterpretation() const
{
    return m_photometricInterpretation;
}

void Image::setRows( int rows  )
{
    m_rows = rows;
}

int Image::getRows() const
{
    return m_rows;
}

void Image::setColumns( int columns  )
{
    m_columns = columns;
}

int Image::getColumns() const
{
    return m_columns;
}

void Image::setBitsAllocated( int bits )
{
    m_bitsAllocated = bits;
}

int Image::getBitsAllocated() const
{
    return m_bitsAllocated;
}

void Image::setBitsStored( int bits )
{
    m_bitsStored = bits;
}

int Image::getBitsStored() const
{
    return m_bitsStored;
}

void Image::setHighBit( int highBit )
{
    m_highBit = highBit;
}

int Image::getHighBit() const
{
    return m_highBit;
}

void Image::setPixelRepresentation( int representation )
{
    m_pixelRepresentation = representation;
}

int Image::getPixelRepresentation() const
{
    return m_pixelRepresentation;
}

void Image::setRescaleSlope( double slope )
{
    m_rescaleSlope = slope;
}

double Image::getRescaleSlope() const
{
    return m_rescaleSlope;
}

void Image::setRescaleIntercept( double intercept )
{
    m_rescaleIntercept = intercept;
}

double Image::getRescaleIntercept() const
{
    return m_rescaleIntercept;
}

void Image::setSliceLocation( QString sliceLocation )
{
    m_sliceLocation = sliceLocation;
}

QString Image::getSliceLocation() const
{
    return m_sliceLocation;
}

void Image::addWindowLevel( double window, double level )
{
    QPair<double, double> windowLevel( window, level );
    m_windowLevelList << windowLevel;
}

QPair<double,double> Image::getWindowLevel( int index ) const
{
    if( index >= 0 && index < m_windowLevelList.size() )
        return m_windowLevelList.at(index);
    else
    {
        DEBUG_LOG("Index out of range");
        return QPair<double,double>();
    }
}

int Image::getNumberOfWindowLevels()
{
    return m_windowLevelList.size();
}

void Image::setNumberOfFrames( int frames )
{
    m_numberOfFrames = frames;
}

int Image::getNumberOfFrames() const
{
    return m_numberOfFrames;
}

bool Image::isMultiFrame() const
{
    if( getNumberOfFrames() > 1 )
        return true;
    else
        return false;
}

void Image::addWindowLevelExplanation( QString explanation )
{
    m_windowLevelExplanationList << explanation;
}

void Image::setWindowLevelExplanations( const QStringList &explanations )
{
    m_windowLevelExplanationList = explanations;
}

QString Image::getWindowLevelExplanation( int index ) const
{
    if( index >= 0 && index < m_windowLevelExplanationList.size() )
        return m_windowLevelExplanationList.at(index);
    else
    {
        return QString();
    }
}

void Image::setRetrievedDate(QDate retrievedDate)
{
    m_retrievedDate = retrievedDate;
}

void Image::setRetrievedTime(QTime retrievedTime)
{
    m_retrieveTime = retrievedTime;
}

QDate Image::getRetrievedDate()
{
    return m_retrievedDate;
}

QTime Image::getRetrievedTime()
{
    return m_retrieveTime;
}

void Image::setImageType( const QString &imageType )
{
    m_imageType = imageType;
}

QString Image::getImageType() const
{
    return m_imageType;
}

void Image::setFrameType( const QString &frameType )
{
    m_frameType = frameType;
}

QString Image::getFrameType() const
{
    return m_frameType;
}

void Image::setViewPosition( const QString &viewPosition )
{
    m_viewPosition = viewPosition;
}

QString Image::getViewPosition() const
{
    return m_viewPosition;
}

void Image::setImageLaterality( const QChar &imageLaterality )
{
    m_imageLaterality = imageLaterality;
}

QChar Image::getImageLaterality() const
{
    return m_imageLaterality;
}

void Image::setViewCodeMeaning( const QString &viewCodeMeaning )
{
    m_viewCodeMeaning = viewCodeMeaning;
}

QString Image::getViewCodeMeaning() const
{
    return m_viewCodeMeaning;
}

void Image::setFrameNumber( int frameNumber )
{
    m_frameNumber = frameNumber;
}

int Image::getFrameNumber() const
{
    return m_frameNumber;
}

void Image::setContentTime( const QString &contentTime )
{
    m_contentTime = contentTime;
}

QString Image::getContentTime() const
{
    return m_contentTime;
}

QString Image::getFormattedContentTime() const
{
    // TODO Ara hem de llegir de disc, ja que aquesta informació no s'obté dels fillers steps i tampoc s'insereix a la base de dades.
    QString time;
    DICOMTagReader reader( m_path );
    time = reader.getAttributeByName( DICOMContentTime );
    if( !time.isEmpty() )
    {
        // Seguim la suggerència de la taula 6.2-1 de la Part 5 del DICOM standard de tenir en compte el format hh:mm:ss.frac
        time = time.remove(":");

        QStringList split = time.split(".");
        QTime convertedTime = QTime::fromString(split[0], "hhmmss");

        if (split.size() == 2) // Té fracció al final
        {
            // Trunquem a milisegons i no a milionèssimes de segons
            convertedTime = convertedTime.addMSecs( split[1].leftJustified(3,'0',true).toInt() );
        }
        time = convertedTime.toString("HH:mm:ss");
    }

    return time;
}

QString Image::getKeyIdentifier() const
{
    return m_SOPInstanceUID + "#" + m_frameNumber;
}

void Image::setParentSeries( Series *series )
{
    m_parentSeries = series;
    this->setParent( m_parentSeries );
}

Series *Image::getParentSeries() const
{
    return m_parentSeries;
}

void Image::setPath( QString path )
{
    m_path = path;
}

QString Image::getPath() const
{
    return m_path;
}

void Image::addReferencedImage( Image *image )
{
    QString uid = image->getSOPInstanceUID();
    if( uid.isEmpty() )
    {
        DEBUG_LOG("L'uid de la imatge està buit! No la podem insertar per inconsistent");
    }
    else
    {
        m_referencedImageSequence.push_back( image );
    }

}

Image *Image::getReferencedImage( QString SOPInstanceUID ) const
{
    int i = 0;
    bool found = false;
    while( i < m_referencedImageSequence.size() && !found )
    {
        if( m_referencedImageSequence.at(i)->getSOPInstanceUID() == SOPInstanceUID )
            found = true;
        else
            i++;
    }
    if( !found )
        return 0;
    else
        return m_referencedImageSequence.at(i);
}

QList<Image *> Image::getReferencedImages() const
{
    return m_referencedImageSequence;
}

bool Image::hasReferencedImages() const
{
    return ! m_referencedImageSequence.isEmpty();
}

QPixmap Image::getThumbnail(int resolution)
{
    ThumbnailCreator thumbnailCreator;

    if (m_thumbnail.isNull())
    {
        m_thumbnail = QPixmap::fromImage(thumbnailCreator.getThumbnail(this, resolution));
    }
    return m_thumbnail;
}

QString Image::getProjectionLabelFromPlaneOrientation( const QString &orientation )
{
    QString label;
    
    QStringList axisList = orientation.split("\\");
    // comprovem si tenim les annotacions esperades
    if( axisList.count() >= 2 )
    {
        QString rowAxis = axisList.at(0).trimmed();
        QString columnAxis = axisList.at(1).trimmed();

        if( !rowAxis.isEmpty() && !columnAxis.isEmpty() )
        {
            if( (rowAxis.startsWith("R") || rowAxis.startsWith("L")) && (columnAxis.startsWith("A") || columnAxis.startsWith("P")) )
                label="AXIAL";
            else if( (columnAxis.startsWith("R") || columnAxis.startsWith("L")) && (rowAxis.startsWith("A") || rowAxis.startsWith("P")) )
                label="AXIAL";
            else if ( (rowAxis.startsWith("R") || rowAxis.startsWith("L")) && (columnAxis.startsWith("H") || columnAxis.startsWith("F")) )
                label="CORONAL";
            else if( (columnAxis.startsWith("R") || columnAxis.startsWith("L")) && (rowAxis.startsWith("H") || rowAxis.startsWith("F")) )
                label="CORONAL";
            else if( (rowAxis.startsWith("A") || rowAxis.startsWith("P")) && (columnAxis.startsWith("H") || columnAxis.startsWith("F")) )
                label="SAGITAL";
            else if( (columnAxis.startsWith("A") || columnAxis.startsWith("P")) && (rowAxis.startsWith("H") || rowAxis.startsWith("F")) )
                label="SAGITAL";
            else
                label="OBLIQUE";
        }
        else
        {
            label="N/A";
        }
    }

    return label;
}

QStringList Image::getSupportedModalities()
{
    // Modalitats extretes de DICOM PS 3.3 C.7.3.1.1.1
    QStringList supportedModalities;
    // Modalitats que sabem que són d'imatge i que en principi hem de poder suportar
    supportedModalities << "CR" << "CT" << "MR" << "US" << "BI" << "DD" << "ES" << "PT" << "ST" << "XA" << "RTIMAGE" << "DX" << "IO" << "GM" << "XC" << "OP" << "NM" << "OT" << "CD" << "DG" << "LS" << "RG" << "TG" << "RF" << "MG" << "PX" << "SM" << "ECG" << "IVUS";
    // Modalitats "no estàndars" però que es correspondrien amb imatges que podem suportar
    supportedModalities << "SC";

    // Aquestes modalitats en principi no són d'imatge. Les mantenim documentades per si calgués incloure-les a la llista
    // TODO Cal comprovar si són modalitats d'imatge i eliminar-les segons el cas
    // "RTSTRUCT" << "RTRECORD" << "EPS" << "RTDOSE" << "RTPLAN" << "HD" << "SMR" << "AU"

    return supportedModalities;
}

}
