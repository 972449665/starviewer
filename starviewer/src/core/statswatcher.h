/**
 Observador de widgets per obtenir estad�stiques
*/

#ifndef UDGSTATSWATCHER_H
#define UDGSTATSWATCHER_H

#include <QObject>

// fwd declarations
class QAbstractButton;

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

private slots:
    /// Registra en el log l'objecte sobre el qual s'ha fet el click
    /// Es comprova si l'objecte �s "checkable" (tipus QAbstractButton)
    /// Segons aix� es reguistra un missatge diferent per poder diferenciar
    /// objectes �nicament clickables i objectes que es poden activar o desactivar
    void registerClick(bool checked);

private:
    // Afegeix informaci� adicional sobre el contexte que estem fent l'observaci�
    QString m_context;
};

} //end namespace udg

#endif
