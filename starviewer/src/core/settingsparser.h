#ifndef UDGSETTINGSPARSER_H
#define UDGSETTINGSPARSER_H

#include "singleton.h"

#include <QMap>

namespace udg {

/**
    Classe encarregada de parsejar strings de settings que poden
    contenir un seguit de paraules clau. Nom�s tindrem una sola inst�ncia
    ja que es tracta d'una classe Singleton.

    Les paraules clau disponibles es carreguen inicialment al crear l'objecte
    conjuntament amb els seus corresponents valors.
    
    El cat�leg de paraules clau �s el seg�ent:

    %HOSTNAME%: Nom de la m�quina local (aka localHostName)
    %IP%: Adre�a ip de la m�quina local en format aaa.bbb.ccc.dddd 
    %IP.1%: Primer prefix de l'adre�a ip de la m�quina local
    %IP.2%: Segon prefix de l'adre�a ip de la m�quina local
    %IP.3%: Tercer prefix de l'adre�a ip de la m�quina local
    %IP.4%: Quart prefix de l'adre�a ip de la m�quina local
    %USERNAME%: Nom d'usuari actual
    %HOMEPATH%: Ruta al directori "home" d'usuari

    El seu tractament ser� "case sensitive".
*/
class SettingsParser : public Singleton<SettingsParser> {
public:
    /// Donada una string la parseja i ens retorna el resultat
    QString parse( const QString &stringToParse );

protected:
    /// Cal declarar-ho friend perqu� sin� haur�em de fer p�blics 
    /// el constructor i destructor i trencar�em aix� la filosofia d'un Singleton
    friend class Singleton<SettingsParser>;
    SettingsParser();
    ~SettingsParser();

private:
    /// Inicialitza la taula amb les paraules clau i els seus corresponents valors
    void initializeParseableStringsTable();

private:
    /// Mapa que cont� la relaci� de paraules clau amb el seu valor
    QMap<QString, QString> m_parseableStringsTable;

};

} // end namespace udg 

#endif
