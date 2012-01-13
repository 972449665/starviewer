#ifndef UDGPERPENDICULARDISTANCETOOL_H
#define UDGPERPENDICULARDISTANCETOOL_H

#include "tool.h"

#include <QPointer>

namespace udg {

class DrawerLine;
class Q2DViewer;

/**
    Eina per mesurar dist�ncies perpendiculars en un visor 2D. Ens permet fixar dos punts a la pantalla per dibuixar una primera l�nia.
    Llavors amb un tercer clic es dibuixa una altra l�nia perpendicular a la primera que passa pel punt del tercer clic i es calcula la llargada d'aquesta.

    Per marcar cadascun dels punts es far� amb un clic o doble-clic amb el bot� esquerre del ratol�.
    Un cop marcat el primer punt, es mostrar� una l�nia que unir� el primer punt marcat amb el punt on es trobi el punter del ratol�.
    Un cop marcat el segon punt, es mostrar� una l�nia perpendicular a la primera que unir� aquesta amb el punt on es trobi el punter del ratol�.
    Mentre l'usuari no hagi marcat el tercer punt, no es mostrar� cap informaci� respecte a la dist�ncia en aquell moment.

    Un cop marcat el tercer punt, la mesura es mostrar� en mil�l�metres si existeix informaci� d'espaiat i en p�xels altrament.

    Un cop acabada la mesura, es poden annotar successivament tantes mesures com es desitgi.

    Les mesures anotades nom�s apareixeran sobre les imatges sobre les que s'han realitzat, per� no s'esborren si canviem d'imatge.
    Si canviem d'imatge per� tornem a mostrar-la m�s endavant les anotacions fetes sobre aquesta anteriorment tornaran a apar�ixer.

    Quan es canvi� l'input del visor, les anotacions fetes fins aquell moment s'esborraran.
    Quan es desactivi l'eina, les anotacions fetes fins aquell moment es mantindran.
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
    enum State { NotDrawing, DrawingFirstLine, DrawingDistanceLine };

private:

    /// Decideix qu� s'ha de fer quan es rep un clic.
    void handleClick();
    /// Afegeix el primer punt, per comen�ar a dibuixar la primera l�nia.
    void addFirstPoint();
    /// Afegeix el segon punt, per acabar de dibuixar la primera l�nia i comen�ar la segona, la de dist�ncia.
    void addSecondPoint();
    /// Afegeix el tercer punt, per acabar de dibuixar la segona l�nia i calcular la dist�ncia.
    void addThirdPoint();
    /// Actualitza la primera l�nia perqu� vagi del primer punt a la posici� actual del ratol�.
    void updateFirstLine();
    /// Actualitza la l�nia de dist�ncia perqu� sigui perpendicular a la primera l�nia i vagi des d'aquesta a la posici� actual del ratol� (o al punt m�s proper
    /// de manera que encara toqui amb la primera l�nia).
    void updateDistanceLine();
    /// Actualitza la primera l�nia i refresca la visualitzaci�.
    void updateFirstLineAndRender();
    /// Actualitza la l�nia de dist�ncia i refresca la visualitzaci�.
    void updateDistanceLineAndRender();
    /// Calcula la dist�ncia i la dibuixa.
    void drawDistance() const;
    /// Calcula i retorna la llargada de la l�nia de dist�ncia en forma de text.
    QString getDistanceText() const;
    /// Avorta el dibuix de la dist�ncia perpendicular.
    void abortDrawing();
    /// Equalitza la profunditat dels elements que formen la dist�ncia perpendicular final.
    void equalizeDepth();

private slots:

    /// Torna l'eina al seu estat inicial.
    void reset();

private:

    /// Visor 2D sobre el qual treballem.
    Q2DViewer *m_2DViewer;
    /// La primera l�nia.
    QPointer<DrawerLine> m_firstLine;
    /// La segona l�nia, perpendicular a la primera, sobre la qual mesurem la dist�ncia.
    QPointer<DrawerLine> m_distanceLine;
    /// Estat de l'eina.
    State m_state;

};

}

#endif
