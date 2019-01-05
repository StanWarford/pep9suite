#include "fullmicrocodedcpu.h"

#include <QApplication>
#include <QTimer>

#include "amemorydevice.h"
#include "microcode.h"
#include "microcodeprogram.h"
#include "pep.h"
#include "newcpudata.h"
#include "symbolentry.h"
#include "fullmicrocodedmemoizer.h"
FullMicrocodedCPU::FullMicrocodedCPU(QSharedPointer<AMemoryDevice> memoryDev, QObject* parent) noexcept: ACPUModel (memoryDev, parent),
    InterfaceMCCPU(Enu::CPUType::TwoByteDataBus),
    InterfaceISACPU()
{
    memoizer = new FullMicrocodedMemoizer(*this);
    data = new NewCPUDataSection(Enu::CPUType::TwoByteDataBus, memoryDev, parent);
    dataShared = QSharedPointer<NewCPUDataSection>(data);
    setDebugLevel(Enu::DebugLevels::ALL);
}

FullMicrocodedCPU::~FullMicrocodedCPU()
{
    //This object should last the lifetime of the  program, it does not need to be cleaned up
    delete memoizer;
}

QSharedPointer<NewCPUDataSection> FullMicrocodedCPU::getDataSection()
{
    return dataShared;
}

bool FullMicrocodedCPU::getStatusBitCurrent(Enu::EStatusBit bit) const
{
#pragma message ("TODO: Handle case where mupc is !0 at the start of the cycle")
    return data->getStatusBit(bit);
}

bool FullMicrocodedCPU::getStatusBitStart(Enu::EStatusBit bit) const
{
#pragma message ("TODO: Handle case where mupc is !0 at the start of the cycle")
    if (microprogramCounter == 0) {
        return getStatusBitCurrent(bit);
    }
    else return memoizer->getStatusBitStart(bit);
}

quint8 FullMicrocodedCPU::getCPURegByteCurrent(Enu::CPURegisters reg) const
{
    return data->getRegisterBankByte(reg);
}

quint16 FullMicrocodedCPU::getCPURegWordCurrent(Enu::CPURegisters reg) const
{
    return data->getRegisterBankWord(reg);
}

quint8 FullMicrocodedCPU::getCPURegByteStart(Enu::CPURegisters reg) const
{
#pragma message ("TODO: Handle case where mupc is !0 at the start of the cycle")
    if (microprogramCounter == 0) {
        return getCPURegByteCurrent(reg);
    }
    else return memoizer->getRegisterByteStart(reg);
}

quint16 FullMicrocodedCPU::getCPURegWordStart(Enu::CPURegisters reg) const
{
#pragma message ("TODO: Handle case where mupc is !0 at the start of the cycle")
    if (microprogramCounter == 0) {
        return getCPURegWordCurrent(reg);
    }
    else return memoizer->getRegisterWordStart(reg);
}

void FullMicrocodedCPU::initCPU()
{
    // Initialize CPU with proper stack pointer value in SP register.
#pragma message ("TODO: Init cpu with proper SP when not burned in at 0xffff")
    data->onSetRegisterByte(4,0xFB);
    data->onSetRegisterByte(5,0x82);
}

bool FullMicrocodedCPU::stoppedForBreakpoint() const noexcept
{
    return microBreakpointHit || asmBreakpointHit;
}

QString FullMicrocodedCPU::getErrorMessage() const noexcept
{
    if(memory->hadError()) return memory->getErrorMessage();
    else if(data->hadErrorOnStep()) return data->getErrorMessage();
    else if(hadErrorOnStep()) return errorMessage;
    else return "";
}

bool FullMicrocodedCPU::hadErrorOnStep() const noexcept
{
    return controlError || data->hadErrorOnStep() || memory->hadError();
}

Enu::DebugLevels FullMicrocodedCPU::getDebugLevel() const noexcept
{
    return memoizer->getDebugLevel();
}

