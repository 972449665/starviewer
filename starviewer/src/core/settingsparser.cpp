#include "settingsparser.h"

#include <QHostInfo> // localhostname, ip
#include <QRegExp>
#include <QDir>
#include <QProcess> // pel systemEnvironment
#include "logging.h"

namespace udg {

// car�cter delimitador per les paraules clau
const QChar delimiterChar('%');

SettingsParser::SettingsParser()
{
    initializeParseableStringsTable();
}

SettingsParser::~SettingsParser()
{
}

QString SettingsParser::parse(const QString &stringToParse)
{
    // Construirem una expressi� regular que trobar� les claus definides.
    // Tamb� tindr� en compte si la clau va acompanyada d'un sufix de m�scara
    // i l'aplicar� en el cas que en tingui
    QRegExp regExp;

    // Obtenim les claus existents i les juntem en un �nic string per formar l'expressi� regular %KEY1%|%KEY2%|...|%KEYN%
    QStringList keys = m_parseableStringsTable.uniqueKeys();
    QString keysPattern = "(%" + keys.join("%|%") + "%)";
    // M�scara de truncatge [n:c], on 'c' pot ser qualsevol car�cter o res, excepte un whitespace (\S==non-whitespace character)
    QString maskPattern = "(\\[\\d+:\\S?\\])?";
    // TODO l'aplicaci� de la m�scara assumeix que trunquem de "dreta a esquerra" (right Justified) i que fem el padding com a prefix de
    // la cadena truncada. Aquest comportament es podria fer m�s flexible afegint m�s par�metres a l'expressi� de truncatge per indicar
    // si el truncatge i/o el padding es fan per la dreta o l'esquerra

    // Expressi� regular: Qualsevol de les claus, que pot anar acompanyada opcionalment d'una mascara de truncatge
    regExp.setPattern(keysPattern + maskPattern);

    QString parsedString = stringToParse; // String on anirem parsejant els resultats
    int keyIndex = 0; // �ndex de l'string on comen�a el patr� trobat
    QString capturedKey; // Clau trobada
    QString capturedMask; // M�scara trobada
    QString keyToReplace; // Clau que voldrem substituir
    QString maskedString; // String que parseja la m�scara
    int truncate = 0; // nombre de car�cters a truncar --->> en comptes de truncate, posar-li width
    QChar paddingChar; // car�cter amb el que farem el padding

    // Mentres hi hagi expressions, les capturem i parsejem
    // Els "replace" es fan d'un en un, ja que podem tenir claus repetides i cal fer-ho pas a pas,
    // tal com anem tractant cada expressi� regular
    while ((keyIndex = regExp.indexIn(parsedString)) != -1)
    {
        capturedKey = regExp.cap(1); // la clau trobada, 1a part de l'expressi� regular
        capturedMask = regExp.cap(2); // la m�scara trobada, 2a part de l'expressi� regular
        keyToReplace = QString(capturedKey).replace("%", ""); // li eliminem els '%'
        // Si s'ha trobat sufix de m�scara, el parsejem
        if (!capturedMask.isEmpty())
        {
            // Obtenim les sub-parts de la m�scara ([n:c])
            QRegExp maskRegExp("\\[(\\d+):(\\S)?\\]");
            if (maskRegExp.indexIn(capturedMask) != -1)
            {
                truncate = maskRegExp.cap(1).toInt(); // nombre de car�cters a truncar
                // trunquem
                maskedString = QString(m_parseableStringsTable.value(keyToReplace)).right(truncate);
                // si hi ha car�cter de padding, tractem de fer el padding
                if (!maskRegExp.cap(2).isEmpty())
                {
                    paddingChar = maskRegExp.cap(2).at(0); // car�cter de padding
                    maskedString = maskedString.rightJustified(truncate, paddingChar);
                }
                // Substituim el valor a parsejar i la m�scara
                parsedString.replace(keyIndex, capturedKey.size() + capturedMask.size(), maskedString);
            }
            else
            {
                DEBUG_LOG("EP! Hem com�s algun error de sintaxi amb l'expressi� regular!");
            }
        }
        else // altrament, substituim �nicament la clau
        {
            parsedString.replace(keyIndex, capturedKey.size(), m_parseableStringsTable.value(keyToReplace));
        }
    }

    return parsedString;
}

void SettingsParser::initializeParseableStringsTable()
{
    // omplim els valors de les diferents paraules clau
    QString localHostName = QHostInfo::localHostName();
    m_parseableStringsTable["HOSTNAME"] = localHostName;

    // obtenci� de la ip
    QStringList ipV4Addresses = getLocalHostIPv4Addresses();
    QString ip;
    if (!ipV4Addresses.isEmpty())
    {
        // Assumim que la primera de la llista �s la IP bona
        ip = ipV4Addresses.first();

        m_parseableStringsTable["IP"] = ip;

        // "partim" els prefixos de la ip
        QStringList ipParts = ip.split(".");
        // Aix� no hauria de fallar mai ja que la llista d'IPs ha de contenir valors correctament formatats ja que aquests han estat pr�viament validats.
        if (ipParts.count() == 4)
        {
            m_parseableStringsTable["IP.1"] = ipParts.at(0);
            m_parseableStringsTable["IP.2"] = ipParts.at(1);
            m_parseableStringsTable["IP.3"] = ipParts.at(2);
            m_parseableStringsTable["IP.4"] = ipParts.at(3);
        }
    }
    else
    {
        // No tenim cap adre�a IP
        m_parseableStringsTable["IP"] = "N/A";
        m_parseableStringsTable["IP.1"] = "[N/A]";
        m_parseableStringsTable["IP.2"] = "[N/A]";
        m_parseableStringsTable["IP.3"] = "[N/A]";
        m_parseableStringsTable["IP.4"] = "[N/A]";
        WARN_LOG("No s'ha recongeut cap adre�a IPv4 en l'equip.");
    }

    // home path
    m_parseableStringsTable["HOMEPATH"] = QDir::homePath();

    // nom d'usuari
    QStringList environmentList = QProcess::systemEnvironment();
    int index = environmentList.indexOf("USERNAME"); // windows
    if (index != -1)
        m_parseableStringsTable["USERNAME"] = environmentList.at(index);
    else
    {
        index = environmentList.indexOf("USER"); // linux, mac
        if (index != -1)
            m_parseableStringsTable["USERNAME"] = environmentList.at(index);
    }
}

bool SettingsParser::isIPv4Address(const QString &ipAddress)
{
    QString zeroTo255Range("([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])");
    QRegExp ipv4AddressRegularExpression("^" + zeroTo255Range + "\\." + zeroTo255Range + "\\." + zeroTo255Range + "\\." + zeroTo255Range + "$");

    return ipv4AddressRegularExpression.exactMatch(ipAddress);
}

QStringList SettingsParser::getLocalHostIPv4Addresses()
{
    QStringList ipV4List;

    QHostInfo hostInfo = QHostInfo::fromName(QHostInfo::localHostName());
    // TODO Tamb� es podria optar per fer servir QNetworkInterface::allAddresses(), tot i que ens retorna l'adre�a 127.0.0.1 a m�s a m�s,
    // en comptes de fer servir hostInfo.addresses()
    foreach (QHostAddress ip, hostInfo.addresses())
    {
        QString ipString = ip.toString();
        if (isIPv4Address(ipString))
        {
            ipV4List << ipString;
            DEBUG_LOG(ipString + " -> �s una adre�a IPv4 v�lida");
        }
        else
        {
            DEBUG_LOG(ipString + " -> NO �s una adre�a IPv4 v�lida");
        }
    }

    return ipV4List;
}

} // end namespace udg
