#ifndef PACSSERVER
#define PACSSERVER


#include "pacsdevice.h"

struct T_ASC_Network;
struct T_ASC_Parameters;
struct T_ASC_Association;
class OFCondition;
class QString;

/** Aquest classe és la que ens ajuda interectuar amb el pacs.
 * La classe conté les principals funcions i accions per connectar-nos en el pacs amb l'objectiu, de fer un echo, buscar informació o descarregar imatges. Alhora de connectar-nos al constructor passem els paràmetres mínim per connectar-nos. És molt important llegir molt bé la documentació d'aquesta classe per saber quins paràmetres utiltizar, si no voleu tenir problemes alhora de buscar informació,descarrega imatges
 */
namespace udg{

class PacsConnection;
class PacsNetwork;
class Status;

class PacsServer
{

public:

    enum modalityConnection { query, retrieveImages, echoPacs, storeImages };

    /** Constuctor de la classe. Se li ha de passar un objecte PacsDevice, amb els paràmetres del pacs correctament especificats
     * @param Parametres del Pacs a connectar
     */
    PacsServer(PacsDevice);

    /// Constructor buit de la classe
    PacsServer();

    /** Aquesta funció ens intenta connectar al PACS
     * @param Especifica en quina modalitat ens volem connectar, fer echo, busca informació o descarregar imatges
     * @return retorna l'estat de la connexió
     */
    Status connect(modalityConnection);

    /** Ens permet fer un echo al PACS. Per defecte per qualsevol modalitat de connexió bé sigui busca informació o descarregar imatges per defecte permet fer un echo, per comprovar si els PACS està viu
     * @return retorna l'estatus del echo
     */
    Status echo();//This function makes an echo to the PACS

    /** Estableix un pacs per a la connexió
     * @param Pacs amb els paràmetres per a la connexió
     */
    void setPacs(PacsDevice);

    /** retorna els paràmetres del PACS
     * @return paràmetres del Pacs
     */
    PacsDevice getPacs();

    /** Retorna una connexió per a poder buscar informació, o descarregar imatges
     * @return retorna una connexió de PACS
     */
    PacsConnection getConnection();

    /** Retorna una configuració de xarxa. Només és necessària quan l'objectiu de la connexió sigui el de descarregar imatges
     * @return retorna la configuració de la xarxa
     */
    T_ASC_Network * getNetwork();

    /// This action close the session with PACS's machine and release all the resources
    void disconnect();

private:

    PacsDevice m_pacs;

    /** Aquesta funció és privada. És utilitzada per especificar en el PACS, que una de les possibles operacions que volem fer amb ell és un echo. Per defecte en qualsevol modalitat de connexió podrem fer un echo
     * @return retorna l'estat de la configuració
     */
    OFCondition configureEcho();

    /** Aquesta funció privada, configura els paràmetres de la connexió per especificar, que el motiu de la nostre connexió és buscar informació.
     * @return retorna l'estat de la configuració
     */
    OFCondition configureFind();

    /** Aquesta funció privada permet configurar la connexió per a descarregar imatges al ordinador local. IMPORTANT!!! Abans de connectar s'ha d'invocar la funció setLocalhostPort
     * @return retorna l'estat de la configuracióDUL_PRESENTATIONCONTEXTID
     */
    OFCondition configureMove();

    /** Aquesta funció privada permet configurar la connexió per a guardar estudis en el pacs.
     * @return retorna l'estat de la configuració
     */
    OFCondition configureStore();

    /** Aquesta funció privada, configura els paràmetres pel tipus de connexió per descarregat imatges segons la màquina que tinguem
     * @param paràmetres de la connexió
     * @param Identificador de funcio
     * @param Objectiu de la funcio
     * @return retorna l'estat de la funció
     */
    OFCondition addPresentationContextMove(T_ASC_Parameters *, int, const char*);

    /** Construeix l'adreça del servidor en format ip:port, per connectar-se al PACS
     * @param adreça del servidor
     * @param port del servidor
     */
    QString constructPacsServerAddress(modalityConnection modality, PacsDevice pacsDevice ); //construct PACS address

    /** Afegeix tots els PresentationContexts necessaris per a guardar imatges en el PACS
    * @return retorna l'estat del mètode
    */
    OFCondition addStoragePresentationContexts();

    /** Afageix un objecte SOP a la connexió amb el PACS
     * @param presentationContextId número de pid
     * @param abstractSyntax classe SOP a afegir
     * @param transferSyntaxList
     * @return estat del mètode
     */
    OFCondition addPresentationContext(int presentationContextId, const QString &abstractSyntax, QList<const char*> transferSyntaxList);

    /** Afageix un objecte SOP a la connexió amb el PACS
     * @param presentationContextId número de pid
     * @param abstractSyntax classe SOP a afegir
     * @param transferSyntax
     * @return estat del mètode
     */
    OFCondition addPresentationContext(int presentationContextId, const QString &abstractSyntax, const char *transferSyntax);

private:

    T_ASC_Network *m_net; // network struct, contains DICOM upper layer FSM etc.
    T_ASC_Parameters *m_associationParameters; // parameters of association request
    T_ASC_Association *m_dicomAssociation; // request DICOM association;
    PacsNetwork *m_pacsNetwork; //configures the T_ASC_Network

};
};
#endif
