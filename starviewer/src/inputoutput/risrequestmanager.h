/***************************************************************************
 *   Copyright (C) 2005-2007 by Grup de Gràfics de Girona                  *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
/***************************************************************************
 *   Copyright (C) 2005 by Grup de Gr�fics de Girona                       *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/

#ifndef UDGRISREQUESTMANAGER_H
#define UDGRISREQUESTMANAGER_H

#include <QQueue>

#include "qpopuprisrequestsscreen.h"
#include "listenrisrequestthread.h"
#include "pacsdevice.h"

namespace udg {

class DicomMask;
class PacsManager;
class Patient;
class Study;
class DicomMask;

/** Classe manager que ens permet rebre peticions del RIS i processar-les
*/
/*TODO Aquesta classe hauria de ser l'encarregada també de descarregar l'estudi que ens ha sol·licitat el ris, ara mateix 
      fa un signal i una classe externa el descarrega. Això no es podrà fer fins que s'hagi fet refactoring de la part de descarregar
      estudis */
class RISRequestManager: public QObject{
Q_OBJECT
public:

    ///Destructor de la classe
    ~RISRequestManager();

    ///Iniciem l'escolta de les peticions del RIS pel port especificat a la configuració
    void listen();

signals:

    ///Signal que indica que s'ha descarregar un estudi sol·licitat pel RIS
    void retrieveStudyFromRISRequest(QString pacsID, Study *study);

public slots:

    ///Slot que s'activa quan ha finalitzat la descàrrega d'un estudi, comprovem si aquell estudi havia estat demanat pel RIS, i si és així
    ///comprovem si queden més estudis pendents de descàrrega per comunicar-ho al QPopUpRisRequestsScreen
    void studyRetrieveFinished(QString studyInstanceUID);

    ///Comuniquem a la QPopUpRisRequestScreen que ha fallat la descàrrega d'un estudi
    void studyRetrieveFailed(QString studyInstanceUID);

private slots:

    ///Processa una petició del RIS per descarregar l'estudi que compleixi la màscara de cerca
    void processRISRequest(DicomMask mask);

    ///Cerca en els PACS marcats per defecte la màscara que ens ha indicat el RIS
    void queryPACSRISStudyRequest(DicomMask mask);

    ///SLOT que s'activa quan es reben resultats d'una cerca a un PACS, amb la màscara indicada per la petició del RIS. Si s'ha trobat un estudi que compelix la cerca es fa signal per descarregar l'estudi
    void queryStudyResultsReceived(QList<Patient*> patients, QHash<QString, QString> hashTablePacsIDOfStudyInstanceUID);

    ///SLOT que s'activa quan ha finalitza la consulta de la cerca del l'estudi sol·licitat pel RIS
    void queryRequestRISFinished();

    ///Mostra un missatge indicant que s'ha produït un error al fer la consulta a un PACS
    void errorQueryingStudy(PacsDevice pacsDeviceError);

    ///Mostrar un missatge indicant que s'ha produït un error escoltant peticions del RIS
    void showListenRISRequestThreadError(ListenRISRequestThread::ListenRISRequestThreadError error);

private:

    /*No podem executar diverses peticions de RIS a la vegada, per això creem aquesta cua, que ens permetrà en el cas que se'ns 
     *demani una petició, quan ja n'hi hagi un altre executant, encuar la sol·licitud i esperar a llançar-la que l'actual hagi finalitzat.
     El motiu de que no podem executar més d'una sol·licitud a la vegada, és degut a la naturalesa assíncrona del PacsManager,
      quan retorna els resultats no sabem a quina sol·licitud del RIS pertany, no hi ha cap ordre establert, dificultant les coses.
     Necessitem saber si per un determinada sol·licitud hem trobat un estudi que compleixi el criteri de cerca, controls 
     d'errors, etc.. si processim més d'una sol·licitud a la vegada, no sabríem de quina sol·licitud són els resultats o error, 
     dificultant el control de les sol·licituds*/
    QQueue<DicomMask> m_queueRISRequests;
    ListenRISRequestThread *m_listenRISRequestThread;

    QPopUpRisRequestsScreen *m_qpopUpRisRequestsScreen;

    PacsManager *m_pacsManager;

    /*Pot ser que diversos PACS continguin el mateix estudi amb un mateix accession number, per evitar descarregar-lo més d'una vegada ens guardem en una
      llista quins són els estudis descarregats.*/
    QStringList m_studiesInstancesUIDRequestedToRetrieve;
    QStringList m_studiesPendingOfRetrieve;

    /**Inicialitza les variables globals per escoltar i executar peticions del RIS.
      *No inicialitzem al construtor perquè si no ens indiquen que hem d'escoltar no cal, inicialitzar les variables i ocupar memòria
      */
    void initialize();

    ///Crea les connexions entre Signals i Slots
    void createConnections();
};

};  //  end  namespace udg

#endif
