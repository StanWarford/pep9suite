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
    quint64 getCycleCount();
    quint64 getInstructionCount();
    const QVector<quint32> getInstructionHistogram();
private:
    IsaCpu& cpu;
    CPUState state;
};

#endif // ISACPUMEMOIZER_H
