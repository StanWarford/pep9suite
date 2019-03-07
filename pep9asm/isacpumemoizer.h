#ifndef ISACPUMEMOIZER_H
#define ISACPUMEMOIZER_H
#include <QtCore>
#include "enu.h"

class IsaCpu;
class IsaCpuMemoizer
{
    struct CPUState
    {
        QVector<quint32> instructionsCalled = QVector<quint32>(256, 0);
      //QVector<callStack> call_tracer;
    };
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

#endif // ISACPUMEMOIZER_H
