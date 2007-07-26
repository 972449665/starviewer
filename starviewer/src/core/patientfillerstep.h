/***************************************************************************
 *   Copyright (C) 2005-2006 by Grup de Gràfics de Girona                  *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#ifndef UDGPATIENTFILLERSTEP_H
#define UDGPATIENTFILLERSTEP_H

#include <QStringList>

namespace udg {

class PatientFillerInput;

/**
Classe pare dels mòduls que omplen parts específiques de l'estructura Patient

	@author Grup de Gràfics de Girona  ( GGG ) <vismed@ima.udg.es>
*/
class PatientFillerStep{
public:
    /// Flags que defineixen les diferents prioritats que poden tenir els Steps
    enum PriorityFlags{ LowPriority, NormalPriority, HighPriority };

    PatientFillerStep();

    ~PatientFillerStep();

    /// Li assignem les dades d'entrada que li caldrà processar
    void setInput( PatientFillerInput *input );

    /// Retorna la llista d'etiquetes que s'han de complir per poder processar aquest step.
    QStringList getRequiredLabels() const { return m_requiredLabelsList; };

    /// Retorna la prioritat que té assignada
    PriorityFlags getPriority() const { return m_priority; }

    /// Ens diu si és un bon candidat per processar l'input \TODO cal passar paràmetres adicionals?
    bool isCandidate();

    /// Donat l'input, omple la part de l'estructura Patient que li pertoca a l'step. Si no és capaç de tractar el que li toca retorna fals, true altrament
    virtual bool fill();

private:
    /// L'input a tractar
    PatientFillerInput *m_input;

    /// Llista d'etiquetes necessàries per poder processar aquest step.
    QStringList m_requiredLabelsList;

    /// prioritat de l'step. Per defecte tindrem prioritat normal en tots els steps, excepte en aquells que es defineixi alguna prioritat diferent.
    PriorityFlags m_priority;

};

}

#endif
