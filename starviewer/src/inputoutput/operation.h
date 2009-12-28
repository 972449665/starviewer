/***************************************************************************
 *   Copyright (C) 2005 by Grup de Gràfics de Girona                       *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#ifndef UDGOPERATION_H
#define UDGOPERATION_H

#include "dicommask.h"
#include "pacsdevice.h"

class QString;

namespace udg {


/** Defineix les possibles operacions que es poden dur a terme amb el PACS, descarregar imatges i visualitzar images (la cerca es tracta en d'altres classes )
	@author Grup de Gràfics de Girona  ( GGG ) <vismed@ima.udg.es>
*/
class Operation{

public:

    enum OperationPriority { Low = 99, Medium = 50, High = 0 };
    enum OperationAction { Retrieve, Print , Move, View, Load };

    Operation();

    /** Especifica la màscara de l'estudi per aquella operacio,
     * @param mask màscara de l'operació
     */
    void setDicomMask( DicomMask mask );

    /** Estableix la prioritat de l'aplicació si no s'especifica, el setOperation automaticament l'assigna en funció del tipus d'operacio
     * @param prioritat
     */
    void setPriority( OperationPriority priority );

    /** Especifica quina operacio es portara a terma en aquesta operacio, si no s'ha assignat prioritat, aquest mètode l'assigna en funció de l'operació
     * @param operacio a realitzar
     */
    void setOperation( OperationAction operation );

    /** Estableix a quin PACS es dura a terme la operacio
     * @param Pacs al qual es dura a terme l'operacio
     */
    void setPacsDevice( PacsDevice parameters );

   /** Retorna la màscara de l'operació
    * @return màscara de l'operació
    */
    DicomMask getDicomMask();

    /** Retorna la prioritat de l'operació
     * @return prioritat de l'operació
     */
    OperationPriority getPriority();

    /** Retorna la operacio a realitzar
     * @return operacio a realitzar
     */
    OperationAction getOperation();

    /** Retorna el pacsParameters de l'objecte operation
     * @return PacsDevice de l'objecte operation
     */
    PacsDevice getPacsDevice();

    /// Operador per ordernar per prioritats
    bool operator < ( Operation ) const;

    /// Operador per poder dir si dues operacions són la mateixa
    bool operator ==(const Operation &operation);

    /** defineix el nom del pacient pel qual es portarà a terme l'operacio
     * @param patientName nom del pacient
     */
    void setPatientName( QString patientName );

    /** defieneix el patient ID pel qual es potarà a terma l'operació
     * @param patientID id del pacient
     */
    void setPatientID( QString patientID );

    /** defineix el uid de l'estudi amb el qual es portarà a terme l'operació
     * @param studyUID uid de l'estudi
     */
    void setStudyUID( QString studyUID );

    /** defineix el id de l'estudi amb el qual es dur a terme l'operació
     * @param studyID id de l'estudi
     */
    void setStudyID( QString studyID );

	///Destructor de la classe
    ~Operation();

    /** retorna el nom del pacient de l'estudi pel que es dur a terme l'operació
     * @return nom del pacient
     */
    QString getPatientName();

    /** retorna el id del paicent de l'estudi pel que es dur a terme l'operació
     * @return id del pacient
     */
    QString getPatientID();

    /** retorna el id de l'estudi pel que es dur a terme l'operació
     * @return id de l'estudi
     */
    QString getStudyID();

    /** retorna el UID  de l'estudi pel que es dur a terme l'operació
     * @return UID de l'estudi
     */
    QString getStudyUID();

private :

    DicomMask m_mask;
    OperationPriority m_priority;
    OperationAction m_operation;
    PacsDevice m_pacsDevice;

    // AQUESTA INFORMACIO ES NECESSARIA, JA QUE ES LA QUE PASSA A LA QRETRIEVESCREEN, pantalla que mostra l'estat de les operacions
    QString m_patientName, m_patientID, m_studyUID, m_studyID;

};

}

#endif
