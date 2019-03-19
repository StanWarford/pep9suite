#ifndef MEMOIZERHELPER_H
#define MEMOIZERHELPER_H
#include <QtCore>
#include "enu.h"
#include "symboltable.h"
struct CPUState
{
    QVector<quint32> instructionsCalled = QVector<quint32>(256, 0);
  //QVector<callStack> call_tracer;
};
QString formatNum(quint16 number);
QString formatNum(quint8 number);
QString formatAddress(quint16 address);
QString mnemonDecode(quint8 instrSpec);
QString mnemonDecode(Enu::EMnemonic instrSpec);
QString formatIS(quint8 instrSpec);
QString formatUnary(quint8 instrSpec);
QString formatNonUnary(SymbolTable* symTable, quint8 instrSpec, quint16 oprSpec);
QString formatInstr(SymbolTable* symTable, quint8 instrSpec, quint16 oprSpec);
QString generateStackFrame(CPUState &state, bool enter = true);
QString generateTrapFrame(CPUState &state, bool enter = true);
QString attemptAddrReplace(SymbolTable* symTable, quint16 number);
QString attemptOperSpecReplace(SymbolTable* symTable, quint16 number);
#endif // MEMOIZERHELPER_H
