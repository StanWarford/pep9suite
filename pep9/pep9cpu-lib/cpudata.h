// File: cpudata.h
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
#ifndef CPUDATASECTION_H
#define CPUDATASECTION_H

#include <QException>
#include <QObject>
#include <QString>
#include <QVector>

#include "cpu/registerfile.h"
#include "pep/constants.h"

#include "cpudefs.h"
#include "pep9.h"

class AMemoryDevice;
class InterfaceMCCPU;
class MemorySection;
class MicroCode;

class CPUDataSection: public QObject
{
    Q_OBJECT
    friend class CPUControlSection;
    friend class InterfaceMCCPU;
public:
    CPUDataSection(PepCore::CPUType type, QSharedPointer<const Pep9::Definition> pep_version,
                   QSharedPointer<AMemoryDevice> memDevice, QObject *parent = nullptr );
    virtual ~CPUDataSection();


    PepCore::CPUType getCPUType() const;
    //Access CPU registers
    RegisterFile& getRegisterBank();
    const RegisterFile& getRegisterBank() const;
    quint8 getRegisterBankByte(quint8 registerNumber) const;
    quint16 getRegisterBankWord(quint8 registerNumber) const; //Follows even/odd conventions of pep/9
    //quint8 getRegisterBankByte(Enu::CPURegisters registerNumber) const;
    //quint16 getRegisterBankWord(Enu::CPURegisters registerNumber) const; //Follows even/odd conventions of pep/9
    quint8 getMemoryRegister(Pep9::uarch::EMemoryRegisters registerNumber)const;

    //Access register & Memory Buses
    bool valueOnABus(quint8& result) const;
    bool valueOnBBus(quint8& result) const;
    bool valueOnCBus(quint8& result) const;
    Pep9::uarch::MainBusState getMainBusState() const;

    //Test for Signals and Registers
    quint8 getControlSignals(Pep9::uarch::EControlSignals controlSignal) const;
    bool getClockSignals(Pep9::uarch::EClockSignals) const;
    bool getStatusBit(Pep9::uarch::EStatusBit) const;

    bool setSignalsFromMicrocode(const MicroCode* line);
    void setEmitEvents(bool b);
    //Return information about errors on the last step
    bool hadErrorOnStep() const;
    QString getErrorMessage() const;

    /*
     * Information about CPU internals
     */
    //Is the CPU function Unary?
    bool aluFnIsUnary() const;
    //Return true if AMux has output, and set result equal to the value of the output.
    //Works for one and two byte buses
    bool getAMuxOutput(quint8 &result) const;
    //Return  true if CSMux has an ouput, and set result equal to the output if present
    bool calculateCSMuxOutput(bool& result) const;
    //Return if the ALU has an ouput, and set result & NZVC bits according to the ALU function
    bool calculateALUOutput(quint8& result,quint8 &NZVC) const;

    //Helper functions to clear different aspects of the data section
    void clearControlSignals() noexcept;
    void clearClockSignals() noexcept;

private:
    void setMemoryDevice(QSharedPointer<AMemoryDevice> newDevice);
    QSharedPointer<const Pep9::Definition> pep_version;
    QSharedPointer<AMemoryDevice> memDevice;

    PepCore::CPUType cpuFeatures;
    Pep9::uarch::MainBusState mainBusState;
    bool emitEvents;

    //Data registers
    QSharedPointer<RegisterFile> registerBank;
    QVector<quint8> memoryRegisters;

    //Control Signals
    QVector<quint8> controlSignals;
    QVector<bool> clockSignals;

    //Error handling
    bool hadDataError;
    QString errorMessage;

    // After thorough profiling, the ALU calculation is the second most computationally expensive part of the data section.
    // The ALU calculation is called multiple times per cycle, yet the result can't change within a cycle.
    // So, to cut back on wasteful calculations, the output of the alu is cached within a cycle.
    // At start of a step, isALUCacheValid must be set to false
    mutable bool isALUCacheValid, ALUHasOutputCache;
    mutable quint8 ALUOutputCache, ALUStatusBitCache;

    //Set the values values of the sequential data registers (numbers 22-31)
    void presetStaticRegisters() noexcept;

    void clearRegisters() noexcept;
    void clearErrors() noexcept;

    //Simulation stepping logic
    void handleMainBusState() noexcept;
    void stepOneByte() noexcept;
    void stepTwoByte() noexcept;

public slots:
    void onSetStatusBit(Pep9::uarch::EStatusBit,bool val);
    void onSetRegisterByte(quint8 reg,quint8 val);
    void onSetRegisterWord(quint8 reg,quint16 val);
    void onSetMemoryRegister(Pep9::uarch::EMemoryRegisters,quint8 val);
    void onSetClock(Pep9::uarch::EClockSignals, bool value);
    void onSetControlSignal(Pep9::uarch::EControlSignals, quint8 value);
    void onStep() noexcept;
    void onClock() noexcept;
    void onClearCPU() noexcept;
    void onSetCPUType(PepCore::CPUType type);

signals:
    void registerChanged(quint8 reg, quint8 oldVal, quint8 newVal); //Thrown whenever a register in the register bank is changed.
    void memoryRegisterChanged(Pep9::uarch::EMemoryRegisters, quint8 oldVal, quint8 newVal); //Thrown whenever a memory register is changed.
    void statusBitChanged(Pep9::uarch::EStatusBit status, bool value);
    void CPUTypeChanged(PepCore::CPUType type);

};

#endif // CPUDATASECTION_H