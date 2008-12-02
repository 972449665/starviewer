/***************************************************************************
 *   Copyright (C) 2005 by Grup de Gràfics de Girona                       *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#include "qconfigurationscreen.h"

#include <QString>
#include <QLineEdit>
#include <QCheckBox>
#include <QMessageBox>
#include <QPushButton>
#include <QCursor>
#include <QFileDialog>
#include <QLabel>
#include <QTreeView>
#include <QCloseEvent>
#include <QHeaderView>
#include <QList>

#include "pacsparameters.h"
#include "status.h"
#include "pacslistdb.h"
#include "starviewersettings.h"

#include "localdatabasemanager.h"
#include "logging.h"
#include "pacsserver.h"
#include "pacsnetwork.h"

namespace udg {


QConfigurationScreen::QConfigurationScreen( QWidget *parent ) : QWidget(parent)
{
    setupUi( this );
    fillPacsListView(); //emplena el listview amb les dades dels pacs, que tenim configurats

    m_selectedPacsID = "";

    loadCacheDefaults();
    loadPacsDefaults();
    loadInstitutionInformation();
    m_buttonApplyCache->setEnabled(false);
    m_buttonApplyPacs->setEnabled(false);

    m_configurationChanged = false;
    m_createDatabase = false;

    createConnections();
    setIconButtons();
    configureInputValidator();

    setWidthColumns();
}

QConfigurationScreen::~QConfigurationScreen()
{
}

void QConfigurationScreen::createConnections()
{
    //connecta el boto examinar de la cache amb el dialog per escollir el path de la base de dades
    connect( m_buttonExaminateDataBase , SIGNAL( clicked() ), SLOT( examinateDataBaseRoot() ) );

    //connecta el boto examinar de la cache amb el dialog per escollir el path de la base de dades
    connect( m_buttonExaminateCacheImage , SIGNAL( clicked() ), SLOT( examinateCacheImagePath() ) );

    //connecta el boto aplicar de la cache amb l'slot apply
    connect( m_buttonApplyCache , SIGNAL( clicked() ),  SLOT( applyChanges() ) );

    //connecta el boto aplicar del Pacs amb l'slot apply
    connect( m_buttonApplyPacs , SIGNAL( clicked() ),  SLOT( applyChanges() ) );

    //connecta el boto aplicar de l'informació de l'institució amb l'slot apply
    connect( m_buttonCreateDatabase , SIGNAL( clicked() ),  SLOT( createDatabase() ) );

    //activen el boto apply quant canvia el seu valor
    connect( m_textDatabaseRoot, SIGNAL( textChanged(const QString &) ), SLOT( enableApplyButtons() ) );
    connect( m_textCacheImagePath, SIGNAL( textChanged(const QString &) ), SLOT( enableApplyButtons() ) );
    connect( m_textMinimumSpaceRequiredToRetrieve, SIGNAL( textChanged(const QString &) ), SLOT( enableApplyButtons() ) );
    connect( m_textAETitleMachine, SIGNAL( textChanged(const QString &) ) , SLOT( enableApplyButtons() ) );
    connect( m_textTimeout, SIGNAL( textChanged(const QString &) ), SLOT( enableApplyButtons() ) );
    connect( m_textDatabaseRoot, SIGNAL( textChanged(const QString &) ), SLOT ( configurationChangedDatabaseRoot() ) );
    connect( m_textLocalPort, SIGNAL( textChanged(const QString &) ), SLOT( enableApplyButtons() ) );
    connect( m_textMaxConnections, SIGNAL( textChanged(const QString &) ), SLOT( enableApplyButtons() ) );
    connect( m_textInstitutionName, SIGNAL( textChanged(const QString &) ), SLOT( enableApplyButtons() ) );
    connect( m_textInstitutionAddress, SIGNAL( textChanged(const QString &) ), SLOT( enableApplyButtons() ) );
    connect( m_textInstitutionTown, SIGNAL( textChanged(const QString &) ), SLOT( enableApplyButtons() ) );
    connect( m_textInstitutionZipCode, SIGNAL( textChanged(const QString &) ), SLOT( enableApplyButtons() ) );
    connect( m_textInstitutionCountry, SIGNAL( textChanged(const QString &) ), SLOT( enableApplyButtons() ) );
    connect( m_textInstitutionPhoneNumber, SIGNAL( textChanged(const QString &) ), SLOT( enableApplyButtons() ) );
    connect( m_textInstitutionEmail, SIGNAL( textChanged(const QString &) ), SLOT( enableApplyButtons() ) );
    connect( m_textMaximumDaysNotViewed, SIGNAL( textChanged(const QString &) ), SLOT( enableApplyButtons() ) );

    //mateniment base de dades
    connect( m_buttonDeleteStudies , SIGNAL( clicked() ), SLOT( deleteStudies() ) );
    connect( m_buttonCompactDatabase , SIGNAL( clicked() ), SLOT( compactCache() ) );

    //afegeix la / al final del Path de la cache d'imatges
    connect( m_textCacheImagePath , SIGNAL( editingFinished() ), SLOT( cacheImagePathEditingFinish() ) );

    //manteniment PACS
    connect( m_buttonAddPacs , SIGNAL( clicked() ), SLOT( addPacs() ) );
    connect( m_buttonDeletePacs , SIGNAL( clicked() ), SLOT( deletePacs() ) );
    connect( m_buttonUpdatePacs , SIGNAL( clicked() ), SLOT( updatePacs() ) );
    connect( m_buttonTestPacs , SIGNAL( clicked() ), SLOT( test() ) );
    connect( m_PacsTreeView , SIGNAL( itemClicked ( QTreeWidgetItem * , int) ), SLOT( selectedPacs( QTreeWidgetItem * , int ) ) );
}

void QConfigurationScreen::configureInputValidator()
{
    m_textPort->setValidator( new QIntValidator(0, 65535, m_textPort) );
    m_textLocalPort->setValidator( new QIntValidator(0, 65535, m_textPort) );
    m_textTimeout->setValidator( new QIntValidator(0, 99, m_textTimeout) );
    m_textMaxConnections->setValidator( new QIntValidator(0, 99, m_textMaxConnections) );
    m_textMaximumDaysNotViewed->setValidator( new QIntValidator(0, 9999, m_textMaximumDaysNotViewed) );
    m_textMinimumSpaceRequiredToRetrieve->setValidator( new QIntValidator(0, 999, m_textMinimumSpaceRequiredToRetrieve) );
}

void QConfigurationScreen::setWidthColumns()
{
    StarviewerSettings settings;

    m_PacsTreeView->setColumnHidden(0, true); //La columna pacsID està amagada

    for ( int index = 1; index < m_PacsTreeView->columnCount(); index++ )
    {   //Al haver un QSplitter el nom del Pare del TabCache és l'splitter
            m_PacsTreeView->header()->resizeSection( index ,settings.getQConfigurationPacsDeviceColumnWidth(  index ) );
    }
}

void QConfigurationScreen::setIconButtons()
{
    m_buttonApplyPacs->setIcon( QIcon( ":images/apply.png" ) );
    m_buttonApplyCache->setIcon( QIcon( ":images/apply.png" ) );
}

void QConfigurationScreen::loadCacheDefaults()
{
    StarviewerSettings settings;

    m_textDatabaseRoot->setText(settings.getDatabasePath());
    m_textCacheImagePath->setText(settings.getCacheImagePath());
    m_textMaximumDaysNotViewed->setText(settings.getMaximumDaysNotViewedStudy());
    m_textMinimumSpaceRequiredToRetrieve->setText(QString().setNum(settings.getMinimumSpaceRequiredToRetrieveInGbytes()));
}

void QConfigurationScreen::loadPacsDefaults()
{
    QString result;
    StarviewerSettings settings;

    m_textAETitleMachine->setText( settings.getAETitleMachine() );
    m_textLocalPort->setText( settings.getLocalPort() );
    m_textTimeout->setText( settings.getTimeout() );
    m_textMaxConnections->setText( settings.getMaxConnections() );
}

void QConfigurationScreen::loadInstitutionInformation()
{
    StarviewerSettings settings;

    m_textInstitutionName->setText( settings.getInstitutionName() );
    m_textInstitutionAddress->setText( settings.getInstitutionAddress() );
    m_textInstitutionTown->setText( settings.getInstitutionTown() );
    m_textInstitutionZipCode->setText( settings.getInstitutionZipCode() );
    m_textInstitutionCountry->setText( settings.getInstitutionCountry() );
    m_textInstitutionPhoneNumber->setText( settings.getInstitutionPhoneNumber() );
    m_textInstitutionEmail->setText( settings.getInstitutionEmail() );
}

/************************************************************************************************************************/
/*                                              PACS DEVICE                                                             */
/************************************************************************************************************************/

