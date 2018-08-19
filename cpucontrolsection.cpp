#include "cpucontrolsection.h"
#include <QApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <qmutex.h>
#include <QMutexLocker>

#include "cpudatasection.h"
#include "microcode.h"
#include "microcodeprogram.h"
#include "symbolentry.h"
#include "pep.h"
#include "symbolentry.h"
#include "symboltable.h"
#include "symboltable.h"
#include "memorysection.h"
#include "cpumemoizer.h"

QElapsedTimer timer;
CPUControlSection *CPUControlSection::_instance = nullptr;
CPUControlSection::CPUControlSection(CPUDataSection * data, MemorySection* memory): QObject(nullptr),
    memoizer(new CPUMemoizer(*this)), data(data), memory(memory),
    microprogramCounter(0), microCycleCounter(0), instructionCounter(0), callDepth(0),
    inSimulation(false), hadControlError(false), isPrefetchValid(false), inDebug(false), microBreakpointHit(false), microBreakpointHandled(false),
    asmBreakpointHit(false), asmBreakpointHandled(false)
{

}

CPUControlSection *CPUControlSection::getInstance()
{
    if(_instance == nullptr) {
        _instance = new CPUControlSection(CPUDataSection::getInstance(),MemorySection::getInstance());
    }
    return _instance;
}

CPUControlSection::~CPUControlSection()
{
    //This object should last the lifetime of the  program, it does not need to be cleaned up
}

void CPUControlSection::initCPU()
{
    // Initialize CPU with proper stack pointer value in SP register.
#pragma message ("TODO: Init cpu with proper SP when not burned in at 0xffff")
    data->onSetRegisterByte(4,0xFB);
    data->onSetRegisterByte(5,0xF8);
}

void CPUControlSection::setMicrocodeProgram(MicrocodeProgram *program)
{
    this->program = program;
    microprogramCounter = 0;
}

void CPUControlSection::setDebugLevel(Enu::DebugLevels level)
{
    if(!inSimulation) this->memoizer->setDebugLevel(level);
}

const CPUMemoizer *CPUControlSection::getCPUMemoizer() const
{
    return memoizer;
}

void CPUControlSection::setPCBreakpoints(QSet<quint16> breakpoints)
{
    pcBreakpoints = breakpoints;
}

const QSet<quint16> CPUControlSection::getPCBreakpoints() const
{
    return pcBreakpoints;
}

int CPUControlSection::getLineNumber() const
{
    return microprogramCounter;
}

int CPUControlSection::getCallDepth() const
{
    return callDepth;
}

const MicrocodeProgram *CPUControlSection::getProgram() const
{
    return program;
}

const MicroCode *CPUControlSection::getCurrentMicrocodeLine() const
{
    return program->getCodeLine(microprogramCounter);
}

QString CPUControlSection::getErrorMessage() const
{
    if(memory->hadErrorOnStep()) return memory->getErrorMessage();
    else if(data->hadErrorOnStep()) return data->getErrorMessage();
    else if(hadErrorOnStep()) return errorMessage;
    else return "";
}

bool CPUControlSection::stoppedForBreakpoint() const
{
    return program->getCodeLine(microprogramCounter)->hasBreakpoint() ||
            (microprogramCounter == 0)? pcBreakpoints.contains(memoizer->getRegisterStart(CPURegisters::PC)) : false;
}

Enu::DebugLevels CPUControlSection::getDebugLevel() const
{
    return memoizer->getDebugLevel();
}

bool CPUControlSection::hadErrorOnStep() const
{
    return hadControlError || data->hadErrorOnStep() || memory->hadErrorOnStep();
}

bool CPUControlSection::getExecutionFinished() const
{
    return executionFinished;
}

void CPUControlSection::onSimulationStarted()
{
    inDebug = false;
    inSimulation = true;
    executionFinished = false;
    memoizer->clear();
}

void CPUControlSection::onSimulationFinished()
{
    data->clearClockSignals();
    data->clearControlSignals();
    memory->onCancelWaiting();
    executionFinished = true;
    inDebug = false;
}

void CPUControlSection::onDebuggingStarted()
{
    onSimulationStarted();
    inDebug = true;
    microBreakpointHit = false;
    microBreakpointHandled = false;
    asmBreakpointHit = false;
    asmBreakpointHandled = false;
}

void CPUControlSection::onDebuggingFinished()
{
    onSimulationFinished();
    inDebug = false;
}

void CPUControlSection::onRemoveAllPCBreakpoints()
{
    pcBreakpoints.clear();
}

