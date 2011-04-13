#ifndef UDGEDITORTOOLDATA_H
#define UDGEDITORTOOLDATA_H

#include "tooldata.h"

namespace udg {

/**
    @author Laboratori de Gràfics i Imatge  ( GILab ) <vismed@ima.udg.es>
*/
class EditorToolData : public ToolData {
Q_OBJECT
public:
    EditorToolData(QObject *parent = 0);
    ~EditorToolData();

    void setVolumeVoxels( int vol );

    int getVolumeVoxels( );

private:
    /// Hi guardem la posició de la llavor
    int m_volumeCont;

};

}

#endif
