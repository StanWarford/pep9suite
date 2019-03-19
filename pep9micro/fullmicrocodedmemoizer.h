#ifndef FULLMICROCODEMEMOIZER_H
#define FULLMICROCODEMEMOIZER_H

#include <QString>
#include "enu.h"
#include "memoizerhelper.h"
class CPUControlSection;

struct callStack
{
    quint16 greatest_SP = 0, least_SP = 0;
};


class FullMicrocodedCPU;
class FullMicrocodedMemoizer
{
public:
    explicit FullMicrocodedMemoizer(FullMicrocodedCPU& item);

    void clear();
    void storeStateInstrEnd();
    void storeStateInstrStart();
    QString memoize();
    QString finalStatistics();

private:
    FullMicrocodedCPU& cpu;
    CPUState state;
};

#endif // FULLMICROCODEMEMOIZER_H