void QConfigurationScreen:: clear()
{
    m_textAETitle->clear();
    m_textAddress->clear();
    m_textPort->clear();
    m_textInstitution->clear();
    m_textLocation->clear();
    m_textDescription->clear();
    m_checkDefault->setChecked( false );
    m_selectedPacsID = "";
}

void QConfigurationScreen::addPacs()
{
    PacsParameters pacs;
    PacsListDB pacsList;

    if (validatePacsParameters())
    {
        pacs.setAEPacs( m_textAETitle->text() );
        pacs.setPacsPort( m_textPort->text() );
        pacs.setPacsAddress( m_textAddress->text() );
        pacs.setInstitution( m_textInstitution->text() );
        pacs.setLocation( m_textLocation->text() );
        pacs.setDescription( m_textDescription->text() );
        if ( m_checkDefault->isChecked() )
        {
            pacs.setDefault( "S" );
        }
        else pacs.setDefault( "N" );

        INFO_LOG( "Afegir PACS " + m_textAETitle->text() );

        if ( !pacsList.insertPacs(pacs) )
        {
            QMessageBox::warning(this, tr("Starviewer"), tr("AETitle %1 exists").arg( pacs.getAEPacs() ));
        }
        else
        {
            fillPacsListView();
            clear();
            emit configurationChanged("Pacs/ListChanged");
        }
    }
}

