#ifndef FULLMICROCODEMEMOIZER_H
#define FULLMICROCODEMEMOIZER_H

#include <QString>
#include "enu.h"
class CPUControlSection;

struct callStack
{
    quint16 greatest_SP = 0, least_SP = 0;
};

struct CPUState
{
    QVector<quint32> instructionsCalled = QVector<quint32>(256, 0);
  //QVector<callStack> call_tracer;
};


class FullMicrocodedCPU;
class FullMicrocodedMemoizer
{
public:
    explicit FullMicrocodedMemoizer(FullMicrocodedCPU& item);
    Enu::DebugLevels getDebugLevel() const;

    void clear();
    void storeStateInstrEnd();
    void storeStateInstrStart();
    QString memoize();
    QString finalStatistics();
    void setDebugLevel(Enu::DebugLevels level);

private:
    FullMicrocodedCPU& cpu;
    CPUState state;
    Enu::DebugLevels level;

    QMultiMap<quint16, QString> OSSymTable;
    QString formatNum(quint16 number);
    QString formatNum(quint8 number);
    QString formatAddress(quint16 address);
    QString mnemonDecode(quint8 instrSpec);
    QString mnemonDecode(Enu::EMnemonic instrSpec);
    QString formatIS(quint8 instrSpec);
    QString formatUnary(quint8 instrSpec);
    QString formatNonUnary(quint8 instrSpec, quint16 oprSpec);
    QString formatInstr(quint8 instrSpec, quint16 oprSpec);
    QString generateStackFrame(CPUState &state, bool enter = true);
    QString generateTrapFrame(CPUState &state, bool enter = true);
    QString attempSymOprReplace(quint16 number);
    QString attempSymAddrReplace(quint16 number);
    void loadSymbols();
};

#endif // FULLMICROCODEMEMOIZER_H