void CPUControlSection::onRemovePCBreakpoint(quint16 address)
{
    pcBreakpoints.remove(address);
}

void CPUControlSection::onAddPCBreakpoint(quint16 address)
{
    pcBreakpoints.insert(address);
}

void CPUControlSection::onStep() noexcept
{
    // Do step logic
    if(microprogramCounter == 0) {
        // Store PC at the start of the cycle, so that we know where the instruction started from.
        // Also store any other values needed for detailed statistics
        memoizer->storeStateInstrStart();
        memory->onInstructionStarted();
    }

    const MicroCode* prog = program->getCodeLine(microprogramCounter);

    // If running in debug mode, first check if this line has any microcode breakpoints.
    if(inDebug) {
        if(microprogramCounter == 0) {
            if(asmBreakpointHandled) {
                asmBreakpointHandled = false;
                asmBreakpointHit = false;
            }
            else if( pcBreakpoints.contains(data->getRegisterBankWord(CPURegisters::PC))) {
                asmBreakpointHandled = false;
                asmBreakpointHit = true;
                emit simulationHitASMBreakpoint();
                QApplication::processEvents();
                return;
            }
        }
                // If the line has a breakpoint, but onBreakPointHandled() has been called
        else if(microBreakpointHandled) {
            // Ignore the line's breakpoint, and reset breakpointHandled in case another breakpoint appears.
            microBreakpointHandled = false;
            microBreakpointHit = false;
        }
        else if(prog->hasBreakpoint()) {
            microBreakpointHandled = false;
            microBreakpointHit = true;
            emit simulationHitMicroBreakpoint();
            // Process Events immediately, and return to prevent step from executing
            QApplication::processEvents();
            return;
        }
        else {
            microBreakpointHit = false;
            asmBreakpointHit = false;
        }
    }
    this->setSignalsFromMicrocode(prog);
    data->setSignalsFromMicrocode(prog);
    data->onStep();
    branchHandler();
    microCycleCounter++;

    if(microprogramCounter==0 ||executionFinished) {
        memoizer->storeStateInstrEnd();
        updateAtInstructionEnd();
        emit simulationInstructionFinished();
        instructionCounter++;
    }
    // Nothing do do on an error
    else if(hadErrorOnStep()) return;
}

void CPUControlSection::onISAStep() noexcept
{
    // Execute steps until the microprogram counter comes back to 0 OR there is an error on step OR a breakpoint is hit.
    do {
        onStep();
        if(hadErrorOnStep()) {
            if(memory->hadErrorOnStep()) {
                qDebug() << "Memory section reporting an error";
                break;
            }
            else if(data->hadErrorOnStep()) {
                qDebug() << "Data section reporting an error";
                break;
            }
            else {
                qDebug() << "Control section reporting an error";
                break;
            }
        }
    } while(!executionFinished && microprogramCounter != 0 && !microBreakpointHit);
    if(executionFinished) emit simulationFinished();
}

void CPUControlSection::onClock() noexcept
{
    //Do clock logic
    if(!inSimulation) {
        data->onClock();
    }
    else {
        //One should not get here, otherwise that would mean that we clocked in a simulation
    }
}

void CPUControlSection::onRun() noexcept
{
    timer.start();
    // If debugging, there is the potential to hit breakpoints, so a different main loop is needed.
    // Partially, this is to handle breakpoints gracefully, and partially to prevent "run" mode from being slowed down by debug features.
    if(inDebug) {
        // Always execute at least once. onStep() will return before executing any code if the line has a breakpoint that hasn't been handled.
        do{
            // Since the sim runs at about 5Mhz, do not process events every single cycle to increase performance.
            if(microCycleCounter % 5000 == 0) {
                QApplication::processEvents();
            }
            onStep();
            //If there was an error on the control flow
            if(hadErrorOnStep()) {
                if(memory->hadErrorOnStep()) {
                    qDebug() << "Memory section reporting an error";
                    break;
                }
                else if(data->hadErrorOnStep()) {
                    qDebug() << "Data section reporting an error";
                    break;
                }
                else {
                    qDebug() << "Control section reporting an error";
                    break;
                }
            }
        // If a breakpoint has been hit, stop execution now.
        } while(!executionFinished && !microBreakpointHit &&!asmBreakpointHit);
    }
    else {
        while(!executionFinished) {
            // Since the sim runs at about 5Mhz, do not process events every single cycle to increase performance.
            if(microCycleCounter % 5000 == 0) {
                QApplication::processEvents();
            }
            onStep();
            //If there was an error on the control flow
            if(hadErrorOnStep()) {
                if(memory->hadErrorOnStep()) {
                    qDebug() << "Memory section reporting an error";
                    break;
                }
                else if(data->hadErrorOnStep()) {
                    qDebug() << "Data section reporting an error";
                    break;
                }
                else {
                    qDebug() << "Control section reporting an error";
                    break;
                }
            }
        }
    }

    // If a breakpoint was reached, return before final statistics are computed or the simulation is finished.
    if(microBreakpointHit || asmBreakpointHit) {
        return;
    }
    auto value = timer.elapsed();
    //qDebug().nospace().noquote() << memoizer->finalStatistics() << "\n";
    qDebug().nospace().noquote() <<"Executed "<<instructionCounter<<" instructions in "<<microCycleCounter<< " cycles.";
    qDebug().nospace().noquote() <<"Averaging "<<microCycleCounter / instructionCounter<<" cycles per instruction.";
    qDebug().nospace().noquote() <<"Execution time (ms): " << value;
    qDebug().nospace().noquote() <<"Cycles per second: "<< microCycleCounter / (((float)value/1000));
    qDebug().nospace().noquote() <<"Instructions per second: "<< instructionCounter / (((float)value/1000));
    emit simulationFinished();
}

