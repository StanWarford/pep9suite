// File: fullmicrocodedmemoizer.h
/*
    Pep9Micro is a complete CPU simulator for the Pep/9 instruction set,
    and is capable of assembling programs to object code, executing
    object code programs, and executing microcode fragments.

    Copyright (C) 2018  J. Stanley Warford & Matthew McRaven, Pepperdine University

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
#ifndef FULLMICROCODEMEMOIZER_H
#define FULLMICROCODEMEMOIZER_H

#include <QString>

#include "cpu/memoizerhelper.h"
#include "pep/enu.h"


class FullMicrocodedCPU;
class FullMicrocodedMemoizer
{
public:
    explicit FullMicrocodedMemoizer(FullMicrocodedCPU& item);

    void clear();
    void storeStateInstrEnd();
    // Must initialize InterfaceISACPU:opValCache here for FullMicrocoded CPU
    // to fulfill its contract with InterfaceISACPU.
    void storeStateInstrStart();
    QString memoize();
    QString finalStatistics(bool includeOS);
    quint64 getCycleCount(bool includeOS);
    quint64 getInstructionCount(bool includeOS);
    const QVector<quint32> getInstructionHistogram(bool includeOS);

private:
    FullMicrocodedCPU& cpu;
    bool inOS;
    quint32 cyclesLast;
    quint32 cyclesUser, cyclesOS;
    CPUState stateUser, stateOS;
    // The InterfaceISACPU requires that the CPU capture the value of the decoded
    // operand specifier. The below set of functions handle the different possible
    // ways an operand might need to be decoded.
    // calculateOpVal() is the entry point, and will dispatch to the correct helper method.
    void calculateOpVal() const;
    void calculateOpValStoreHelper(Enu::EAddrMode addrMode, quint16 opSpec) const;
    void calculateOpValByteHelper(Enu::EAddrMode addrMode, quint16 opSpec) const;
    void calculateopValWordHelper(Enu::EAddrMode addrMode, quint16 opSpec) const;
};

#endif // FULLMICROCODEMEMOIZER_H
