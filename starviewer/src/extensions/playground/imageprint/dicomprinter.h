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
    
    /// Posa les mides disponibles d'impressi� (\p _fileSize �s una llista d'opcions separades per "\").
    void setFileSize(const QString &_fileSize);	

    /// Posa els layouts disponibles per la impressi� (\p _layout �s una llista d'opcions separades per "\").
    void setLayout(const QString &_layout);	

    /// Posa el Magnification type disponibles per la impressi� (\p _magnificationType �s una llista d'opcions separades per "\").
    void setMagnificationType(const QString &_magnificationType);	
    
    /// Posa la Reflecci� per defecte.
    void setDefaultReflection(const int &_defaultReflection);

    /// Posa la resoluci� m�xima de la impressi�.
    void setMaxPrintResolution(const int &_x, const int &_v);

    /// Posa la resoluci� m�nima de la impressi�.
    void setMinPrintResolution(const int &_x, const int &_v);
    
    /// Posa la resoluci� m�xima de previsualitzaci� de la impressi�.
    void setMaxPreviewResolution(const int &_x, const int &_v);

    /// Posa si es suporta l'�s de TimBox o no.
    void setSupportsTrim(bool _trim);
	
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
    QString  getFileSize() const;
    QString  getLayout() const;
    QString	 getMagnificationType() const;
	int		 getDefaultReflection() const;
    int		 getMaxPrintResolutionH()const;
	int		 getMaxPrintResolutionV()const;
	int		 getMinPrintResolutionH()const;
	int		 getMinPrintResolutionV()const;
	int		 getMaxPreviewResolutionH()const;
	int		 getMaxPreviewResolutionV()const;
	bool     getSupportsTrim() const;
    QString  getBorderDensity() const;
    QString  getEmptyImageDensity() const;
    QString  getFilmDestination() const;

	DicomPrinter& operator= (const DicomPrinter&);

private:		
	
    QString		m_description;
    QString		m_mediumType;
    QString		m_fileSize;
    QString		m_layout;
    QString		m_magnificationType;
    int         m_defaultReflection;
    int         m_maxPrintResolutionH;
    int         m_maxPrintResolutionV;	
    int         m_minPrintResolutionH;
    int         m_minPrintResolutionV;
    int         m_maxPreviewResolutionH;
    int         m_maxPreviewResolutionV;
    bool        m_trim;
    QString     m_borderDensity;
    QString     m_emptyImageDensity;
    QString     m_filmDestination;
};
}; 
#endif
