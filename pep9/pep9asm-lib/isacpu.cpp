// File: isacpu.cpp
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
#include "isacpu.h"

#include <functional>

#include <QApplication>

#include "assembler/asmprogram.h"
#include "assembler/asmprogrammanager.h"
#include "cache/cachememory.h"
#include "cpu/acpumodel.h"
#include "cpu/interrupthandler.h"
#include "memory/amemorydevice.h"
#include "pep/pep.h"

#include "isacpumemoizer.h"

const auto NBit_t = Pep9::Definition::getStatusBitOffset(Pep9::uarch::EStatusBit::STATUS_N);
const auto ZBit_t = Pep9::Definition::getStatusBitOffset(Pep9::uarch::EStatusBit::STATUS_Z);
const auto VBit_t = Pep9::Definition::getStatusBitOffset(Pep9::uarch::EStatusBit::STATUS_V);
const auto CBit_t = Pep9::Definition::getStatusBitOffset(Pep9::uarch::EStatusBit::STATUS_C);
const auto SBit_t = Pep9::Definition::getStatusBitOffset(Pep9::uarch::EStatusBit::STATUS_S);

IsaCpu::IsaCpu(const AsmProgramManager *input_manager, QSharedPointer<const Pep9::Definition> pep_version,
               QSharedPointer<AMemoryDevice> memDevice, QObject *parent):
    ACPUModel(memDevice, parent), Pep9InterfaceISACPU(memDevice.get(), input_manager),
    registerBank(pep_version->maxRegisterNumber(), pep_version->maxStatusBitNumber()), memoizer(new IsaCpuMemoizer(*this)),
    a_reg(pep_version->get_global_register_number(APepVersion::global_registers::A)),
    x_reg(pep_version->get_global_register_number(APepVersion::global_registers::X)),
    sp_reg(pep_version->get_global_register_number(APepVersion::global_registers::SP)),
    pc_reg(pep_version->get_global_register_number(APepVersion::global_registers::PC)),
    is_reg(pep_version->get_global_register_number(APepVersion::global_registers::IS)),
    os_reg(pep_version->get_global_register_number(APepVersion::global_registers::OS))
{
    // Create & register callbacks for breakpoint interrupts.
    std::function<void(void)> bpHandler = [this](){breakpointAsmHandler();};
    ACPUModel::handler->registerHandler(Interrupts::BREAKPOINT_ASM, bpHandler);
}

IsaCpu::~IsaCpu()
{
    delete memoizer;
}

void IsaCpu::stepOver()
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

bool IsaCpu::canStepInto() const
{
    quint8 byte;
    memory->getByte(getCPURegWordStart(Pep9::ISA::CPURegisters::PC), byte);
    Enu::EMnemonic mnemon = Pep::decodeMnemonic[byte];
    // Can only step into calls, trap instructions.
    return (mnemon == Enu::EMnemonic::CALL) || Pep::isTrapMap[mnemon];
}

void IsaCpu::stepInto()
{
    // Clear at start, so as to preserve highlighting AFTER finshing a write.
    memory->clearAllByteCaches();

    // Step into executes a single step, as a single step would
    // effectively enter the trap / call.
    onISAStep();
}

void IsaCpu::stepOut()
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

quint64 IsaCpu::getCycleCount(bool includeOS)
{
    return memoizer->getInstructionCount(includeOS);
}

quint64 IsaCpu::getInstructionCount(bool includeOS)
{
    return memoizer->getInstructionCount(includeOS);
}

const QVector<quint32> IsaCpu::getInstructionHistogram(bool includeOS)
{
    return memoizer->getInstructionHistogram(includeOS);
}

bool IsaCpu::hasCacheStats()
{
    //return dynamic_cast<CacheMemory*>(memory.get()) != nullptr;
    return memoizer->hasCacheStats();
}

const CacheHitrates IsaCpu::getCacheHitRates(bool includeOS)
{
    return memoizer->getCacheHitRates(includeOS);
}

RegisterFile &IsaCpu::getRegisterBank()
{
    return registerBank;
}

const RegisterFile &IsaCpu::getRegisterBank() const
{
    return registerBank;
}

void IsaCpu::onISAStep()
{
    using namespace Pep9::ISA;

    asmBreakpointHit = false;
    // Store PC at the start of the cycle, so that we know where the instruction started from.
    // Also store any other values needed for detailed statistics
    memoizer->storeStateInstrStart();
    memory->onCycleStarted();
    calculateStackChangeStart(this->getCPURegByteStart(CPURegisters::IS));

    // Load PC from register bank, allocate space for operand if it exists.
    quint16 opSpec, pc = registerBank.readRegisterWordCurrent(to_uint8_t(CPURegisters::PC));
    quint16 startPC = pc;
    quint8 is;

    memory->beginTransaction(AMemoryDevice::AccessType::INSTRUCTION);
    bool okay = memory->readByte(pc, is);

    registerBank.writeRegisterByte(to_uint8_t(CPURegisters::IS), is);
    Enu::EMnemonic mnemon = Pep::decodeMnemonic[is];
    Enu::EAddrMode addrMode;

    pc += 1;
    registerBank.writeRegisterWord(to_uint8_t(CPURegisters::PC), pc);
    if(Pep::isTrapMap[mnemon]) {
        memory->endTransaction();
        executeTrap(mnemon);
    }
    else if(Pep::isUnaryMap[mnemon]) {
        memory->endTransaction();
        executeUnary(mnemon);
    }
    else {
        okay &= memory->readWord(pc, opSpec);
        memory->endTransaction();
        registerBank.writeRegisterWord(to_uint8_t(CPURegisters::OS), opSpec);
        addrMode = Pep::decodeAddrMode[is];
        pc += 2;
        registerBank.writeRegisterWord(to_uint8_t(CPURegisters::PC), pc);
        executeNonunary(mnemon, opSpec, addrMode);
    }

    if(!okay) {
        controlError = true;
        errorMessage = "Error: Failed to perform memory access.";
    }

    // Post instruction execution cleanup
    calculateStackChangeEnd(this->getCPURegByteCurrent(CPURegisters::IS),
                                             this->getCPURegWordCurrent(CPURegisters::OS),
                                             this->getCPURegWordStart(CPURegisters::SP),
                                             this->getCPURegWordStart(CPURegisters::PC),
                                             this->getCPURegWordCurrent(CPURegisters::A));
    memoizer->storeStateInstrEnd();
    memory->onCycleFinished();
    updateAtInstructionEnd();
    emit asmInstructionFinished();
    asmStepCount++;

    // qDebug().noquote().nospace() << memoizer->memoize();

    registerBank.flattenFile();

    // Modulus must be greater than 1, or there will be no gaurentee of forward progress.
    // If modulus were 1, then debug debug breakpoints that were signaled externally
    // during process events would never be cleared by branch handler.
    if(asmStepCount % 500 == 0) {
        QApplication::processEvents();
    }

    // If execution finished on this instruction, then restore original starting program counter,
    // as the instruction at the current program counter will not be executed.
    if(executionFinished || hadErrorOnStep()) {
        registerBank.overwriteRegisterWordStart(to_uint8_t(CPURegisters::PC), startPC);
        emit simulationFinished();
    }


    if(inDebug && breakpointsISA.contains(getCPURegWordCurrent(CPURegisters::PC))) {
        ACPUModel::handler->interupt(Interrupts::BREAKPOINT_ASM);
    }
    ACPUModel::handler->handleQueuedInterrupts();
}

