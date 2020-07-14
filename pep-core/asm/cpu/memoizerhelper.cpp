// File: memoizerhelper.cpp
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
#include "memoizerhelper.h"
#include "pep/enu.h"
#include "pep/pep.h"
#include "symbol/symbolentry.h"
#include "symbol/symbolvalue.h"

const QString stackFrameEnter("%1\n===CALL===\n");
const QString stackFrameLeave("%1\n===RET====\n");
const QString trapEnter("%1\n===TRAP===\n");
const QString trapLeave("%1\n===RETR===\n");
static quint8 max_symLen = 0;
static quint8 inst_size = 6;
// static quint8 oper_addr_size = 12;

// Properly formats a number as a 4 char hex
QString formatNum(quint16 number)
{
    return QString("%1").arg(QString::number(number, 16), 4, '0').toUpper();
}

// Properly format a number as 2 char hex
QString formatNum(quint8 number)
{
    return QString("%1").arg(QString::number(number, 16), 2, '0').toUpper();
}

// Properly format a 16 bit address
QString formatAddress(quint16 address)
{
    return "0x" + formatNum(address);
}

// Convert a mnemonic into it's string
QString mnemonDecode(quint8 instrSpec)
{
    static QMetaEnum metaenum = Enu::staticMetaObject.enumerator(Enu::staticMetaObject.indexOfEnumerator("EMnemonic"));
    return QString(metaenum.valueToKey((int)Pep::decodeMnemonic[instrSpec])).toLower();
}

QString formatIS(quint8 instrSpec)
{
    return QString(mnemonDecode(instrSpec)).leftJustified(inst_size,' ');
}

QString formatUnary(quint8 instrSpec)
{
    return formatIS(instrSpec).leftJustified(inst_size+max_symLen+2+4);
}

QString formatNonUnary(SymbolTable* symTable, quint8 instrSpec,quint16 oprSpec)
{
    return formatIS(instrSpec).leftJustified(inst_size) %
            QString(attemptOperSpecReplace(symTable, oprSpec)).rightJustified(max_symLen) %
            ", " % Pep::intToAddrMode(Pep::decodeAddrMode[instrSpec]).leftJustified(4,' ');
}

QString formatInstr(SymbolTable* symTable, quint8 instrSpec,quint16 oprSpec)
{
    if(Pep::isUnaryMap[Pep::decodeMnemonic[instrSpec]]) {
        return formatUnary(instrSpec);
    }
    else {
        return formatNonUnary(symTable, instrSpec, oprSpec);
    }
}

QString generateStackFrame(CPUState&, bool /*enter*/)
{
    return "";
    /*if(enter) {
        //return stackFrameEnter.arg(" SP=%1").arg(formatAddress(registers.regState.reg_SP));
    }
    else {
        //return stackFrameLeave.arg(" SP=%1").arg(formatAddress(registers.regState.reg_SP));
    }*/
}

QString generateTrapFrame(CPUState&, bool /*enter*/)
{
    return "";
    /*if(enter) {
        //return trapEnter.arg(" SP=%1").arg(formatAddress(registers.regState.reg_SP));
    }
    else {
        //return trapLeave.arg(" SP=%1").arg(formatAddress(registers.regState.reg_SP));
    }*/
}

QString attemptOperSpecReplace(SymbolTable *symTable, quint16 number)
{
    if(symTable == nullptr) return formatNum(number);
    int count = 0;
    QString name;
    for(auto it : symTable->getSymbolMap()) {
        if(it->getRawValue()->getSymbolType() == SymbolType::ADDRESS) continue;
        if(it->getValue() == number) {
            count++;
            name = it->getName();
        }
    }
    if(count == 1) return name;
    else return formatNum(number);
}

QString attemptAddrReplace(SymbolTable *symTable, quint16 number)
{
    if(symTable == nullptr) return formatNum(number);
    int count = 0;
    QString name;
    for(auto it : symTable->getSymbolMap()) {
        if(it->getRawValue()->getSymbolType() == SymbolType::NUMERIC_CONSTANT) continue;
        if(it->getValue() == number) {
            count++;
            name = it->getName();
        }
    }
    if(count == 1) return name;
    else return formatNum(number);
}

void MemoryAccessStatistics::clear()
{
    this->read_hit = 0;
    this->read_miss = 0;
    this->write_hit = 0;
    this->write_miss = 0;
}
