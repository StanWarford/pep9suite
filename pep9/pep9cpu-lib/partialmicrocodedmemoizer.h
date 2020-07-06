// File: partialmicrocodedmemoizer.h
/*
    Pep9CPU is a CPU simulator for executing microcode sequences to
    implement instructions in the instruction set of the Pep/9 computer.

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

#include "pep/enu.h"

class CPUControlSection;
struct callStack
{
    quint16 greatest_SP = 0, least_SP = 0;
};

class PartialMicrocodedCPU;
class PartialMicrocodedMemoizer
{
public:
    explicit PartialMicrocodedMemoizer(PartialMicrocodedCPU& item);

    void clear();
    void storeStateInstrEnd();
    void storeStateInstrStart();
    QString memoize();
    QString finalStatistics();

private:
    PartialMicrocodedCPU& cpu;
    QString formatNum(quint16 number);
    QString formatNum(quint8 number);
    QString formatAddress(quint16 address);
};

#endif // FULLMICROCODEMEMOIZER_H
