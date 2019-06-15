// File: memoizerhelper.h
/*
    Pep9 is a virtual machine for writing machine language and assembly
    language programs.

    Copyright (C) 2019  J. Stanley Warford & Matthew McRaven, Pepperdine University

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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