void IsaCpu::updateAtInstructionEnd()
{
    using namespace Pep9::ISA;

    auto IS = this->getCPURegByteCurrent(CPURegisters::IS);
    // Handle changing of call stack depth if the executed instruction affects the call stack.
    if(Pep::decodeMnemonic[IS] == Enu::EMnemonic::CALL){
        callDepth++;
    }
    else if(Pep::isTrapMap[Pep::decodeMnemonic[IS]]){
        callDepth++;
    }
    else if(Pep::decodeMnemonic[IS] == Enu::EMnemonic::RET){
        callDepth--;
    }
    else if(Pep::decodeMnemonic[IS] == Enu::EMnemonic::RETTR){
        callDepth--;
    }
    if(hadErrorOnStep()) {
        executionFinished = true;
    }

}

bool IsaCpu::readOperandWordValue(quint16 operand, Enu::EAddrMode addrMode, quint16 &opVal)
{
    bool rVal = operandWordValueHelper(operand, addrMode, &AMemoryDevice::readWord, opVal);
    // For instructions that perform loads, cache the decoded operand value.
    // This is in contrast to writes, where the operand value is the memory
    // address of the location to be written.
    InterfaceISACPU::opValCache = opVal;
    return rVal;
}

bool IsaCpu::readOperandByteValue(quint16 operand, Enu::EAddrMode addrMode, quint8 &opVal)
{
    bool rVal = operandByteValueHelper(operand, addrMode, &AMemoryDevice::readByte, opVal);
    // For instructions that perform loads, cache the decoded operand value.
    // This is in contrast to writes, where the operand value is the memory
    // address of the location to be written.
    InterfaceISACPU::opValCache = opVal;
    return rVal;
}

void IsaCpu::initCPU()
{
    using namespace Pep9::ISA;

    // Initialize CPU with proper stack pointer value in SP register.
    if(manager->getOperatingSystem().isNull()) {
        // If there is somehow no opeeating system, default to the correct SP.
        registerBank.writeRegisterWord(to_uint8_t(CPURegisters::SP), 0xFBF8);
    }
    // Otherwise, get the correct value from the memory vectors.
    else {
        // Get the offset from the bottom of memory.
        quint16 offset = manager->getMemoryVectorOffset(AsmProgramManager::MemoryVectors::UserStack);
        quint16 value;
        // The value starts at max address minus offset.
        memory->getWord(static_cast<quint16>(memory->maxAddress()) - offset,value);
        registerBank.writeRegisterWord(to_uint8_t(CPURegisters::SP), value);
    }
    registerBank.flattenFile();
}

bool IsaCpu::getStatusBitCurrent(Enu::EStatusBit statusBit) const
{
    switch(statusBit)
    {
    // Mask out bit of interest, then convert to bool
    case Enu::STATUS_N:
        return registerBank.readStatusBitCurrent(NBit_t);
    case Enu::STATUS_Z:
        return registerBank.readStatusBitCurrent(ZBit_t);
    case Enu::STATUS_V:
        return registerBank.readStatusBitCurrent(VBit_t);
    case Enu::STATUS_C:
        return registerBank.readStatusBitCurrent(CBit_t);
    case Enu::STATUS_S:
        return registerBank.readStatusBitCurrent(SBit_t);
    default:
        // Should never occur, but might happen if a bad status bit is passed
        return false;
    }
}

bool IsaCpu::getStatusBitStart(Enu::EStatusBit statusBit) const
{
    switch(statusBit)
    {
    // Mask out bit of interest, then convert to bool
    case Enu::STATUS_N:
        return registerBank.readStatusBitStart(NBit_t);
    case Enu::STATUS_Z:
        return registerBank.readStatusBitStart(ZBit_t);
    case Enu::STATUS_V:
        return registerBank.readStatusBitStart(VBit_t);
    case Enu::STATUS_C:
        return registerBank.readStatusBitStart(CBit_t);
    case Enu::STATUS_S:
        return registerBank.readStatusBitStart(SBit_t);
    default:
        // Should never occur, but might happen if a bad status bit is passed
        return false;
    }
}

quint8 IsaCpu::getCPURegByteCurrent(Pep9::ISA::CPURegisters reg) const
{
    return getCPURegByteCurrent(to_uint8_t(reg));
}

quint16 IsaCpu::getCPURegWordCurrent(Pep9::ISA::CPURegisters reg) const
{
    return getCPURegWordCurrent(to_uint8_t(reg));
}

quint8 IsaCpu::getCPURegByteStart(Pep9::ISA::CPURegisters reg) const
{
    return getCPURegByteStart(to_uint8_t(reg));
}

quint16 IsaCpu::getCPURegWordStart(Pep9::ISA::CPURegisters reg) const
{
    return getCPURegWordStart(to_uint8_t(reg));
}

QString IsaCpu::getErrorMessage() const noexcept
{
    if(memory->hadError()) return memory->getErrorMessage();
    else if(hadErrorOnStep()) return errorMessage;
    else return "";
}

quint8 IsaCpu::getCPURegByteCurrent(PepCore::CPURegisters_number_t reg) const
{
    return registerBank.readRegisterByteCurrent(reg);
}

quint16 IsaCpu::getCPURegWordCurrent(PepCore::CPURegisters_number_t reg) const
{
    return registerBank.readRegisterWordCurrent(reg);
}

quint8 IsaCpu::getCPURegByteStart(PepCore::CPURegisters_number_t reg) const
{
    return registerBank.readRegisterByteStart(reg);
}

quint16 IsaCpu::getCPURegWordStart(PepCore::CPURegisters_number_t reg) const
{
    return registerBank.readRegisterWordStart(reg);
}

bool IsaCpu::hadErrorOnStep() const noexcept
{
    return memory->hadError() || controlError;
}

bool IsaCpu::stoppedForBreakpoint() const noexcept
{
    return asmBreakpointHit;
}

void IsaCpu::onSimulationStarted()
{
    inDebug = false;
    inSimulation = false;
    executionFinished = false;
    asmBreakpointHit = false;
    memoizer->clear();
    memoizer->onSimultationStarted();
    memory->clearErrors();
    ACPUModel::handler->clearQueuedInterrupts();
}

void IsaCpu::onSimulationFinished()
{
    executionFinished = true;
    inDebug = false;
    ACPUModel::handler->clearQueuedInterrupts();
    #pragma message("TODO: Inform memory that execution is finished")
}

void IsaCpu::enableDebugging()
{
    inDebug = true;
}

void IsaCpu::forceBreakpoint(PepCore::BreakpointTypes breakpoint)
{
    switch(breakpoint){
    case PepCore::BreakpointTypes::ASSEMBLER:
        ACPUModel::handler->interupt(Interrupts::BREAKPOINT_ASM);
        break;
    default:
        //Ignore any other breakpoint types
        break;
    }
}

void IsaCpu::onCancelExecution()
{
    executionFinished = true;
    inDebug = false;
}

