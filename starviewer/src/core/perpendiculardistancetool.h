#ifndef UDGPERPENDICULARDISTANCETOOL_H
#define UDGPERPENDICULARDISTANCETOOL_H

#include "tool.h"

#include <QPointer>

namespace udg {

class DrawerLine;
class DrawerText;
class Q2DViewer;

/**
    Eina per fer mesures TA-GT.

    El funcionament �s el seg�ent:
    - Primer es dibuixa una l�nia.
    - Un cop dibuixada la l�nia es tra�a una l�nia perpendicular a la primera l�nia segons la posici� del mouse.
    - Un cop acabada la segona l�nia, es tra�a una tercera l�nia paral�lela a l'anterior.
    - Un cop fixada la l�nia paral�lela, apareix la dist�ncia entre les l�nies paral�leles.

    Per marcar cadascun dels punts es far� amb un clic o doble-clic amb el bot� esquerre del ratol�.
    Un cop marcat el primer punt, es mostrar� una l�nia que unir� el primer punt marcat amb el punt on es trobi el punter del ratol�.
    Un cop marcat el segon punt, es mostrar� una l�nia perpendicular a la primera que unir� aquesta amb el punt on es trobi el punter del ratol�.
    Un cop marcat el tercer punt, es mostrar� una l�nia perpendicular a la primera i paral�lela a la segona que unir� aquesta amb el punt on es trobi el punter
    del ratol�.
    Mentre l'usuari no hagi marcat l'�ltim punt, no es mostrar� cap informaci� respecte a la dist�ncia en aquell moment.

    Un cop marcat l'�ltim punt, la mesura es mostrar� en mil�l�metres si existeix informaci� d'espaiat i en p�xels altrament.

    Un cop acabada la mesura, es poden annotar successivament tantes mesures com es desitgi.

    Les mesures anotades nom�s apareixeran sobre les imatges sobre les que s'han realitzat, per� no s'esborren si canviem d'imatge.
    Si canviem d'imatge per� tornem a mostrar-la m�s endavant les anotacions fetes sobre aquesta anteriorment tornaran a apar�ixer.

    Quan es canvi� l'input del visor, les anotacions fetes fins aquell moment s'esborraran.
    Quan es desactivi l'eina, les anotacions fetes fins aquell moment es mantindran.

    TODO Canviar el nom de la classe.
  */
class PerpendicularDistanceTool : public Tool {

Q_OBJECT

public:

    PerpendicularDistanceTool(QViewer *viewer, QObject *parent = 0);
    ~PerpendicularDistanceTool();

    /// Decideix qu� s'ha de fer per cada esdeveniment rebut.
    virtual void handleEvent(long unsigned eventId);

private:

    /// Possibles estats de l'eina.
    enum State { NotDrawing, DrawingReferenceLine, DrawingFirstPerpendicularLine, DrawingSecondPerpendicularLine };

private:
    /// Computes distance for the current drawn TA-GT
    double computeDistance() const;

    /// Places the measurement text in an proper position relative to the drawn TA-GT
    void placeMeasurementText(DrawerText *text);
    
    /// Decideix qu� s'ha de fer quan es rep un clic.
    void handleClick();
    /// Afegeix el primer punt, per comen�ar a dibuixar la l�nia de refer�ncia.
    void addFirstPoint();
    /// Afegeix el segon punt, per acabar de dibuixar la l�nia de refer�ncia i comen�ar la primera perpendicular.
    void addSecondPoint();
    /// Afegeix el tercer punt, per acabar de dibuixar la primera l�nia perpendicular i comen�ar la segona.
    void addThirdPoint();
    /// Afegeix el quart i �ltim punt, per acabar de dibuixar la segona l�nia perpendicular i calcular la dist�ncia.
    void addFourthPoint();
    /// Actualitza la l�nia de refer�ncia perqu� vagi del primer punt a la posici� actual del ratol�.
    void updateReferenceLine();
    /// Actualitza la primera l�nia perpendicular perqu� sigui perpendicular a la l�nia de refer�ncia i vagi des d'aquesta a la posici� actual del ratol� (o al
    /// punt m�s proper de manera que encara toqui amb la l�nia de refer�ncia).
    void updateFirstPerpendicularLine();
    /// Actualitza la segona l�nia perpendicular perqu� sigui perpendicular a la l�nia de refer�ncia i pel mateix costat que la primera l�nia perpendicular i
    /// vagi des d'aquesta a la posici� actual del ratol� (o al punt m�s proper de manera que encara toqui amb la l�nia de refer�ncia).
    void updateSecondPerpendicularLine();
    /// Actualitza la l�nia de refer�ncia i refresca la visualitzaci�.
    void updateReferenceLineAndRender();
    /// Actualitza la primera l�nia perpendicular i refresca la visualitzaci�.
    void updateFirstPerpendicularLineAndRender();
    /// Actualitza la segona l�nia perpendicular i refresca la visualitzaci�.
    void updateSecondPerpendicularLineAndRender();
    /// Calcula la dist�ncia i la dibuixa.
    void drawDistance();
    /// Dibuixa la l�nia de dist�ncia.
    void drawDistanceLine();
    /// Calcula i retorna la dist�ncia entre les dues l�nies perpendiculars a la de refer�ncia en forma de text.
    QString getDistanceText() const;
    /// Avorta el dibuix de l'anotaci� TA-GT.
    void abortDrawing();
    /// Equalitza la profunditat dels elements que formen l'anotaci� TA-GT final.
    void equalizeDepth();

private slots:

    /// Torna l'eina al seu estat inicial.
    void reset();

private:

    /// Visor 2D sobre el qual treballem.
    Q2DViewer *m_2DViewer;
    /// La l�nia de refer�ncia, per fer despr�s dues l�nies perpendiculars a aquesta.
    QPointer<DrawerLine> m_referenceLine;
    /// La primera l�nia perpendicular a la de refer�ncia, que �s un extrem de la dist�ncia.
    QPointer<DrawerLine> m_firstPerpendicularLine;
    /// La segona l�nia perpendicular a la de refer�ncia, que �s l'altre extrem de la dist�ncia.
    QPointer<DrawerLine> m_secondPerpendicularLine;
    /// La l�nia de dist�ncia entre les dues perpendiculars, paral�lela a la de refer�ncia.
    QPointer<DrawerLine> m_distanceLine;
    /// Estat de l'eina.
    State m_state;

};

}

#endif
