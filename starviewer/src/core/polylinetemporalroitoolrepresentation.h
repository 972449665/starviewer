/***************************************************************************
 *   Copyright (C) 2005-2007 by Grup de Gr�fics de Girona                  *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#ifndef UDGPOLYLINEROITOOLREPRESENTATION_H
#define UDGPOLYLINEROITOOLREPRESENTATION_H

#include "toolrepresentation.h"
//#include "volume.h"

#include <QPointer>
//#include <vtkImageData.h>
#include <itkImage.h>

namespace udg {
class DrawerPolyline;
class DrawerText;
class PolylineTemporalROIToolData;
class ToolData;

/**
    Tool per calcular l'�rea i la mitjana de grisos d'un pol�gon
*/
class PolylineTemporalROIToolRepresentation : public ToolRepresentation {
Q_OBJECT
public:
    PolylineTemporalROIToolRepresentation(Drawer *drawer, QObject *parent = 0);
    ~PolylineTemporalROIToolRepresentation();

    /// Se li passa la linia ja dibuixada per un outliner
    void setPolyline(DrawerPolyline *polyline);

    /// Se li passen els parametres necessaris pel calcul
    void setParams(int view, int slice, double *origin, double *spacing);

    /// Fa els calculs pertinents i escriu el text
    void calculate();

    /// Declaraci� de tipus
    // typedef PolylineTemporalROIToolData::TemporalImageType TemporalImageType;
    typedef itk::Image<double, 4> TemporalImageType;

    /// Retorna les dades pr�pies de la seed
    ToolData* getToolData() const;

    /// Assigna les dades pr�pies de l'eina (persistent data)
    void setToolData(ToolData *data);

    /// Calcula el voxel a partir de l'espaiat de la imatge i la coordenada i retorna el valor de gris
    //Volume::VoxelType getGrayValue(double *coords);

private:
    /// M�tode que converteix el volum de l'input (que t� fases) en imatge itk temporal
    void convertInputImageToTemporalImage();

    /// M�tode per calcular la mitjana temporal de la regio del polyline
    double computeTemporalMean();

    /// M�tode que retorna la serie temporal de mida size en una determinada coordenada 
    QVector<double> getGraySerie(double *coords, int size);

    /// metode per escriure a pantalla les dades calculades.
    //void printData();

    ///metode per buscar els valors de gris que serviran per calcular la mitjana i la desviaci� est�ndard
    //void computeGrayValues();

    /// metode per calcular la mitjana de gris de la regio del polyline
    //double computeGrayMean();

    ///m�tode per calcular la desviaci� est�ndard de gris de la regi� del polyline
    //double computeStandardDeviation();

    /// Crea els handlers
    void createHandlers();

    /// Mapeja els handlers amb els punts corresponents
    QList<double *> mapHandlerToPoint(ToolHandler *toolHandler);

private slots:
    /// Edita els v�rtexs de les DrawerPrimitive
    void moveAllPoints(double *movement);

private:
    /// L�nia que es dibuixa
    QPointer<DrawerPolyline> m_polyline;

    ///Text amb el resultat del c�lcul
    //QPointer<DrawerText> m_text;

    ///Parametres necessaris per la representacio
    int m_view;
    int m_slice;
    //const double *m_pixelSpacing;
    double *m_origin;
    double *m_spacing;
    /*Volume *m_input;
    vtkImageData *m_slabProjection;
    int m_use;*/

    ///llista amb els valors de gris per calcular la mitjana i la desviaci� et�ndard
    //QList< double > grayValues;

    /// Dades espec�fiques de la tool
    PolylineTemporalROIToolData *m_myData;
};

}

#endif