bool IsaCpu::onRun()
{
    timer.start();
    std::function<bool(void)> func = [this]() {
        return !hadErrorOnStep() && !executionFinished && !(inDebug && asmBreakpointHit);};
    // Always execute at least once, otherwise cannot progress past breakpoints
    doISAStepWhile(func);

    // If a breakpoint was reached, or if there was an error on the control flow.
    // return before final statistics are computed or the simulation is finished.
    if(asmBreakpointHit || hadErrorOnStep()) {
        return false;
    }

    // auto value = timer.elapsed();
    // qDebug().nospace().noquote() << memoizer->finalStatistics() << "\n";
    // qDebug().nospace().noquote() << "Executed "<< asmInstructionCounter << " instructions.";
    // qDebug().nospace().noquote() << "Execution time (ms): " << value;
    // qDebug().nospace().noquote() << "Instructions per second: " << asmInstructionCounter / (((float)value/1000));
    return true;
}

void IsaCpu::onResetCPU()
{
    // Reset all internal state, but keep loaded micropgoram & breakpoints
    ACPUModel::memory->clearErrors();
    ACPUModel::handler->clearQueuedInterrupts();
    memoizer->clear();
    InterfaceISACPU::reset();
    inSimulation = false;
    inDebug = false;
    callDepth = 0;
    controlError = false;
    executionFinished = false;
    errorMessage = "";
    asmBreakpointHit = false;
    registerBank.clearRegisters();
    registerBank.clearStatusBits();

}

bool IsaCpu::operandWordValueHelper(quint16 operand, Enu::EAddrMode addrMode,
                               bool (AMemoryDevice::*readFunc)(quint16, quint16 &) const,
                               quint16 &opVal)
{
    using namespace  Pep9::ISA;

    bool rVal = true;
    quint16 effectiveAddress = 0;
    switch(addrMode) {
    case Enu::EAddrMode::I:
        opVal = operand;
        break;
    case Enu::EAddrMode::D:
        effectiveAddress = operand;
        memory->beginTransaction(AMemoryDevice::AccessType::DATA);
        rVal  = std::invoke(readFunc, memory.get(), effectiveAddress, opVal);
        memory->endTransaction();
        break;
    case Enu::EAddrMode::S:
        effectiveAddress = operand + getCPURegWordCurrent(CPURegisters::SP);
        memory->beginTransaction(AMemoryDevice::AccessType::DATA);
        rVal  = std::invoke(readFunc, memory.get(), effectiveAddress, opVal);
        memory->endTransaction();
        break;
    case Enu::EAddrMode::X:
        effectiveAddress = operand + getCPURegWordCurrent(CPURegisters::X);
        memory->beginTransaction(AMemoryDevice::AccessType::DATA);
        rVal  = std::invoke(readFunc, memory.get(), effectiveAddress, opVal);
        memory->endTransaction();
        break;
    case Enu::EAddrMode::SX:
        effectiveAddress = operand
                + getCPURegWordCurrent(CPURegisters::SP)
                + getCPURegWordCurrent(CPURegisters::X);
        memory->beginTransaction(AMemoryDevice::AccessType::DATA);
        rVal  = std::invoke(readFunc, memory.get(), effectiveAddress, opVal);
        memory->endTransaction();
        break;
    case Enu::EAddrMode::N:
        effectiveAddress = operand;
        memory->beginTransaction(AMemoryDevice::AccessType::DATA);
        rVal  = std::invoke(readFunc, memory.get(), effectiveAddress, effectiveAddress);
        memory->endTransaction();
        memory->beginTransaction(AMemoryDevice::AccessType::DATA);
        rVal  &= std::invoke(readFunc, memory.get(), effectiveAddress, opVal);
        memory->endTransaction();
        break;
    case Enu::EAddrMode::SF:
        effectiveAddress = operand + getCPURegWordCurrent(CPURegisters::SP);
        memory->beginTransaction(AMemoryDevice::AccessType::DATA);
        rVal  = std::invoke(readFunc, memory.get(), effectiveAddress, effectiveAddress);
        memory->endTransaction();
        memory->beginTransaction(AMemoryDevice::AccessType::DATA);
        rVal  &= std::invoke(readFunc, memory.get(), effectiveAddress, opVal);
        memory->endTransaction();
        break;
    case Enu::EAddrMode::SFX:
        effectiveAddress = operand + getCPURegWordCurrent(CPURegisters::SP);
        memory->beginTransaction(AMemoryDevice::AccessType::DATA);
        rVal  = std::invoke(readFunc, memory.get(), effectiveAddress, effectiveAddress);
        memory->endTransaction();
        effectiveAddress += getCPURegWordCurrent(CPURegisters::X);
        memory->beginTransaction(AMemoryDevice::AccessType::DATA);
        rVal  &= std::invoke(readFunc, memory.get(), effectiveAddress, opVal);
        memory->endTransaction();
        break;
    default:
        break;
    }
    return rVal;
}

bool IsaCpu::operandByteValueHelper(quint16 operand, Enu::EAddrMode addrMode, bool (AMemoryDevice::*readFunc)(quint16, quint8 &) const, quint8 &opVal)
{
    using namespace Pep9::ISA;

    bool rVal = true;
    quint16 effectiveAddress = 0;
    quint8 tempByteHi, tempByteLo;
    // Having tested it, it definitely does the wrong thing
    switch(addrMode) {
    case Enu::EAddrMode::I:
        opVal = static_cast<quint8>(operand & 0xff);
        break;
    case Enu::EAddrMode::D:
        effectiveAddress = operand;
        memory->beginTransaction(AMemoryDevice::AccessType::DATA);
        rVal  = std::invoke(readFunc, memory.get(), effectiveAddress, opVal);
        memory->endTransaction();
        break;
    case Enu::EAddrMode::S:
        effectiveAddress = operand + getCPURegWordCurrent(CPURegisters::SP);
        memory->beginTransaction(AMemoryDevice::AccessType::DATA);
        rVal  = std::invoke(readFunc, memory.get(), effectiveAddress, opVal);
        memory->endTransaction();
        break;
    case Enu::EAddrMode::X:
        effectiveAddress = operand + getCPURegWordCurrent(CPURegisters::X);
        memory->beginTransaction(AMemoryDevice::AccessType::DATA);
        rVal  = std::invoke(readFunc, memory.get(), effectiveAddress, opVal);
        memory->endTransaction();
        break;
    case Enu::EAddrMode::SX:
        effectiveAddress = operand
                + getCPURegWordCurrent(CPURegisters::SP)
                + getCPURegWordCurrent(CPURegisters::X);
        memory->beginTransaction(AMemoryDevice::AccessType::DATA);
        rVal  = std::invoke(readFunc, memory.get(), effectiveAddress, opVal);
        memory->endTransaction();
        break;
    case Enu::EAddrMode::N:
        effectiveAddress = operand;
        memory->beginTransaction(AMemoryDevice::AccessType::DATA);
        rVal  = std::invoke(readFunc, memory.get(), effectiveAddress, tempByteHi);
        rVal  &= std::invoke(readFunc, memory.get(), effectiveAddress + 1, tempByteLo);
        memory->endTransaction();
        effectiveAddress = static_cast<quint16>(tempByteHi << 8 | tempByteLo);
        memory->beginTransaction(AMemoryDevice::AccessType::DATA);
        rVal  &= std::invoke(readFunc, memory.get(), effectiveAddress , opVal);
        memory->endTransaction();
        break;
    case Enu::EAddrMode::SF:
        effectiveAddress = operand + getCPURegWordCurrent(CPURegisters::SP);
        memory->beginTransaction(AMemoryDevice::AccessType::DATA);
        rVal  = std::invoke(readFunc, memory.get(), effectiveAddress, tempByteHi);
        rVal  &= std::invoke(readFunc, memory.get(), effectiveAddress + 1, tempByteLo);
        memory->endTransaction();
        effectiveAddress = static_cast<quint16>(tempByteHi << 8 | tempByteLo);
        memory->beginTransaction(AMemoryDevice::AccessType::DATA);
        rVal  &= std::invoke(readFunc, memory.get(), effectiveAddress, opVal);
        memory->endTransaction();
        break;
    case Enu::EAddrMode::SFX:
        effectiveAddress = operand + getCPURegWordCurrent(CPURegisters::SP);
        memory->beginTransaction(AMemoryDevice::AccessType::DATA);
        rVal  = std::invoke(readFunc, memory.get(), effectiveAddress, tempByteHi);
        rVal  &= std::invoke(readFunc, memory.get(), effectiveAddress + 1, tempByteLo);
        memory->endTransaction();
        effectiveAddress = static_cast<quint16>(tempByteHi << 8 | tempByteLo);
        effectiveAddress += getCPURegWordCurrent(CPURegisters::X);
        memory->beginTransaction(AMemoryDevice::AccessType::DATA);
        rVal  &= std::invoke(readFunc, memory.get(), effectiveAddress, opVal);
        memory->endTransaction();
        break;
    default:
        break;
    }
    return rVal;
}

