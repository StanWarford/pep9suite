// File: boundedexecmicrocpu.h
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
#ifndef BOUNDEXCECMICROCPU_H
#define BOUNDEXCECMICROCPU_H

#include "fullmicrocodedcpu.h"

/*
 * This class extends the functionality of the default Pep/9Micro simulator
 * by checking that the number of cycles executed is less than maxCycles.
 * If the condition is violated, the simulation terminates,
 * an control error is raised, and the corresponding error message indicates the
 * possible presence of an endless loop.
 *
 * When using Pep9Term in a grading script, automatic termination prevents
 * a malformed program from crashing the script.
 */
class BoundExecMicroCpu : public FullMicrocodedCPU
{
public:
    explicit BoundExecMicroCpu(quint64 maxCycles,
                               const AsmProgramManager* manager, QSharedPointer<const Pep9> pep_version,
                               QSharedPointer<AMemoryDevice> memDevice, QObject* parent = nullptr);
    virtual ~BoundExecMicroCpu() override;

    // Default maximum number of cycles the simulator will execute.
    static quint64 getDefaultMaxCycles();

public slots:
    bool onRun() override;

private:
    quint64 maxCycles;
    // Default to a large number of cycles, since
    // an instruction takes ~20 microcde instructions.
    static const quint64 defaultMaxCycles = 250000;
};

#endif // BOUNDEXCECMICROCPU_H