void QConfigurationScreen::selectedPacs( QTreeWidgetItem * selectedItem , int )
{
    QList<PacsParameters> pacsList;
    PacsParameters selectedPacs;
    PacsListDB pacsListDB;

    if ( selectedItem != NULL )
    {
        selectedPacs = pacsListDB.queryPacs(selectedItem->text(0));// selectedItem->text(0) --> ID del pacs seleccionat al TreeWidget

        //emplenem els textots
        m_textAETitle->setText( selectedPacs.getAEPacs() );
        m_textPort->setText( selectedPacs.getPacsPort() );
        m_textAddress->setText( selectedPacs.getPacsAddress() );
        m_textInstitution->setText( selectedPacs.getInstitution() );
        m_textLocation->setText( selectedPacs.getLocation() );
        m_textDescription->setText( selectedPacs.getDescription() );
        m_selectedPacsID = selectedPacs.getPacsID();
        if ( selectedPacs.getDefault() == "S" )
        {
            m_checkDefault->setChecked( true );
        }
        else m_checkDefault->setChecked( false );
    }
    else m_selectedPacsID = "";
}

void QConfigurationScreen::updatePacs()
{
    PacsParameters pacs;
    PacsListDB pacsList;

    if ( m_selectedPacsID == "" )
    {
        QMessageBox::warning( this , tr( "Starviewer" ) , tr( "Select a PACS to update" ) );
        return;
    }

    if ( validatePacsParameters() )
    {
        pacs.setAEPacs( m_textAETitle->text() );
        pacs.setPacsPort( m_textPort->text() );
        pacs.setPacsAddress( m_textAddress->text() );
        pacs.setInstitution( m_textInstitution->text() );
        pacs.setLocation( m_textLocation->text() );
        pacs.setDescription( m_textDescription->text() );
        pacs.setPacsID( m_selectedPacsID );
        if ( m_checkDefault->isChecked() )
        {
            pacs.setDefault( "S" );
        }
        else pacs.setDefault( "N" );


        INFO_LOG( "Actualitzant dades del PACS: " + m_textAETitle->text() );

        pacsList.updatePacs(pacs);

        fillPacsListView();
        clear();
        emit configurationChanged("Pacs/ListChanged");
    }
}