bool IsaCpu::writeOperandWord(quint16 operand, quint16 value, Enu::EAddrMode addrMode)
{
    using namespace Pep9::ISA;

    bool rVal = true;
    quint16 effectiveAddress = 0;
    switch(addrMode) {
    case Enu::EAddrMode::I:
        rVal = false;
        break;
    case Enu::EAddrMode::D:
        effectiveAddress = operand;
        memory->beginTransaction(AMemoryDevice::AccessType::DATA);
        rVal = memory->writeWord(effectiveAddress, value);
        memory->endTransaction();
        break;
    case Enu::EAddrMode::S:
        effectiveAddress = operand + getCPURegWordCurrent(CPURegisters::SP);
        memory->beginTransaction(AMemoryDevice::AccessType::DATA);
        rVal = memory->writeWord(effectiveAddress, value);
        memory->endTransaction();
        break;
    case Enu::EAddrMode::X:
        effectiveAddress = operand + getCPURegWordCurrent(CPURegisters::X);
        memory->beginTransaction(AMemoryDevice::AccessType::DATA);
        rVal = memory->writeWord(effectiveAddress, value);
        memory->endTransaction();
        break;
    case Enu::EAddrMode::SX:
        effectiveAddress = operand
                + getCPURegWordCurrent(CPURegisters::SP)
                + getCPURegWordCurrent(CPURegisters::X);
        memory->beginTransaction(AMemoryDevice::AccessType::DATA);
        rVal = memory->writeWord(effectiveAddress, value);
        memory->endTransaction();
        break;
    case Enu::EAddrMode::N:
        effectiveAddress = operand;
        memory->beginTransaction(AMemoryDevice::AccessType::DATA);
        rVal = memory->readWord(effectiveAddress, effectiveAddress);
        memory->endTransaction();
        memory->beginTransaction(AMemoryDevice::AccessType::DATA);
        rVal &= memory->writeWord(effectiveAddress, value);
        memory->endTransaction();
        break;
    case Enu::EAddrMode::SF:
        effectiveAddress = operand + getCPURegWordCurrent(CPURegisters::SP);
        memory->beginTransaction(AMemoryDevice::AccessType::DATA);
        rVal = memory->readWord(effectiveAddress, effectiveAddress);
        memory->endTransaction();
        memory->beginTransaction(AMemoryDevice::AccessType::DATA);
        rVal &= memory->writeWord(effectiveAddress, value);
        memory->endTransaction();
        break;
    case Enu::EAddrMode::SFX:
        effectiveAddress = operand + getCPURegWordCurrent(CPURegisters::SP);
        memory->beginTransaction(AMemoryDevice::AccessType::DATA);
        rVal = memory->readWord(effectiveAddress, effectiveAddress);
        memory->endTransaction();
        effectiveAddress += getCPURegWordCurrent(CPURegisters::X);
        memory->beginTransaction(AMemoryDevice::AccessType::DATA);
        rVal &= memory->writeWord(effectiveAddress, value);
        memory->endTransaction();
        break;
    default:
        rVal = false;
        break;
    }

    // For write instructions, the operand value is the address in memory
    // where a value is to be written.
    // This is in contrast to loads, where the decoded operand value is
    // mem[write's operand value].
    InterfaceISACPU::opValCache = effectiveAddress;
    return rVal;

}

bool IsaCpu::writeOperandByte(quint16 operand, quint8 value, Enu::EAddrMode addrMode)
{
    using namespace Pep9::ISA;

    bool rVal = true;
    quint16 effectiveAddress = 0;
    switch(addrMode) {
    case Enu::EAddrMode::I:
        rVal = false;
        break;
    case Enu::EAddrMode::D:
        effectiveAddress = operand;
        memory->beginTransaction(AMemoryDevice::AccessType::DATA);
        rVal = memory->writeByte(effectiveAddress, value);
        memory->endTransaction();
        break;
    case Enu::EAddrMode::S:
        effectiveAddress = operand + getCPURegWordCurrent(CPURegisters::SP);
        memory->beginTransaction(AMemoryDevice::AccessType::DATA);
        rVal = memory->writeByte(effectiveAddress, value);
        memory->endTransaction();
        break;
    case Enu::EAddrMode::X:
        effectiveAddress = operand + getCPURegWordCurrent(CPURegisters::X);
        memory->beginTransaction(AMemoryDevice::AccessType::DATA);
        rVal = memory->writeByte(effectiveAddress, value);
        memory->endTransaction();
        break;
    case Enu::EAddrMode::SX:
        effectiveAddress = operand
                + getCPURegWordCurrent(CPURegisters::SP)
                + getCPURegWordCurrent(CPURegisters::X);
        memory->beginTransaction(AMemoryDevice::AccessType::DATA);
        rVal = memory->writeByte(effectiveAddress, value);
        memory->endTransaction();
        break;
    case Enu::EAddrMode::N:
        effectiveAddress = operand;
        memory->beginTransaction(AMemoryDevice::AccessType::DATA);
        rVal = memory->readWord(effectiveAddress, effectiveAddress);
        memory->endTransaction();
        memory->beginTransaction(AMemoryDevice::AccessType::DATA);
        rVal &= memory->writeByte(effectiveAddress, value);
        memory->endTransaction();
        break;
    case Enu::EAddrMode::SF:
        effectiveAddress = operand + getCPURegWordCurrent(CPURegisters::SP);
        memory->beginTransaction(AMemoryDevice::AccessType::DATA);
        rVal = memory->readWord(effectiveAddress, effectiveAddress);
        memory->endTransaction();
        memory->beginTransaction(AMemoryDevice::AccessType::DATA);
        rVal &= memory->writeByte(effectiveAddress, value);
        memory->endTransaction();
        break;
    case Enu::EAddrMode::SFX:
        effectiveAddress = operand + getCPURegWordCurrent(CPURegisters::SP);
        memory->beginTransaction(AMemoryDevice::AccessType::DATA);
        rVal = memory->readWord(effectiveAddress, effectiveAddress);
        memory->endTransaction();
        effectiveAddress += getCPURegWordCurrent(CPURegisters::X);
        memory->beginTransaction(AMemoryDevice::AccessType::DATA);
        rVal &= memory->writeByte(effectiveAddress, value);
        memory->endTransaction();
        break;
    default:
        rVal = false;
        break;
    }
    // For write instructions, the operand value is the address in memory
    // where a value is to be written.
    // This is in contrast to loads, where the decoded operand value is
    // mem[write's operand value].
    InterfaceISACPU::opValCache = effectiveAddress;
    return rVal;
}

