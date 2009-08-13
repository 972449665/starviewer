/***************************************************************************
 *   Copyright (C) 2005-2006 by Grup de Gràfics de Girona                  *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#ifndef UDGQCREATEDICOMDIR_H
#define UDGQCREATEDICOMDIR_H

#include <QMenu>
#include <ui_qcreatedicomdirbase.h>
#include "createdicomdir.h"

class QSignalMapper;
class QProcess;

namespace udg {

class Study;
class Status;

/**
	@author Grup de Gràfics de Girona  ( GGG ) <vismed@ima.udg.es>
*/
class QCreateDicomdir : public QDialog , private Ui::QCreateDicomdirBase{
Q_OBJECT
public:
    QCreateDicomdir( QWidget *parent = 0 );
    ~QCreateDicomdir();

    /** Afegeix un estudi per convertir a dicomdir
     * @param study estudi per convertir a dicomdir
     */
    void addStudy(Study* study);

    ///Neteja el directori temporal utilitzat crear els dicomdir que es gravaran en cd o dvd
    void clearTemporaryDir();

    /** Comprova si l'estudi amb UID passat per paràmetre està dins la llista d'estudis pendents de passa a Dicomdir
     * @param studyUID UID de l'estudi que s'ha de comprovar si existeix dins la llista
     * @return indica si existeix l'estudi a la llista d'estudis pendents de passa a DicomDir
     */
    bool studyExists( QString studyUID );

public slots:
    ///Slot que esborra l'estudi seleccionat de la llista
    void removeSelectedStudy();

    ///Slot que esborra tots els estudis de la llista
    void removeAllStudies();

    ///slot que s'activa quan es fa click al botó examinar, obre filedialog, per especificar a quina carpeta es vol guardar el dicomdir
    void examineDicomdirPath();

    ///Slot que s'activa quan es fa click al botó create Dicomdir, i comença el procés de crear el dicomdir
    void createDicomdir();

protected :
    /** Event que s'activa al tancar al rebren un event de tancament
     * @param event de tancament
     */
    void closeEvent( QCloseEvent* ce );

private:
    ///Crea les connexions de signals i slots de la interfície
    void createConnections();

    ///crea el menu contextual
    void createContextMenu();

    ///Dona valor a l'etiqueta que indica l'espai que ocupa el Dicomdir
    void setDicomdirSize();

    /**comprova si hi ha suficient espai lliure al disc dur per crear el dicomdir, comprova que l'espai lliure sigui superior a l'espai que ocuparà el nou directori dicomdir
     * @return indica si hi ha prou espai lliure al disc per crear el dicomdir
     */
    bool enoughFreeSpace( QString path );

    /** Tracta els errors que s'han produït a la base de dades en general
     * @param state  Estat del mètode
     */
    void showDatabaseErrorMessage( const Status &state );

    /** Crea el dicomdir amb els estudis seleccionats, en el directori on se li passa per paràmetre
     * @param dicomdirPath directori on s'ha de crear el dicomdir
     * @return retorna l'estat del mètode
     */
    Status startCreateDicomdir( QString dicomdirPath );

    /** Crear el dicomdir en un cd o dvd
     */
    Status createDicomdirOnCdOrDvd();

    /** Crea el dicomdir al disc dur , dispositiu externs, usb o memòries flash
     */
    void createDicomdirOnHardDiskOrFlashMemories();

    ///Comprova que el directori sigui buit
    bool dicomdirPathIsEmpty(QString dicomdirPath);

    /** Comprova si aquest directori ja és un dicomdir
     * dicomdir a comprova sir és un directori
     */
    bool dicomdirPathIsADicomdir( QString dicomdirPath );

    /** genera una imatge iso i crida el k3b per gravar la imatge
     * @param device dispositiu al que es grava, pot ser cd o dvd
     */
    void burnDicomdir( CreateDicomdir::recordDeviceDicomDir device );

    /**
     * Mostra un msgbox amb el corresponent missatge d'error segons l'estat del procés
     * @param process procés que hem executat
     * @param name nom del procés
     */
    void showProcessErrorMessage( const QProcess &process, QString name );

    /** neteja la pantalla de dicomdir, després que s'hagi creat un dicomdir amb exit
     */
    void clearQCreateDicomdirScreen();

    /// Inicialitza les QActions
    void createActions();

private slots:
    /// Es passa per paràmetre l'identificador del dispositiu i es fan les pertinents accions
    void deviceChanged( int value );

private:

    static const int CDRomSizeMb = 700;
    static const int DVDRomSizeMb = 4800;
    static const int HardDiskSizeMb = 9999999;

    static const quint64 CDRomSizeBytes = ( quint64 ) CDRomSizeMb * ( quint64 ) ( 1024 * 1024 );
    static const quint64 DVDRomSizeBytes = ( quint64 ) DVDRomSizeMb * ( quint64 ) ( 1024 * 1024 );
    static const quint64 HardDiskSizeBytes = ( quint64 ) HardDiskSizeMb * ( quint64 ) ( 1024 * 1024 );

    quint64 m_dicomdirSizeBytes;
    quint64 m_DiskSpaceBytes;
    QMenu m_contextMenu;///<Menu contextual

    ///Agrupa les accions dels dispositius on gravarem el DICOMDIR
    QActionGroup *m_devicesActionGroup;

    /// Mapejador d'accions
    QSignalMapper *m_signalMapper;

    /// Accions
    QAction *m_cdromAction;
    QAction *m_dvdromAction;
    QAction *m_hardDiskAction;
    QAction *m_pendriveAction;

    QString m_lastDicomdirDirectory;//Guarda l'últim directori on s'ha creat el Dicomdir

    /// Variable que ens diu quin és el dispositiu seleccionat en aquell moment
    CreateDicomdir::recordDeviceDicomDir m_currentDevice;

    /**En windows la opció de crear dicomdir en cd/dvd ara mateix està deshabilitat perquè no tenim implementada la part 
      *de comunicar-nos algun cap sw de crear cd/dvd, el que es fa en Windows és avisar de que està desactivat el servei i que el
      *que es pot fer es crear el dicomdir al disc dur i després gravar-lo a cd, i s'activa per defecte la creació de dicomdir al disc dur
      */
    void dvdCdDicomdirDesactivatedOnWindows();

    /// Permet comprovar si la configuració es correcte per poder gravar el dicomdir en un cd o dvd
    void checkDICOMDIRBurningApplicationConfiguration();
};

}

#endif
