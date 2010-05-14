/***************************************************************************
 *   Copyright (C) 2005 by marc                                            *
 *   marc@localhost.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef RETRIEVEIMAGES
#define RETRIEVEIMAGES

#include <QString>

#include "ofcond.h"
#include "assoc.h"

struct T_ASC_Network;
struct T_DIMSE_C_MoveRQ;
struct T_DIMSE_C_MoveRSP;
struct T_DIMSE_C_StoreRQ;
struct T_DIMSE_StoreProgress;
struct T_DIMSE_C_StoreRSP;
struct T_DIMSE_Message;
struct StoreCallbackData;

class DcmDataset;
class DcmFileFormat;

namespace udg{

class Status;
class PacsConnection;
class DicomMask;
class DICOMTagReader;

/** 
    This class helps to interactive with the pacs, retrieve images that match with the mask
 */
class RetrieveImages
{
public:
   RetrieveImages();

   /** 
    * This action sets the connection that we will use to connect to the pacs
    * @param connection [in] Study's Open connection to the pacs
    */
   void setConnection(PacsConnection connection);

   /** 
    * Sets the network to use for retrieve the images
    * @param Network to use
    */
   void setNetwork(T_ASC_Network * network);

    /** 
     * This action sets the mask that we will use to retrieve the image in to the pacs.
     * @param mask Màscara de cerca
     */
   void setMask( DicomMask mask );

   /** 
    * Download the study's Image
    * @return state of the method
    */
   Status retrieve();

private:
    /// Request DICOM association;
    T_ASC_Association *m_assoc; 
    T_ASC_Network *m_net;
    DcmDataset *m_mask;

    /// En aquesta funció acceptem la connexió que se'ns sol·licita per transmetre'ns imatges, i indiquem quins transfer syntax suportem
    static OFCondition acceptSubAssoc( T_ASC_Network * aNet, T_ASC_Association ** assoc );

    static void moveCallback( void *callbackData, T_DIMSE_C_MoveRQ *req, int responseCount, T_DIMSE_C_MoveRSP *response );

    /// Responem a una petició d'echo
    static OFCondition echoSCP( T_ASC_Association * assoc, T_DIMSE_Message * msg, T_ASC_PresentationContextID presID );

    /** 
     * Aquesta funció s'encarrega de guardar cada trama DICOM que rebem
     * Paràmetres d'entrada
     * @param callbackData 
     * @param progress Progress state
     * @param req Original store request
     * @param imageFileName
     * @param imageDataSet Being received into
     * Paràmetres de sortida
     * @param rsp Final store response
     * @param statusDetail
     */
    static void storeSCPCallback( void *callbackData, T_DIMSE_StoreProgress *progress, T_DIMSE_C_StoreRQ *req, char *imageFileName, DcmDataset **imageDataSet, T_DIMSE_C_StoreRSP *rsp, DcmDataset **statusDetail );

    static OFCondition storeSCP( T_ASC_Association *assoc, T_DIMSE_Message *msg, T_ASC_PresentationContextID presID );

    /// Accepta la connexió que ens fa el PACS, per convertir-nos en un scp
    static OFCondition subOpSCP( T_ASC_Association **subAssoc );

    static void subOpCallback( void * /*subOpCallbackData*/, T_ASC_Network *aNet, T_ASC_Association **subAssoc );

    /// Guarda una composite instance descarregada
    static OFCondition save(DcmFileFormat *fileRetrieved, QString imageFileNameToSave);

    /// Retorna el nom del fitxer amb que s'ha de guardar l'objecte descarregat, composa el path on s'ha de guardar + el nom del fitxer, si el path on s'ha de guardar la imatge no existeix el crea
    static QString getCompositeInstanceFileName(DcmDataset *imageDataset);

    /// En cas d'error processa la resposta rebuda per part del SCP, grava l'error el log i el retorna en forma d'objecte Status
    Status processErrorResponseFromMoveSCP(T_DIMSE_C_MoveRSP *response, DcmDataset *statusDetail);
};

};
#endif
