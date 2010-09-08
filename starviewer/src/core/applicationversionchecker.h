/***************************************************************************
 *   Copyright (C) 2005 by Grup de Gr�fics de Girona                       *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/

#ifndef UDGAPPLICATIONVERSIONCHECKER_H
#define UDGAPPLICATIONVERSIONCHECKER_H

#include "qreleasenotes.h"
#include <QUrl>
#include <QObject>
class QNetworkReply;
class QNetworkProxy;
class QNetworkAccessManager;


namespace udg {

    class QReleaseNotes;

/** Aquesta classe comprovar� si s'han de mostrar les notes d'una versi� instalada. En cas que no s'hagin de mostrar
    buscar� si hi ha una nova versi� disponible. Les notes es mostraran a trav�s de la classe QReleaseNotes.
    Treballa sobre els settings:
        - ShowReleaseNotesFirstTime: boole� que indica si es la primera vegada que s'obre des de l'actualitzaci�
        - NeverShowReleaseNotes: L'usuari decideix no mostrar m�s els missatges de les release notes
        - LastVersionChecked: Quina �s la �ltima versi� que s'ha comprobat.
        - LastVersionCheckedDate: En quina data s'ha fet la comprobaci�. 
        - CheckVersionInterval: Cada quants dies es comproba si hi ha una nova versi�.
  */
    class ApplicationVersionChecker : QObject {
Q_OBJECT


public:
    /// Constructor per defecte
    ApplicationVersionChecker();
    /// Destructor
    ~ApplicationVersionChecker();
    /// Comprobar� que els url existeixin i es puguin obrir i llavors mostrara la finestra
    void showIfCorrect();
    /// Permet especificar un nou interval de temps entre la comprovaci� de noves versions modificant el setting CheckVersionInterval
    /// Aquest �s el m�tode que s'ha de cridar si es vol canviar aquest interval des de per exemple un menu d'opcions
    void setCheckVersionInterval(int interval);
    
//public slots:

//protected:

private:
    /// Genera la url local del fitxer on hi ha les release notes
    QUrl createLocalUrl();

    /// Comproba que la url local de les release notes existeixi
    bool checkLocalUrl(QUrl url);

    /// Comprobar si ha passat prou temps per mirar si hi ha una nova versi�
    bool checkTimeInterval();

    /// Fer la crida al servidor per obtenir si hi ha una nova versi�
    void checkVersionOnServer();

    /// Genera la url per fer la crida al servidor i obtenir la versi�
    QString createWebServiceUrl();

    /// Codifica en base64 una QByteArray i a m�s el transforma per que sigui una part d'una url v�lida
    QString encryptBase64Url(QString url);

    /// Assigna el proxy per defecte, si n'hi ha, a un QNetworkAccessManager, si no n'hi ha el busca.
    void setProxy(QUrl url);

    /// Guardar els settings
    void writeSettings();

    /// Llegir els settings
    void readSettings();

private slots:
    /// Tracta la resposta del webservice obtenint la versi� i la url de les notes d'aquesta nova versi�
    void webServiceReply(QNetworkReply *reply);
    /// Tracta la resposta de les notes de la nova versi� disponible i posa el seu contingut al WebView
    void updateNotesUrlReply(QNetworkReply *reply);
    /// Event que es crida quan es tanca la finestra de les QReleaseNotes
    void closeEvent();

private:
    /// Atribut que indica si s'esta comprobant la nova versi� o mostrant les release notes
    bool m_checkNewVersion;
    /// Atribut per guardar temporalment la versi� que s'esta comprobant
    QString m_checkedVersion;

    /// Atribut per gestionar les connexions http
    QNetworkAccessManager *m_manager;

    /// Referencia a les QReleaseNotes que controlaran la finestra on es mostren les notes
    QReleaseNotes *m_releaseNotes;

    /// Settings
    bool m_showReleaseNotesFirstTime;
    bool m_neverShowReleaseNotes;
    QString m_lastVersionCheckedDate;
    QString m_lastVersionChecked;
    int m_checkVersionInterval;
};

} // end namespace udg

#endif