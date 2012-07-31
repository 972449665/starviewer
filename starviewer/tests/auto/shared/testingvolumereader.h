#ifndef TESTINGVOLUMEREADER_H
#define TESTINGVOLUMEREADER_H

#include "volumereader.h"

using namespace udg;

namespace testing {

/// De moment nom�s serveix per comprovar si es crida read() o no.
class TestingVolumeReader : public udg::VolumeReader {

public:

    TestingVolumeReader(bool &readHasBeenCalled, QObject *parent = 0);

    virtual void read(Volume *volume);

private:

    /// Es posar� a cert si es crida read() (sempre i quan no sigui nul).
    bool &m_readHasBeenCalled;

};

}

#endif // TESTINGVOLUMEREADER_H