void QConfigurationScreen::deletePacs()
{
    PacsListDB pacsList;

    if ( m_selectedPacsID == "" )
    {
        QMessageBox::warning( this , tr( "Starviewer" ) , tr( "Select a PACS to delete" ) );
        return;
    }

    INFO_LOG( "Esborrant el PACS: " + m_textAETitle->text() );

    pacsList.deletePacs( m_selectedPacsID );

    fillPacsListView();
    clear();
    emit configurationChanged("Pacs/ListChanged");
}

void QConfigurationScreen::fillPacsListView()
{
    QList<PacsParameters> pacsList;
    PacsListDB pacsListDB;

    m_PacsTreeView->clear();

    pacsList = pacsListDB.queryPacsList();

    foreach(PacsParameters pacs, pacsList)
    {
        QTreeWidgetItem* item = new QTreeWidgetItem( m_PacsTreeView );

        item->setText(0, pacs.getPacsID());
        item->setText(1, pacs.getAEPacs());
        item->setText(2, pacs.getPacsAddress());
        item->setText(3, pacs.getPacsPort());
        item->setText(4, pacs.getInstitution());
        item->setText(5, pacs.getLocation());
        item->setText(6, pacs.getDescription());

        if (pacs.getDefault() == "S")
        {
            item->setText(7, tr("Yes"));
        }
        else item->setText(7, tr("No"));
    }
}

void QConfigurationScreen::test()
{
    Status state;
    PacsParameters pacs;
    PacsListDB pacsList;
    PacsServer pacsServer;
    QString message;
    StarviewerSettings settings;

    //mirem que hi hagi algun element (pacs) seleccionat per a poder testejar, altrament informem de que cal seleccionar un node
    if ( m_PacsTreeView->selectedItems().count() > 0 )
    {
        //Agafem les dades del PACS que estan el textbox per testejar
        pacs.setAEPacs( m_textAETitle->text() );
        pacs.setPacsPort( m_textPort->text() );
        pacs.setPacsAddress( m_textAddress->text() );
        pacs.setAELocal( settings.getAETitleMachine() );
        pacsServer.setPacs( pacs );

        state = pacsServer.connect( PacsServer::echoPacs , PacsServer::studyLevel );

        if ( !state.good() )
        {
            message = tr( "PACS \"%1\" doesn't responds\nBe sure that the IP and AETitle of the PACS are correct" ).arg( pacs.getAEPacs() );
            QMessageBox::warning( this , tr("Starviewer") , message );
            INFO_LOG( "Doing echo PACS " + pacs.getAEPacs() + " doesn't responds. PACS ERROR : " + state.text() );
        }
        else
        {
            state = pacsServer.echo();

            if ( state.good() )
            {
                message = tr( "Test of PACS \"%1\" is correct").arg( pacs.getAEPacs() );
                QMessageBox::information( this , tr("Starviewer") , message );
                // TODO realment cal fer un INFO LOG d'això?
                INFO_LOG( "Test of PACS " + pacs.getAEPacs() + "is correct" );
            }
            else
            {
                message = tr( "PACS \"%1\" doesn't responds correctly\nBe sure that the IP and AETitle of the PACS are correct" ).arg( pacs.getAEPacs() );
                QMessageBox::warning( this , tr("Starviewer") , message );
                INFO_LOG( "Doing echo PACS " + pacs.getAEPacs() + " doesn't responds correctly. PACS ERROR : " + state.text() );
            }
        }
    }
    else
        QMessageBox::information( this , tr("Information") , tr("To test a PACS it is necessary to select an item of the list.") );

}

bool QConfigurationScreen::validatePacsParameters()
{
    QString text;

    //Per força tot els pacs han de tenir algun AETitle
    text = m_textAETitle->text();
    if ( text.length() == 0 )
    {
         QMessageBox::warning( this , tr("Starviewer") , tr("AETitle field can't be empty") );
        return false;
    }

    //adreça del pacs no pot estar en blanc
    text = m_textAddress->text();
    if ( text.length() == 0 )
    {
        QMessageBox::warning( this , tr( "Starviewer" ) , tr ( "Incorrect server address" ) );
        return false;
    }

    //el port ha d'estar entre 0 i 65535
    text = m_textPort->text();
    if ( !( text.toInt( NULL , 10 ) >= 0 && text.toInt( NULL , 10 ) <= 65535 ) || text.length() ==0 )
    {
        QMessageBox::warning( this , tr( "Starviewer" ) , tr( "PACS Port has to be between 0 and 65535" ) );
        return false;
    }

    //la institució no pot estar en blanc
    text = m_textInstitution->text();
    if ( text.length() == 0 )
    {
        QMessageBox::warning( this , tr( "Starviewer" ) , tr( "Institution field can't be empty" ) );
        return false;
    }

    return true;
}

