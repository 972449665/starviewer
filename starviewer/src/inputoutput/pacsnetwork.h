/***************************************************************************
 *   Copyright (C) 2005 by Grup de Gràfics de Girona                       *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#ifndef UDGPACSNETWORK_H
#define UDGPACSNETWORK_H

/** Gestiona els diferents networks que necessitarem per conectar als diferents PACS, el network, só objectes que configuren la nostra xarxa, indiquem per quin port esperem rebre la informació, si serà d'entrada/sortida, etc.. Pel nostre cas tindrem dos tipus de network, un per a fer querys al pacs i un altre pels retrieves d'imatges
 *
 * Aquesta classe es un siglenton, ja que totes les connexions sempre ha d'utitlitzar el mateix network, no podem tenir diferents networks per al mateix  propòsit
 */

struct T_ASC_Network;

namespace udg {

class Status;
class PacsNetwork{
public:

	///Constructor de la classe
	PacsNetwork();

    /** Preparar un network per a fer l'acció de query, si el network ja existeix, els parametres de timeout i port s'ignoren i retorna el creat anterioment
     *            @param timeout dels query
     */
    Status createNetworkQuery(int time);

    /** Prepara un network per a fer l'acció de retrieve imatges, si el network ja existeix, els parametres de timeout i port s'ignoren i retorna el creat anterioment
     * @param port a utilitzar per rebre les images
     * @param timeout de l'accio retrieve
     */
    Status createNetworkRetrieve(int port,int timeout);

    /** retorna la configuració network de per fer queries
     * @return Retorna el network per fer queries
     */
    T_ASC_Network * getNetworkQuery();

    /** retorna la configuració network de per fer els retrieves
     * @return Retorna el network per fer els retrieves
     */
    T_ASC_Network * getNetworkRetrieve();

	/// Desconnecta els network
    void disconnect();

    ///Destructor de la classe
    ~PacsNetwork();

private :

    T_ASC_Network *m_networkQuery , *m_networkRetrieve;

};

};
#endif
