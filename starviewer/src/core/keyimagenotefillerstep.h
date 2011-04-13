#ifndef UDGKEYIMAGENOTEFILLERSTEP_H
#define UDGKEYIMAGENOTEFILLERSTEP_H

#include "patientfillerstep.h"

namespace udg {

class Patient;
class Series;

/**
Mòdul que s'encarrega d'omplir la informació d'objectes KIN. Un dels seus prerequisits serà que s'hagi superat el mòdul DICOMFileClassifierFillerStep. Les Series a processar han de de ser de modalitat KO.

	@author Grup de Gràfics de Girona  ( GGG ) <vismed@ima.udg.es>
*/
class KeyImageNoteFillerStep : public PatientFillerStep
{
public:
    KeyImageNoteFillerStep();

    ~KeyImageNoteFillerStep();

    bool fillIndividually(){return false;};

    void postProcessing(){};

    QString name() {  return "KeyImageNoteFillerStep";  }

private:
    // TODO mètode per implementar
    void processKeyImageNote();
};

}

#endif
