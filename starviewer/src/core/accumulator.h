#ifndef UDGACCUMULATOR_H
#define UDGACCUMULATOR_H

#include <QString>

namespace udg {

template <class T> class Accumulator {
public:
    Accumulator() {}
    virtual ~Accumulator() {}

    virtual void initialize() = 0;
    virtual void accumulate(T input) = 0;
    virtual T getValue() const = 0;
};

class AccumulatorFactory {
public:
    enum AccumulatorType { Maximum = 0, Minimum = 1, Average = 2 };
    template <class T> static Accumulator<T> * getAccumulator(AccumulatorType type, unsigned long size);
    template <class T> static Accumulator<T> * getAccumulator(const QString &type, unsigned long size);
};

}

#include "accumulator.cpp"

#endif
