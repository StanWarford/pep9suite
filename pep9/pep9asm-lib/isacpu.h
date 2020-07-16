// File: isacpu.h
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
#ifndef ISACPU_H
#define ISACPU_H

#include <QElapsedTimer>

#include "cpu/interfaceisacpu.h"
#include "cpu/registerfile.h"
#include "memory/amemorydevice.h"

#include "pep9.h"
#include "pep9interfaceisacpu.h"

/* Though not part of the specification, the trap mechanism  must
 * set the index register to 0 to prevent a bug in OS where
 * non-unary instructions fail due to junk in the high order byte of the index
 * register. This flag enables or disables this behavior.
 */
#define performTrapFix true

/* Though not part of the specification, the trap mechanism  must
 * increment the program counter by 2 in Pep/9 for the operating system
 * to function correctly. This flag enables or disables this behvaior
 */
#define hardwarePCIncr true
class CPUDataSection;
class IsaCpuMemoizer;

class IsaCpu: public ACPUModel, public Pep9InterfaceISACPU
{
    friend class IsaCpuMemoizer;
public:
    explicit IsaCpu(const AsmProgramManager* manager, QSharedPointer<const Pep9::Definition> pep_version,
                    QSharedPointer<AMemoryDevice>, QObject* parent = nullptr);
    virtual ~IsaCpu() override;
    // InterfaceISACPU interface
public:
    void stepOver() override;
    bool canStepInto() const override;
    void stepInto() override;
    void stepOut() override;
    quint64 getCycleCount(bool includeOS) override;
    quint64 getInstructionCount(bool includeOS) override;
    const QVector<quint32> getInstructionHistogram(bool includeOS) override;
    bool hasCacheStats() override;
    const CacheHitrates getCacheHitRates(bool includeOS) override;

    RegisterFile& getRegisterBank();
    const RegisterFile& getRegisterBank() const;

    quint8 getCPURegByteCurrent(Pep9::ISA::CPURegisters reg) const;
    quint16 getCPURegWordCurrent(Pep9::ISA::CPURegisters reg) const;
    quint8 getCPURegByteStart(Pep9::ISA::CPURegisters reg) const;
    quint16 getCPURegWordStart(Pep9::ISA::CPURegisters reg) const;

    bool getStatusBitCurrent(Pep9::ISA::EStatusBit bit) const;
    bool getStatusBitStart(Pep9::ISA::EStatusBit bit) const;

protected:
    void onISAStep() override;
    void updateAtInstructionEnd() override;
    bool readOperandWordValue(quint16 operand, Pep9::ISA::EAddrMode addrMode, quint16& opVal);
    bool readOperandByteValue(quint16 operand, Pep9::ISA::EAddrMode addrMode, quint8& opVal);


    // ACPUModel interface
public:
    void initCPU() override;
    bool getStatusBitCurrent(PepCore::CPUStatusBits_name_t bit) const override;
    bool getStatusBitStart(PepCore::CPUStatusBits_name_t bit) const override;

    QString getErrorMessage() const noexcept override;
    quint8 getCPURegByteCurrent(PepCore::CPURegisters_number_t reg) const override;
    quint16 getCPURegWordCurrent(PepCore::CPURegisters_number_t reg) const override;
    // Return the value of a register at the start of an instruction.
    quint8 getCPURegByteStart(PepCore::CPURegisters_number_t reg) const override;
    quint16 getCPURegWordStart(PepCore::CPURegisters_number_t reg) const override;
    bool hadErrorOnStep() const noexcept override;
    bool stoppedForBreakpoint() const noexcept override;

public slots:
    void onSimulationStarted() override;
    void onSimulationFinished() override;
    void enableDebugging() override;
    void forceBreakpoint(PepCore::BreakpointTypes) override;
    void onCancelExecution() override;
    bool onRun() override;
    void onResetCPU() override;


private:
    QSharedPointer<const APepVersion> pep_version;
    RegisterFile registerBank;
    QElapsedTimer timer;
    IsaCpuMemoizer* memoizer;
    bool operandWordValueHelper(quint16 operand, Pep9::ISA::EAddrMode addrMode,
                         bool (AMemoryDevice::*readFunc)(quint16, quint16 &) const, quint16& opVal);
    bool operandByteValueHelper(quint16 operand, Pep9::ISA::EAddrMode addrMode,
                         bool (AMemoryDevice::*readFunc)(quint16, quint8&) const, quint8& opVal);
    bool writeOperandWord(quint16 operand, quint16 value, Pep9::ISA::EAddrMode addrMode);
    bool writeOperandByte(quint16 operand, quint8 value, Pep9::ISA::EAddrMode addrMode);
    void executeUnary(Pep9::ISA::EMnemonic mnemon);
    void executeNonunary(Pep9::ISA::EMnemonic mnemon, quint16 opSpec, Pep9::ISA::EAddrMode addrMode);
    void executeTrap(Pep9::ISA::EMnemonic mnemon);
    // Callback function to handle InteruptHandler's BREAKPOINT_ASM.
    void breakpointAsmHandler();
    void writeStatusBit(Pep9::ISA::EStatusBit bit, bool value);
    const PepCore::CPURegisters_number_t a_reg, x_reg, sp_reg, pc_reg, is_reg, os_reg;



};

#endif // ISACPU_H
