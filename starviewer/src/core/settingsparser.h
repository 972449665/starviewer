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

    A m�s a m�s, a aquestes paraules clau se'ls pot afegir un sufix que els hi apliqui una m�scara.

    == M�scara de truncatge ==
    
    Aquesta m�scara el que fa �s truncar a 'n' car�cters la clau %KEY%. Podem especificar adicionalment
    un car�cter de padding amb el que s'omplin el nombre de car�cters buits si la longitud de %KEY% �s 
    inferior al nombre de car�cters truncats.
    
    La sintaxi �s la seg�ent
    
    %KEY%[n:c]  
    
    on:
      * %KEY%: paraula clau del cat�leg (HOSTANAME,IP, etc)
      * n: nombre de car�cters a truncar de %KEY%. Ha de ser un nombre natural > 0
      * c: car�cter de padding amb el que omplirem els espais buits. 
           El car�cter de padding pot ser qualsevol car�cter, excepte l'espai en blanc

    
    Exemples:
    
    Per obtenir els 5 �ltims d�gits de l'adre�a IP i omplir els espais buits amb el car�cter 'x', escriur�em
    %IP.3%[2:x]%IP.4%[3:x]
    Si tingu�ssim la adre�a IP 10.80.9.2 el resultat seria "x9xx2"
    
    Per obtenir els 4 �ltims d�gits, sense padding escriur�em
    %IP.3%[1:]%IP.4%[3:]
    i en aquest cas, per la mateixa adre�a IP que l'anterior, el resultat seria "92"

    
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

    /// Ens diu si la cadena de texte passada �s una adre�a IPv4 v�lida.
    /// Nom�s accepta com a resultats v�lids cadenes de texte que tinguin �nicament l'adre�a IP
    /// Si la cadena cont� una adre�a IP a m�s d'altres car�cters, aquesta ser� donada com a inv�lida.
    /// Es considera una adre�a IP v�lida una cadena amb la forma xxx.xxx.xxx.xxx on xxx �s un n�mero 
    /// dins del rang 0..255. La cadena 192.168.2.1 seria considerada com a v�lida, 
    /// per� la cadena 192.168.002.001 no ho seria pas.
    /// TODO Aquest m�tode hauria de formar part d'una classe amb utilitats de sistema o xarxa
    /// accessible des de qualsevol altre lloc de l'aplicaci� per fer aquest tipu de comprovacions.
    bool isIPv4Address(const QString &ipAddress);

    /// Ens retorna una llista amb les adreces IPv4 de la m�quina local.
    /// Normalment el primer element de la llista hauria de ser la IP corresponent 
    /// a la connexi� de xarxa principal.
    /// TODO Aquest m�tode hauria de formar part d'una classe amb utilitats de sistema o xarxa
    /// accessible des de qualsevol altre lloc de l'aplicaci� per fer obtenir aquest tipu d'informaci�.
    QStringList getLocalHostIPv4Addresses();

private:
    /// Mapa que cont� la relaci� de paraules clau amb el seu valor
    QMap<QString, QString> m_parseableStringsTable;

};

} // end namespace udg 

#endif