void IsaCpu::executeUnary(Enu::EMnemonic mnemon)
{
    quint16 temp, sp, acc, idx;
    quint8 tempByte;

    sp = registerBank.readRegisterWordCurrent(sp_reg);
    acc = registerBank.readRegisterWordCurrent(a_reg);
    idx = registerBank.readRegisterWordCurrent(x_reg);

    switch(mnemon) {
    case Enu::EMnemonic::STOP:
        executionFinished = true;
        break;

    case Enu::EMnemonic::RET:
        memory->readWord(sp, temp);
        registerBank.writeRegisterWord(pc_reg, temp);
        sp += 2;
        registerBank.writeRegisterWord(sp_reg, sp);
        break;

    case Enu::EMnemonic::RETTR:
        memory->readByte(sp, tempByte);
        // Mask out bits that don't matter
        registerBank.writeStatusBit(NBit_t, tempByte & Pep9::uarch::EMask::NMask);
        registerBank.writeStatusBit(ZBit_t, tempByte & Pep9::uarch::EMask::ZMask);
        registerBank.writeStatusBit(VBit_t, tempByte & Pep9::uarch::EMask::VMask);
        registerBank.writeStatusBit(CBit_t, tempByte & Pep9::uarch::EMask::CMask);

        memory->readWord(sp + 1, temp);
        registerBank.writeRegisterWord(a_reg, temp);
        memory->readWord(sp + 3, temp);
        registerBank.writeRegisterWord(x_reg, temp);
        memory->readWord(sp + 5, temp);
        registerBank.writeRegisterWord(pc_reg, temp);
        memory->readWord(sp + 7, temp);
        registerBank.writeRegisterWord(sp_reg, temp);
        break;

    case Enu::EMnemonic::MOVSPA:
        registerBank.writeRegisterWord(a_reg, sp);
        break;

    case Enu::EMnemonic::MOVFLGA:
        tempByte = 0;
        tempByte |= registerBank.readStatusBitCurrent(NBit_t) * Pep9::uarch::EMask::NMask;
        tempByte |= registerBank.readStatusBitCurrent(ZBit_t) * Pep9::uarch::EMask::ZMask;
        tempByte |= registerBank.readStatusBitCurrent(VBit_t) * Pep9::uarch::EMask::VMask;
        tempByte |= registerBank.readStatusBitCurrent(CBit_t) * Pep9::uarch::EMask::CMask;
        registerBank.writeRegisterWord(a_reg, tempByte);
        break;

    case Enu::EMnemonic::MOVAFLG:
        // Only move the low order byte of accumulator to the status bits.
        tempByte = static_cast<quint8>(acc);
        // Mask out bits that don't matter
        registerBank.writeStatusBit(NBit_t, tempByte & Pep9::uarch::EMask::NMask);
        registerBank.writeStatusBit(ZBit_t, tempByte & Pep9::uarch::EMask::ZMask);
        registerBank.writeStatusBit(VBit_t, tempByte & Pep9::uarch::EMask::VMask);
        registerBank.writeStatusBit(CBit_t, tempByte & Pep9::uarch::EMask::CMask);
        break;

    case Enu::EMnemonic::NOTA: // Modifies NZ bits
        acc = ~acc;
        registerBank.writeRegisterWord(a_reg, acc);
        // Is negative if high order bit is 1.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_N, acc & 0x8000);
         // Is zero if all bits are 0's.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_Z, acc == 0);
        break;

    case Enu::EMnemonic::NOTX: // Modifies NZ bits
        idx = ~idx;
        registerBank.writeRegisterWord(x_reg, idx);
        // Is negative if high order bit is 1.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_N, idx & 0x8000);
         // Is zero if all bits are 0's.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_Z, idx == 0);
        break;

    case Enu::EMnemonic::NEGA: // Modifies NZV bits
        acc = ~acc + 1;
        registerBank.writeRegisterWord(a_reg, acc);
        // Is negative if high order bit is 1.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_N, acc & 0x8000);
         // Is zero if all bits are 0's.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_Z, acc == 0);
        // Only a signed overflow if register is 0x8000.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_V, acc == 0x8000);
        break;

    case Enu::EMnemonic::NEGX: // Modifies NZV bits
        idx = ~idx + 1;
        registerBank.writeRegisterWord(x_reg, idx);
        // Is negative if high order bit is 1.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_N, idx & 0x8000);
         // Is zero if all bits are 0's.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_Z, idx == 0);
        // Only a signed overflow if register is 0x8000.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_V, idx == 0x8000);
        break;

    // Arithmetic shift instructions
    case Enu::EMnemonic::ASLA: // Modifies NZVC bits
        temp = static_cast<quint16>(acc << 1);
        registerBank.writeRegisterWord(a_reg, temp);
        // Is negative if high order bit is 1.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_N, temp & 0x8000);
         // Is zero if all bits are 0's.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_Z, temp == 0);
        // Signed overflow occurs when the starting & ending values of the high order bit differ (a xor temp == 1).
        // Then shift the result over by 15 places to only keep high order bit (which is the sign).
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_V, (acc ^ temp) >> 15);
        // Carry out if register starts with high order 1.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_C, acc & 0x8000);
        break;

    case Enu::EMnemonic::ASLX: // Modifies NZVC bits
        temp = static_cast<quint16>(idx << 1);
        registerBank.writeRegisterWord(x_reg, temp);
        // Is negative if high order bit is 1.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_N, temp & 0x8000);
         // Is zero if all bits are 0's.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_Z, temp == 0);
        // Signed overflow occurs when the starting & ending values of the high order bit differ (a xor temp == 1).
        // Then shift the result over by 15 places to only keep high order bit (which is the sign).
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_V, (idx ^ temp) >> 15);
        // Carry out if register starts with high order 1.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_C, idx & 0x8000);

        break;

    case Enu::EMnemonic::ASRA: // Modifies NZC bits
        // Shift all bits to the right by 1 position. Since using unsigned shift, must explicitly
        // perform sign extension by hand.
        temp = static_cast<quint16>(acc >> 1 |
                                    // If the high order bit is 1, then sign extend with 1, else 0.
                                    ((acc & 0x8000) ? 1<<15 : 0));
        registerBank.writeRegisterWord(a_reg, temp);
        // Is negative if high order bit is 1.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_N, temp & 0x8000);
         // Is zero if all bits are 0's.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_Z, temp == 0);
        // Carry out if register starts with low order 1.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_C, acc & 0x01);
        break;

    case Enu::EMnemonic::ASRX: // Modifies NZC bits
        // Shift all bits to the right by 1 position. Since using unsigned shift, must explicitly
        // perform sign extension by hand.
        temp = static_cast<quint16>(idx >> 1 |
                                    // If the high order bit is 1, then sign extend with 1, else 0.
                                    ((idx & 0x8000) ? 1<<15 : 0));
        registerBank.writeRegisterWord(x_reg, temp);
        // Is negative if high order bit is 1.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_N, temp & 0x8000);
         // Is zero if all bits are 0's.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_Z, temp == 0);
        // Carry out if register starts with low order 1.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_C, idx & 0x01);
        break;

    // Rotate instructions.
    case Enu::EMnemonic::RORA: // Modifies C bits
        temp = static_cast<quint16>(acc >> 1
                                    // Shift the carry to high order bit.
                                    | (registerBank.readStatusBitCurrent(Enu::EStatusBit::STATUS_C)
                                        ? 1<<15 : 0));
        registerBank.writeRegisterWord(a_reg, temp);
        // Carry out if register starts with low order 1.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_C, acc & 0x01);
        break;

    case Enu::EMnemonic::RORX: // Modifies C bit
        temp = static_cast<quint16>(idx >> 1
                                    // Shift the carry to high order bit.
                                    | (registerBank.readStatusBitCurrent(Enu::EStatusBit::STATUS_C)
                                        ? 1<<15 : 0));
        registerBank.writeRegisterWord(x_reg, temp);
        // Carry out if register starts with low order 1.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_C, idx & 0x01);
        break;

    case Enu::EMnemonic::ROLA: // Modifies C bit
        temp = static_cast<quint16>(acc << 1
                                    // Shift the carry in to low order bit.
                                    | (registerBank.readStatusBitCurrent(Enu::EStatusBit::STATUS_C)
                                        ? 1 : 0));
        registerBank.writeRegisterWord(a_reg, temp);
        // Carry out if register starts with high order 1.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_C, acc & 0x8000);
        break;

    case Enu::EMnemonic::ROLX: // Modifies C bit
        temp = static_cast<quint16>(acc << 1
                                    // Shift the carry in to low order bit.
                                    | (registerBank.readStatusBitCurrent(Enu::EStatusBit::STATUS_C)
                                        ? 1 : 0));
        registerBank.writeRegisterWord(x_reg, temp);
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_C, idx & 0x8000);
        break;

    case Enu::EMnemonic::NOP0:
        if(Pep::isTrapMap[Enu::EMnemonic::NOP0]) {
            controlError = true;
            executionFinished = true;
            errorMessage = "Error: NOP0 is not a unary instruction.";
        }
        break;
    default:
        // Should never occur, but gaurd against to make compiler happy.
        controlError = true;
        executionFinished = true;
        errorMessage = "Error: Attempedt to execute invalid unary instruction.";
        return;
    }
}

