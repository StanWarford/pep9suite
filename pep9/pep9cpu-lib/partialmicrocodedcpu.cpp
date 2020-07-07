// File: partialmicrocodedcpu.cpp
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
#include "partialmicrocodedcpu.h"

#include <QApplication>
#include <QTimer>

#include "cpu/cpudata.h"
#include "cpu/interrupthandler.h"
#include "cpu/registerfile.h"
#include "memory/amemorydevice.h"
#include "microassembler/microcode.h"
#include "microassembler/microcodeprogram.h"
#include "pep/pep.h"
#include "symbol/symbolentry.h"

#include "partialmicrocodedmemoizer.h"

PartialMicrocodedCPU::PartialMicrocodedCPU(Enu::CPUType type, QSharedPointer<const Pep9> pep_version,
                                           QSharedPointer<AMemoryDevice> memoryDev, QObject* parent) noexcept: ACPUModel (memoryDev, parent),
    InterfaceMCCPU(type), memoizer(new PartialMicrocodedMemoizer(*this))
{
    data = new CPUDataSection(type, pep_version, memoryDev, parent);
    dataShared = QSharedPointer<CPUDataSection>(data);
    // Create & register callbacks for breakpoint interrupts.
    std::function<void(void)> bpHandler = [this](){breakpointMicroHandler();};
    ACPUModel::handler->registerHandler(Interrupts::BREAKPOINT_MICRO, bpHandler);
}

PartialMicrocodedCPU::~PartialMicrocodedCPU()
{
    //This object should last the lifetime of the  program, it does not need to be cleaned up
    delete memoizer;
}

QSharedPointer<CPUDataSection> PartialMicrocodedCPU::getDataSection()
{
    return dataShared;
}

bool PartialMicrocodedCPU::getStatusBitCurrent(Enu::EStatusBit bit) const
{
    return data->getRegisterBank().readStatusBitCurrent(bit);
}

bool PartialMicrocodedCPU::getStatusBitStart(Enu::EStatusBit bit) const
{
    return data->getRegisterBank().readStatusBitStart(bit);
}

quint8 PartialMicrocodedCPU::getCPURegByteCurrent(PepCore::CPURegisters_number_t reg) const
{
    return data->getRegisterBank().readRegisterByteCurrent(reg);
}

quint16 PartialMicrocodedCPU::getCPURegWordCurrent(PepCore::CPURegisters_number_t reg) const
{
    return data->getRegisterBank().readRegisterWordCurrent(reg);
}

quint8 PartialMicrocodedCPU::getCPURegByteStart(PepCore::CPURegisters_number_t reg) const
{
    return data->getRegisterBank().readRegisterByteStart(reg);
}

quint16 PartialMicrocodedCPU::getCPURegWordStart(PepCore::CPURegisters_number_t reg) const
{
    return data->getRegisterBank().readRegisterWordStart(reg);
}

quint8 PartialMicrocodedCPU::getCPURegByteCurrent(Pep9::CPURegisters reg) const
{
    return getCPURegByteCurrent(to_uint8_t(reg));
}

quint16 PartialMicrocodedCPU::getCPURegWordCurrent(Pep9::CPURegisters reg) const
{
    return getCPURegWordCurrent(to_uint8_t(reg));
}

quint8 PartialMicrocodedCPU::getCPURegByteStart(Pep9::CPURegisters reg) const
{
    return getCPURegByteStart(to_uint8_t(reg));
}

quint16 PartialMicrocodedCPU::getCPURegWordStart(Pep9::CPURegisters reg) const
{
    return getCPURegWordStart(to_uint8_t(reg));
}

void PartialMicrocodedCPU::initCPU()
{
    // No init needs to be performed on CPU.
}

bool PartialMicrocodedCPU::stoppedForBreakpoint() const noexcept
{
    return microBreakpointHit;
}

QString PartialMicrocodedCPU::getErrorMessage() const noexcept
{
    if(memory->hadError()) return memory->getErrorMessage();
    else if(data->hadErrorOnStep()) return data->getErrorMessage();
    else if(hadErrorOnStep()) return errorMessage;
    else return "";
}

bool PartialMicrocodedCPU::hadErrorOnStep() const noexcept
{
    return controlError || data->hadErrorOnStep() || memory->hadError();
}

void PartialMicrocodedCPU::setCPUType(Enu::CPUType type)
{
    this->type = type;
    this->data->onSetCPUType(type);
}

void PartialMicrocodedCPU::onSimulationStarted()
{
    inDebug = false;
    inSimulation = false;
    executionFinished = false;
    microBreakpointHit = false;
    memoizer->clear();
    memory->clearErrors();
    ACPUModel::handler->clearQueuedInterrupts();
}

