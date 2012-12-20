#ifndef UDGSTUDYLAYOUTMAPPER_H
#define UDGSTUDYLAYOUTMAPPER_H

#include <QList>
#include <QPair>

namespace udg {

class StudyLayoutConfig;
class ViewersLayout;
class Patient;
class Volume;
class Study;

class StudyLayoutMapper {
public:
    StudyLayoutMapper();
    ~StudyLayoutMapper();

    /// Aplica la configuraci� sobre el layout amb els estudis del pacient donats
    void applyConfig(const StudyLayoutConfig &config, ViewersLayout *layout, Patient *patient);

private:
    /// Donada una configuraci� ens retorna la llista d'estudis del pacient que donen amb el perfil de la configuraci�
    QList<Study*> getMatchingStudies(const StudyLayoutConfig &config, Patient *patient);
    
    /// Donada una llista de volums amb la corresponent llesca, els col�loca al layout segons la configuraci� donada
    void placeImagesInCurrentLayout(const QList<QPair<Volume*, int> > &volumesToPlace, const StudyLayoutConfig &config, ViewersLayout *layout);
};

} // End namespace udg

#endif