void IsaCpu::executeNonunary(Enu::EMnemonic mnemon, quint16 opSpec, Enu::EAddrMode addrMode)
{
    quint16 tempWord, a, x, sp, result;
    quint8 tempByte;
    a = registerBank.readRegisterWordCurrent(a_reg);
    x = registerBank.readRegisterWordCurrent(x_reg);
    sp = registerBank.readRegisterWordCurrent(sp_reg);
    // I am confident that tempword will be initialized correctly by readOperandWordValue()
    // but the static analyzer is not so sure. Default initialize it to supress this warning.
    tempWord = 0;
    bool memSuccess = true;
    switch(mnemon) {

    case Enu::EMnemonic::BR:
        memSuccess = readOperandWordValue(opSpec, addrMode, tempWord);
        registerBank.writeRegisterWord(pc_reg, tempWord);
        break;

    case Enu::EMnemonic::BRLE:
        if(registerBank.readStatusBitCurrent(Enu::EStatusBit::STATUS_N) ||
                registerBank.readStatusBitCurrent(Enu::EStatusBit::STATUS_Z)) {
            memSuccess = readOperandWordValue(opSpec, addrMode, tempWord);
            registerBank.writeRegisterWord(pc_reg, tempWord);
        }
        break;

    case Enu::EMnemonic::BRLT:
        if(registerBank.readStatusBitCurrent(Enu::EStatusBit::STATUS_N)) {
            memSuccess = readOperandWordValue(opSpec, addrMode, tempWord);
            registerBank.writeRegisterWord(pc_reg, tempWord);
        }
        break;

    case Enu::EMnemonic::BREQ:
        if(registerBank.readStatusBitCurrent(Enu::EStatusBit::STATUS_Z)) {
            memSuccess = readOperandWordValue(opSpec, addrMode, tempWord);
            registerBank.writeRegisterWord(pc_reg, tempWord);
        }
        break;

    case Enu::EMnemonic::BRNE:
        if(!registerBank.readStatusBitCurrent(Enu::EStatusBit::STATUS_Z)) {
            memSuccess = readOperandWordValue(opSpec, addrMode, tempWord);
            registerBank.writeRegisterWord(pc_reg, tempWord);
        }
        break;

    case Enu::EMnemonic::BRGE:
        if(!registerBank.readStatusBitCurrent(Enu::EStatusBit::STATUS_N)) {
            memSuccess = readOperandWordValue(opSpec, addrMode, tempWord);
            registerBank.writeRegisterWord(pc_reg, tempWord);
        }
        break;

    case Enu::EMnemonic::BRGT:
        if(!registerBank.readStatusBitCurrent(Enu::EStatusBit::STATUS_N) &&
                !registerBank.readStatusBitCurrent(Enu::EStatusBit::STATUS_Z)) {
            memSuccess = readOperandWordValue(opSpec, addrMode, tempWord);
            registerBank.writeRegisterWord(pc_reg, tempWord);
        }
        break;

    case Enu::EMnemonic::BRV:
        if(registerBank.readStatusBitCurrent(Enu::EStatusBit::STATUS_V)) {
            memSuccess = readOperandWordValue(opSpec, addrMode, tempWord);
            registerBank.writeRegisterWord(pc_reg, tempWord);
        }
        break;

    case Enu::EMnemonic::BRC:
        if(registerBank.readStatusBitCurrent(Enu::EStatusBit::STATUS_C)) {
            memSuccess = readOperandWordValue(opSpec, addrMode, tempWord);
            registerBank.writeRegisterWord(pc_reg, tempWord);
        }
        break;

    case Enu::EMnemonic::CALL:
        memSuccess = readOperandWordValue(opSpec, addrMode, tempWord);
        sp -= 2;
        memSuccess &= memory->writeWord(sp, registerBank.readRegisterWordCurrent(pc_reg));
        registerBank.writeRegisterWord(pc_reg, tempWord);
        registerBank.writeRegisterWord(sp_reg, sp);
        break;

    case Enu::EMnemonic::ADDSP:
        memSuccess = readOperandWordValue(opSpec, addrMode, tempWord);
        registerBank.writeRegisterWord(sp_reg, sp + tempWord);
        break;

    case Enu::EMnemonic::SUBSP:
        memSuccess = readOperandWordValue(opSpec, addrMode, tempWord);
        registerBank.writeRegisterWord(sp_reg, sp - tempWord);
        break;

    case Enu::EMnemonic::ADDA:
        memSuccess = readOperandWordValue(opSpec, addrMode, tempWord);
        // The result is the decoded operand specifier plus the accumulator
        result = a + tempWord;
        registerBank.writeRegisterWord(a_reg, result);
        // Is negative if high order bit is 1.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_N, result & 0x8000);
         // Is zero if all bits are 0's.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_Z, result == 0);
        // There is a signed overflow iff the high order bits of the register and operand
        //are the same, and one input & the output differ in sign.
        // >> Shifts in 0's (unsigned shorts), so after shift, only high order bit remain.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_V, (~(a ^ tempWord) & (a ^ result)) >> 15);
        // Carry out iff result is unsigned less than register or operand.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_C, result < a  || result < tempWord);
        break;

    case Enu::EMnemonic::ADDX:
        memSuccess = readOperandWordValue(opSpec, addrMode, tempWord);
        // The result is the decoded operand specifier plus the index reg.
        result = x + tempWord;
        registerBank.writeRegisterWord(x_reg, result);
        // Is negative if high order bit is 1.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_N, result & 0x8000);
         // Is zero if all bits are 0's.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_Z, result == 0);
        // There is a signed overflow iff the high order bits of the register and operand
        //are the same, and one input & the output differ in sign.
        // >> Shifts in 0's (unsigned shorts), so after shift, only high order bit remain.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_V, (~(x ^ tempWord) & (x ^ result)) >> 15);
        // Carry out iff result is unsigned less than register or operand.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_C, result < x  || result < tempWord);
        break;

    case Enu::EMnemonic::SUBA:
        memSuccess = readOperandWordValue(opSpec, addrMode, tempWord);
        // The result is the two's complement of the decoded operand specifier plus a.
        tempWord = ~tempWord + 1;
        // The result is the decoded operand specifier plus the accumulator.
        result = a + tempWord;
        registerBank.writeRegisterWord(a_reg, result);
        // Is negative if high order bit is 1.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_N, result & 0x8000);
         // Is zero if all bits are 0's.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_Z, result == 0);
        // There is a signed overflow iff the high order bits of the register and operand
        //are the same, and one input & the output differ in sign.
        // >> Shifts in 0's (unsigned shorts), so after shift, only high order bit remain.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_V, (~(a ^ tempWord) & (a ^ result)) >> 15);
        // Carry out iff result is unsigned less than register or operand.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_C, result < a  || result < tempWord);
        break;

    case Enu::EMnemonic::SUBX:
        memSuccess = readOperandWordValue(opSpec, addrMode, tempWord);
        // The result is the two's complement of the decoded operand specifier plus a.
        tempWord = ~tempWord + 1;
        // The result is the decoded operand specifier plus the index reg.
        result = x + tempWord;
        registerBank.writeRegisterWord(x_reg, result);
        // Is negative if high order bit is 1.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_N, result & 0x8000);
         // Is zero if all bits are 0's.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_Z, result == 0);
        // There is a signed overflow iff the high order bits of the register and operand
        //are the same, and one input & the output differ in sign.
        // >> Shifts in 0's (unsigned shorts), so after shift, only high order bit remain.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_V, (~(x ^ tempWord) & (x ^ result)) >> 15);
        // Carry out iff result is unsigned less than register or operand.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_C, result < x  || result < tempWord);
        break;

    case Enu::EMnemonic::ANDA:
        memSuccess = readOperandWordValue(opSpec, addrMode, tempWord);
        // The result is the decoded operand specifier bitwise and'ed with the accumulator.
        result = a & tempWord;
        registerBank.writeRegisterWord(a_reg, result);
        // Is negative if high order bit is 1.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_N, result & 0x8000);
         // Is zero if all bits are 0's.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_Z, result == 0);
        break;

    case Enu::EMnemonic::ANDX:
        memSuccess = readOperandWordValue(opSpec, addrMode, tempWord);
        // The result is the decoded operand specifier bitwise and'ed the index reg.
        result = x & tempWord;
        registerBank.writeRegisterWord(x_reg, result);
        // Is negative if high order bit is 1.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_N, result & 0x8000);
         // Is zero if all bits are 0's.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_Z, result == 0);
        break;

    case Enu::EMnemonic::ORA:
        memSuccess = readOperandWordValue(opSpec, addrMode, tempWord);
        // The result is the decoded operand specifier bitwise or'ed with the accumulator.
        result = a | tempWord;
        registerBank.writeRegisterWord(a_reg, result);
        // Is negative if high order bit is 1.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_N, result & 0x8000);
         // Is zero if all bits are 0's.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_Z, result == 0);
        break;
    case Enu::EMnemonic::ORX:
        memSuccess = readOperandWordValue(opSpec, addrMode, tempWord);
        // The result is the decoded operand specifier bitwise or'ed the index reg.
        result = x | tempWord;
        registerBank.writeRegisterWord(x_reg, result);
        // Is negative if high order bit is 1.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_N, result & 0x8000);
         // Is zero if all bits are 0's.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_Z, result == 0);
        break;

    case Enu::EMnemonic::CPWA:
        memSuccess = readOperandWordValue(opSpec, addrMode, tempWord);
        // The result is the two's complement of the decoded operand specifier plus a.
        tempWord = ~tempWord + 1;
        result = a + tempWord;
        // Is negative if high order bit is 1.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_N, result & 0x8000);
         // Is zero if all bits are 0's.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_Z, result == 0);
        // There is a signed overflow iff the high order bits of the register and operand
        //are the same, and one input & the output differ in sign.
        // >> Shifts in 0's (unsigned shorts), so after shift, only high order bit remain.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_V, (~(a ^  tempWord) & (a ^ result)) >> 15);
        // Carry out iff result is unsigned less than register or operand.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_C, result < a  || result < tempWord);
        // If there was a signed overflow, selectively invert N bit.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_N, registerBank.readStatusBitCurrent(Enu::EStatusBit::STATUS_N)
                                    ^ registerBank.readStatusBitCurrent(Enu::EStatusBit::STATUS_V));
        break;

    case Enu::EMnemonic::CPWX:
        memSuccess = readOperandWordValue(opSpec, addrMode, tempWord);
        // The result is the two's complement of the decoded operand specifier plus x.
        tempWord = ~tempWord + 1;
        result = x + tempWord;
        // Is negative if high order bit is 1.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_N, result & 0x8000);
         // Is zero if all bits are 0's.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_Z, result == 0);
        // There is a signed overflow iff the high order bits of the register and operand
        //are the same, and one input & the output differ in sign.
        // >> Shifts in 0's (unsigned shorts), so after shift, only high order bit remain.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_V, (~(x ^ tempWord) & (x ^ result)) >> 15);
        // Carry out iff result is unsigned less than register or operand.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_C, result < a  || result < tempWord);
        // If there was a signed overflow, selectively invert N bit.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_N, registerBank.readStatusBitCurrent(Enu::EStatusBit::STATUS_N)
                                    ^ registerBank.readStatusBitCurrent(Enu::EStatusBit::STATUS_V));
        break;

    case Enu::EMnemonic::LDWA:
        memSuccess = readOperandWordValue(opSpec, addrMode, tempWord);
        registerBank.writeRegisterWord(a_reg, tempWord);
        // Is negative if high order bit is 1.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_N, tempWord & 0x8000);
         // Is zero if all bits are 0's.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_Z, tempWord == 0);
        break;

    case Enu::EMnemonic::LDWX:
        memSuccess = readOperandWordValue(opSpec, addrMode, tempWord);
        registerBank.writeRegisterWord(x_reg, tempWord);
        // Is negative if high order bit is 1.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_N, tempWord & 0x8000);
         // Is zero if all bits are 0's.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_Z, tempWord == 0);
        break;

    case Enu::EMnemonic::STWA:
        tempWord = registerBank.readRegisterWordCurrent(a_reg);
        memSuccess = writeOperandWord(opSpec, tempWord, addrMode);
        break;

    case Enu::EMnemonic::STWX:
        tempWord = registerBank.readRegisterWordCurrent(x_reg);
        memSuccess = writeOperandWord(opSpec, tempWord, addrMode);
        break;

    // Single byte instructions
    case Enu::EMnemonic::CPBA:
        memSuccess = readOperandByteValue(opSpec, addrMode, tempByte);
        // The result is the two's complement of the decoded operand specifier plus a.
        // Narrow a and operand to 1 byte before widening to 2 bytes for result.
        tempWord = ~tempByte + 1;
        result = (a + tempWord) & 0xff;
        // Is negative if high order bit is 1.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_N, result & 0x80);
         // Is zero if all bits are 0's.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_Z, result == 0);
        // RTL specifies zeroing out V, C bits.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_V, false);
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_C, false);
        break;

    case Enu::EMnemonic::CPBX:
        memSuccess = readOperandByteValue(opSpec, addrMode, tempByte);
        // The result is the two's complement of the decoded operand specifier plus x.
        // Narrow a and operand to 1 byte before widening to 2 bytes for result.
        tempWord = ~tempByte + 1;
        result = (x + tempWord) & 0xff;
        // Is negative if high order bit is 1.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_N, result & 0x80);
         // Is zero if all bits are 0's.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_Z, result == 0);
        // RTL specifies zeroing out V, C bits.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_V, false);
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_C, false);
        break;

    case Enu::EMnemonic::LDBA:
        memSuccess = readOperandByteValue(opSpec, addrMode, tempByte);
        tempWord = a & 0xff00;
        tempWord |= tempByte;
        registerBank.writeRegisterWord(a_reg, tempWord);
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_N, false);
         // Is zero if all bits are 0's.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_Z, (tempWord & 0xff) == 0);

        break;

    case Enu::EMnemonic::LDBX:
        memSuccess = readOperandByteValue(opSpec, addrMode, tempByte);
        tempWord = x & 0xff00;
        tempWord |= tempByte;
        registerBank.writeRegisterWord(x_reg, tempWord);
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_N, false);
         // Is zero if all bits are 0's.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_Z, (tempWord & 0xff) == 0);
        break;

    case Enu::EMnemonic::STBA:
        tempByte = static_cast<quint8>(0xff & registerBank.readRegisterWordCurrent(a_reg));
        memSuccess = writeOperandByte(opSpec, tempByte, addrMode);
        break;

    case Enu::EMnemonic::STBX:
        tempByte = static_cast<quint8>(0xff & registerBank.readRegisterWordCurrent(x_reg));
        memSuccess = writeOperandByte(opSpec, tempByte, addrMode);
        break;
    default:
        controlError = true;
        executionFinished = true;
        errorMessage = "Error: Attempted to execute invalid nonunary instruction";
        break;
    }
    if(!memSuccess){
        controlError = true;
        errorMessage = "Error: Failed to perform memory access.";
    }
}

