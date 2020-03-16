// File: fullmicrocodedcpu.cpp
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
#include "fullmicrocodedcpu.h"

#include <QApplication>
#include <QTimer>

#include "amemorydevice.h"
#include "asmprogrammanager.h"
#include "cpudata.h"
#include "fullmicrocodedmemoizer.h"
#include "interrupthandler.h"
#include "microcode.h"
#include "microcodeprogram.h"
#include "pep.h"
#include "registerfile.h"
#include "symbolentry.h"
FullMicrocodedCPU::FullMicrocodedCPU(const AsmProgramManager* manager, QSharedPointer<AMemoryDevice> memoryDev, QObject* parent) noexcept: ACPUModel (memoryDev, parent),
    InterfaceMCCPU(Enu::CPUType::TwoByteDataBus),
    InterfaceISACPU(memoryDev.get(), manager), memoizer(new FullMicrocodedMemoizer(*this))
{
    data = new CPUDataSection(Enu::CPUType::TwoByteDataBus, memoryDev, parent);
    dataShared = QSharedPointer<CPUDataSection>(data);
    // Create & register callbacks for breakpoint interrupts.
    std::function<void(void)> mcHandler = [this](){this->breakpointMicroHandler();};
    std::function<void(void)> asmHandler = [this](){this->breakpointAsmHandler();};
    ACPUModel::handler->registerHandler(Interrupts::BREAKPOINT_MICRO, mcHandler);
    ACPUModel::handler->registerHandler(Interrupts::BREAKPOINT_ASM, asmHandler);

}

FullMicrocodedCPU::~FullMicrocodedCPU()
{
    //This object should last the lifetime of the  program, it does not need to be cleaned up.
    delete memoizer;
    // No need to delete data, as it will be cleaned up by dataShared.
}

QSharedPointer<CPUDataSection> FullMicrocodedCPU::getDataSection()
{
    return dataShared;
}

bool FullMicrocodedCPU::atMicroprogramStart() const noexcept
{
    bool rVal = microprogramCounter == startLine || microprogramCounter == 0;
    return rVal;
}

void FullMicrocodedCPU::setMicroPCToStart() noexcept
{
    microprogramCounter = startLine;
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
    microBreakpointHit = false;
    asmBreakpointHit = false;
    if(sharedProgram->getSymTable()->exists(Pep::defaultStartSymbol)) {
        startLine = static_cast<quint16>(sharedProgram->getSymTable()->getValue(Pep::defaultStartSymbol)->getValue());
    } else {
        qDebug() << "Default start symbol did not exists: " << Pep::defaultStartSymbol;
        startLine = 0;
    }
    memoizer->clear();
    calculateInstrJT();
    calculateAddrJT();
    ACPUModel::handler->clearQueuedInterrupts();
}

void FullMicrocodedCPU::onSimulationFinished()
{
    data->clearClockSignals();
    data->clearControlSignals();
    executionFinished = true;
    inDebug = false;
    ACPUModel::handler->clearQueuedInterrupts();
#pragma message("TODO: Inform memory that execution is finished")
}

void FullMicrocodedCPU::enableDebugging()
{
    inDebug = true;
}

void FullMicrocodedCPU::forceBreakpoint(Enu::BreakpointTypes breakpoint)
{
    switch(breakpoint){
    case Enu::BreakpointTypes::ASSEMBLER:
        ACPUModel::handler->interupt(Interrupts::BREAKPOINT_ASM);
        break;
    case Enu::BreakpointTypes::MICROCODE:
        ACPUModel::handler->interupt(Interrupts::BREAKPOINT_MICRO);
        break;
    }
}

void FullMicrocodedCPU::onCancelExecution()
{
    executionFinished = true;
    inDebug = false;
}

