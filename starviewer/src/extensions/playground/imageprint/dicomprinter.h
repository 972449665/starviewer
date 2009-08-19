#ifndef UDGDICOMPRINTER_H
#define UDGDICOMPRINTER_H

#include <QString>

#include "printer.h"

///Representaci� dels par�metres d'una impressora DICOM.

namespace udg
{   

class DicomPrinter : public Printer
{
public:
    DicomPrinter();
    ~DicomPrinter();
    
    /// Posa el nom de la impressora.
    void setName(const QString &_name);

    /// Posa el tipus de la impressora.
    void setType(const QString &_type);
    
    /// Posa la descripci� de la impressora.
    void setDescription(const QString &_description);	
    
    ///Especifica/retorna el AETitle de la impressora
    void setAETitle(const QString &aeTitle);
    QString getAETitle() const;

    /// Posa el HostName (IP) de la impressora.
    void setHostname(const QString &_hostname);
    
    /// Posa el Port de la impressora.
    void setPort(const int &_port);

    /// Posa els mitjans disponibles per la impressi� (\p _mediumType �s una llista d'opcions separades per "\").
    void setDefaultMediumType(const QString &_mediumType);
    
    /// Posa les mides disponibles d'impressi� (\p filmSize �s una llista d'opcions separades per "\").
    void setDefaultFilmSize(const QString &filmSize);	

    /// Posa els layouts disponibles per la impressi� (\p _layout �s una llista d'opcions separades per "\").
    void setDefaultFilmLayout(const QString &_filmLayout);	

    ///Especifica retorna la orientaci� (horitzontal/vertical) per defecte amb la que s'ha d'imprimir la placa
    void setDefaultFilmOrientation(const QString &filmOrientation);	
    QString getDefaultFilmOrientation() const;

    /// Posa el Magnification type disponibles per la impressi� (\p _magnificationType �s una llista d'opcions separades per "\").
    void setDefaultMagnificationType(const QString &_magnificationType);	
    
    ///Especifica/retorna la resoluci� m�xima de la impressi�.
    void setDefaultMaxDensity(ushort maxDensity);
    ushort getDefaultMaxDensity();

    ///Especifica/retorna la resoluci� m�nima de la impressi�
    void setDefaultMinDensity(ushort minDensity);
    ushort getDefaultMinDensity();
    
    /// Posa si es suporta l'�s de TimBox o no.
    void setDefaultTrim(bool _trim);
	
    /// Posa les densitats disponibles en el Border  (\p _borderDensity �s una llista d'opcions separades per "\").
    void setDefaultBorderDensity(const QString &_borderDensity);

    /// Posa les densitats disponibles en la zona buida (EmptyImage) de la impressi�  (\p _borderDensity �s una llista d'opcions separades per "\").
    void setDefaultEmptyImageDensity(const QString &_emptyImageDensity);

    /// Posa els destins disponibles per la impressi�  (\p _borderDensity �s una llista d'opcions separades per "\").
    void setDefaultFilmDestination(const QString &_filmDestination);

    /// Retorna el nom de la impressora.
    QString  getName() const;
    
    /// Retorna el tipus de la impressora.
    QString  getType() const;

    /// Retorna la descripci� de la impressora.
	QString  getDescription() const;

    /// Retorna el HostName (IP) de la impressora.
   	QString  getHostname() const;

    /// Retorna el Port de la impressora.
 	int		 getPort() const;
    
    QString  getDefaultMediumType() const; 
    QString  getDefaultFilmSize() const;
    QString  getDefaultFilmLayout() const;
    QString	 getDefaultMagnificationType() const;
	bool     getDefaultTrim() const;
    QString  getDefaultBorderDensity() const;
    QString  getDefaultEmptyImageDensity() const;
    QString  getDefaultFilmDestination() const;

    ///Especifica/Retorna el smoothing type per defecte amb la que s'imprimir�
    void setDefaultSmoothingType(const QString &smoothingType);
    QString getDefaultSmoothingType() const;

    ///Especifica/retorna la prioritat per defecte dels jobs que s'imprimeixin amb aquesta impressora
    void setDefaultPrintPriority(const QString &priority);
    QString getDefaultPrintPriority() const;

    ///Especifica/retorna la polaritat per defecte de l'impressora
    void setDefaultPolarity(const QString &polarity);
    QString getDefaultPolarity() const;

	DicomPrinter& operator= (const DicomPrinter&);

private:		
	
    QString m_aeTitle;
    QString		m_defaultDescription;
    QString		m_defaultMediumType;
    QString		m_defaultFilmSize;
    QString		m_defaultFilmLayout;
    QString m_defaultFilmOrientation;
    QString		m_defaultMagnificationType;
    bool        m_defaultTrim;
    QString     m_defaultBorderDensity;
    QString     m_defaultEmptyImageDensity;
    QString     m_defaultFilmDestination;
    QString     m_defaultSmoothingType;
    QString     m_defaultPrintPriority;
    QString     m_defaultPolarity;
    ushort m_defaultMinDensity, m_defaultMaxDensity;
};
}; 
#endif
