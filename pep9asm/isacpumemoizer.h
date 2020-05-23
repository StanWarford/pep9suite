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
class CacheMemory;
class IsaCpuMemoizer
{

public:
    explicit IsaCpuMemoizer(IsaCpu& cpu);
    ~IsaCpuMemoizer();
    void onSimultationStarted();
    void clear();
    void storeStateInstrEnd();
    void storeStateInstrStart();
    QString memoize();
    QString finalStatistics(bool includeOS);
    quint64 getCycleCount(bool includeOS);
    quint64 getInstructionCount(bool includeOS);
    const QVector<quint32> getInstructionHistogram(bool includeOS);
    bool hasCacheStats() const;
    const CacheHitrates getCacheHitRates(bool includeOS);
private:
    IsaCpu& cpu;
    bool inOS;
    CPUState stateUser, stateOS;
    CacheHitrates cacheUser, cacheOS;
    std::optional<CacheMemory*> cacheDevice;
};

#endif // ISACPUMEMOIZER_H
