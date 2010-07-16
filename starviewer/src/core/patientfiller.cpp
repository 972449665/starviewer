/***************************************************************************
 *   Copyright (C) 2005-2006 by Grup de Gràfics de Girona                  *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#include "patientfiller.h"

#include <QApplication>
#include <QTime>
#include <QtAlgorithms>

#include "patientfillerinput.h"
#include "logging.h"
#include "dicomtagreader.h"

// TODO Include's temporals mentre no tenim un registre:
#include "imagefillerstep.h"
#include "dicomfileclassifierfillerstep.h"
#include "temporaldimensionfillerstep.h"
#include "mhdfileclassifierstep.h"
#include "orderimagesfillerstep.h"
// TODO encara no hi ha suport a KINs i Presentation States, per tant
// fins que no tinguem suport i implementem correctament els respectius 
// filler steps no caldrà afegir-los dins del pipeline
//#include "keyimagenotefillerstep.h"
//#include "presentationstatefillerstep.h"

namespace udg {

PatientFiller::PatientFiller(QObject * parent) : QObject(parent)
{
    registerSteps();
    m_patientFillerInput = new PatientFillerInput();
    m_imageCounter = 0;
}

PatientFiller::~PatientFiller()
{
    foreach (PatientFillerStep* fillerStep, m_registeredSteps)
    {
        delete fillerStep;
    }

    delete m_patientFillerInput;
}

// Mètode intern per poder realitzar l'ordenació dels patientfiller
bool patientFillerMorePriorityFirst(const PatientFillerStep *s1, const PatientFillerStep *s2)
{
    return (*s1) < (*s2);
}

void PatientFiller::registerSteps()
{
    m_registeredSteps.append(new ImageFillerStep() );
    m_registeredSteps.append(new DICOMFileClassifierFillerStep() );
    m_registeredSteps.append(new MHDFileClassifierStep() );
    m_registeredSteps.append(new OrderImagesFillerStep() );
    // \TODO Donat que al postProcessing no tenim política d'etiquetes, s'ha posat el Temporal al final
    // perquè necessita que s'hagi executat l'Order abans. S'hauria de millorar.
    m_registeredSteps.append(new TemporalDimensionFillerStep() );
    
	// TODO encara no hi ha suport a KINs i Presentation States, per tant
	// fins que no tinguem suport i implementem correctament els respectius 
	// filler steps no caldrà afegir-los dins del pipeline
	//m_registeredSteps.append(new KeyImageNoteFillerStep() );
	//m_registeredSteps.append(new PresentationStateFillerStep() );
}

void PatientFiller::processDICOMFile(DICOMTagReader *dicomTagReader)
{
    Q_ASSERT(dicomTagReader);

    m_patientFillerInput->setDICOMFile(dicomTagReader);

    QList<PatientFillerStep*> processedFillerSteps;
    QList<PatientFillerStep*> candidatesFillerSteps = m_registeredSteps;
    bool continueIterating = true;

    while (!candidatesFillerSteps.isEmpty() && continueIterating)
    {
        QList<PatientFillerStep*> fillerStepsToProcess;
        QList<PatientFillerStep*> newCandidatesFillerSteps;
        continueIterating = false;

        for (int i = 0; i < candidatesFillerSteps.size(); ++i)
        {
            if (m_patientFillerInput->hasAllLabels( candidatesFillerSteps.at(i)->getRequiredLabels() ))
            {
                fillerStepsToProcess.append( candidatesFillerSteps.at(i) );
                continueIterating = true;
            }
            else
            {
                newCandidatesFillerSteps.append( candidatesFillerSteps.at(i) );
            }
        }
        candidatesFillerSteps = newCandidatesFillerSteps;

        qSort(fillerStepsToProcess.begin(), fillerStepsToProcess.end(), patientFillerMorePriorityFirst); // Ordenem segons la seva prioritat

        foreach (PatientFillerStep *fillerStep, fillerStepsToProcess)
        {
            fillerStep->setInput(m_patientFillerInput);
            fillerStep->fillIndividually();
        }
    }

    m_patientFillerInput->initializeAllLabels();

    m_imageCounter++;
    emit progress(m_imageCounter);
}

void PatientFiller::finishDICOMFilesProcess()
{
    foreach (PatientFillerStep *fillerStep, m_registeredSteps)
    {
        fillerStep->postProcessing();
    }

    emit patientProcessed(m_patientFillerInput->getPatient());

    // Al acabar hem de reiniciar el comptador d'imatges
    m_imageCounter = 0;
}

QList<Patient*> PatientFiller::processDICOMFileList(QStringList dicomFiles)
{
    // HACK per fer el cas especial dels mhd. Això està així perquè perquè el mètode
    // processDICOMFile s'espera un DICOMTagReader, que no podem crear a partir d'un mhd.
    // El filler d'mhd realment no s'està utilitzant a dintre del process de fillers com la resta.
    if(dicomFiles.first().contains(".mhd"))
    {
        PatientFillerInput patientFillerInput;
        foreach(QString file, dicomFiles)
        {
            patientFillerInput.setFile(file);

            MHDFileClassifierStep mhdFileClassiferStep;
            mhdFileClassiferStep.setInput(&patientFillerInput);
            if(!mhdFileClassiferStep.fillIndividually())
            {
                ERROR_LOG("No s'ha pogut processar un fitxer MHD: " + file);
            }
        }

        return patientFillerInput.getPatientsList();
    }

    m_imageCounter = 0;

    foreach(QString dicomFile, dicomFiles)
    {
        DICOMTagReader *dicomTagReader = new DICOMTagReader(dicomFile);
        if( dicomTagReader->canReadFile() )
            this->processDICOMFile( dicomTagReader );

        emit progress(++m_imageCounter);
    }

    foreach(PatientFillerStep *fillerStep, m_registeredSteps)
    {
        fillerStep->postProcessing();
    }

    return m_patientFillerInput->getPatientsList();
}
}
