#ifndef UDGDICOMPRINTER_H
#define UDGDICOMPRINTER_H

#include <QString>
#include "printer.h"

///Representaci� d'una impressora DICOM amb les eines DCMTK.

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
    
    /// Posa el HostName (IP) de la impressora.
    void setHostname(const QString &_hostname);
    
    /// Posa el Port de la impressora.
    void setPort(const int &_port);

    /// Posa els mitjans disponibles per la impressi� (\p _mediumType �s una llista d'opcions separades per "\").
    void setMediumType(const QString &_mediumType);
    
    /// Posa les mides disponibles d'impressi� (\p filmSize �s una llista d'opcions separades per "\").
    void setFilmSize(const QString &filmSize);	

    /// Posa els layouts disponibles per la impressi� (\p _layout �s una llista d'opcions separades per "\").
    void setFilmLayout(const QString &_filmLayout);	

    /// Posa el Magnification type disponibles per la impressi� (\p _magnificationType �s una llista d'opcions separades per "\").
    void setMagnificationType(const QString &_magnificationType);	
    
    ///Especifica/retorna la resoluci� m�xima de la impressi�.
    void setMaxDensity(ushort maxDensity);
    ushort getMaxDensity();

    ///Especifica/retorna la resoluci� m�nima de la impressi�
    void setMinDensity(ushort minDensity);
    ushort getMinDensity();
    
    /// Posa la resoluci� m�xima de previsualitzaci� de la impressi�.
    void setMaxPreviewResolution(const int &_x, const int &_v);

    /// Posa si es suporta l'�s de TimBox o no.
    void setTrim(bool _trim);
	
    /// Posa les densitats disponibles en el Border  (\p _borderDensity �s una llista d'opcions separades per "\").
    void setBorderDensity(const QString &_borderDensity);

    /// Posa les densitats disponibles en la zona buida (EmptyImage) de la impressi�  (\p _borderDensity �s una llista d'opcions separades per "\").
    void setEmptyImageDensity(const QString &_emptyImageDensity);

    /// Posa els destins disponibles per la impressi�  (\p _borderDensity �s una llista d'opcions separades per "\").
    void setFilmDestination(const QString &_filmDestination);

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
    
    QString  getMediumType() const; 
    QString  getFilmSize() const;
    QString  getFilmLayout() const;
    QString	 getMagnificationType() const;
	bool     getTrim() const;
    QString  getBorderDensity() const;
    QString  getEmptyImageDensity() const;
    QString  getFilmDestination() const;

    ///Especifica/Retorna el smoothing type per defecte amb la que s'imprimir�
    void setSmoothingType(const QString &smoothingType);
    QString getSmoothingType() const;

    ///Especifica/retorna la prioritat per defecte dels jobs que s'imprimeixin amb aquesta impressora
    void DicomPrinter::setJobPriority(const QString &priority);
    QString DicomPrinter::getJobPriority() const;

    ///Especifica/retorna la polaritat per defecte de l'impressora
    void DicomPrinter::setPolarity(const QString &polarity);
    QString DicomPrinter::getPolarity() const;

	DicomPrinter& operator= (const DicomPrinter&);

private:		
	
    QString		m_description;
    QString		m_mediumType;
    QString		m_filmSize;
    QString		m_filmLayout;
    QString		m_magnificationType;
    bool        m_trim;
    QString     m_borderDensity;
    QString     m_emptyImageDensity;
    QString     m_filmDestination;
    QString     m_smoothingType;
    QString     m_priority;
    QString     m_polarity;
    ushort m_minDensity, m_maxDensity;
};
}; 
#endif
