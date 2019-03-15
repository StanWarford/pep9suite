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
#include "registerfile.h"
PartialMicrocodedCPU::PartialMicrocodedCPU(Enu::CPUType type, QSharedPointer<AMemoryDevice> memoryDev, QObject* parent) noexcept: ACPUModel (memoryDev, parent),
    InterfaceMCCPU(type)
{
    memoizer = new PartialMicrocodedMemoizer(*this);
    data = new NewCPUDataSection(type, memoryDev, parent);
    dataShared = QSharedPointer<NewCPUDataSection>(data);
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
    return data->getRegisterBank().readStatusBitCurrent(bit);
}

bool PartialMicrocodedCPU::getStatusBitStart(Enu::EStatusBit bit) const
{
    return data->getRegisterBank().readStatusBitStart(bit);
}

quint8 PartialMicrocodedCPU::getCPURegByteCurrent(Enu::CPURegisters reg) const
{
    return data->getRegisterBank().readRegisterByteCurrent(reg);
}

quint16 PartialMicrocodedCPU::getCPURegWordCurrent(Enu::CPURegisters reg) const
{
    return data->getRegisterBank().readRegisterWordCurrent(reg);
}

quint8 PartialMicrocodedCPU::getCPURegByteStart(Enu::CPURegisters reg) const
{
    return data->getRegisterBank().readRegisterByteStart(reg);
}

quint16 PartialMicrocodedCPU::getCPURegWordStart(Enu::CPURegisters reg) const
{
    return data->getRegisterBank().readRegisterWordStart(reg);
}

void PartialMicrocodedCPU::initCPU()
{
#pragma message ("Should there be any init done here?")
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
    executionFinished = true;
    inDebug = false;
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
            return false;
        }
        else if(data->hadErrorOnStep()) {
            qDebug() << "Data section reporting an error";
            return false;
        }
        else {
            qDebug() << "Control section reporting an error";
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
}

void PartialMicrocodedCPU::onMCStep()
{

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

    if(/*microprogramCounter == 0 ||*/ executionFinished) {
        memoizer->storeStateInstrEnd();
        data->getRegisterBank().flattenFile();
        if(executionFinished) emit simulationFinished();

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
    else if(executionFinished) {
        return;
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
