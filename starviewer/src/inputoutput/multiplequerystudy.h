/***************************************************************************
 *   Copyright (C) 2005 by Grup de Gràfics de Girona                       *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#ifndef UDGMULTQUERYSTUDY_H
#define UDGMULTQUERYSTUDY_H

#include <QObject>
#include <QList>

#include "dicommask.h"
#include "dicomstudy.h"
#include "dicomseries.h"
#include "dicomimage.h"

class QSemaphore;

namespace udg {

class Status;
class PacsParameters;

/** Classe que permet fer diverses cerques simultànies, a diversos pacs a la vegada mitjançant la utilitzacio de threads
	@author Grup de Gràfics de Girona  ( GGG ) <vismed@ima.udg.es>
*/
class MultipleQueryStudy : public QObject
{
Q_OBJECT
public:
    ///constructor de la classe
    MultipleQueryStudy( QObject *parent = 0 );

    ///destructor de la classe
    ~MultipleQueryStudy();

    /** Ens permet indicar quina màscara utilitzarem per fer la query als PACS
     * @param DicomMask Màscara del estudis a cercar
     */
    void setMask( DicomMask );

    /** Estableix la llista de PACS als quals es farà la cerca
     * @param PacsList amb els pacs als quals es cercarà
     */
    void setPacsList( QList<PacsParameters> pacsListToQuery );

    /** Una vegada haguem especificat la màscara, i tots els PACS als que volem realitzar la query, aquesta acció iniciara el procés de cerca a tots els PACS
     * @return Estat del mètode
     */
    Status StartQueries();

    /// retorna els estudis trobats
    QList<DICOMStudy> getStudyList();

    ///Retorna les sèries trobades 
    QList<DICOMSeries> getSeriesList();

    ///Retorna les imatges trobades
    QList<DICOMImage> getImageList();

signals:
    /** signal que s'emete si s'ha produit algun error alhora de connectar amb algun pacs
     * @param  pacsID del pacs amb que s'ha produit error
     */
    void errorConnectingPacs( QString );

    /** signal que s'envia per indicar que hi hagut un error cercant al PACS
     * @param ID del pacs que ha produit l'error
     */
    void errorQueringStudiesPacs( QString pacsID );

public slots:

    /// Slot que s'activa pel signal de QQuerStudyThread, quan un thread acaba allibera un recurs del semàfor, perquè es pugui iniciar una altre thread per continuar amb la cerca
    void threadFinished();

private:

    DicomMask m_searchMask;

    QList<PacsParameters> m_pacsListToQuery;
    int m_maxThreads;//Nombre màxim de threads que es poden executar a la vegada
    // Per raons d'optimització nomes es podran tenir un límit threads alhora executant la query, per aconseguir això utilitzem un semàfor
    QSemaphore *m_semaphoreActiveThreads;

    QList<DICOMStudy> m_studyList;
    QList<DICOMSeries> m_seriesList;
    QList<DICOMImage> m_imageList;

    ///Neteja les QList que contindran els resultats de les cerques
    void initializeResultsList();

};

}

#endif
