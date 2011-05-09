#ifndef UDGIMAGEFILLERSTEP_H
#define UDGIMAGEFILLERSTEP_H

#include "patientfillerstep.h"

namespace udg {

class Patient;
class Series;
class Image;
class DICOMTagReader;
class DICOMSequenceItem;

/**
    Mòdul que s'encarrega d'omplir la informació general d'objectes DICOM que són imatges. Un dels seus requisits és que es tingui l'etiqueta de DICOMClassified i que la Series a tractar sigui d'imatges

*/
class ImageFillerStep : public PatientFillerStep {
public:
    ImageFillerStep();
    ~ImageFillerStep();

    bool fillIndividually();

    void postProcessing() {}

    QString name()
    {
        return "ImageFillerStep";
    }

private:
    /// Mètode per processar la informació específica de pacient,series i imatge
    bool processImage(Image *image, DICOMTagReader *dicomReader);

    /// Mètode encarregat de processar el fitxer DICOM per extreure'n el conjunt de les imatges
    /// que el conformen, omplint la informació necessària
    QList<Image*> processDICOMFile(DICOMTagReader *dicomReader);

    /// Mètode específic per processar els arxius que siguin de tipus Enhanced
    QList<Image*> processEnhancedDICOMFile(DICOMTagReader *dicomReader);

    /// Donat un dicomReader guardem a la cache el corresponent thumbnail.
    /// La intenció d'aquest mètode és estalviar temps en la càrrega de thumbnails per arxius
    /// multiframe i enhanced ja que actualment és molt costós perquè hem de carregar tot el volum
    /// a memòria i aquí podem aprofitar que el dataset està a memòria evitant la càrrega posterior
    /// Tot i així es pot fer servir en altres casos que es cregui necessari avançar la creació del thumbnail
    void saveThumbnail(DICOMTagReader *dicomReader);

    /// Omple la informació comú a totes les imatges.
    /// image i dicomReader han de ser objectes vàlids.
    bool fillCommonImageInformation(Image *image, DICOMTagReader *dicomReader);

    /// Omple l'image donat amb la informació dels functional groups continguts en l'ítem proporcionat
    /// Aquest mètode està pensat per fer-se servir amb els ítems obtinguts
    /// tant amb la Shared Functional Groups Sequence com amb la Per-Frame Functional Groups Sequence
    void fillFunctionalGroupsInformation(Image *image, DICOMSequenceItem *frameItem);

    /// Transforma l'string que obtenim del tag ImageOrientationPatient a un vector de doubles
    void imageOrientationPatientStringToDoubleVector(const QString &imageOrientationPatientString, double imageOrientationPatient[6]);

    /// Transforma el vector de doubles amb la informació d'ImageOrientationPatient a l'string equivalent a PatientOrientation
    /// El vector imageOrientationPatient tindrà 3 vectors de 3 dimensions (row, column i normal[producte vectorial dels anteriors])
    QString makePatientOrientationFromImageOrientationPatient(const double imageOrientationPatient[9]);

    /// Calcula el pixel spacing i se l'assigna a l'image donada en cas de que aquest es pugui calcular
    /// @param image Image a la que li assignarem el pixel spacing
    /// @param dicomReader Reader de DICOM que conté la font de dades de la Image associada
    void computePixelSpacing(Image *image, DICOMTagReader *dicomReader);

    /// Helper method per obtenir l'string corresponent a un direction cosines. Donat un vector de direcció determina la seva etiqueta d'orientació R,L,A,P,S,I
    QString mapDirectionCosinesToOrientationString(double vector[3]);

    /// Ens diu si les imatges són de mides diferents
    bool areOfDifferentSize(Image *firstImage, Image *secondImage);

    /// Ens diu si les imatges tenen photometric interpretations diferents
    bool areOfDifferentPhotometricInterpretation(Image *firstImage, Image *secondImage);
};

}

#endif