void FullMicrocodedCPU::setDebugLevel(Enu::DebugLevels level)
{
    memoizer->setDebugLevel(level);
}

void FullMicrocodedCPU::setCPUType(Enu::CPUType)
{
    throw std::logic_error("Can't change CPU type on fullmicrococdedcpu, it must always be two byte bus");
}

void FullMicrocodedCPU::onSimulationStarted()
{
    inDebug = false;
    inSimulation = false;
    executionFinished = false;
    memoizer->clear();
}

void FullMicrocodedCPU::onSimulationFinished()
{
    data->clearClockSignals();
    data->clearControlSignals();
    executionFinished = true;
    inDebug = false;
#pragma message("TODO: Inform memory that execution is finished")
}

void FullMicrocodedCPU::onDebuggingStarted()
{
    onSimulationStarted();
    inDebug = true;
    microBreakpointHit = false;
    asmBreakpointHit = false;
}

void FullMicrocodedCPU::onDebuggingFinished()
{
    onSimulationFinished();
    inDebug = false;
}

void FullMicrocodedCPU::onCancelExecution()
{
    #pragma message("TODO: Cancel execution")
    throw -1;
}

bool FullMicrocodedCPU::onRun()
{
    timer.start();
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
        } while(!hadErrorOnStep() && !executionFinished && !(microBreakpointHit || asmBreakpointHit));
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
            emit simulationFinished();
        }
        else if(data->hadErrorOnStep()) {
            qDebug() << "Data section reporting an error";
            return false;
            emit simulationFinished();
        }
        else {
            qDebug() << "Control section reporting an error";
            return false;
            emit simulationFinished();
        }
    }

    // If a breakpoint was reached, return before final statistics are computed or the simulation is finished.
    if(microBreakpointHit || asmBreakpointHit) {
        return false;
    }

    auto value = timer.elapsed();
    qDebug().nospace().noquote() << memoizer->finalStatistics() << "\n";
    qDebug().nospace().noquote() << "New Model";
    qDebug().nospace().noquote() << "Executed "<< asmInstructionCounter << " instructions in "<<microCycleCounter<< " cycles.";
    qDebug().nospace().noquote() << "Averaging " << microCycleCounter / asmInstructionCounter << " cycles per instruction.";
    qDebug().nospace().noquote() << "Execution time (ms): " << value;
    qDebug().nospace().noquote() << "Cycles per second: " << microCycleCounter / (((float)value/1000));
    qDebug().nospace().noquote() << "Instructions per second: " << asmInstructionCounter / (((float)value/1000));
    emit simulationFinished();
    return true;
}

void FullMicrocodedCPU::onResetCPU()
{
    // Reset all internal state, but keep loaded micropgoram & breakpoints
    data->onClearCPU();
    ACPUModel::memory->clearErrors();
    memoizer->clear();
    InterfaceMCCPU::reset();
    InterfaceISACPU::reset();
    inSimulation = false;
    inDebug = false;
    callDepth = 0;
    controlError = false;
    executionFinished = false;
    isPrefetchValid = false;
    errorMessage = "";
    microBreakpointHit = false;
    asmBreakpointHit = false;
}

