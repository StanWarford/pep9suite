/*
    Pep9CPU is a CPU simulator for executing microcode sequences to
    implement instructions in the instruction set of the Pep/9 computer.

    Copyright (C) 2018  Matthew McRaven, Pepperdine University

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
#ifndef AMCCPUMODEL_H
#define AMCCPUMODEL_H

#include "acpumodel.h"
#include "enu.h"
class MicroCode;
class MicrocodeProgram;
/*
 * InterfaceMCCPU describes the operations that may be performed on a microcoded
 * CPU. Inherit in combination with ACPUModel to form a complete description of
 * a working CPU.
 *
 */
class InterfaceMCCPU
{
public:
    explicit InterfaceMCCPU(Enu::CPUType type);
    virtual ~InterfaceMCCPU();

    unsigned int getCycleCounter() const;
    unsigned int getMicrocodeLineNumber() const;
    const MicrocodeProgram* getProgram() const;
    const MicroCode* getCurrentMicrocodeLine() const;

    // Change the CPU between one and two byte data buses
    void setMicrocodeProgram(MicrocodeProgram* program);
    Enu::CPUType getCPUType() const;

    // Clear program counters and breakpoints
    void reset();

    virtual void setCPUType(Enu::CPUType type) = 0;
    virtual void onMCStep() = 0;
    virtual void onClock() = 0;

protected:
    virtual void branchHandler() = 0; // Based on the current instruction, set the µPC correctly
    unsigned int microprogramCounter, microCycleCounter;
    bool microBreakpointHit;
    MicrocodeProgram* program;
    Enu::CPUType type;
};

#endif // AMCCPUMODEL_H
