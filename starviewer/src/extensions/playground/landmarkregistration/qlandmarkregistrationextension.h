/***************************************************************************
 *   Copyright (C) 2005-2006 by Grup de Gràfics de Girona                  *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#ifndef UDGQLANDMARKREGISTRATIONEXTENSION_H
#define UDGQLANDMARKREGISTRATIONEXTENSION_H

#include <QWidget>
#include "ui_qlandmarkregistrationextensionbase.h"

#include "landmarkregistrator.h"

#include <QString>

// FWD declarations
class QAction;
class vtkActor;

namespace udg {

class Volume;
class ToolManager;

/**
	@author Grup de Gràfics de Girona  ( GGG ) <vismed@ima.udg.es>
*/
class QLandmarkRegistrationExtension : public QWidget , private ::Ui::QLandmarkRegistrationExtensionBase
{
Q_OBJECT
public:
    QLandmarkRegistrationExtension(QWidget *parent = 0);
    ~QLandmarkRegistrationExtension();

public slots:
    /// Li assigna el volum principal
    void setInput( Volume *input );

    /// Canvia la fase del volum principal
    void setPhase( int phase );

    /// Li assigna el volum secundari. Aquest mètode només és de conveniència i és temporal
    void setSecondInput( Volume *input );

    /// Canvia la fase del segon volum
    void setSecondPhase( int phase );

private:
    void initializeTools();

    /// Crea les connexions entre signals i slots
    void createConnections();

    /// Guarda i llegeix les característiques
    void readSettings();
    void writeSettings();

    void setLandmarks();

private slots:
     /// gestiona els events del m_2DView
    void landmarkEventHandler( unsigned long id );
     /// gestiona els events del m_2DView_2
    void landmarkEventHandler2( unsigned long id );

    /// gestiona els events del moviment de ratolí
    void mouseMoveEventHandler( int idVolume );

    /// gestiona els events del botó esquerre
    void leftButtonEventHandler( int idVolume );

    /// gestiona els events del botó dret
    void rightButtonPressEventHandler( int idVolume );
    void rightButtonReleaseEventHandler( int idVolume );

    /// Aplica el mètode de registre
    void applyMethod();

    ///Posa una nova llavor
    void setNewSeedPosition( int idVolume );

    ///Reajustem la finestra per tal de veure un volum o l'altre
    void viewFirstVolume( );
    void viewSecondVolume( );

    ///Canvia transparència Overlay
    void setOpacity( int op );

    ///Activa les llavors
    void seed1Activated( int row, int aux);
     void seed2Activated( int row, int aux);

    ///Visualitza les llavors de la llesca
    void sliceChanged1( int s );
    void sliceChanged2( int s );

    ///Desa la transformada resultant del fitxer de registre
    void saveTransform(  );

    ///Carrega una transformada a partir del fitxer de registre
    void loadTransform(  );

    ///Desa el volum transformat resultant del registre
    void saveTransformedVolume(  );
    void loadVolume(  );

    ///Reinicialitza l'extensió
    void restore(  );

    ///Reinicialitza l'extensió, sense esborrar els punts
    void tryAgain(  );
private:
    /// El volum principal
    Volume *m_firstVolume;
    Volume *m_inputVolume;

    /// El volum on hi guardem el segon volum a registrar
    Volume *m_secondVolume;
    Volume *m_secondInputVolume;

    /// El volum on hi guardem el resultat del registre (imatge transformada)
    Volume *m_registeredVolume;

    /// Mètode de registre
    LandmarkRegistrator::Pointer m_registrationMethod;
    typedef itk::Point<double,3> PointType;
    itk::VectorContainer<int, PointType>::Pointer m_seedSet1;
    itk::VectorContainer<int, PointType>::Pointer m_seedSet2;
    //LandmarkRegistrator::LandmarkSetType  m_seedSet1;
    //LandmarkRegistrator::LandmarkSetType  m_seedSet2;

    typedef LandmarkRegistrator::TransformType LandmarkRegTransformType;

    LandmarkRegTransformType::Pointer landmarkRegTransform;

    ///Listes de les llavors
    std::list<std::vector<double> > m_seedList1;
    std::list<std::vector<double> > m_seedList2;

    ///Listes de les llavors
    std::vector<vtkActor*>  m_seedActorVector1;
    std::vector<vtkActor*>  m_seedActorVector2;
    std::vector<vtkActor*>  m_seedActorVectorReg;
    std::vector<int>  m_seedSliceVector1;
    std::vector<int>  m_seedSliceVector2;
    std::vector<int>  m_seedSliceVectorReg;
    int m_seedLastActor1;
    int m_seedLastActor2;
    int m_seedLastActorReg;

    vtkActor* m_selectedActor;
    bool m_movingSeed;
    int m_idSelectedSeed;
    std::list<std::vector<double> >::iterator m_itListSelected;

    ///Directori on guardem les dades
    QString m_savingDirectory;

    /// El Tool Manager
    ToolManager *m_toolManager;
};

}

#endif
