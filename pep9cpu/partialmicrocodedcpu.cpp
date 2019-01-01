#include "partialmicrocodedcpu.h"

#include <QApplication>
#include <QTimer>

#include "amemorydevice.h"
#include "microcode.h"
#include "microcodeprogram.h"
#include "pep.h"
#include "newcpudata.h"
#include "symbolentry.h"
#include "partialmicrocodedmemoizer.h"
PartialMicrocodedCPU::PartialMicrocodedCPU(QSharedPointer<AMemoryDevice> memoryDev, QObject* parent) noexcept: ACPUModel (memoryDev, parent),
    InterfaceMCCPU(Enu::CPUType::TwoByteDataBus)
{
    memoizer = new PartialMicrocodedMemoizer(*this);
    data = new NewCPUDataSection(Enu::CPUType::TwoByteDataBus, memoryDev, parent);
    dataShared = QSharedPointer<NewCPUDataSection>(data);
    setDebugLevel(Enu::DebugLevels::NONE);
}

PartialMicrocodedCPU::~PartialMicrocodedCPU()
{
    //This object should last the lifetime of the  program, it does not need to be cleaned up
    delete memoizer;
}

QSharedPointer<NewCPUDataSection> PartialMicrocodedCPU::getDataSection()
{
    return dataShared;
}

bool PartialMicrocodedCPU::getStatusBitCurrent(Enu::EStatusBit bit) const
{
    return data->getStatusBit(bit);
}

bool PartialMicrocodedCPU::getStatusBitStart(Enu::EStatusBit bit) const
{
    if (microprogramCounter == 0) {
        return getStatusBitCurrent(bit);
    }
    else return memoizer->getStatusBitStart(bit);
}

quint8 PartialMicrocodedCPU::getCPURegByteCurrent(Enu::CPURegisters reg) const
{
    return data->getRegisterBankByte(reg);
}

quint16 PartialMicrocodedCPU::getCPURegWordCurrent(Enu::CPURegisters reg) const
{
    return data->getRegisterBankWord(reg);
}

quint8 PartialMicrocodedCPU::getCPURegByteStart(Enu::CPURegisters reg) const
{
    if (microprogramCounter == 0) {
        return getCPURegByteCurrent(reg);
    }
    else return memoizer->getRegisterByteStart(reg);
}

quint16 PartialMicrocodedCPU::getCPURegWordStart(Enu::CPURegisters reg) const
{
    if (microprogramCounter == 0) {
        return getCPURegWordCurrent(reg);
    }
    else return memoizer->getRegisterWordStart(reg);
}

void PartialMicrocodedCPU::initCPU()
{
    // Initialize CPU with proper stack pointer value in SP register.
    data->onSetRegisterByte(4,0xFB);
    data->onSetRegisterByte(5,0x82);
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

Enu::DebugLevels PartialMicrocodedCPU::getDebugLevel() const noexcept
{
    return memoizer->getDebugLevel();
}

void PartialMicrocodedCPU::setDebugLevel(Enu::DebugLevels level)
{
    memoizer->setDebugLevel(level);
}

void PartialMicrocodedCPU::setCPUType(Enu::CPUType)
{
    throw std::logic_error("Can't change CPU type on fullmicrococdedcpu, it must always be two byte bus");
}

void PartialMicrocodedCPU::onSimulationStarted()
{
    inDebug = false;
    inSimulation = false;
    executionFinished = false;
    memoizer->clear();
}

void PartialMicrocodedCPU::onSimulationFinished()
{
    data->clearClockSignals();
    data->clearControlSignals();
    executionFinished = true;
    inDebug = false;
}

void PartialMicrocodedCPU::onDebuggingStarted()
{
    onSimulationStarted();
    inDebug = true;
    microBreakpointHit = false;
}

void PartialMicrocodedCPU::onDebuggingFinished()
{
    onSimulationFinished();
    inDebug = false;
}

void PartialMicrocodedCPU::onCancelExecution()
{
    #pragma message("TODO: Cancel execution")
    throw -1;
}

bool PartialMicrocodedCPU::onRun()
{
    // If debugging, there is the potential to hit breakpoints, so a different main loop is needed.
    // Partially, this is to handle breakpoints gracefully, and partially to prevent "run" mode from being slowed down by debug features.
    if(inDebug) {
        // Always execute at least once, otherwise cannot progress past breakpoints
        do {
            // Since the sim runs at about 5Mhz, do not process events every single cycle to increase performance.
            if(microCycleCounter % 5000 == 0) {
                QApplication::processEvents();
            }
            onMCStep();
        } while(!hadErrorOnStep() && !executionFinished && !(microBreakpointHit));
    }
    else {
        while(!hadErrorOnStep() && !executionFinished) {
            // Since the sim runs at about 5Mhz, do not process events every single cycle to increase performance.
            if(microCycleCounter % 5000 == 0) {
                QApplication::processEvents();
            }
            onMCStep();
        }
    }

    //If there was an error on the control flow
    if(hadErrorOnStep()) {
        if(memory->hadError()) {
            qDebug() << "Memory section reporting an error";
            emit simulationFinished();
            return false;
        }
        else if(data->hadErrorOnStep()) {
            qDebug() << "Data section reporting an error";
            emit simulationFinished();
            return false;
        }
        else {
            qDebug() << "Control section reporting an error";
            emit simulationFinished();
            return false;
        }
    }

    // If a breakpoint was reached, return before final statistics are computed or the simulation is finished.
    if(microBreakpointHit) {
        return false;
    }
    emit simulationFinished();
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
}

void PartialMicrocodedCPU::onMCStep()
{

#pragma message("TODO: make micro program counter loop point variable")
    if(microprogramCounter == 0) {
        // Store PC at the start of the cycle, so that we know where the instruction started from.
        // Also store any other values needed for detailed statistics
        memoizer->storeStateInstrStart();
        memory->onCycleStarted();
    }

    // Do step logic
    const MicroCode* prog = sharedProgram->getCodeLine(microprogramCounter);

    data->setSignalsFromMicrocode(prog);
    data->onStep();
    branchHandler();
    microCycleCounter++;
    //qDebug().nospace().noquote() << prog->getSourceCode();

    if(microprogramCounter == 0 || executionFinished) {
        memoizer->storeStateInstrEnd();
        emit asmInstructionFinished();
        if(memoizer->getDebugLevel() != Enu::DebugLevels::NONE) {
            qDebug().noquote().nospace() << memoizer->memoize();
        }

    }
    // Upon entering an instruction that is going to trap
    // If running in debug mode, first check if this line has any microcode breakpoints.
    if(inDebug) {
        breakpointHandler();
    }
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
    else {
        microprogramCounter++;
    }
}

void PartialMicrocodedCPU::breakpointHandler()
{
    // If the CPU is not being debugged, breakpoints make no sense. Abort.
    if(!inDebug) return;
    // Trap on micrcode breakpoints
    else if(sharedProgram->getCodeLine(microprogramCounter)->hasBreakpoint()) {
        microBreakpointHit = true;
        emit hitBreakpoint(Enu::BreakpointTypes::MICROCODE);
        return;
    }
    else {
        microBreakpointHit = false;
    }
}
