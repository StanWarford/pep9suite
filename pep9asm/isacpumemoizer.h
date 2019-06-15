// File: isacpumemoizer.h
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
