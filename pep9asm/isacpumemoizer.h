#ifndef ISACPUMEMOIZER_H
#define ISACPUMEMOIZER_H
#include <QtCore>
#include "enu.h"
#include "memoizerhelper.h"
class IsaCpu;
class IsaCpuMemoizer
{

public:
    explicit IsaCpuMemoizer(IsaCpu& cpu);
    ~IsaCpuMemoizer();
    void clear();
    void storeStateInstrEnd();
    void storeStateInstrStart();
    QString memoize();
    QString finalStatistics();

private:
    IsaCpu& cpu;
    CPUState state;
};

#endif // ISACPUMEMOIZER_H