bool FullMicrocodedCPU::onRun()
{
    timer.start();

    std::function<bool(void)> cond = [this] () {
        bool rVal = !(hadErrorOnStep() || executionFinished || (inDebug && (microBreakpointHit || asmBreakpointHit)));
        // If execution would be finished this cycle, don't clear the last written bytes
        // so that the bytes modified by the last instruction are displayed.
        if(rVal && microprogramCounter == startLine) {
            // Clear at start, so as to preserve highlighting AFTER finshing a write.
            // When running in debug, clear written bytes after every instruction,
            // otherwise too many writes may queue up.
            memory->clearAllByteCaches();
        }
        return rVal;
    };
    // Execute multiple steps until the condition function is false.
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
    if(microBreakpointHit || asmBreakpointHit) {
        return false;
    }

    auto value = timer.elapsed();
    // qDebug().nospace().noquote() << memoizer->finalStatistics() << "\n";
    qDebug().nospace().noquote() << "Executed "<< asmInstructionCounter << " instructions in "<<microCycleCounter<< " cycles.";
    qDebug().nospace().noquote() << "Averaging " << microCycleCounter / asmInstructionCounter << " cycles per instruction.";
    qDebug().nospace().noquote() << "Execution time (ms): " << value;
    qDebug().nospace().noquote() << "Cycles per second: " << microCycleCounter / (static_cast<double>(value)/1000);
    qDebug().nospace().noquote() << "Instructions per second: " << asmInstructionCounter / ((static_cast<double>(value)/1000)) << "\n";
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
    ACPUModel::handler->clearQueuedInterrupts();
}

void FullMicrocodedCPU::onMCStep()
{
    microBreakpointHit = false;
    asmBreakpointHit = false;

    if(microprogramCounter == startLine) {
        // Store PC at the start of the cycle, so that we know where the instruction started from.
        // Also store any other values needed for detailed statistics.
        // Also, must initialize InterfaceISACPU:opValCache here for FullMicrocoded CPU
        // to fulfill its contract with InterfaceISACPU.
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

    // Step inside the data section, then hnalde updating microprogram counter.
    data->onStep();
    branchHandler();
    microCycleCounter++;

    // If we just finished an entire ISA level instruction, perform additional
    // simulation logic needed to mantain ISA level state.
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
        if(executionFinished || hadErrorOnStep()) {
            data->getRegisterBank().writePCStart(progCounter);
            emit simulationFinished();
        }

    }

    // Modulus must be greater than 1, or there will be no gaurentee of forward progress.
    // If modulus were 1, then debug debug breakpoints that were signaled externally
    // during process events would never be cleared by branch handler.
    if(microCycleCounter % 5000 == 0) {
        QApplication::processEvents();
        if(inDebug && (microBreakpointHit || asmBreakpointHit)) {
            // If a breakpoint was forced on us by the processEvents(), react to it now.
            // Clear breakpoint flags, otherwise we might get stuck
            // reacting to this breakpoint forever.
            return;
        }
    }

    // Upon entering an instruction that is going to trap
    // If running in debug mode, first check if this line has any microcode breakpoints.
    if(inDebug) {
        // Only trap assembly breakpoints once on the first line of microcode.
        if((microprogramCounter == startLine) && breakpointsISA.contains(data->getRegisterBankWord(Enu::CPURegisters::PC))) {
            ACPUModel::handler->interupt(Interrupts::BREAKPOINT_ASM);
        }
        // Trap on micrcode breakpoints
        else if(sharedProgram->getCodeLine(microprogramCounter)->hasBreakpoint()) {
            ACPUModel::handler->interupt(Interrupts::BREAKPOINT_MICRO);
        }
    }

    ACPUModel::handler->handleQueuedInterrupts();
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
    // Execute steps until the microprogram counter comes back to start
    // OR there is an error on step OR a breakpoint is hit OR the program is
    // otherwise terminated.
    std::function<bool(void)> func = [this](){return !hadErrorOnStep()
                && !executionFinished
                && !microBreakpointHit
                && microprogramCounter != startLine;};

    // If microprogram counter is 0, and the start of the von-neuman cycle
    // is not 0, execute microcode until "start" is hit. This is to prevent
    // the user from needing to hit "step" twice to execute the first ISA
    // level instruction.
    if(microprogramCounter < startLine && startLine != 0) {
        doMCStepWhile(func);
    }

    // If prelude hit an error OR a breakpoint OR stopped for some other reason,
    // do not continue executing.
    if(!hadErrorOnStep() && !executionFinished && !microBreakpointHit) {
        doMCStepWhile(func);
    }

}