void IsaCpu::executeTrap(Enu::EMnemonic mnemon)
{
    quint16 pc;
    quint8 tempByte;
    // The
    quint16 tempAddr, temp = manager->getOperatingSystem()->getBurnValue() - 9;
    memory->readWord(temp, tempAddr);
    quint16 pcAddr = manager->getOperatingSystem()->getBurnValue() - 1;
    bool memSuccess = true;
    switch(mnemon) {
    // Non-unary traps
    case Enu::EMnemonic::NOP:;
        [[fallthrough]];
    case Enu::EMnemonic::DECI:;
        [[fallthrough]];
    case Enu::EMnemonic::DECO:;
        [[fallthrough]];
    case Enu::EMnemonic::HEXO:;
        [[fallthrough]];
    case Enu::EMnemonic::STRO:;
#if hardwarePCIncr
        // Though not part of the specification, the Pep9 hardware must increment the program counter
        // in order for non-unary traps to function correctly.
        pc = registerBank.readRegisterWordCurrent(pc_reg) + 2;
        registerBank.writeRegisterWord(pc_reg, pc);
#endif
        [[fallthrough]];
    // Unary traps
    case Enu::EMnemonic::NOP0:;
        [[fallthrough]];
    case Enu::EMnemonic::NOP1:;
        // Writes to mem[T-1].
        memSuccess &= memory->writeByte(tempAddr - 1, registerBank.readRegisterByteCurrent(is_reg) /*IS*/);
        // Writes to mem[T-2], mem[T-3].
        memSuccess &= memory->writeWord(tempAddr - 3, registerBank.readRegisterWordCurrent(sp_reg) /*SP*/);
        // Writes to mem[T-4], mem[T-5].
        memSuccess &= memory->writeWord(tempAddr - 5, registerBank.readRegisterWordCurrent(pc_reg) /*PC*/);
        // Writes to mem[T-6], mem[T-7].
        memSuccess &= memory->writeWord(tempAddr - 7, registerBank.readRegisterWordCurrent(x_reg) /*X*/);
        // Writes to mem[T-8], mem[T-9].
        memSuccess &= memory->writeWord(tempAddr - 9, registerBank.readRegisterWordCurrent(a_reg) /*A*/);
        // Writes to mem[T-10].
        tempByte = 0;
        tempByte |= registerBank.readStatusBitCurrent(NBit_t) * Pep9::uarch::EMask::NMask;
        tempByte |= registerBank.readStatusBitCurrent(ZBit_t) * Pep9::uarch::EMask::ZMask;
        tempByte |= registerBank.readStatusBitCurrent(VBit_t) * Pep9::uarch::EMask::VMask;
        tempByte |= registerBank.readStatusBitCurrent(CBit_t) * Pep9::uarch::EMask::CMask;
        memSuccess &= memory->writeByte(tempAddr - 10, tempByte /*NZVC*/);
        memSuccess &= memory->readWord(pcAddr, pc);
        registerBank.writeRegisterWord(sp_reg, tempAddr - 10);
        registerBank.writeRegisterWord(pc_reg, pc);
#if performTrapFix
        // Though not part of the specification, clear out the index register to
        // prevent bug in OS where non-unary instructions fail due to junk
        // in the high order byte of the index register. The book is published,
        // so we have to fix it here.
        registerBank.writeRegisterWord(x_reg, 0);
#endif
        break;
    default:
        controlError = true;
        executionFinished = true;
        errorMessage = "Error: Attempted to execute invalid trap instruction";
        break;
    }
    if(!memSuccess){
        controlError = true;
        errorMessage = "Error: Failed to perform memory access.";
    }
}

void IsaCpu::breakpointAsmHandler()
{
    // Callback function
    asmBreakpointHit = true;
    emit hitBreakpoint(PepCore::BreakpointTypes::ASSEMBLER);
    return;
}
