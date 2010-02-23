#ifndef UDGIMAGEDATASOURCE_H
#define UDGIMAGEDATASOURCE_H

#include <QString>

namespace udg {

/**
 * Aquesta classe encapsula la informaci� font a partir de les quals generem Images
 * Cont� informaci� comuna i relativa a l'arxiu font. M�ltiples Image poden apuntar
 * al mateix ImageDataSource.
 */
class ImageDataSource {
public:
    ImageDataSource();
    ~ImageDataSource();

    /// Assigna/obt� el SOPInstanceUID de la inst�ncia
    void setSOPInstanceUID( const QString &uid );
    QString getSOPInstanceUID() const;

    /// Assigna/obt� el n�mero d'inst�ncia
    void setInstanceNumber( const QString &number );
    QString getInstanceNumber() const;

    /// Assigna/Obt� el nombre de frames
    void setNumberOfFrames( int frames );
    int getNumberOfFrames() const;

    /// Ens diu si les dades s�n de tipus multi-frame
    bool isMultiFrame() const;

    /// Assignar/Obtenir la descripci� del tipus d'imatge
    void setImageType( const QString &imageType );
    QString getImageType() const;
    
    /// Assigna/retorna el path absolut de la font de dades
    void setFilePath( const QString &path );
    QString getFilePath() const;

private:
    /// Atributs DICOM
    
    /// Identificador de la inst�ncia (0008,0018)
    QString m_SOPInstanceUID;

    /// N�mero d'inst�ncia (0020,0013) Tipus 2
    QString m_instanceNumber;
    
    /// Tipus d'imatge. Ens pot definir si es tracta d'un localizer, per exemple. Cont� els valors separats per '\\'
    /// Es troba al m�dul General Image C.7.6.1 i als m�duls Enhanced MR/CT/XA/XRF Image (C.8.13.1/C.8.15.2/C.8.19.2)
    QString m_imageType;

    /// Nombre de frames de la imatge. (0028,0008) Tipus 1
    int m_numberOfFrames;

    /// Atributs NO-DICOM

    /// Path de l'arxiu font
    QString m_filePath;
};

}

#endif
