/*
    Pep9 is a virtual machine for writing machine language and assembly
    language programs.

    Copyright (C) 2019 J. Stanley Warford & Matthew McRaven, Pepperdine University

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
#include "stacktrace.h"
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
 * In addition to implementing the methods as described, several other
 * data members must be updated.
 *
 * 1) Upon starting a non-unary instruction, the decoded operand value
 * must be pre-fetched (without modifying memory state) & decoded
 * according to the incoming instruction specifier. It must then be stored
 * in opValCache, which will cache the decoded operand for use by the UI.
 *
 * It also contains convenience methods for handling assembler breakpoints.
 */
class InterfaceISACPU
{
public:
    explicit InterfaceISACPU(const AMemoryDevice* dev, const AsmProgramManager* manager) noexcept;
    virtual ~InterfaceISACPU();
    // Add, remove, & get breakpoints for the program counter.
    // Simulation will trap if the program counter is an element of the breakpointSet.
    const QSet<quint16> getPCBreakpoints() const noexcept;
    void breakpointsSet(QSet<quint16> addresses) noexcept;
    void breakpointsRemoveAll() noexcept;
    void breakpointRemoved(quint16 address) noexcept;
    void breakpointAdded(quint16 address) noexcept;
    QSharedPointer<const MemoryTrace> getMemoryTrace() const;
    // Return the decoded value of the last executed
    quint16 getOperandValue() const;

    // Clear program counters & breakpoint status
    void reset() noexcept;

    // Output breakpoint changes to the console.
    void setDebugBreakpoints(bool doDebug) noexcept;


    // Stores the call depth, and continues to execute ISA instructions
    // until the new call depth equals the old call depth.
    virtual void stepOver() = 0;
    // Is the next instruction a call or a trap that can be stepped into?
    virtual bool canStepInto() const = 0;
    // Uncoditionally executes the next ISA instruction, including going into function calls and traps.
    virtual void stepInto() = 0;
    // Executes the next ISA instructions until the call depth is decreased by 1.
    virtual void stepOut() = 0;

    // Returns how many cycles were used by the CPU. For an ISA level CPU, this will be
    // equal to getInstructionCount(), but for a microcoded implementation it may be different.
    virtual quint64 getCycleCount(bool includeOS) = 0;
    // Returns how many ISA level instructions (including those in the OS) were executed.
    virtual quint64 getInstructionCount(bool includeOS) = 0;
    // Returns a 256 element vector that indicates how many times each opcode was used.
    virtual const QVector<quint32> getInstructionHistogram(bool includeOS) = 0;
protected:
    // Execute a single ISA instruction.
    virtual void onISAStep() = 0;
    // Execute multiple ISA steps while condition() is true.
    // There seems to be a stigma with std::function as "too slow" compared to function pointers.
    // In our use case, it came with a neglible perfomance penalty (<5%), as the only
    // cost associated with a function object is the additional memory allocated to
    // mantain function state and one extra indirection upon calling (which would be the same overhead of a function pointer).
    // As these functions are only created once per run / step, the overhead of allocating them is
    // amortized across many cycles, making the perfomance overhead is irrelevant. The background activity
    // of the host machine has a far greater effect on performance than these function wrappers wever will.
    virtual void doISAStepWhile(std::function<bool(void)> condition);

    // Update simulation state at the start of a assembly level instruction
    virtual void updateAtInstructionEnd() = 0;
    void calculateStackChangeStart(quint8 instr);
    void calculateStackChangeEnd(quint8 instr, quint16 opspec, quint16 sp, quint16 pc, quint16 acc);

    const AsmProgramManager* manager;
    // Decoded operand value. The UI needs this value to render properly,
    // but the act of simulating might modify the value after the fact.
    // So cache the operand during decoding to prevent the UI from becoming out
    // of sync with machine state.
    quint16 opValCache;

    //Breakpoint information
    QSet<quint16> breakpointsISA;
    quint64 asmStepCount;
    bool asmBreakpointHit, doDebug;

    //Stack tracing information
    bool firstLineAfterCall, isTrapped;
    QSharedPointer<MemoryTrace> memTrace;
    QStack<stackAction> userActions, osActions, *activeActions;
    quint16 heapPtr;
};

#endif // AISACPU_H