bool QConfigurationScreen::validateChanges()
{
    QDir dir;

    if ( m_textLocalPort->isModified() )
    {
        if ( m_textLocalPort->text().toInt( NULL , 10 ) < 0 || m_textLocalPort->text().toInt( NULL , 10 ) > 65535 )
        {
            QMessageBox::warning( this , tr( "Starviewer" ) , tr( "Local Port has to be between 0 and 65535" ) );
            return false;
        }
    }

    if ( m_textMaxConnections->isModified() )
    {
        if ( m_textMaxConnections->text().toInt( NULL , 10 ) < 1 || m_textMaxConnections->text().toInt( NULL , 10 ) > 15 )
        {
            QMessageBox::warning( this , tr( "Starviewer" ) , tr( "Maximum simultaenious connections has to be between 1 and 15" ) );
            return false;
        }
    }

    if ( m_textDatabaseRoot->isModified() )
    {
        if ( !dir.exists(m_textDatabaseRoot->text() ) && m_createDatabase == false ) // si el fitxer indicat no existeix i no s'ha demanat que es crei una nova base de dades, el path és invàlid
        {
            QMessageBox::warning( this , tr( "Starviewer" ) , tr( "Invalid database path" ) );
            return false;
        }
    }

    if ( m_textCacheImagePath->isModified() )
    {
        if ( !dir.exists(m_textCacheImagePath->text() ) )
        {
            switch ( QMessageBox::question( this ,
                    tr( "Create directory ?" ) ,
                    tr( "The cache image directory doesn't exists. Do you want to create it ?" ) ,
                    tr( "&Yes" ) , tr( "&No" ) , 0 , 1 ) )
            {
                case 0:
                    if ( !dir.mkpath( m_textCacheImagePath->text() ) )
                    {
                        QMessageBox::critical( this , tr( "Starviewer" ) , tr( "Can't create the directory. Please check users permission" ) );
                        return false;
                    }
                    else return true;
                    break;
                case 1:
                    return false;
                    break;
            }
        }
    }

    if (m_textMinimumSpaceRequiredToRetrieve->isModified())
    {
        if (m_textMinimumSpaceRequiredToRetrieve->text().toUInt() < 1)
        {
            QMessageBox::warning(this, tr("Starviewer"), tr("At least 1 GByte of free space is necessary."));
            return false;
        }
    }

    return true;
}

bool QConfigurationScreen::applyChanges()
{
    if (validateChanges())
    {
        applyChangesPacs();
        applyChangesCache();
        applyChangesInstitution();

        if ( m_textDatabaseRoot->isModified() && m_createDatabase == false ) // només s'ha de reiniciar en el cas que que s'hagi canviat el path de la base de dades, per una ja existent. En el cas que la base de dades no existeixi, a l'usuari al fer click al botó crear base de dades, ja se li haurà informat que s'havia de reiniciar l'aplicació
        {
            QMessageBox::warning( this , tr( "Starviewer" ) , tr( "The application has to be restarted to apply the changes" ) );
        }

        m_configurationChanged = false;

        return true;
    }
    else return false;
}

