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
    // Initialize a Interface for a microcoded CPU with the passed features
    explicit InterfaceMCCPU(Enu::CPUType type) noexcept;
    virtual ~InterfaceMCCPU();

    // Get the number of elapsed cycles since the simulation started
    quint64 getCycleCounter() const noexcept;
    // Get the index of the currently executing line of microcode
    quint16 getMicrocodeLineNumber() const noexcept;

    // May be nullptr if no program has been loaded
    QSharedPointer<const MicrocodeProgram> getProgram() const noexcept;
    const MicroCode* getCurrentMicrocodeLine() const noexcept;


    void setMicrocodeProgram(QSharedPointer<MicrocodeProgram> sharedProgram);
    Enu::CPUType getCPUType() const noexcept;

    // Clear program counters & breakpoint status
    void reset() noexcept;

    // Change the CPU between one and two byte data buses
    // This can raise an exception if a cpu implementation doesn't support multiple
    // modes, so callers
    virtual void setCPUType(Enu::CPUType type) = 0;
    // Perform a single hardware cycle (instruction).
    virtual void onMCStep() = 0;
    // Execute the control signals that have been set, clear the signals, and perform no µbranch.
    virtual void onClock() = 0;

protected:
    virtual void branchHandler() = 0; // Based on the current instruction, set the µPC correctly
    quint16 microprogramCounter;
    quint64 microCycleCounter;
    bool microBreakpointHit;
    QSharedPointer<MicrocodeProgram> sharedProgram;
    Enu::CPUType type;
};

#endif // AMCCPUMODEL_H
