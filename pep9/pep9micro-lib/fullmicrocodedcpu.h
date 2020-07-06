// File: fullmicrocodedcpu.h
/*
    Pep9Micro is a complete CPU simulator for the Pep/9 instruction set,
    and is capable of assembling programs to object code, executing
    object code programs, and executing microcode fragments.

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
#ifndef FULLMICROCODEDCPU_H
#define FULLMICROCODEDCPU_H

#include <array>


#include <QElapsedTimer>
#include <QObject>

#include "cpu/interfacemccpu.h"
#include "cpu/interfaceisacpu.h"
class CPUDataSection;
class FullMicrocodedMemoizer;
class FullMicrocodedCPU : public ACPUModel, public InterfaceMCCPU, public InterfaceISACPU
{
    Q_OBJECT
    friend class CPUMemoizer;
    friend class FullMicrocodedMemoizer;
public:
    FullMicrocodedCPU(const AsmProgramManager* manager, QSharedPointer<AMemoryDevice>, QObject* parent = nullptr) noexcept;
    virtual ~FullMicrocodedCPU() override;
    QSharedPointer<CPUDataSection> getDataSection();
    // Returns true if the microprogram counter is at the
    // start of the von neumann cycle.
    bool atMicroprogramStart() const noexcept;
    // Set the microprogram counter to whatever the value of "start" is.
    // This can be used to skip the initialization steps at the top
    // of a microcode program.
    void setMicroPCToStart() noexcept;

    // ACPUModel interface
    bool getStatusBitCurrent(Enu::EStatusBit) const override;
    bool getStatusBitStart(Enu::EStatusBit) const override;
    quint8 getCPURegByteCurrent(Enu::CPURegisters reg) const override;
    quint16 getCPURegWordCurrent(Enu::CPURegisters reg) const override;
    quint8 getCPURegByteStart(Enu::CPURegisters reg) const override;
    quint16 getCPURegWordStart(Enu::CPURegisters reg) const override;
    void initCPU() override;
    bool stoppedForBreakpoint() const noexcept override;
    QString getErrorMessage() const noexcept override;
    bool hadErrorOnStep() const noexcept override;

    // InterfaceMCCPU interface
    void setCPUType(Enu::CPUType type)  override;
    void onMCStep() override;
    void onClock() override;

    // InterfaceISACPU interface
    void stepOver() override;
    bool canStepInto() const override;
    void stepInto() override;
    void stepOut() override;
    quint64 getCycleCount(bool includeOS) override;
    quint64 getInstructionCount(bool includeOS) override;
    const QVector<quint32> getInstructionHistogram(bool includeOS) override;
    bool hasCacheStats() override {return false;}
    const CacheHitrates getCacheHitRates(bool /*includeOS*/) override {return {};}

public slots:
    void onSimulationStarted() override;
    void onSimulationFinished() override;
    void enableDebugging() override;
    void forceBreakpoint(Enu::BreakpointTypes) override;
    void onCancelExecution() override;
    bool onRun() override;
    void onResetCPU() override;

protected:
    // InterfaceISACPU interface
    void onISAStep() override;

private:
    bool isPrefetchValid;
    QElapsedTimer timer;
    CPUDataSection *data;
    QSharedPointer<CPUDataSection> dataShared;
    FullMicrocodedMemoizer *memoizer;
    // A class to represent a single item in the instruction specifier
    // or addressing mode decoder.
    struct decoder_entry {
        quint16 addr;
        bool isValid;
    };

    // For each instruction in the instruction set, map the instruction to
    // the first line of microcode that implements it.This calculation is
    // done each time a microprogram is run to account for mnemonic redefinitons.
    // Any modification to the Pep:: instruction mappings while the simulator is running
    // could cause the microprogram to error in unexpected ways.
    std::array<decoder_entry, 256> instrSpecJT;

    // For each of the 256 instruction specifier values, map the
    // addressing mode associated with that IS to the first line of
    // microcode implementing that addressing mode. Do not modify
    // any of the Pep:: addressing mode maps while the simulator
    // is running, else a microprogram might fail unexpectedly.
    std::array<decoder_entry, 256> addrModeJT;
    quint16 startLine = 0;

    void breakpointAsmHandler();
    void breakpointMicroHandler();
    void setSignalsFromMicrocode(const MicroCode *line);
    void branchHandler() override;
    void updateAtInstructionEnd() override;
    // For all 256 instructions in the Pep/9 insturction set,
    // map the instruction to the first line of microcode that implements it.
    void calculateInstrJT();
    void calculateAddrJT();

};

#endif // FULLMICROCODEDCPU_H
