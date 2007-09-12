/***************************************************************************
 *   Copyright (C) 2005 by Grup de Gràfics de Girona                       *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#ifndef UDGQOPERATIONSTATESCREEN_H
#define UDGQOPERATIONSTATESCREEN_H

#include <QString>
#include <semaphore.h>

#include "dicomstudy.h"
#include "ui_qoperationstatescreenbase.h"

/// Interfície que implementa la llista d'operacions realitzades cap a un PACS
namespace udg {

class Status;
class Operation;

class QOperationStateScreen : public QDialog , private Ui::QOperationStateScreenBase{
Q_OBJECT
public:

    /** Constructor de la classe
     * @param parent
     * @return
     */
    QOperationStateScreen( QWidget *parent = 0 );

	/// destructor de la classe
    ~QOperationStateScreen();

public slots :


    /** Insereixu una nova operació
     *  @param operation operació a inserir
     */
    void insertNewOperation( Operation *operation );

    /** slot que s'invoca quant un StarviewerProcessImage emet un signal imageRetrieved
     * @param uid de l'estudi que ha finalitzat una operació d'una imatge
     * @param número d'imatges descarregades
     */
    void imageCommit( QString stidyUID , int numberOfImages );

    /** Augmenta en un el nombre de series descarregades
     * @param UID de l'estudi que s'ha descarregat una sèrie
     */
    void seriesCommit( QString studyUID );

    /** S'invoca quant s'ha acabat una operació. S'indica a la llista que l'operació relacionada amb l'estudi ha finalitzat
     * @param  UID de l'estudi descarregat
     */
    void setOperationFinished( QString studyUID );

    /** S'invoca quant es produeix algun error durant el processament de l'operació
     * @param studyUID UID de l'estudi descarregat
     */
	void setErrorOperation( QString studyUID );

    /** S'invoca quan es comença l'operació d'un estudi, per indicar-ho a la llista que aquell estudi ha començat l'operació
     * @param  UID de l'estudi que es comença l'operació
     */
	void setOperating( QString );

    /// Neteja la llista d'estudis excepte dels que s'estant descarregant en aquells moments
    void clearList();

protected :

    /** Event que s'activa al tancar al rebren un event de tancament
     * @param event de tancament
     */
    void closeEvent( QCloseEvent* ce );

private:

    /** Carreguem la mida de les columnes del QTreeWidget de l'última vegada que es va tancar la pantalla. La mida de les columnes la tenim guardada al StarviewerSettings
     */
    void setWidthColumns();

    /// Crea les connexions pels signals i slots
    void createConnections();

    /** Esborra l'estudi enviat per parametre
     * @param UID de l'estudi
     */
    void deleteStudy( QString studyUID );

    /** Guarda la mida de les columnes del QTreeWidget al StarviewerSettings
     */
    void saveColumnsWidth();
};

};

#endif

