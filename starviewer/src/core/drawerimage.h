/***************************************************************************
 *   Copyright (C) 2005-2007 by Grup de Gr�fics de Girona                  *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#ifndef UDGDRAWERIMAGE_H
#define UDGDRAWERIMAGE_H

#include "drawerprimitive.h"

class vtkImageData;
class vtkImageActor;
class vtkTransform;
class vtkImageMapToWindowLevelColors;

namespace udg {

/**
Primitiva de polil�nia pel Drawer

	@author Grup de Gr�fics de Girona  ( GGG ) <vismed@ima.udg.es>
*/
class DrawerImage : public DrawerPrimitive
{
Q_OBJECT
public:
    DrawerImage(QObject *parent = 0);

    ~DrawerImage();

    /**
    * Afegim la imatge a la primitiva
    * @param data vtkImageData a afegir
    */
    void addImage( vtkImageData *data );

    /**
    * Afegim els punts que formen la polil�nia envolvent
    * @param points QList<double *> conjunt de punts que formen la polil�nia
    */
    void setBoundingPolyline( QList<double *> points );

    /**
    * Afegim la vista des d'on es veu la primitiva.
    */
    void setView( int view );

    ///Afegim el Window i Level
    void setWindowLevel( double window, double level );

    /// Passem la windowlevelLutMapper
    void setWindowLevelLutMapper( vtkImageMapToWindowLevelColors *windowLevelLUTMapper );

    /// Canviem la transpar�ncia [0.0, 1.0]
    void setOpacity( double opacity );

    ///Apliquem la m�scara per amagar el negre
    void applyMask();

    /// desplaca la imatge
    void move( double *movement );

    /// gira la imatge
    void rotate( vtkTransform *transform );

    ///ens retorna la imatge com a objecte VTK de representaci�
    vtkProp *getAsVtkProp();

    ///retorna les dades d'imatge
    vtkImageData *getImageData();

    ///calcula la dist�ncia que t� respecte al punt passat per par�metre
    double getDistanceToPoint( double *point3D );

    ///mira si est� dins dels l�mits que marquen els punts
    bool isInsideOfBounds( double p1[3], double p2[3], int view );

	///Obt� els limits
    void getBounds(double bounds[6]);

public slots:
    void update();

protected slots:
    void updateVtkProp();

private:
    /// M�tode que comprova si el punt est� sobre la imatge envoltada per la polil�nia
    bool isPointInPolygon( double *point3D );

    ///ens diu  si point es correnspon amb algun dels dos punts que formen un determinat segment de la polil�nia
    bool isPointIncludedInLineBounds( double point[3], double *p1, double *p2 );

    /// mirar si el punt �s dins els l�mits de la imatge
    bool isPointInsideOfBounds( double *point, double p1[3], double p2[3], int view );

private:
    /// Llista de punts de la polil�nia envolvent
    QList< double* > m_pointsList;

    /// Estructures de vtk per a la imatge
    vtkImageData *m_vtkImageData;
    vtkImageActor *m_vtkImageActor ;

    /// Vista de la primitiva
    int m_view;

    ///Dades d'imatge
    double m_range;
    double m_shift;
    double m_window;
    double m_level;
    vtkImageMapToWindowLevelColors *m_windowLevelLUTMapper;

    ///Indica si s'ha d'utilitzar isInsideOfBounds per calcular la Distance to Point
    bool m_useInsideBounds;
};

}

#endif