void QConfigurationScreen::applyChangesPacs()
{
    // TODO realment cal fer INFO LOGS d'això?
    StarviewerSettings settings;

    if ( m_textAETitleMachine->isModified() )
    {
        INFO_LOG( "Modificació del AETitle de la màquina: " + m_textAETitleMachine->text() );
        settings.setAETitleMachine(m_textAETitleMachine->text());
    }

    if ( m_textTimeout->isModified() )
    {
        INFO_LOG( "Modificació del valor del timeout " + m_textTimeout->text() );
        settings.setTimeout(m_textTimeout->text());
    }

    if ( m_textLocalPort->isModified() )
    {
        INFO_LOG( "Modificació del Port d'entrada dels estudis" + m_textLocalPort->text() );
        settings.setLocalPort( m_textLocalPort->text() );
    }

    if ( m_textMaxConnections->isModified() )
    {
        INFO_LOG( "Modificació del nombre màxim de connexions " + m_textMaxConnections->text() );
        settings.setMaxConnections( m_textMaxConnections->text() );
    }

    m_buttonApplyPacs->setEnabled( false );
}

void QConfigurationScreen::enableApplyButtons()
{
    m_buttonApplyPacs->setEnabled( true );
    m_buttonApplyCache->setEnabled( true );
    m_configurationChanged = true;
}

void QConfigurationScreen::configurationChangedDatabaseRoot()
{
    m_createDatabase= false; //indiquem no s'ha demanat que es creï la base de dades indicada a m_textDatabaseRoot
    enableApplyButtons();
}


void QConfigurationScreen::examinateDataBaseRoot()
{
      //a la pàgina de QT indica que en el cas que nomes deixem seleccionar un fitxer, agafar el primer element de la llista i punt, no hi ha cap mètode que te retornin directament el fitxer selccionat

    QFileDialog *dlg = new QFileDialog( 0 , QFileDialog::tr( "Open" ) , "./" , "Starviewer Database (*.sdb)" );

    dlg->setFileMode( QFileDialog::ExistingFile );

    if ( dlg->exec() == QDialog::Accepted )
    {
        if ( !dlg->selectedFiles().empty() )
        {
            m_textDatabaseRoot->setText( dlg->selectedFiles().takeFirst() );
            m_textDatabaseRoot->setModified( true );// indiquem que m_textDatabaseRoot ha modificat el seu valor
        }
    }

    delete dlg;
}

void QConfigurationScreen::examinateCacheImagePath()
{
    QFileDialog *dlg = new QFileDialog( 0 , QFileDialog::tr( "Open" ) , "./" , tr( "Cache Directory" ) );
    QString path;

    dlg->setFileMode( QFileDialog::DirectoryOnly );

    if ( dlg->exec() == QDialog::Accepted )
    {
        if ( !dlg->selectedFiles().empty() ) m_textCacheImagePath->setText( dlg->selectedFiles().takeFirst() );
        cacheImagePathEditingFinish();//afegeix la '/' al final
    }

    delete dlg;
}

void QConfigurationScreen::applyChangesCache()
{
    StarviewerSettings settings;

    //Aquest els guardem sempre
    settings.setCacheImagePath( m_textCacheImagePath->text() );
    settings.setDatabasePath( m_textDatabaseRoot->text() );

    if ( m_textMinimumSpaceRequiredToRetrieve->isModified() )
    {
        INFO_LOG( "Es modificarà l'espai mínim necessari per descarregar" + m_textMinimumSpaceRequiredToRetrieve->text() );
        settings.setMinimumSpaceRequiredToRetrieveInGbytes(m_textMinimumSpaceRequiredToRetrieve->text().toUInt());
    }

    if ( m_textCacheImagePath->isModified() )
    {
        INFO_LOG( "Es modificarà el directori de la cache d'imatges " + m_textCacheImagePath->text() );
        settings.setCacheImagePath( m_textCacheImagePath->text() );
    }

    if ( m_textMaximumDaysNotViewed->isModified() )
    {
        INFO_LOG( "Es modificarà el nombre maxim de dies d'un estudi a la cache" + m_textMaximumDaysNotViewed->text() );
        settings.setMaximumDaysNotViewedStudy( m_textMaximumDaysNotViewed->text() );
    }

    m_buttonApplyCache->setEnabled( false );
}

