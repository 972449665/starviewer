/***************************************************************************
 *   Copyright (C) 2006-2007 by Grup de Gràfics de Girona                  *
 *   http://iiia.udg.edu/GGG/index.html                                    *
 *                                                                         *
 *   Universitat de Girona                                                 *
 ***************************************************************************/


#ifndef UDGOPTIMALVIEWPOINTINPUTPARAMETERSFORM_H
#define UDGOPTIMALVIEWPOINTINPUTPARAMETERSFORM_H


#include "qinputparameters.h"
#include "ui_optimalviewpointinputparametersformbase.h"

#include "transferfunction.h"

namespace udg {

class OptimalViewpointParameters;
class Volume;

/**
 * Interfície per definir tots els paràmetres del mètode de visualització del
 * Punt de Vista Òptim.
 */
class OptimalViewpointInputParametersForm
    : public udg::QInputParameters, private ::Ui::OptimalViewpointInputParametersFormBase {

    Q_OBJECT

public:

    OptimalViewpointInputParametersForm( QWidget * parent = 0 );
    virtual ~OptimalViewpointInputParametersForm();

    /// Assigna l'objecte que guardarà els paràmetres.
    void setParameters( OptimalViewpointParameters * parameters );

public slots:

    /**
     * Slot que ens serveix per indicar que hem d'actualitzar el paràmetre que
     * ens diguin mitjançant un identificador (que, en realitat, serà un enum).
     * Serveix per canviar els valors a partir d'una classe Parameters.
     */
    virtual void readParameter( int index );

    /**
     * Escriu tots els valors de paràmetres que té actualment al Parameters
     * associat.
     */
    virtual void writeAllParameters();

    /// Escriu els paràmetres de la segmentació al Parameters associat.
    void writeSegmentationParameters();

    /// Assigna la funció de transferència actual.
    void setTransferFunction( const TransferFunction & transferFunction );

    void setNumberOfSlices( unsigned short numberOfSlices );

    void setRangeMax( unsigned char rangeMax );

protected:

    /**
     * Mètode reimplementat per inicialitzar la funció de transferència quan es
     * mostra el widget per primer cop. Si no es fa així hi ha problemes amb el
     * GradientEditor.
     */
    virtual void showEvent( QShowEvent * event );

private slots:

    void setAdjustedTransferFunction( const TransferFunction & adjustedTransferFunction );
    void setNumberOfPlanes( const QString & numberOfPlanes );
    void openSegmentationFile();
    /// Demana el tipus de segmentació adequat segons les opcions triades.
    void requestSegmentation();
    void loadTransferFunction();
    void saveTransferFunction();
    void setClusterFirst( int slice );
    void setClusterLast( int slice );

    void toggleSegmentationParameters();

    void requestNewMethod();

    void requestRenderPlane();

signals:

    /// Demana que es faci la segmentació.
    void segmentationRequested();

    /// Demana que es carregui la segmentació des d'un fitxer.
    void loadSegmentationRequested();
    /// Demana que s'executi la segmentació automàtica.
    void automaticSegmentationRequested();
    void executionRequested();

    /// Demana que s'executi un dels nous mètodes.
    void newMethod2Requested( int step, bool normalized );

    void renderPlaneRequested( short plane );

private:

    /// Objecte que guardarà els paràmetres.
    OptimalViewpointParameters * m_parameters;

    /// Serà cert quan l'usuari hagi triat el fitxer de segmentació.
    bool m_segmentationFileChosen;

    /// Funció de transferència actual.
    TransferFunction m_transferFunction;

    /// Cert si s'ha inicialitzat la funció de transferència i fals altrament.
    bool m_inited;

};


};


#endif
