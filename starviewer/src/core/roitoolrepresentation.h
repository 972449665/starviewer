/***************************************************************************
 *   Copyright (C) 2005-2007 by Grup de Gr�fics de Girona                  *
 *   http://iiia.udg.es/GGG/index.html?langu=uk                            *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/
#ifndef UDGROITOOLREPRESENTATION_H
#define UDGROITOOLREPRESENTATION_H

#include "toolrepresentation.h"
#include "volume.h"

#include <QPointer>

class vtkImageData;

namespace udg {

class DrawerPolygon;
class DrawerText;

/**
    Representaci� base per tools de ROI
*/
class ROIToolRepresentation : public ToolRepresentation {
Q_OBJECT
public:
    ROIToolRepresentation(Drawer *drawer, QObject *parent = 0);
    ~ROIToolRepresentation();

    /// Se li passa el pol�gon ja dibuixat per un outliner
    void setPolygon(DrawerPolygon *polygon);

    /// Se li passen els par�metres necessaris pels c�lculs de la ROI
    void setParameters(int view, int slice, Volume *input, vtkImageData *slabProjection, bool m_thickSlabInUse);

    /// Fa els c�lculs pertinents i escriu el text
    void calculate();

protected:
    /// M�tode per escriure a pantalla les dades calculades.
    void printData();

    /// Col�loca el texte al seu lloc corresponent segons els l�mits i forma de la ROI
    void placeText();
    
    virtual void createHandlers() = 0;

    QList<double *> mapHandlerToPoint(ToolHandler *toolHandler);

protected:
    /// Pol�gon que cont� la forma final dibuixada
    QPointer<DrawerPolygon> m_roiPolygon;

    /// Par�metres necessaris per la representaci�
    int m_view;
    int m_slice;
    // TODO Fer que amb m_slabProjection sigui suficient i no necessitar m_thickSlabInUse
    Volume *m_input;
    vtkImageData *m_slabProjection;
    bool m_thickSlabInUse;

private:
    /// Calcula el voxel a partir de l'espaiat de la imatge i la coordenada i retorna el valor de gris
    Volume::VoxelType getGrayValue(double *coordinate);
    
    /// Calcula les dades estad�stiques de la ROI. 
    /// Ser� necessari cridar aquest m�tode abans si volem obtenir la mitjana i/o la desviaci� est�ndar
    void computeStatisticsData();

private slots:
    /// Edita els v�rtexs de les DrawerPrimitive
    void moveAllPoints(double *movement);

private:
    /// Texte amb el resultat del c�lcul
    QPointer<DrawerText> m_text;

    /// Llista amb els valors de gris per calcular la mitjana i la desviaci� et�ndard
    QList<double> m_grayValues;

    /// Controlar� si cal o no recalcular les dades estad�stiques
    /// Quan modifiquem la roi (afegint punts, creant una de nova, etc) el valor ser� true
    /// Un cop s'hagin calculat, ser� false fins que no es torni a modificar
    /// Per defecte el valor �s true
    bool m_hasToComputeStatisticsData;
    
    /// Mitjana de valors de la ROI
    double m_mean;

    /// Desviaci� est�ndar de la ROI
    double m_standardDeviation;
};

}

#endif


