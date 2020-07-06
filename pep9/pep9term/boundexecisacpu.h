// File: boundedexecisacpu.h
/*
    Pep9Term is a  command line tool utility for assembling Pep/9 programs to
    object code and executing object code programs.

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

/*
 * This class extends the functionality of the default Pep/9 ISA level simulator
 * by checking that the number of instructions executed is less than maxSteps.
 * If the condition is violated, the simulation terminates,
 * an control error is raised, and the corresponding error message indicates the
 * possible presence of an endless loop.
 *
 * When using Pep9Term in a grading script, automatic termination prevents
 * a malformed program from crashing the script.
 */
class BoundExecIsaCpu : public IsaCpu
{
public:
    explicit BoundExecIsaCpu(quint64 stepCount,const AsmProgramManager* manager,
                     QSharedPointer<AMemoryDevice> memDevice, QObject* parent = nullptr);
    virtual ~BoundExecIsaCpu() override;

    // Get the default maximum number of instructions to execute.
    static quint64 getDefaultMaxSteps();

public slots:
    bool onRun() override;

private:
    quint64 maxSteps;
    // Default to a large number of instructions, since
    // system calls may take many hundreds of instructions.
    static const quint64 defaultMaxSteps = 25000;
};

#endif // BOUNDEXCECISACPU_H
