#ifndef UDGPRINTDICOMSPOOL_H
#define UDGPRINTDICOMSPOOL_H

#include <QString>

#include "dicomprinter.h"
#include "dicomprintjob.h"

///Representaci� d'una impressora DICOM amb les eines DCMTK.
class DVPSStoredPrint;
class DVPSPrintMessageHandler;
class DcmDataset;
class OFCondition;

namespace udg
{   

class PrintDicomSpool
{
public:

    void printBasicGrayscale(DicomPrinter dicomPrinter, DicomPrintJob dicomPrintjob, const QString &storedPrintDcmtkFilePath, const QString &spoolDirectoryPath);

private:		
	
    /*TODO: Potser que daria m�s ent�s no guardar les variables com a membre i desdle m�tode printSpool passar-la al m�tode que 
     *que les necessiti. Per exemple printSCUCreateBasicFilmSession necessita DicomPrintJob*/
    DicomPrinter m_dicomPrinter;
    DicomPrintJob m_dicomPrintJob;

    DVPSStoredPrint* loadStoredPrintFileDcmtk(const QString &pathStoredPrintDcmtkFile);

    void printStoredPrintDcmtkContent(DVPSPrintMessageHandler &printerConnection, const QString storedPrintDcmtkFilePath, const QString &spoolDirectoryPath);

    //Retorna els atributs del FilmSession en un Dataset
    DcmDataset getAttributesBasicFilmSession();

    ///Una vegada creada la FilmSession i FilmBox aquest m�tode ens permet enviar una imatge a imprimir cap a la impressora
    OFCondition createAndSendBasicGrayscaleImageBox(DVPSPrintMessageHandler& printConnection, DVPSStoredPrint *storedPrintDcmtk, size_t imageNumber, const QString &spoolDirectoryPath);
};
}; 
#endif
