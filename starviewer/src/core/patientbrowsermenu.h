/***************************************************************************
 *   Copyright (C) 2005-2006 by Grup de Gràfics de Girona                  *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#ifndef UDGPATIENTBROWERMENU_H
#define UDGPATIENTBROWERMENU_H

#include <QWidget>

class QPoint;

namespace udg {

class Patient;
class PatientBrowserMenuExtendedItem;
class PatientBrowserMenuList;
class Study;
class Series;

/**
Classe que correspon al widget que es mostra al pulsar el botó dret del ratolí.

	@author Grup de Gràfics de Girona  ( GGG ) <vismed@ima.udg.es>
*/
class PatientBrowserMenu : public QWidget
{
Q_OBJECT
public:
    PatientBrowserMenu(QWidget *parent = 0);
    ~PatientBrowserMenu();

    /// Assignem un pacient per representar al menu
    void setPatient( Patient *patient );

public slots:
    /// Fem que es mostri el menú en la posició indicada
    void popup(const QPoint &point, const QString &identifier);

signals:
    /// senyal que envia la serie escollida per ser visualitzada
    void selectedSeries( Series * );

private slots:
    /// TODO mètode transicional per fer el refactoring pas a pas. Actualitza les vistes relacionades amb l'item actiu 
    /// que ara per ara serà una Series, però més endavant tindrà una forma més abstracta, com pot ser "Item" o "ID"
    void updateActiveItemView( const QString &identifier );

    void emitSelected( const QString &identifier );

private:
    /// Actualitza la posició de la informació addicional
    void updatePosition();

private:
    /// Atribut que guarda el punter al menú basic que representa al pacient
    PatientBrowserMenuList * m_patientBrowserList;

    /// Atribut que guarda el punter al menú amb informació addicional de la serie seleccionada
    PatientBrowserMenuExtendedItem * m_patientAdditionalInfo;

    /// Pacient que ens proporciona les dades d'entrada
    Patient *m_patient;
};

}

#endif