void FullMicrocodedCPU::stepOver()
{

    // Clear at start, so as to preserve highlighting AFTER finshing a write.
    memory->clearAllByteCaches();

    int localCallDepth = getCallDepth();
    // Execute instructions until there is an error, or one is at the same depth of the call stack as prior to execution.
    std::function<bool(void)> cond = [this, &localCallDepth](){return localCallDepth < getCallDepth()
            && !getExecutionFinished()
            && !stoppedForBreakpoint()
            && !hadErrorOnStep();};
    doISAStepWhile(cond);
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
    memory->clearAllByteCaches();

    // Step into is jus texecuting a single step, as a single step would enter the trap / call.
    onISAStep();
}

void FullMicrocodedCPU::stepOut()
{
    // Clear at start, so as to preserve highlighting AFTER finshing a write.
    memory->clearAllByteCaches();

    int localCallDepth = getCallDepth();
    // Execute instructions until there is an error, or one is at a higher depth of the call stack as prior to execution.
    std::function<bool(void)> cond = [this, &localCallDepth](){return localCallDepth <= getCallDepth()
            && !getExecutionFinished()
            && !stoppedForBreakpoint()
            && !hadErrorOnStep();};
    doISAStepWhile(cond);
}

quint64 FullMicrocodedCPU::getCycleCount()
{
    return memoizer->getCycleCount();
}

quint64 FullMicrocodedCPU::getInstructionCount()
{
    return memoizer->getInstructionCount();
}

const QVector<quint32> FullMicrocodedCPU::getInstructionHistogram()
{
    return memoizer->getInstructionHistogram();
}

void FullMicrocodedCPU::branchHandler()
{
    // If execution is already finished, then nothing to update.
    if(executionFinished) return;
    else if(hadErrorOnStep()) executionFinished = true;
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
            tempString = Pep::defaultEnumToMicrocodeAddrSymbol[addrMode];
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
            tempString = Pep::defaultEnumToMicrocodeInstrSymbol[mnemon];
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
        microprogramCounter = static_cast<quint16>(temp);
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
        QString tempString = Pep::instSpecToMicrocodeInstrSymbol[it];
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
            entry.addr = static_cast<quint16>(val->getValue());
        }
        instrSpecJT[static_cast<quint8>(it)] = entry;
    }
}

void FullMicrocodedCPU::calculateAddrJT()
{
    // Symbol table of the current microprogram
    QSharedPointer<const SymbolTable> symTable = this->sharedProgram->getSymTable();
    QSharedPointer<SymbolEntry> val;
    FullMicrocodedCPU::decoder_entry entry;
    for(int it = 0; it <= 255; ++it) {
        QString tempString = Pep::instSpecToMicrocodeAddrSymbol[it];
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
            entry.addr = static_cast<quint16>(val->getValue());
        }
        addrModeJT[static_cast<quint8>(it)] = entry;
    }
}

void FullMicrocodedCPU::breakpointAsmHandler()
{
    asmBreakpointHit = true;
    emit hitBreakpoint(Enu::BreakpointTypes::ASSEMBLER);
    return;
}

void FullMicrocodedCPU::breakpointMicroHandler()
{
    microBreakpointHit = true;
    emit hitBreakpoint(Enu::BreakpointTypes::MICROCODE);
    return;
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