void PartialMicrocodedCPU::onSimulationFinished()
{
    data->clearClockSignals();
    data->clearControlSignals();
    executionFinished = true;
    inDebug = false;
    ACPUModel::handler->clearQueuedInterrupts();
}

void PartialMicrocodedCPU::enableDebugging()
{
    inDebug = true;
}

void PartialMicrocodedCPU::forceBreakpoint(PepCore::BreakpointTypes breakpoint)
{
    switch(breakpoint){
    case PepCore::BreakpointTypes::MICROCODE:
        ACPUModel::handler->interupt(Interrupts::BREAKPOINT_MICRO);
        break;
    default:
        // Don't handle other kinds of breakpoints if they are generated.
        return;
    }

}

void PartialMicrocodedCPU::onCancelExecution()
{
    executionFinished = true;
    inDebug = false;
}

bool PartialMicrocodedCPU::onRun()
{
    std::function<bool(void)> cond = [this] () {
        bool rVal = !hadErrorOnStep() && !executionFinished && !(inDebug && (microBreakpointHit));
        // Don't clear written bytes on last cycle, so that the user may see what
        // the last instruction modified.
        if(rVal) memory->clearAllByteCaches();
        return rVal;
    };
    // Execute microcode steps until the condition function is false.
    doMCStepWhile(cond);

    //If there was an error on the control flow
    if(hadErrorOnStep()) {
        if(memory->hadError()) {
            qDebug() << "Memory section reporting an error";
            //emit simulationFinished();
            return false;
        }
        else if(data->hadErrorOnStep()) {
            qDebug() << "Data section reporting an error";
            //emit simulationFinished();
            return false;
        }
        else {
            qDebug() << "Control section reporting an error";
            //emit simulationFinished();
            return false;
        }
    }

    // If a breakpoint was reached, return before final statistics are computed or the simulation is finished.
    if(microBreakpointHit) {
        return false;
    }
    return true;
}

void PartialMicrocodedCPU::onResetCPU()
{
    // Reset all internal state, but keep loaded micropgoram & breakpoints
    data->onClearCPU();
    ACPUModel::memory->clearErrors();
    memoizer->clear();
    InterfaceMCCPU::reset();
    inSimulation = false;
    inDebug = false;
    callDepth = 0;
    controlError = false;
    executionFinished = false;
    errorMessage = "";
    microBreakpointHit = false;
    ACPUModel::handler->clearQueuedInterrupts();
}

void PartialMicrocodedCPU::onMCStep()
{
    microBreakpointHit = false;
    if(microprogramCounter == 0) {
        // Store PC at the start of the cycle, so that we know where the instruction started from.
        // Also store any other values needed for detailed statistics
        memoizer->storeStateInstrStart();
        memory->onCycleStarted();
    }

    // Do step logic
    const MicroCode* prog = sharedProgram->getCodeLine(microprogramCounter);

    try {
        data->setSignalsFromMicrocode(prog);
    }
    /*
     * An invalid_argument execption will be thrown if data's memcpy would fail.
     * By catching this exception, we convert a hard failure to a friendly error message
     * for the user application. Any other exceptions should crash the program, because
     * we do not expect other issues to arise from memcpy.
     *
     * Since exceptions are zero-cost during error-free execution, and failing to set microcode
     * signals would be irreparable, the decisions to communicate failure through execptions
     * was made as a perfomance decision.
     */
    catch (std::invalid_argument &) {
        return;
    }

    data->onStep();
    branchHandler();
    microCycleCounter++;
    //qDebug().nospace().noquote() << prog->getSourceCode();

    if(executionFinished || hadErrorOnStep()) {
        memoizer->storeStateInstrEnd();
        data->getRegisterBank().flattenFile();
        emit simulationFinished();

    }

    memory->onCycleFinished();
    // Upon entering an instruction that is going to trap
    // If running in debug mode, first check if this line has any microcode breakpoints.
    if(inDebug && sharedProgram->getCodeLine(microprogramCounter)->hasBreakpoint()) {
        ACPUModel::handler->interupt(Interrupts::BREAKPOINT_MICRO);
    }

    ACPUModel::handler->handleQueuedInterrupts();
}

void PartialMicrocodedCPU::onClock()
{
    //Do clock logic
    if(!inSimulation) {
        data->onClock();
    }
    else {
        //One should not get here, otherwise that would mean that we clocked in a simulation
    }
}

void PartialMicrocodedCPU::branchHandler()
{
    if(sharedProgram->getCodeLine(microprogramCounter)->getBranchFunction() == Enu::EBranchFunctions::Stop) {
        executionFinished = true;
    }
    else if(executionFinished) {
        return;
    }
    else {
        microprogramCounter++;
    }
}

void PartialMicrocodedCPU::breakpointMicroHandler()
{
    microBreakpointHit = true;
    emit hitBreakpoint(PepCore::BreakpointTypes::MICROCODE);
    return;
}
