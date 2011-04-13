#include "extensionworkspace.h"

#include <QToolButton>
#include <iostream>
#include "logging.h"

namespace udg {

ExtensionWorkspace::ExtensionWorkspace(QWidget *parent)
 : QTabWidget( parent )
{
    // Fem que cada pestanya tingui el seu botonet de tancar
    setTabsClosable(true);

    this->setTabPosition( QTabWidget::South );
    this->setDarkBackgroundColorEnabled(true);

    createConnections();
}

ExtensionWorkspace::~ExtensionWorkspace()
{
}

void ExtensionWorkspace::createConnections()
{
    connect( this, SIGNAL( tabCloseRequested(int) ), SLOT( closeApplicationByTabIndex(int) ) );
}

void ExtensionWorkspace::setDarkBackgroundColorEnabled(bool enabled)
{
    if (enabled)
    {
        this->setStyleSheet("QTabWidget QStackedWidget { background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #222222, stop: 1 #333333); }");
    }
    else
    {
        this->setStyleSheet("");
    }
}

void ExtensionWorkspace::addApplication( QWidget *application , QString caption, const QString &extensionIdentifier )
{
    INFO_LOG( "Afegim l'extensió: " + caption + " al workspace" );
    this->addTab( application , caption );
    this->setCurrentIndex( this->indexOf( application ) );
    // Afegim l'extensió a la llista d'extensions actives
    m_activeExtensions.insert(application,extensionIdentifier);

    this->setDarkBackgroundColorEnabled(false);
}

void ExtensionWorkspace::removeApplication( QWidget *application )
{
    if( application )
    {
        INFO_LOG( "Tancant extensió: " + application->objectName() );
        this->removeTab( this->indexOf( application ) );
        // Eliminem l'extensió de la llista d'extensions actives
        m_activeExtensions.remove(application);

        if (m_activeExtensions.count() <= 0)
        {
            this->setDarkBackgroundColorEnabled(true);
        }
        // Esborrem de memòria l'extensió
        delete application;
    }
    else
    {
        DEBUG_LOG( "S'ha donat una widget nul per eliminar" );
    }
}

void ExtensionWorkspace::killThemAll()
{
    int numberOfExtensions = this->count();
    for( int i = numberOfExtensions-1; i >=0; i-- )
    {
        QWidget *currentExtension = this->widget(i);
        removeApplication( currentExtension );
    }
}

QMap<QWidget *,QString> ExtensionWorkspace::getActiveExtensions() const
{
    return m_activeExtensions;
}

void ExtensionWorkspace::closeCurrentApplication()
{
    QWidget *w = this->currentWidget();
    removeApplication( w );
}

void ExtensionWorkspace::closeApplicationByTabIndex(int index)
{
    removeApplication( this->widget(index) );
}

};  // end namespace udg