void FullMicrocodedCPU::onMCStep()
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

    this->setSignalsFromMicrocode(prog);
    data->setSignalsFromMicrocode(prog);
    data->onStep();
    branchHandler();
    microCycleCounter++;
    //qDebug().nospace().noquote() << prog->getSourceCode();

    if(microprogramCounter == 0 || executionFinished) {
        memoizer->storeStateInstrEnd();
        updateAtInstructionEnd();
        emit asmInstructionFinished();
        asmInstructionCounter++;
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

void FullMicrocodedCPU::onClock()
{
    //Do clock logic
    if(!inSimulation) {
        data->onClock();
    }
    else {
        //One should not get here, otherwise that would mean that we clocked in a simulation
    }
}

void FullMicrocodedCPU::onISAStep()
{
    // Execute steps until the microprogram counter comes back to 0 OR there is an error on step OR a breakpoint is hit.
    do {
        onMCStep();
    } while(!hadErrorOnStep() && !executionFinished && microprogramCounter != 0 && !microBreakpointHit);
    if(hadErrorOnStep()) {
        if(memory->hadError()) {
            qDebug() << "Memory section reporting an error";
            emit simulationFinished();
        }
        else if(data->hadErrorOnStep()) {
            qDebug() << "Data section reporting an error";
            emit simulationFinished();
        }
        else {
            qDebug() << "Control section reporting an error";
            emit simulationFinished();
        }
    }
    if(executionFinished) emit simulationFinished();
}

void FullMicrocodedCPU::branchHandler()
{
    const MicroCode* prog = sharedProgram->getCodeLine(microprogramCounter);
    int temp = microprogramCounter;
    quint8 byte = 0;
    QString tempString;
    QSharedPointer<const SymbolTable> symTable = this->sharedProgram->getSymTable();
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
            controlError = true;
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
            controlError = true;
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
        controlError = true;
        errorMessage = "ERROR: µBranch Handler attempted to process invalid µFunction.";
        break;

    }
    if(controlError) {
        //If there was an error in the control section, make sure the CPU stops
        executionFinished = true;
    }
    else if(temp == microprogramCounter && prog->getBranchFunction() != Enu::Stop) {
        executionFinished  = true;
        controlError = true;
        errorMessage = "ERROR: µInstructions cannot branch to themselves";
    }
    else {
        microprogramCounter = temp;
    }
}

void FullMicrocodedCPU::updateAtInstructionEnd()
{
    // Handle changing of call stack depth if the executed instruction affects the call stack.
    if(Pep::decodeMnemonic[data->getRegisterBankByte(Enu::CPURegisters::IS)] == Enu::EMnemonic::CALL){
        callDepth++;
    }
    else if(Pep::isTrapMap[Pep::decodeMnemonic[data->getRegisterBankByte(Enu::CPURegisters::IS)]]){
        callDepth++;
    }
    else if(Pep::decodeMnemonic[data->getRegisterBankByte(Enu::CPURegisters::IS)] == Enu::EMnemonic::RET){
        callDepth--;
    }
    else if(Pep::decodeMnemonic[data->getRegisterBankByte(Enu::CPURegisters::IS)] == Enu::EMnemonic::RETTR){
        callDepth--;
    }
}

void FullMicrocodedCPU::breakpointHandler()
{
    // If the CPU is not being debugged, breakpoints make no sense. Abort.
    if(!inDebug) return;
    // Only trap assembly breakpoints once on the first line of microcode.
    if((microprogramCounter == 0) && breakpointsISA.contains(data->getRegisterBankWord(Enu::CPURegisters::PC))) {
        #pragma message("TODO: make micro program counter loop point variable")
        asmBreakpointHit = true;
        emit hitBreakpoint(Enu::BreakpointTypes::ASSEMBLER);
        return;
    }
    // Trap on micrcode breakpoints
    else if(sharedProgram->getCodeLine(microprogramCounter)->hasBreakpoint()) {
        microBreakpointHit = true;
        emit hitBreakpoint(Enu::BreakpointTypes::MICROCODE);
        return;
    }
    else {
        microBreakpointHit = false;
        asmBreakpointHit = false;
    }
}

void FullMicrocodedCPU::setSignalsFromMicrocode(const MicroCode *line)
{
    int val;
    if(line->getClockSignal(Enu::EClockSignals::PValidCk)) {
        val = line->getControlSignal(Enu::EControlSignals::PValid);
        if(val == Enu::signalDisabled) {
            errorMessage = "Error: Asserted PValidCk, but PValid was disabled.";
            controlError = true;
        }
        else {
            isPrefetchValid = val;
        }
    }
}
