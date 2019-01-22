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
#ifndef AISACPUMODEL_H
#define AISACPUMODEL_H

#include "acpumodel.h"
#include <QSet>
#include <QtCore>
#include <ostream>
class AMemoryDevice;
class AsmProgramManager;
enum class stackAction {
    locals, params, call
};
/*
 * InterfaceISACPU describes the operations that may be done on a ISA level CPU.
 * When inherited in combination with the data desciption of a CPU (ACPUModel), a
 * CPU at the ISA level is described.
 *
 * It also contains convenience methods for handling assembler breakpoints.
 */

class InterfaceISACPU
{
public:
    explicit InterfaceISACPU(const AsmProgramManager* manager) noexcept;
    virtual ~InterfaceISACPU();
    // Add, remove, & get breakpoints for the program counter.
    // Simulation will trap if the program counter is an element of the breakpointSet.
    const QSet<quint16> getPCBreakpoints() const noexcept;
    void breakpointsSet(QSet<quint16> addresses) noexcept;
    void breakpointsRemoveAll() noexcept;
    void breakpointRemoved(quint16 address) noexcept;
    void breakpointAdded(quint16 address) noexcept;

    // Clear program counters & breakpoint status
    void reset() noexcept;

    // Output breakpoint changes to the console.
    void setDebugBreakpoints(bool doDebug) noexcept;

    // Execute the next ISA instruction.
    virtual void onISAStep() = 0;
protected:
    virtual void updateAtInstructionEnd() = 0; // Update simulation state at the start of a assembly level instruction
    void calculateStackChangeStart(quint8 instr);
    void calculateStackChangeEnd(quint8 instr, quint16 opspec);

    const AsmProgramManager* manager;
    QSet<quint16> breakpointsISA;
    int asmInstructionCounter;
    bool asmBreakpointHit, doDebug;

    bool firstLineAfterCall, isTrapped;
    QStack<stackAction> userActions, osActions, *activeActions;
    bool userStackIntact, osStackIntact, *activeIntact;
};

#endif // AISACPU_H
