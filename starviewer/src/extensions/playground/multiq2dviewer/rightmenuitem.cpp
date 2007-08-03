/***************************************************************************
 *   Copyright (C) 2005-2006 by Grup de Gràfics de Girona                  *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#include "rightmenuitem.h"
#include "QPalette"
#include "QListWidgetItem"
#include "QLabel"
#include "QPixmap"
#include "logging.h"
#include "QVBoxLayout"
#include "QMouseEvent"

namespace udg {

RightMenuItem::RightMenuItem( QWidget *parent )
: QFrame(parent)
{
    setupUi( this );
    setAutoFillBackground( true );

//     setStyleSheet(  "border-style: outset;"
//                     "border-width: 2px;"
//                     "border-radius: 10px;"
//                     "border-color: beige;"
//                     "background-color: rgb(239, 243, 247);"
//                     "background-origin: padding;"
//                     "min-width: 10em;"
//                     "padding: 6px;");
    
}

void RightMenuItem::setSerie( Series * serie )
{
    m_serie = serie;

    // Informació auxiliar de la serie

    m_auxiliar = new QWidget( );
    m_auxiliar->setWindowFlags(Qt::Popup);
    m_auxiliar->hide();

    QVBoxLayout * verticalLayout = new QVBoxLayout( m_auxiliar );
    QLabel * icon = new QLabel( m_auxiliar );
    verticalLayout->addWidget( icon );

//     QPixmap pixmap;
//     pixmap = QPixmax.fromImage( m_serie-> ?? )

    icon->setPixmap( QPixmap( "/home/ester/starviewer/src/main/images/axial.png" ) );
    QLabel * text = new QLabel( m_auxiliar );
    verticalLayout->addWidget( text );
    text->setText( "info_auxiliar ");
    text->show();
    icon->show();

}

void RightMenuItem::enterEvent( QEvent * event )
{
//     QPalette palette = this->palette();
//     QBrush selected( QColor( 85, 160, 255, 255 ) );
//     selected.setStyle( Qt::SolidPattern );
//     palette.setBrush( QPalette::Active, QPalette::Window, selected );
//     palette.setBrush( QPalette::Disabled, QPalette::Window, selected );
//     palette.setBrush( QPalette::Inactive, QPalette::Window, selected );
//     setPalette( palette );

//     setStyleSheet( "border-radius: 10px; color: rgb(85, 160, 255)" );

    setStyleSheet(  "border-style: outset;"
                    "border-width: 2px;"
                    "border-radius: 10px;"
                    "border-color: beige;"
                    "background-color: rgb(85, 160, 255);"
                    "background-origin: padding;"
                    "min-width: 10em;"
                    "padding: 6px;");
     
//     QList<QObject*> QObjectList = children();
//     for ( int i = 0; i < QObjectList.size(); i++ )
//     {
//         if ( QObjectList.value(i)->isWidgetType() )
//         {
//             ( (QWidget *) QObjectList.value(i) )->setPalette( palette );
//             ( (QWidget *) QObjectList.value(i) )->show();
//         }
//     }
// 
//     show();

    emit isActive( this->geometry().y(), m_auxiliar );

//     m_auxiliar->show();

}

void RightMenuItem::leaveEvent( QEvent * event )
{
//     QPalette palette = this->palette();
//     QBrush selected(QColor(239, 243, 247, 255));
//     selected.setStyle(Qt::SolidPattern);
//     palette.setBrush(QPalette::Active, QPalette::Window, selected);
//     setPalette(palette);

    setStyleSheet(  "border-style: outset;"
                    "border-width: 2px;"
                    "border-radius: 10px;"
                    "border-color: beige;"
                    "background-color: rgb(239, 243, 247);"
                    "background-origin: padding;"
                    "min-width: 10em;"
                    "padding: 6px;");

    m_auxiliar->hide();
}

void RightMenuItem::mousePressEvent ( QMouseEvent * event )
{
    if (event->button() == Qt::LeftButton) {
        emit selectedSerie( m_serie );
    }
}

}
