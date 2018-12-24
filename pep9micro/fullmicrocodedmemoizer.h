#ifndef FULLMICROCODEMEMOIZER_H
#define FULLMICROCODEMEMOIZER_H

#include <QString>
#include "enu.h"
class CPUControlSection;
struct CPURegisterState
{
    quint16 reg_PC_start = 0, reg_PC_end = 0;
    quint16 reg_A = 0, reg_X = 0, reg_SP = 0, reg_OS = 0;
    quint8 reg_IR = 0, bits_NZVCS = 0;
};

struct callStack
{
    quint16 greatest_SP = 0, least_SP = 0;
};

struct CPUState
{
    CPURegisterState regState = CPURegisterState();
    QVector<quint32> instructionsCalled = QVector<quint32>(256,0);
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
    quint8 getRegisterByteStart(Enu::CPURegisters reg) const;
    quint16 getRegisterWordStart(Enu::CPURegisters reg) const;
    bool getStatusBitStart(Enu::EStatusBit bit) const;

private:
    FullMicrocodedCPU& cpu;
    CPUState registers;
    Enu::DebugLevels level;
    QMultiMap<quint16,QString> OSSymTable;
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
