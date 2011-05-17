#ifndef UDGSENDDICOMFILESTOPACSJOB_H
#define UDGSENDDICOMFILESTOPACSJOB_H

#include <QObject>

#include "pacsjob.h"
#include "pacsdevice.h"
#include "pacsrequeststatus.h"

namespace udg {

/**
    Job que s'encarrega d'enviar fitxers del PACS.
*/

class Study;
class Image;
class SendDICOMFilesToPACS;

class SendDICOMFilesToPACSJob : public PACSJob {
Q_OBJECT
public:

    ///Atenci�, La llista d'imatges ha de contenir l'estructura Pacient, Estudi, Series, Imatges
    SendDICOMFilesToPACSJob(PacsDevice pacsDevice, QList<Image*>);
    ~SendDICOMFilesToPACSJob();

    ///Retorna el tipus de PACSJob que �s l'objecte
    PACSJob::PACSJobType getPACSJobType();

    ///Codi que executar� el job
    void run();

    ///Retorna l'Status del Job
    PACSRequestStatus::SendRequestStatus getStatus();

    /// Retorna l'Status descrit en un QString , aquest QString est� pensat per ser mostrat en QMessageBox per informar a l'usuari de l'estat que ha retornat
    /// el job en el mateixa descripci� s'indica de quin �s l'estudi afectat
    QString getStatusDescription();

    ///Retorna la llista d'imatges que s'han indicat que s'havien de guardar
    QList<Image*> getFilesToSend();

    ///Retorna l'estudi de les imatges que s'havia de guardar
    Study* getStudyOfDICOMFilesToSend();

signals:

    ///Signal que s'emet quan s'enviat una imatge al PACS
    void DICOMFileSent(PACSJob *, int numberOfDICOMFilesSent);

    ///Signal que s'emet quan s'ha enviat un serie completa al PACS
    void DICOMSeriesSent(PACSJob*, int numberOfSeriesSent);

private:

    ///Sol�licita que ens cancel�li el job
    void requestCancelJob();

private slots:

    ///Slot que resp�n al signal de SendDICOMFilesToPACS DICOMFileSent
    void DICOMFileSent(Image *imageSent, int numberOfDICOMFilesSent);

private:

    QList<Image*> m_imagesToSend;
    PACSRequestStatus::SendRequestStatus m_sendRequestStatus;
    SendDICOMFilesToPACS *m_sendDICOMFilesToPACS;
    int m_numberOfSeriesSent;
    QString m_lastDICOMFileSeriesInstanceUID;
};

};

#endif
