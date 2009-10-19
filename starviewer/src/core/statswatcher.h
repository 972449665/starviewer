/**
 Observador de widgets per obtenir estad�stiques
 TODO cal afegir un setting que ens digui si es permeten les estad�stiques o no
 i en funci� del seu valor registrar o no les estad�stques
*/

#ifndef UDGSTATSWATCHER_H
#define UDGSTATSWATCHER_H

#include <QObject>

// fwd declarations
class QAbstractButton;
class QAbstractSlider;
class QAction;
class QMenu;

namespace udg {
class StatsWatcher : public QObject
{
Q_OBJECT
public:
    StatsWatcher( QObject *parent = 0 );
    StatsWatcher( const QString &context, QObject *parent = 0 );
    ~StatsWatcher();

    /// Comptador de clicks. Per cada click del bot� ens dir� el nom de l'objecte
    void addClicksCounter( QAbstractButton *button );

    /// Compta quan una acci� s'ha disparat
    void addTriggerCounter( QMenu *menu );// compta quan es dispara, ja sigui amb un clik o un shortcut

    /// Registra les accions fetes sobre un slider
    void addSliderObserver( QAbstractSlider *slider );
    
    /// M�tode per loggejar missatges estad�stics en el format corresponent
    static void log( const QString &message );

private slots:
    /// Registra en el log l'objecte sobre el qual s'ha fet el click
    /// Es comprova si l'objecte �s "checkable" (tipus QAbstractButton)
    /// Segons aix� es reguistra un missatge diferent per poder diferenciar
    /// objectes �nicament clickables i objectes que es poden activar o desactivar
    void registerClick(bool checked);

    /// Registra l'activaci� (trigger) d'una QAction
    void registerActionTrigger( QAction *action );

    /// Registra l'acci� feta sobre un slider
    void registerSliderAction(int action = 10);

private:
    /// Afegeix informaci� adicional sobre el contexte que estem fent l'observaci�
    QString m_context;
    
    /// Indica si els logs d'estad�stiques es registraran o no
    bool m_registerLogs;
};

} //end namespace udg

#endif
