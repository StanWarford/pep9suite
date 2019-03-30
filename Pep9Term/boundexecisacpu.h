// File: boundedexecisacpu.h
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
#ifndef BOUNDEXCECISACPU_H
#define BOUNDEXCECISACPU_H
#include "isacpu.h"

class BoundExecIsaCpu : public IsaCpu
{
    quint64 maxSteps;
    static const quint64 defaultMaxSteps = 25000;
public:
    explicit BoundExecIsaCpu(quint64 stepCount,const AsmProgramManager* manager,
                     QSharedPointer<AMemoryDevice> memDevice, QObject* parent = nullptr);
    virtual ~BoundExecIsaCpu() override;

    static quint64 getDefaultMaxSteps();

public slots:
    bool onRun() override;
};

#endif // BOUNDEXCECISACPU_H