void CPUControlSection::onClearCPU() noexcept
{
    // Reset all internal state
    data->onClearCPU();
    memory->clearErrors();
    memoizer->clear();
    inSimulation = false;
    inDebug = false;
    microprogramCounter = 0;
    microCycleCounter = 0;
    instructionCounter = 0;
    callDepth = 0;
    hadControlError = false;
    executionFinished = false;
    isPrefetchValid = false;
    errorMessage = "";
    microBreakpointHit = false;
    microBreakpointHandled = false;
    asmBreakpointHit = false;
    asmBreakpointHandled = false;
}

void CPUControlSection::onClearMemory() noexcept
{
    memory->onClearMemory();
}

void CPUControlSection::onMicroBreakpointHandled() noexcept
{
    microBreakpointHandled = true;
}

void CPUControlSection::onASMBreakpointHandled() noexcept
{
    asmBreakpointHandled = true;
}

void CPUControlSection::branchHandler()
{
    const MicroCode* prog = program->getCodeLine(microprogramCounter);
    int temp = microprogramCounter;
    quint8 byte = 0;
    QString tempString;
    const SymbolTable* symTable = this->program->getSymTable();
    QSharedPointer<SymbolEntry> val;
    switch(prog->getBranchFunction())
    {
    case Enu::Unconditional:
        temp = prog->getTrueTarget()->getValue();
        break;
    case Enu::uBRGT:
        if((!data->getStatusBit(Enu::STATUS_N) && !data->getStatusBit(Enu::STATUS_Z))) {
            temp = prog->getTrueTarget()->getValue();
        }
        else {
            temp = prog->getFalseTarget()->getValue();
        }
        break;
    case Enu::uBRGE:
        if((!data->getStatusBit(Enu::STATUS_N))) {
            temp = prog->getTrueTarget()->getValue();
        }
        else {
            temp = prog->getFalseTarget()->getValue();
        }
        break;
    case Enu::uBREQ:
        if(data->getStatusBit(Enu::STATUS_Z)) {
            temp = prog->getTrueTarget()->getValue();
        }
        else {
            temp = prog->getFalseTarget()->getValue();
        }
        break;
    case Enu::uBRLE:
        if(data->getStatusBit(Enu::STATUS_N) || data->getStatusBit(Enu::STATUS_Z)) {
            temp = prog->getTrueTarget()->getValue();
        }
        else {
            temp = prog->getFalseTarget()->getValue();
        }
        break;
    case Enu::uBRLT:
        if(data->getStatusBit(Enu::STATUS_N)) {
            temp = prog->getTrueTarget()->getValue();
        }
        else {
            temp = prog->getFalseTarget()->getValue();
        }
        break;
    case Enu::uBRNE:
        if((!data->getStatusBit(Enu::STATUS_Z))) {
            temp = prog->getTrueTarget()->getValue();
        }
        else {
            temp = prog->getFalseTarget()->getValue();
        }
        break;
    case Enu::uBRV:
        if(data->getStatusBit(Enu::STATUS_V)) {
            temp = prog->getTrueTarget()->getValue();
        }
        else {
            temp = prog->getFalseTarget()->getValue();
        }
        break;
    case Enu::uBRC:
        if(data->getStatusBit(Enu::STATUS_C))  {
            temp = prog->getTrueTarget()->getValue();
        }
        else {
            temp = prog->getFalseTarget()->getValue();
        }
        break;
    case Enu::uBRS:
        if(data->getStatusBit(Enu::STATUS_S)) {
            temp = prog->getTrueTarget()->getValue();
        }
        else {
            temp = prog->getFalseTarget()->getValue();
        }
        break;
    case Enu::IsPrefetchValid:
        if(isPrefetchValid) {
            temp = prog->getTrueTarget()->getValue();
        }
        else {
            temp = prog->getFalseTarget()->getValue();
        }
        break;
    case Enu::IsUnary:
        byte = data->getRegisterBankByte(8);
        // At the hardware level, all traps are unary.
        // If it is a non-unary trap at the ASM level, loading the argument is part of the microcode trap handlers responsibility.
        if(Pep::isUnaryMap[Pep::decodeMnemonic[byte]] || Pep::isTrapMap[Pep::decodeMnemonic[byte]]) {
            temp = prog->getTrueTarget()->getValue();
        }
        else {
            temp = prog->getFalseTarget()->getValue();
        }
        break;
    case Enu::IsPCEven:
        if(data->getRegisterBankByte(7)%2 == 0) {
            temp = prog->getTrueTarget()->getValue();
        }
        else {
            temp = prog->getFalseTarget()->getValue();
        }
        break;
    case Enu::AddressingModeDecoder:
        temp = data->getRegisterBankByte(8);
        tempString = Pep::intToAddrMode(Pep::decodeAddrMode[temp]).toLower()+"Addr";
        val = symTable->getValue(tempString);
        if(val == nullptr || !val->isDefined()) {
            executionFinished = true;
            hadControlError = true;
            if(val == nullptr) errorMessage = "ERROR: AMD jumped to undefined inst - " + tempString;
            else errorMessage = "ERROR: AMD jumped to multiply defined instr - " + tempString;
            break;
        }
        temp = val->getValue();

        break;
    case Enu::InstructionSpecifierDecoder:
        temp = data->getRegisterBankByte(8);
        tempString = Pep::enumToMnemonMap[Pep::decodeMnemonic[temp]].toLower();
        val = symTable->getValue(tempString);
        if(val == nullptr || !val->isDefined()) {
            executionFinished = true;
            hadControlError = true;
            if(val == nullptr) errorMessage = "ERROR: ISD jumped to undefined inst - " + tempString;
            else errorMessage = "ERROR: ISD jumped to multiply defined instr - " + tempString;
            break;
        }
        temp = val->getValue();
        break;
    case Enu::Stop:
        executionFinished = true;
        break;
    default:
        executionFinished = true;
        hadControlError = true;
        errorMessage = "ERROR: µBranch Handler attempted to process invalid µFunction.";
        break;

    }
    if(hadControlError) {
        //If there was an error in the control section, make sure the CPU stops
        executionFinished = true;
    }
    else if(temp == microprogramCounter && prog->getBranchFunction() != Enu::Stop) {
        executionFinished  = true;
        hadControlError = true;
        errorMessage = "ERROR: µInstructions cannot branch to themselves";
    }
    else {
        microprogramCounter = temp;
    }
}

void CPUControlSection::setSignalsFromMicrocode(const MicroCode *line)
{
    int val;
    if(line->getClockSignal(Enu::EClockSignals::PValidCk)) {
        val = line->getControlSignal(Enu::EControlSignals::PValid);
        if(val == Enu::signalDisabled) {
            errorMessage = "Error: Asserted PValidCk, but PValid was disabled.";
            hadControlError = true;
        }
        else {
            isPrefetchValid = val;
        }
    }
}

void CPUControlSection::updateAtInstructionEnd()
{
    // Handle changing of call stack depth if the executed instruction affects the call stack.
    if(Pep::decodeMnemonic[data->getRegisterBankByte(CPURegisters::IS)] == Enu::EMnemonic::CALL){
        callDepth++;
    }
    else if(Pep::isTrapMap[Pep::decodeMnemonic[data->getRegisterBankByte(CPURegisters::IS)]]){
        callDepth++;
    }
    else if(Pep::decodeMnemonic[data->getRegisterBankByte(CPURegisters::IS)] == Enu::EMnemonic::RET){
        callDepth--;
    }
    else if(Pep::decodeMnemonic[data->getRegisterBankByte(CPURegisters::IS)] == Enu::EMnemonic::RETTR){
        callDepth--;
    }
}