void QConfigurationScreen::deleteStudies()
{
    QMessageBox::StandardButton response = QMessageBox::question(this, tr("Starviewer"),
                                                                       tr("Are you sure you want to delete all Studies of the cache ?"),
                                                                       QMessageBox::Yes | QMessageBox::No,
                                                                       QMessageBox::No);
    if(response == QMessageBox::Yes)
    {
        INFO_LOG( "Neteja de la cache" );

        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

        LocalDatabaseManager localDatabaseManager;
        localDatabaseManager.clear();

        QApplication::restoreOverrideCursor();

        if (localDatabaseManager.getLastError() != LocalDatabaseManager::Ok )
        {
            Status state;
            state.setStatus(tr("The cache cannot be delete, an unknown error has ocurred."
                               "\nTry to close all Starviewer windows and try again."
                               "\n\nIf the problem persist contact with an administrator."), false, -1);
            showDatabaseErrorMessage( state );
        }

        emit configurationChanged("Pacs/CacheCleared");
    }
}

void QConfigurationScreen::compactCache()
{
    INFO_LOG( "Compactacio de la base de dades" );

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    LocalDatabaseManager localDatabaseManager;
    localDatabaseManager.compact();

    QApplication::restoreOverrideCursor();

    if (localDatabaseManager.getLastError() != LocalDatabaseManager::Ok )
    {
        Status state;
        state.setStatus(tr("The database cannot be compacted, an unknown error has ocurred."
                "\n Try to close all Starviewer windows and try again."
                "\n\nIf the problem persist contact with an administrator."), false, -1);
        showDatabaseErrorMessage( state );
    }
}

void QConfigurationScreen::cacheImagePathEditingFinish()
{
    if ( !m_textCacheImagePath->text().endsWith( QDir::toNativeSeparators( "/" ) , Qt::CaseInsensitive ) )
    {
        m_textCacheImagePath->setText( m_textCacheImagePath->text() + QDir::toNativeSeparators( "/" ) );
    }
}

void QConfigurationScreen::applyChangesInstitution()
{
    StarviewerSettings settings;

    if ( m_textInstitutionName->isModified() ) settings.setInstitutionName( m_textInstitutionName->text() );

    if ( m_textInstitutionAddress->isModified() ) settings.setInstitutionAddress( m_textInstitutionAddress->text() );

    if ( m_textInstitutionTown->isModified() ) settings.setInstitutionTown( m_textInstitutionTown->text() );

    if ( m_textInstitutionZipCode->isModified() ) settings.setInstitutionZipCode( m_textInstitutionZipCode->text() );

    if ( m_textInstitutionCountry->isModified() ) settings.setInstitutionCountry( m_textInstitutionCountry->text() );

    if ( m_textInstitutionPhoneNumber->isModified() ) settings.setInstitutionPhoneNumber( m_textInstitutionPhoneNumber->text() );

    if ( m_textInstitutionEmail->isModified() ) settings.setInstitutionEmail( m_textInstitutionEmail->text() );
}

void QConfigurationScreen::createDatabase()
{
    StarviewerSettings settings;
    QFile databaseFile;
    QString stringDatabasePath;

    if ( m_textDatabaseRoot->text().right(4) != ".sdb" )
    {
        QMessageBox::warning( this , tr( "Starviewer" ) , tr( "The extension of the database has to be '.sdb'" ) );
    }
    else
    {
        if ( databaseFile.exists( m_textDatabaseRoot->text() ) )
        {
            QMessageBox::warning( this , tr( "Starviewer" ) , tr ( "Starviewer can't create the database because, a database with the same name exists in the directory" ) );
        }
        else
        {
            settings.setDatabasePath( m_textDatabaseRoot->text() );
            QMessageBox::warning( this , tr( "Starviewer" ) , tr( "The application has to be restarted to apply the changes"  ));
            m_createDatabase = true;
        }
    }
}

void QConfigurationScreen::saveColumnsWidth()
{
    StarviewerSettings settings;
    for ( int i = 0; i < m_PacsTreeView->columnCount(); i++ )
    {
        settings.setQConfigurationPacsDeviceColumnWidth( i , m_PacsTreeView->columnWidth( i ) );
    }
}

void QConfigurationScreen::closeEvent( QCloseEvent* ce )
{
    saveColumnsWidth();

    ce->accept();
}

void QConfigurationScreen::showDatabaseErrorMessage( const Status &state )
{
    if( !state.good() )
    {
        QMessageBox::critical( this , tr( "Starviewer" ) , state.text() + tr("\nError Number: %1").arg(state.code() ) );
    }
}

};
