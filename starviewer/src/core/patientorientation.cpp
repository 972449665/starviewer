#include "patientorientation.h"

#include <QRegExp>

#include "dicomvaluerepresentationconverter.h"

namespace udg {

const QString PatientOrientation::LeftLabel("L");
const QString PatientOrientation::RightLabel("R");
const QString PatientOrientation::PosteriorLabel("P");
const QString PatientOrientation::AnteriorLabel("A");
const QString PatientOrientation::HeadLabel("H");
const QString PatientOrientation::FeetLabel("F");

bool PatientOrientation::setDICOMFormattedPatientOrientation(const QString &patientOrientation)
{
    if (validateDICOMFormattedPatientOrientationString(patientOrientation))
    {
        m_patientOrientationString = patientOrientation;
        return true;
    }
    else
    {
        m_patientOrientationString = "";
        return false;
    }
}

QString PatientOrientation::getDICOMFormattedPatientOrientation() const
{
    return m_patientOrientationString;
}

QString PatientOrientation::getOppositeOrientationLabel(const QString &label)
{
    int i = 0;
    QString oppositeLabel;
    while (i < label.size())
    {
        if (QString(label.at(i)) == PatientOrientation::LeftLabel)
        {
            oppositeLabel += PatientOrientation::RightLabel;
        }
        else if (QString(label.at(i)) == PatientOrientation::RightLabel)
        {
            oppositeLabel += PatientOrientation::LeftLabel;
        }
        else if (QString(label.at(i)) == PatientOrientation::AnteriorLabel)
        {
            oppositeLabel += PatientOrientation::PosteriorLabel;
        }
        else if (QString(label.at(i)) == PatientOrientation::PosteriorLabel)
        {
            oppositeLabel += PatientOrientation::AnteriorLabel;
        }
        else if (QString(label.at(i)) == PatientOrientation::HeadLabel)
        {
            oppositeLabel += PatientOrientation::FeetLabel;
        }
        else if (QString(label.at(i)) == PatientOrientation::FeetLabel)
        {
            oppositeLabel += PatientOrientation::HeadLabel;
        }
        else
        {
            oppositeLabel += "?";
        }
        i++;
    }
    return oppositeLabel;
}

bool PatientOrientation::validateDICOMFormattedPatientOrientationString(const QString &string)
{
    // Construim l'expressi� regular que ens comprova que la cadena est� en el format correcte: Cadena buida, o amb 2 o 3 elements.
    
    // Etiquetes v�lides: RLAPHF
    QString validOrientationLabels = RightLabel + LeftLabel + AnteriorLabel + PosteriorLabel + HeadLabel + FeetLabel;
    
    // Expressi� per les etiquetes [RLAPHF]+
    QString validLabelsExpression = "[" + validOrientationLabels + "]+";
    
    // Expressi� pels separadors 
    // Hem de posar dos cops seguits DICOMValueRepresentationConverter::ValuesSeparator, ja que tal com diu la documentaci� de QRegExp:
    //
    // Note: The C++ compiler transforms backslashes in strings. To include a \ in a regexp, enter it twice, i.e. \\. 
    // To match the backslash character itself, enter it four times, i.e. \\\\.
    //
    // Llavors cal posar \\\\ perqu� l'expressi� sigui correcta
    QString separatorExpression = DICOMValueRepresentationConverter::ValuesSeparator + DICOMValueRepresentationConverter::ValuesSeparator;
    
    // L'expressi� final: ([RLAPHF]+\\\\[RLAPHF]+(\\\\[RLAPHF]+)?)*
    // �s a dir, 2 o 3 �tems amb etiquetes v�lides separats per \\ 
    QString regExpString = "(" + validLabelsExpression + separatorExpression + validLabelsExpression +
        "(" + separatorExpression + validLabelsExpression + ")?)*";

    QRegExp validStringExpression(regExpString);
    return validStringExpression.exactMatch(string);
}





} // End namespace udg
