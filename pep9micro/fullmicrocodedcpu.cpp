#include "fullmicrocodedcpu.h"

#include <QApplication>
#include <QTimer>

#include "amemorydevice.h"
#include "asmprogrammanager.h"
#include "microcode.h"
#include "microcodeprogram.h"
#include "pep.h"
#include "newcpudata.h"
#include "symbolentry.h"
#include "fullmicrocodedmemoizer.h"
#include "registerfile.h"
FullMicrocodedCPU::FullMicrocodedCPU(const AsmProgramManager* manager, QSharedPointer<AMemoryDevice> memoryDev, QObject* parent) noexcept: ACPUModel (memoryDev, parent),
    InterfaceMCCPU(Enu::CPUType::TwoByteDataBus),
    InterfaceISACPU(memoryDev.get(), manager)
{
    memoizer = new FullMicrocodedMemoizer(*this);
    data = new NewCPUDataSection(Enu::CPUType::TwoByteDataBus, memoryDev, parent);
    dataShared = QSharedPointer<NewCPUDataSection>(data);
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

bool FullMicrocodedCPU::atMicroprogramStart() const noexcept
{
    bool rVal = microprogramCounter == startLine || microprogramCounter == 0;
    return rVal;
}

bool FullMicrocodedCPU::getStatusBitCurrent(Enu::EStatusBit bit) const
{
    return data->getRegisterBank().readStatusBitCurrent(bit);
}

bool FullMicrocodedCPU::getStatusBitStart(Enu::EStatusBit bit) const
{
    return data->getRegisterBank().readStatusBitStart(bit);
}

quint8 FullMicrocodedCPU::getCPURegByteCurrent(Enu::CPURegisters reg) const
{
    return data->getRegisterBank().readRegisterByteCurrent(reg);
}

quint16 FullMicrocodedCPU::getCPURegWordCurrent(Enu::CPURegisters reg) const
{
    return data->getRegisterBank().readRegisterWordCurrent(reg);
}

quint8 FullMicrocodedCPU::getCPURegByteStart(Enu::CPURegisters reg) const
{
    return data->getRegisterBank().readRegisterByteStart(reg);
}

quint16 FullMicrocodedCPU::getCPURegWordStart(Enu::CPURegisters reg) const
{
    return data->getRegisterBank().readRegisterWordStart(reg);
}

void FullMicrocodedCPU::initCPU()
{
    // Initialize CPU with proper stack pointer value in SP register.

    // If there is no operating system, then the stack pointer should default to 0.
    if(manager->getOperatingSystem().isNull()) {
        data->onSetRegisterByte(4, 0);
        data->onSetRegisterByte(5, 0);
    }
    else {
        // Go to the memory vector
        quint16 addr = manager->getMemoryVectorValue(AsmProgramManager::UserStack);
        data->onSetRegisterByte(4, addr >> 8);
        data->onSetRegisterByte(5, addr &0xff);
    }
    data->getRegisterBank();

    data->getRegisterBank().flattenFile();
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

void FullMicrocodedCPU::setCPUType(Enu::CPUType)
{
    throw std::logic_error("Can't change CPU type on fullmicrococdedcpu, it must always be two byte bus");
}

void FullMicrocodedCPU::onSimulationStarted()
{
    inDebug = false;
    inSimulation = false;
    executionFinished = false;
#pragma message("TODO: Make start symbol variable")
    if(sharedProgram->getSymTable()->exists("start")) {
        startLine = sharedProgram->getSymTable()->getValue("start")->getValue();
    } else {
        startLine = 0;
    }
    memoizer->clear();
    calculateInstrJT();
    calculateAddrJT();
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
            else if(microprogramCounter == startLine) {
                // Clear at start, so as to preserve highlighting AFTER finshing a write.
                // When running in debug, clear written bytes after every instruction,
                // otherwise too many writes may queue up.
                memory->clearBytesWritten();
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
            else if(microprogramCounter == startLine) {
                // Clear at start, so as to preserve highlighting AFTER finshing a write.
                // When running in debug, clear written bytes after every instruction,
                // otherwise too many writes may queue up.
                memory->clearBytesWritten();
            }
            onMCStep();
        }
    }

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
    if(microBreakpointHit || asmBreakpointHit) {
        return false;
    }

    auto value = timer.elapsed();
    //qDebug().nospace().noquote() << memoizer->finalStatistics() << "\n";
    qDebug().nospace().noquote() << "Executed "<< asmInstructionCounter << " instructions in "<<microCycleCounter<< " cycles.";
    qDebug().nospace().noquote() << "Averaging " << microCycleCounter / asmInstructionCounter << " cycles per instruction.";
    qDebug().nospace().noquote() << "Execution time (ms): " << value;
    qDebug().nospace().noquote() << "Cycles per second: " << microCycleCounter / (((float)value/1000));
    qDebug().nospace().noquote() << "Instructions per second: " << asmInstructionCounter / (((float)value/1000));
    //emit simulationFinished();
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

    if(microprogramCounter == startLine) {
        // Store PC at the start of the cycle, so that we know where the instruction started from.
        // Also store any other values needed for detailed statistics
        memoizer->storeStateInstrStart();
        memory->onCycleStarted();
        InterfaceISACPU::calculateStackChangeStart(this->getCPURegByteStart(Enu::CPURegisters::IS));
    }

    // Do step logic

    const MicroCode* prog = sharedProgram->getCodeLine(microprogramCounter);

    this->setSignalsFromMicrocode(prog);
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

    if(microprogramCounter == startLine || executionFinished) {
        quint16 progCounter = getCPURegWordStart(Enu::CPURegisters::PC);
        InterfaceISACPU::calculateStackChangeEnd(this->getCPURegByteCurrent(Enu::CPURegisters::IS),
                                                 this->getCPURegWordCurrent(Enu::CPURegisters::OS),
                                                 this->getCPURegWordStart(Enu::CPURegisters::SP),
                                                 this->getCPURegWordStart(Enu::CPURegisters::PC),
                                                 this->getCPURegWordCurrent(Enu::CPURegisters::A));
        memoizer->storeStateInstrEnd();
        updateAtInstructionEnd();
        emit asmInstructionFinished();
        asmInstructionCounter++;
        // qDebug().noquote().nospace() << memoizer->memoize();
        data->getRegisterBank().flattenFile();
        // If execution finished on this instruction, then restore original starting program counter,
        // as the instruction at the current program counter will not be executed.
        if(executionFinished) {
            data->getRegisterBank().writePCStart(progCounter);
            emit simulationFinished();
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
    } while(!hadErrorOnStep() && !executionFinished && microprogramCounter != startLine && !microBreakpointHit);
    if(hadErrorOnStep()) {
        if(memory->hadError()) {
            qDebug() << "Memory section reporting an error";
        }
        else if(data->hadErrorOnStep()) {
            qDebug() << "Data section reporting an error";
        }
        else {
            qDebug() << "Control section reporting an error";
        }
    }
    // If there was an error, execution is finished.
    // if(executionFinished) emit simulationFinished();
}

void FullMicrocodedCPU::stepOver()
{
    // Clear at start, so as to preserve highlighting AFTER finshing a write.
    memory->clearBytesWritten();
    int localCallDepth = getCallDepth();
    do{
        onISAStep();
    } while(localCallDepth < getCallDepth()
            && !getExecutionFinished()
            && !stoppedForBreakpoint()
            && !hadErrorOnStep());
}

bool FullMicrocodedCPU::canStepInto() const
{
    quint8 byte;
    memory->getByte(getCPURegWordStart(Enu::CPURegisters::PC), byte);
    Enu::EMnemonic mnemon = Pep::decodeMnemonic[byte];
    return (mnemon == Enu::EMnemonic::CALL) || Pep::isTrapMap[mnemon];
}

void FullMicrocodedCPU::stepInto()
{
    // Clear at start, so as to preserve highlighting AFTER finshing a write.
    memory->clearBytesWritten();
    onISAStep();
}

void FullMicrocodedCPU::stepOut()
{
    // Clear at start, so as to preserve highlighting AFTER finshing a write.
    memory->clearBytesWritten();
    int localCallDepth = getCallDepth();
    do{
        onISAStep();
    } while(localCallDepth <= getCallDepth()
            && !getExecutionFinished()
            && !stoppedForBreakpoint()
            && !hadErrorOnStep());
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
        // If the value in the instruction specifier decoder table is invalid,
        // report the unrecoverable error.
        if(!addrModeJT[data->getRegisterBankByte(8)].isValid) {
            executionFinished = true;
            controlError = true;
            // Get the enumerated & string values of current instruction.
            Enu::EAddrMode addrMode = Pep::decodeAddrMode[data->getRegisterBankByte(8)];
            tempString = Pep::enumToMicrocodeAddrSymbol[addrMode];
            // Attempt to lookup the symbol associated with the instruction
            QSharedPointer<const SymbolTable> symTable = this->sharedProgram->getSymTable();
            val = symTable->getValue(tempString);
            // If the symbol table returns a nullptr, the symbol is undefined.
            if(val == nullptr) errorMessage = "ERROR: AMD jumped to undefined addressing mode - " + tempString;
            else errorMessage = "ERROR: AMD jumped to multiply defined addressing mode - " + tempString;
        }
        else{
            // Otherwise branch to the appropriate address in the instruction specifer jump table.
           temp = addrModeJT[data->getRegisterBankByte(8)].addr;
        }

        break;
    case Enu::InstructionSpecifierDecoder:
        // If the value in the instruction specifier decoder table is invalid,
        // report the unrecoverable error.
        if(!instrSpecJT[data->getRegisterBankByte(8)].isValid) {
            executionFinished = true;
            controlError = true;
            // Get the enumerated & string values of current instruction.
            Enu::EMnemonic mnemon = Pep::decodeMnemonic[data->getRegisterBankByte(8)];
            tempString = Pep::enumToMicrocodeInstrSymbol[mnemon];
            // Attempt to lookup the symbol associated with the instruction
            QSharedPointer<const SymbolTable> symTable = this->sharedProgram->getSymTable();
            val = symTable->getValue(tempString);
            // If the symbol table returns a nullptr, the symbol is undefined.
            if(val == nullptr) errorMessage = "ERROR: ISD jumped to undefined inst - " + tempString;
            else errorMessage = "ERROR: ISD jumped to multiply defined instr - " + tempString;
        }
        else{
            // Otherwise branch to the appropriate address in the instruction specifer jump table.
           temp = instrSpecJT[data->getRegisterBankByte(8)].addr;
        }

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

void FullMicrocodedCPU::calculateInstrJT()
{
    // Symbol table of the current microprogram
    QSharedPointer<const SymbolTable> symTable = this->sharedProgram->getSymTable();
    QSharedPointer<SymbolEntry> val;
    FullMicrocodedCPU::decoder_entry entry;
    for(int it = 0; it <= 255; ++it) {
        QString tempString = Pep::enumToMicrocodeInstrSymbol[Pep::decodeMnemonic[it]].toLower();
        val = symTable->getValue(tempString);
        // Instead of causing an error before execution starts,
        // flag the entry as invalid so that the error can be caught at runtime.
        // This allows microprogram fragments that do not define all instructions
        // to be created, which is of instructional value to students
        if(val == nullptr || !val->isDefined()) {
            entry.isValid = false;
        }
        else {
            entry.isValid = true;
            entry.addr = val->getValue();
        }
        instrSpecJT[it] = entry;
    }
}

void FullMicrocodedCPU::calculateAddrJT()
{
    // Symbol table of the current microprogram
    QSharedPointer<const SymbolTable> symTable = this->sharedProgram->getSymTable();
    QSharedPointer<SymbolEntry> val;
    FullMicrocodedCPU::decoder_entry entry;
    for(int it = 0; it <= 255; ++it) {
        QString tempString = Pep::enumToMicrocodeAddrSymbol[Pep::decodeAddrMode[it]];
        val = symTable->getValue(tempString);
        // Instead of causing an error before execution starts,
        // flag the entry as invalid so that the error can be caught at runtime.
        // This allows microprogram fragments that do not define all instructions
        // to be created, which is of instructional value to students
        if(val == nullptr || !val->isDefined()) {
            entry.isValid = false;
        }
        else {
            entry.isValid = true;
            entry.addr = val->getValue();
        }
        addrModeJT[it] = entry;
    }
}

void FullMicrocodedCPU::breakpointHandler()
{
    // If the CPU is not being debugged, breakpoints make no sense. Abort.
    if(!inDebug) return;
    // Only trap assembly breakpoints once on the first line of microcode.
    if((microprogramCounter == startLine) && breakpointsISA.contains(data->getRegisterBankWord(Enu::CPURegisters::PC))) {
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
