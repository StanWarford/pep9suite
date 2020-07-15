// File: interfacemccpu.h
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
#ifndef AMCCPUMODEL_H
#define AMCCPUMODEL_H

#include "cpu/acpumodel.h"
#include "pep/enu.h"

class AExecutableMicrocode;
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
    // Initialize a Interface for a microcoded CPU with the passed features
    explicit InterfaceMCCPU(PepCore::CPUType type) noexcept;
    virtual ~InterfaceMCCPU();
    // Add, remove, & get breakpoints for the program counter.
    // Simulation will trap if the program counter is an element of the breakpointSet.
    void breakpointsSet(QSet<quint16> addressess);
    void breakpointsRemoveAll();
    void breakpointRemoved(quint16 address);
    void breakpointAdded(quint16 address);

    // Get the number of elapsed cycles since the simulation started
    quint64 getCycleCounter() const noexcept;
    // Get the index of the currently executing line of microcode
    quint16 getMicrocodeLineNumber() const noexcept;

    // May be nullptr if no program has been loaded
    QSharedPointer<const MicrocodeProgram> getProgram() const noexcept;
    const AExecutableMicrocode *getCurrentMicrocodeLine() const noexcept;


    void setMicrocodeProgram(QSharedPointer<MicrocodeProgram> sharedProgram);
    PepCore::CPUType getCPUType() const noexcept;

    // Clear program counters & breakpoint status
    void reset() noexcept;

    // Change the CPU between one and two byte data buses
    // This can raise an exception if a cpu implementation doesn't support multiple
    // modes, so callers
    virtual void setCPUType(PepCore::CPUType type) = 0;

    // Perform a single hardware cycle (instruction).
    virtual void onMCStep() = 0;
    // Execute multiple microcode cycles while condition() is true.
    // There seems to be a stigma with std::function as "too slow" compared to function pointers.
    // In our use case, it came with a neglible perfomance penalty (<5%), as the only
    // cost associated with a function object is the additional memory allocated to
    // mantain function state and one extra indirection upon calling (which would be the same overhead of a function pointer).
    // As these functions are only created once per run / step, the overhead of allocating them is
    // amortized across many cycles, making the perfomance overhead is irrelevant. The background activity
    // of the host machine has a far greater effect on performance than these function wrappers wever will.
    virtual void doMCStepWhile(std::function<bool(void)> condition);

    // Execute the control signals that have been set, clear the signals, and perform no µbranch.
    virtual void onClock() = 0;

protected:
    virtual void branchHandler() = 0; // Based on the current instruction, set the µPC correctly
    quint16 microprogramCounter;
    quint64 microCycleCounter;
    bool microBreakpointHit;
    QSharedPointer<MicrocodeProgram> sharedProgram;
    PepCore::CPUType type;
};

#endif // AMCCPUMODEL_H
