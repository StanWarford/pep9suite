#include "isacpu.h"
#include "acpumodel.h"
#include "amemorydevice.h"
#include "pep.h"
#include <functional>
#include <QApplication>
#include "asmprogrammanager.h"
#include "asmprogram.h"
#include "isacpumemoizer.h"
IsaCpu::IsaCpu(const AsmProgramManager *manager, QSharedPointer<AMemoryDevice> memDevice, QObject *parent):
    ACPUModel(memDevice, parent), InterfaceISACPU(memDevice.get(), manager), memoizer(new IsaCpuMemoizer(*this))
{

}

IsaCpu::~IsaCpu()
{
    delete memoizer;
}


void IsaCpu::stepOver()
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

bool IsaCpu::canStepInto() const
{
    quint8 byte;
    memory->getByte(getCPURegWordStart(Enu::CPURegisters::PC), byte);
    Enu::EMnemonic mnemon = Pep::decodeMnemonic[byte];
    return (mnemon == Enu::EMnemonic::CALL) || Pep::isTrapMap[mnemon];
}

void IsaCpu::stepInto()
{
    // Clear at start, so as to preserve highlighting AFTER finshing a write.
    memory->clearBytesWritten();
    onISAStep();
}

void IsaCpu::stepOut()
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

bool IsaCpu::getOperandWordValue(quint16 operand, Enu::EAddrMode addrMode, quint16 &opVal)
{
    return operandWordValueHelper(operand, addrMode, &AMemoryDevice::getWord, opVal);
}

bool IsaCpu::getOperandWordValue(quint16 operand, Enu::EAddrMode addrMode, quint8 &opVal)
{
    return operandByteValueHelper(operand, addrMode, &AMemoryDevice::getByte, opVal);
}

void IsaCpu::onISAStep()
{
    // Store PC at the start of the cycle, so that we know where the instruction started from.
    // Also store any other values needed for detailed statistics
    memoizer->storeStateInstrStart();
    memory->onCycleStarted();
    InterfaceISACPU::calculateStackChangeStart(this->getCPURegByteStart(Enu::CPURegisters::IS));

    quint16 opSpec, pc = registerBank.readRegisterWordCurrent(Enu::CPURegisters::PC);
    quint16 startPC = pc;
    quint8 is;
    bool okay = memory->readByte(pc, is);

    registerBank.writeRegisterByte(Enu::CPURegisters::IS, is);
    Enu::EMnemonic mnemon = Pep::decodeMnemonic[is];
    Enu::EAddrMode addrMode;

    pc += 1;
    registerBank.writeRegisterWord(Enu::CPURegisters::PC, pc);
    if(Pep::isTrapMap[mnemon]) {
        executeTrap(mnemon);
    }
    else if(Pep::isUnaryMap[mnemon]) {
        executeUnary(mnemon);
    }
    else {
        okay &= memory->readWord(pc, opSpec);
        registerBank.writeRegisterWord(Enu::CPURegisters::OS, opSpec);
        addrMode = Pep::decodeAddrMode[is];
        pc += 2;
        registerBank.writeRegisterWord(Enu::CPURegisters::PC, pc);
        executeNonunary(mnemon, opSpec, addrMode);
    }

    // Post instruction execution cleanup
    InterfaceISACPU::calculateStackChangeEnd(this->getCPURegByteCurrent(Enu::CPURegisters::IS),
                                             this->getCPURegWordCurrent(Enu::CPURegisters::OS),
                                             this->getCPURegWordStart(Enu::CPURegisters::SP),
                                             this->getCPURegWordStart(Enu::CPURegisters::PC),
                                             this->getCPURegWordCurrent(Enu::CPURegisters::A));
    memoizer->storeStateInstrEnd();
    updateAtInstructionEnd();
    emit asmInstructionFinished();
    asmInstructionCounter++;

    qDebug().noquote().nospace() << memoizer->memoize();

    registerBank.flattenFile();

    // If execution finished on this instruction, then restore original starting program counter,
    // as the instruction at the current program counter will not be executed.
    if(executionFinished) {
        registerBank.writePCStart(startPC);
        emit simulationFinished();
    }

    breakpointHandler();
}

void IsaCpu::updateAtInstructionEnd()
{

}

bool IsaCpu::readOperandWordValue(quint16 operand, Enu::EAddrMode addrMode, quint16 &opVal)
{
    return operandWordValueHelper(operand, addrMode, &AMemoryDevice::readWord, opVal);
}

bool IsaCpu::readOperandByteValue(quint16 operand, Enu::EAddrMode addrMode, quint8 &opVal)
{
    return operandByteValueHelper(operand, addrMode, &AMemoryDevice::readByte, opVal);
}

void IsaCpu::initCPU()
{
    // Initialize CPU with proper stack pointer value in SP register.
    #pragma message ("TODO: Init cpu with proper SP when not burned in at 0xffff")
    registerBank.writeRegisterWord(Enu::CPURegisters::SP, 0xFB82);
    #pragma message("TODO: Make sure this where registers should be flattened")
    registerBank.flattenFile();
}

bool IsaCpu::getStatusBitCurrent(Enu::EStatusBit statusBit) const
{
    quint8 NZVCSbits = registerBank.readStatusBitsCurrent();
    switch(statusBit)
    {
    // Mask out bit of interest, then convert to bool
    case Enu::STATUS_N:
        return(NZVCSbits & Enu::NMask);
    case Enu::STATUS_Z:
        return(NZVCSbits & Enu::ZMask);
    case Enu::STATUS_V:
        return(NZVCSbits & Enu::VMask);
    case Enu::STATUS_C:
        return(NZVCSbits & Enu::CMask);
    case Enu::STATUS_S:
        return(NZVCSbits & Enu::SMask);
    default:
        // Should never occur, but might happen if a bad status bit is passed
        return false;
    }
}

bool IsaCpu::getStatusBitStart(Enu::EStatusBit statusBit) const
{
    quint8 NZVCSbits = registerBank.readStatusBitsStart();
    switch(statusBit)
    {
    // Mask out bit of interest, then convert to bool
    case Enu::STATUS_N:
        return(NZVCSbits & Enu::NMask);
    case Enu::STATUS_Z:
        return(NZVCSbits & Enu::ZMask);
    case Enu::STATUS_V:
        return(NZVCSbits & Enu::VMask);
    case Enu::STATUS_C:
        return(NZVCSbits & Enu::CMask);
    case Enu::STATUS_S:
        return(NZVCSbits & Enu::SMask);
    default:
        // Should never occur, but might happen if a bad status bit is passed
        return false;
    }
}

quint8 IsaCpu::getCPURegByteCurrent(Enu::CPURegisters reg) const
{
    return registerBank.readRegisterByteCurrent(reg);
}

quint16 IsaCpu::getCPURegWordCurrent(Enu::CPURegisters reg) const
{
    return registerBank.readRegisterWordCurrent(reg);
}

quint8 IsaCpu::getCPURegByteStart(Enu::CPURegisters reg) const
{
    return registerBank.readRegisterByteStart(reg);
}

quint16 IsaCpu::getCPURegWordStart(Enu::CPURegisters reg) const
{
    return registerBank.readRegisterWordStart(reg);
}

QString IsaCpu::getErrorMessage() const noexcept
{
    if(memory->hadError()) return memory->getErrorMessage();
    // else if(data->hadErrorOnStep()) return data->getErrorMessage();
    else if(hadErrorOnStep()) return errorMessage;
    else return "";
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
    memoizer->clear();
}

void IsaCpu::onSimulationFinished()
{
    executionFinished = true;
    inDebug = false;
    #pragma message("TODO: Inform memory that execution is finished")
}

void IsaCpu::onDebuggingStarted()
{
    onSimulationStarted();
    inDebug = true;
    asmBreakpointHit = false;
}

void IsaCpu::onDebuggingFinished()
{
    onSimulationFinished();
    inDebug = false;
}

void IsaCpu::onCancelExecution()
{
    executionFinished = true;
    inDebug = false;
}

bool IsaCpu::onRun()
{
    timer.start();
    // If debugging, there is the potential to hit breakpoints, so a different main loop is needed.
    // Partially, this is to handle breakpoints gracefully, and partially to prevent "run" mode from being slowed down by debug features.
    if(inDebug) {
        // Always execute at least once, otherwise cannot progress past breakpoints
        do {
            // Since the sim runs at about 5Mhz, do not process events every single cycle to increase performance.
            if(asmInstructionCounter % 500 == 0) {
                QApplication::processEvents();
            }
            onISAStep();
        } while(!hadErrorOnStep() && !executionFinished && !(asmBreakpointHit));
    }
    else {
        while(!hadErrorOnStep() && !executionFinished) {
            // Since the sim runs at about 5Mhz, do not process events every single cycle to increase performance.
            if(asmInstructionCounter % 500 == 0) {
                QApplication::processEvents();
            }
            onISAStep();
        }
    }

    //If there was an error on the control flow
    if(hadErrorOnStep()) {
        if(memory->hadError()) {
            qDebug() << "Memory section reporting an error";
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
    if(asmBreakpointHit) {
        return false;
    }

    auto value = timer.elapsed();
    //qDebug().nospace().noquote() << memoizer->finalStatistics() << "\n";
    qDebug().nospace().noquote() << "Executed "<< asmInstructionCounter << " instructions.";
    qDebug().nospace().noquote() << "Execution time (ms): " << value;
    qDebug().nospace().noquote() << "Instructions per second: " << asmInstructionCounter / (((float)value/1000));
    return true;
}

void IsaCpu::onResetCPU()
{
    InterfaceISACPU::reset();
    registerBank.clearRegisters();
    registerBank.clearStatusBits();
}

bool IsaCpu::operandWordValueHelper(quint16 operand, Enu::EAddrMode addrMode,
                               bool (AMemoryDevice::*readFunc)(quint16, quint16 &) const,
                               quint16 &opVal)
{
    bool rVal = true;
    quint16 effectiveAddress = 0;
    switch(addrMode) {
    case Enu::EAddrMode::I:
        opVal = operand;
        break;
    case Enu::EAddrMode::D:
        effectiveAddress = operand;
        rVal  = std::invoke(readFunc, memory.get(), effectiveAddress, opVal);
        break;
    case Enu::EAddrMode::S:
        effectiveAddress = operand + getCPURegWordCurrent(Enu::CPURegisters::SP);
        rVal  = std::invoke(readFunc, memory.get(), effectiveAddress, opVal);
        break;
    case Enu::EAddrMode::X:
        effectiveAddress = operand + getCPURegWordCurrent(Enu::CPURegisters::X);
        rVal  = std::invoke(readFunc, memory.get(), effectiveAddress, opVal);
        break;
    case Enu::EAddrMode::SX:
        effectiveAddress = operand
                + getCPURegWordCurrent(Enu::CPURegisters::SP)
                + getCPURegWordCurrent(Enu::CPURegisters::X);
        rVal  = std::invoke(readFunc, memory.get(), effectiveAddress, opVal);
        break;
    case Enu::EAddrMode::N:
        effectiveAddress = operand;
        rVal  = std::invoke(readFunc, memory.get(), effectiveAddress, effectiveAddress);
        rVal  &= std::invoke(readFunc, memory.get(), effectiveAddress, opVal);
        break;
    case Enu::EAddrMode::SF:
        effectiveAddress = operand + getCPURegWordCurrent(Enu::CPURegisters::SP);
        rVal  = std::invoke(readFunc, memory.get(), effectiveAddress, effectiveAddress);
        rVal  &= std::invoke(readFunc, memory.get(), effectiveAddress, opVal);
        break;
    case Enu::EAddrMode::SFX:
        effectiveAddress = operand + getCPURegWordCurrent(Enu::CPURegisters::SP);
        rVal  = std::invoke(readFunc, memory.get(), effectiveAddress, effectiveAddress);
        effectiveAddress += getCPURegWordCurrent(Enu::CPURegisters::X);
        rVal  &= std::invoke(readFunc, memory.get(), effectiveAddress, opVal);
        break;
    default:
        break;
    }
    return rVal;
}

bool IsaCpu::operandByteValueHelper(quint16 operand, Enu::EAddrMode addrMode, bool (AMemoryDevice::*readFunc)(quint16, quint8 &) const, quint8 &opVal)
{
    bool rVal = true;
    quint16 effectiveAddress = 0;
    quint8 tempByteHi, tempByteLo;
    // Having tested it, it definitely does the wrong thing
#pragma message("Totally untested, might very well not do what I expect")
    switch(addrMode) {
    case Enu::EAddrMode::I:
        opVal = static_cast<quint8>(operand & 0xff);
        break;
    case Enu::EAddrMode::D:
        effectiveAddress = operand;
        rVal  = std::invoke(readFunc, memory.get(), effectiveAddress, opVal);
        break;
    case Enu::EAddrMode::S:
        effectiveAddress = operand + getCPURegWordCurrent(Enu::CPURegisters::SP);
        rVal  = std::invoke(readFunc, memory.get(), effectiveAddress, opVal);
        break;
    case Enu::EAddrMode::X:
        effectiveAddress = operand + getCPURegWordCurrent(Enu::CPURegisters::X);
        rVal  = std::invoke(readFunc, memory.get(), effectiveAddress, opVal);
        break;
    case Enu::EAddrMode::SX:
        effectiveAddress = operand
                + getCPURegWordCurrent(Enu::CPURegisters::SP)
                + getCPURegWordCurrent(Enu::CPURegisters::X);
        rVal  = std::invoke(readFunc, memory.get(), effectiveAddress, opVal);
        break;
    case Enu::EAddrMode::N:
        effectiveAddress = operand;
        rVal  = std::invoke(readFunc, memory.get(), effectiveAddress, tempByteHi);
        rVal  = std::invoke(readFunc, memory.get(), effectiveAddress + 1, tempByteLo);
        effectiveAddress = tempByteHi << 8 | tempByteLo;
        rVal  &= std::invoke(readFunc, memory.get(), effectiveAddress , opVal);
        break;
    case Enu::EAddrMode::SF:
        effectiveAddress = operand + getCPURegWordCurrent(Enu::CPURegisters::SP);
        rVal  = std::invoke(readFunc, memory.get(), effectiveAddress, tempByteHi);
        rVal  = std::invoke(readFunc, memory.get(), effectiveAddress + 1, tempByteLo);
        effectiveAddress = tempByteHi << 8 | tempByteLo;
        rVal  &= std::invoke(readFunc, memory.get(), effectiveAddress, opVal);
        break;
    case Enu::EAddrMode::SFX:
        effectiveAddress = operand + getCPURegWordCurrent(Enu::CPURegisters::SP);
        rVal  = std::invoke(readFunc, memory.get(), effectiveAddress, tempByteHi);
        rVal  = std::invoke(readFunc, memory.get(), effectiveAddress + 1, tempByteLo);
        effectiveAddress = tempByteHi << 8 | tempByteLo;
        effectiveAddress += getCPURegWordCurrent(Enu::CPURegisters::X);
        rVal  &= std::invoke(readFunc, memory.get(), effectiveAddress, opVal);
        break;
    default:
        break;
    }
    return rVal;
}

bool IsaCpu::writeOperandWord(quint16 operand, quint16 value, Enu::EAddrMode addrMode)
{
    bool rVal = true;
    quint16 effectiveAddress = 0;
    switch(addrMode) {
    case Enu::EAddrMode::I:
        rVal = false;
        break;
    case Enu::EAddrMode::D:
        effectiveAddress = operand;
        rVal = memory->writeWord(effectiveAddress, value);
        break;
    case Enu::EAddrMode::S:
        effectiveAddress = operand + getCPURegWordCurrent(Enu::CPURegisters::SP);
        rVal = memory->writeWord(effectiveAddress, value);
        break;
    case Enu::EAddrMode::X:
        effectiveAddress = operand + getCPURegWordCurrent(Enu::CPURegisters::X);
        rVal = memory->writeWord(effectiveAddress, value);
        break;
    case Enu::EAddrMode::SX:
        effectiveAddress = operand
                + getCPURegWordCurrent(Enu::CPURegisters::SP)
                + getCPURegWordCurrent(Enu::CPURegisters::X);
        rVal = memory->writeWord(effectiveAddress, value);
        break;
    case Enu::EAddrMode::N:
        effectiveAddress = operand;
        rVal = memory->readWord(effectiveAddress, effectiveAddress);
        rVal &= memory->writeWord(effectiveAddress, value);
        break;
    case Enu::EAddrMode::SF:
        effectiveAddress = operand + getCPURegWordCurrent(Enu::CPURegisters::SP);
        rVal = memory->readWord(effectiveAddress, effectiveAddress);
        rVal &= memory->writeWord(effectiveAddress, value);
        break;
    case Enu::EAddrMode::SFX:
        effectiveAddress = operand + getCPURegWordCurrent(Enu::CPURegisters::SP);
        rVal = memory->readWord(effectiveAddress, effectiveAddress);
        effectiveAddress += getCPURegWordCurrent(Enu::CPURegisters::X);
        rVal &= memory->writeWord(effectiveAddress, value);
        break;
    }
    return rVal;
}

bool IsaCpu::writeOperandByte(quint16 operand, quint8 value, Enu::EAddrMode addrMode)
{
    bool rVal = true;
    quint16 effectiveAddress = 0;
    switch(addrMode) {
    case Enu::EAddrMode::I:
        rVal = false;
        break;
    case Enu::EAddrMode::D:
        effectiveAddress = operand;
        rVal = memory->writeByte(effectiveAddress, value);
        break;
    case Enu::EAddrMode::S:
        effectiveAddress = operand + getCPURegWordCurrent(Enu::CPURegisters::SP);
        rVal = memory->writeByte(effectiveAddress, value);
        break;
    case Enu::EAddrMode::X:
        effectiveAddress = operand + getCPURegWordCurrent(Enu::CPURegisters::X);
        rVal = memory->writeByte(effectiveAddress, value);
        break;
    case Enu::EAddrMode::SX:
        effectiveAddress = operand
                + getCPURegWordCurrent(Enu::CPURegisters::SP)
                + getCPURegWordCurrent(Enu::CPURegisters::X);
        rVal = memory->writeByte(effectiveAddress, value);
        break;
    case Enu::EAddrMode::N:
        effectiveAddress = operand;
        rVal = memory->readWord(effectiveAddress, effectiveAddress);
        rVal &= memory->writeByte(effectiveAddress, value);
        break;
    case Enu::EAddrMode::SF:
        effectiveAddress = operand + getCPURegWordCurrent(Enu::CPURegisters::SP);
        rVal = memory->readWord(effectiveAddress, effectiveAddress);
        rVal &= memory->writeByte(effectiveAddress, value);
        break;
    case Enu::EAddrMode::SFX:
        effectiveAddress = operand + getCPURegWordCurrent(Enu::CPURegisters::SP);
        rVal = memory->readWord(effectiveAddress, effectiveAddress);
        effectiveAddress += getCPURegWordCurrent(Enu::CPURegisters::X);
        rVal &= memory->writeByte(effectiveAddress, value);
        break;
    }
    return rVal;
}

void IsaCpu::executeUnary(Enu::EMnemonic mnemon)
{
    quint16 temp, sp, acc, idx;
    sp = registerBank.readRegisterWordCurrent(Enu::CPURegisters::SP);
    acc = registerBank.readRegisterWordCurrent(Enu::CPURegisters::A);
    idx = registerBank.readRegisterWordCurrent(Enu::CPURegisters::X);

    switch(mnemon) {
    case Enu::EMnemonic::STOP:
        executionFinished = true;
        break;

    case Enu::EMnemonic::RET:
        memory->readWord(sp, temp);
        registerBank.writeRegisterWord(Enu::CPURegisters::PC, temp);
        sp += 2;
        registerBank.writeRegisterWord(Enu::CPURegisters::SP, sp);
        break;

    case Enu::EMnemonic::RETTR:
        memory->readWord(sp, temp);
        // Function will automatically mask out bits that don't matter
        registerBank.writeStatusBits(temp);
        memory->readWord(sp + 1, temp);
        registerBank.writeRegisterWord(Enu::CPURegisters::A, temp);
        memory->readWord(sp + 3, temp);
        registerBank.writeRegisterWord(Enu::CPURegisters::X, temp);
        memory->readWord(sp + 5, temp);
        registerBank.writeRegisterWord(Enu::CPURegisters::PC, temp);
        memory->readWord(sp + 7, temp);
        registerBank.writeRegisterWord(Enu::CPURegisters::SP, temp);
        break;

    case Enu::EMnemonic::MOVSPA:
        registerBank.writeRegisterWord(Enu::CPURegisters::A, sp);
        break;

    case Enu::EMnemonic::MOVFLGA:
        registerBank.writeRegisterWord(Enu::CPURegisters::A, registerBank.readStatusBitsCurrent());
        break;

    case Enu::EMnemonic::MOVAFLG:
        // Only move the low order byte of accumulator to the status bits.
        registerBank.writeStatusBits(static_cast<quint8>(acc));
        break;

    case Enu::EMnemonic::NOTA: // Modifies NZ bits
        acc = ~acc;
        registerBank.writeRegisterWord(Enu::CPURegisters::A, acc);
        // Is negative if high order bit is 1.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_N, acc & 0x8000);
         // Is zero if all bits are 0's.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_Z, acc == 0);
        break;

    case Enu::EMnemonic::NOTX: // Modifies NZ bits
        idx = ~idx;
        registerBank.writeRegisterWord(Enu::CPURegisters::X, idx);
        // Is negative if high order bit is 1.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_N, idx & 0x8000);
         // Is zero if all bits are 0's.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_Z, idx == 0);
        break;

    case Enu::EMnemonic::NEGA: // Modifies NZV bits
        acc = ~acc + 1;
        registerBank.writeRegisterWord(Enu::CPURegisters::A, acc);
        // Is negative if high order bit is 1.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_N, acc & 0x8000);
         // Is zero if all bits are 0's.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_Z, acc == 0);
        // Only a signed overflow if register is 0x8000.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_V, acc == 0x8000);
        break;

    case Enu::EMnemonic::NEGX: // Modifies NZV bits
        idx = ~idx + 1;
        registerBank.writeRegisterWord(Enu::CPURegisters::X, idx);
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
        registerBank.writeRegisterWord(Enu::CPURegisters::A, temp);
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
        registerBank.writeRegisterWord(Enu::CPURegisters::X, temp);
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
        registerBank.writeRegisterWord(Enu::CPURegisters::A, temp);
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
        registerBank.writeRegisterWord(Enu::CPURegisters::X, temp);
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
        registerBank.writeRegisterWord(Enu::CPURegisters::A, temp);
        // Carry out if register starts with low order 1.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_C, acc & 0x01);
        break;

    case Enu::EMnemonic::RORX: // Modifies C bit
        temp = static_cast<quint16>(idx >> 1
                                    // Shift the carry to high order bit.
                                    | (registerBank.readStatusBitCurrent(Enu::EStatusBit::STATUS_C)
                                        ? 1<<15 : 0));
        registerBank.writeRegisterWord(Enu::CPURegisters::X, temp);
        // Carry out if register starts with low order 1.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_C, idx & 0x01);
        break;

    case Enu::EMnemonic::ROLA: // Modifies C bit
        temp = static_cast<quint16>(acc << 1
                                    // Shift the carry in to low order bit.
                                    | (registerBank.readStatusBitCurrent(Enu::EStatusBit::STATUS_C)
                                        ? 1 : 0));
        registerBank.writeRegisterWord(Enu::CPURegisters::A, temp);
        // Carry out if register starts with high order 1.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_C, acc & 0x8000);
        break;

    case Enu::EMnemonic::ROLX: // Modifies C bit
        temp = static_cast<quint16>(acc << 1
                                    // Shift the carry in to low order bit.
                                    | (registerBank.readStatusBitCurrent(Enu::EStatusBit::STATUS_C)
                                        ? 1 : 0));
        registerBank.writeRegisterWord(Enu::CPURegisters::X, temp);
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_C, idx & 0x8000);
        break;

    case Enu::EMnemonic::NOP0:
        if(Pep::isTrapMap[Enu::EMnemonic::NOP0]) {
            controlError = true;
            executionFinished = true;
            errorMessage = "NOP0 is not a unary instruction.";
        }
        break;
    default:
        // Should never occur, but gaurd against to make compiler happy.
        controlError = true;
        executionFinished = true;
        errorMessage = "Attempt to execute invalid unary instruction";
        return;
    }
}

void IsaCpu::executeNonunary(Enu::EMnemonic mnemon, quint16 opSpec, Enu::EAddrMode addrMode)
{
    quint16 tempWord, a, x, sp, result;
    quint8 tempByte;
    a = registerBank.readRegisterWordCurrent(Enu::CPURegisters::A);
    x = registerBank.readRegisterWordCurrent(Enu::CPURegisters::X);
    sp = registerBank.readRegisterWordCurrent(Enu::CPURegisters::SP);
    switch(mnemon) {

    case Enu::EMnemonic::BR:
        readOperandWordValue(opSpec, addrMode, tempWord);
        registerBank.writeRegisterWord(Enu::CPURegisters::PC, tempWord);
        break;

    case Enu::EMnemonic::BRLE:
        if(registerBank.readStatusBitCurrent(Enu::EStatusBit::STATUS_N) ||
                registerBank.readStatusBitCurrent(Enu::EStatusBit::STATUS_Z)) {
            readOperandWordValue(opSpec, addrMode, tempWord);
            registerBank.writeRegisterWord(Enu::CPURegisters::PC, tempWord);
        }
        break;

    case Enu::EMnemonic::BRLT:
        if(registerBank.readStatusBitCurrent(Enu::EStatusBit::STATUS_N)) {
            readOperandWordValue(opSpec, addrMode, tempWord);
            registerBank.writeRegisterWord(Enu::CPURegisters::PC, tempWord);
        }
        break;

    case Enu::EMnemonic::BREQ:
        if(registerBank.readStatusBitCurrent(Enu::EStatusBit::STATUS_Z)) {
            readOperandWordValue(opSpec, addrMode, tempWord);
            registerBank.writeRegisterWord(Enu::CPURegisters::PC, tempWord);
        }
        break;

    case Enu::EMnemonic::BRNE:
        if(!registerBank.readStatusBitCurrent(Enu::EStatusBit::STATUS_Z)) {
            readOperandWordValue(opSpec, addrMode, tempWord);
            registerBank.writeRegisterWord(Enu::CPURegisters::PC, tempWord);
        }
        break;

    case Enu::EMnemonic::BRGE:
        if(!registerBank.readStatusBitCurrent(Enu::EStatusBit::STATUS_N)) {
            readOperandWordValue(opSpec, addrMode, tempWord);
            registerBank.writeRegisterWord(Enu::CPURegisters::PC, tempWord);
        }
        break;

    case Enu::EMnemonic::BRGT:
        if(!registerBank.readStatusBitCurrent(Enu::EStatusBit::STATUS_N) &&
                !registerBank.readStatusBitCurrent(Enu::EStatusBit::STATUS_Z)) {
            readOperandWordValue(opSpec, addrMode, tempWord);
            registerBank.writeRegisterWord(Enu::CPURegisters::PC, tempWord);
        }
        break;

    case Enu::EMnemonic::BRV:
        if(registerBank.readStatusBitCurrent(Enu::EStatusBit::STATUS_V)) {
            readOperandWordValue(opSpec, addrMode, tempWord);
            registerBank.writeRegisterWord(Enu::CPURegisters::PC, tempWord);
        }
        break;

    case Enu::EMnemonic::BRC:
        if(registerBank.readStatusBitCurrent(Enu::EStatusBit::STATUS_C)) {
            readOperandWordValue(opSpec, addrMode, tempWord);
            registerBank.writeRegisterWord(Enu::CPURegisters::PC, tempWord);
        }
        break;

    case Enu::EMnemonic::CALL:
        readOperandWordValue(opSpec, addrMode, tempWord);
        sp -= 2;
        memory->writeWord(sp, registerBank.readRegisterWordCurrent(Enu::CPURegisters::PC));
        registerBank.writeRegisterWord(Enu::CPURegisters::PC, tempWord);
        registerBank.writeRegisterWord(Enu::CPURegisters::SP, sp);
        break;

    case Enu::EMnemonic::ADDSP:
        readOperandWordValue(opSpec, addrMode, tempWord);
        registerBank.writeRegisterWord(Enu::CPURegisters::SP, sp + tempWord);
        break;

    case Enu::EMnemonic::SUBSP:
        readOperandWordValue(opSpec, addrMode, tempWord);
        registerBank.writeRegisterWord(Enu::CPURegisters::SP, sp - tempWord);
        break;

#pragma message ("first to fix")
    case Enu::EMnemonic::ADDA:
        readOperandWordValue(opSpec, addrMode, tempWord);
        // The result is the decode operand specifier plus the accumulator
        result = a + tempWord;
        registerBank.writeRegisterWord(Enu::CPURegisters::A, result);
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
        readOperandWordValue(opSpec, addrMode, tempWord);
        // The result is the decode operand specifier plus the index reg.
        result = x + tempWord;
        registerBank.writeRegisterWord(Enu::CPURegisters::X, result);
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
        readOperandWordValue(opSpec, addrMode, tempWord);
        // The result is the decode operand specifier minus the accumulator.
        result = a - tempWord;
        registerBank.writeRegisterWord(Enu::CPURegisters::A, result);
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
        readOperandWordValue(opSpec, addrMode, tempWord);
        // The result is the decode operand specifier minus the index reg.
        result = x - tempWord;
        registerBank.writeRegisterWord(Enu::CPURegisters::X, result);
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
        readOperandWordValue(opSpec, addrMode, tempWord);
        // The result is the decode operand specifier bitwise and'ed with the accumulator.
        result = a & tempWord;
        registerBank.writeRegisterWord(Enu::CPURegisters::A, result);
        // Is negative if high order bit is 1.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_N, result & 0x8000);
         // Is zero if all bits are 0's.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_Z, result == 0);
        break;

    case Enu::EMnemonic::ANDX:
        readOperandWordValue(opSpec, addrMode, tempWord);
        // The result is the decode operand specifier bitwise and'ed the index reg.
        result = x & tempWord;
        registerBank.writeRegisterWord(Enu::CPURegisters::X, result);
        // Is negative if high order bit is 1.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_N, result & 0x8000);
         // Is zero if all bits are 0's.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_Z, result == 0);
        break;

    case Enu::EMnemonic::ORA:
        readOperandWordValue(opSpec, addrMode, tempWord);
        // The result is the decode operand specifier bitwise or'ed with the accumulator.
        result = a | tempWord;
        registerBank.writeRegisterWord(Enu::CPURegisters::A, result);
        // Is negative if high order bit is 1.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_N, result & 0x8000);
         // Is zero if all bits are 0's.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_Z, result == 0);
        break;
    case Enu::EMnemonic::ORX:
        readOperandWordValue(opSpec, addrMode, tempWord);
        // The result is the decode operand specifier bitwise or'ed the index reg.
        result = x | tempWord;
        registerBank.writeRegisterWord(Enu::CPURegisters::X, result);
        // Is negative if high order bit is 1.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_N, result & 0x8000);
         // Is zero if all bits are 0's.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_Z, result == 0);
        break;

    case Enu::EMnemonic::CPWA:
        readOperandWordValue(opSpec, addrMode, tempWord);
        // The result is the decode operand specifier minus the index reg.
        result = a - tempWord;
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
        // If there was a signed overflow, selectively invert N bit.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_N, registerBank.readStatusBitCurrent(Enu::EStatusBit::STATUS_N)
                                    ^ registerBank.readStatusBitCurrent(Enu::EStatusBit::STATUS_V));
        break;

    case Enu::EMnemonic::CPWX:
        readOperandWordValue(opSpec, addrMode, tempWord);
        // The result is the decode operand specifier minus the index reg.
        result = x - tempWord;
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
        // If there was a signed overflow, selectively invert N bit.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_N, registerBank.readStatusBitCurrent(Enu::EStatusBit::STATUS_N)
                                    ^ registerBank.readStatusBitCurrent(Enu::EStatusBit::STATUS_V));
        break;

    case Enu::EMnemonic::LDWA:
        readOperandWordValue(opSpec, addrMode, tempWord);
        registerBank.writeRegisterWord(Enu::CPURegisters::A, tempWord);
        // Is negative if high order bit is 1.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_N, tempWord & 0x8000);
         // Is zero if all bits are 0's.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_Z, tempWord == 0);
        break;

    case Enu::EMnemonic::LDWX:
        readOperandWordValue(opSpec, addrMode, tempWord);
        registerBank.writeRegisterWord(Enu::CPURegisters::X, tempWord);
        // Is negative if high order bit is 1.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_N, tempWord & 0x8000);
         // Is zero if all bits are 0's.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_Z, tempWord == 0);
        break;

    case Enu::EMnemonic::STWA:
        tempWord = registerBank.readRegisterWordCurrent(Enu::CPURegisters::A);
        writeOperandWord(opSpec, tempWord, addrMode);
        break;

    case Enu::EMnemonic::STWX:
        tempWord = registerBank.readRegisterWordCurrent(Enu::CPURegisters::X);
        writeOperandWord(opSpec, tempWord, addrMode);
        break;

    // Single byte instructions
    case Enu::EMnemonic::CPBA:
        readOperandByteValue(opSpec, addrMode, tempByte);
        // The result is the decode operand specifier minus the accumulator.
        // Narrow a and operand to 1 byte before widening to 2 bytes.
        result = (a & 0xff) - (tempByte & 0xff);
        // Is negative if high order bit is 1.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_N, result & 0x8000);
         // Is zero if all bits are 0's.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_Z, result == 0);
        // RTL specifies zeroing out V, C bits.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_V, false);
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_C, false);
        break;

    case Enu::EMnemonic::CPBX:
        readOperandByteValue(opSpec, addrMode, tempByte);
        // The result is the decode operand specifier minus the index reg.
        // Narrow a and operand to 1 byte before widening to 2 bytes.
        result = (x & 0xff) - (tempByte & 0xff);
        // Is negative if high order bit is 1.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_N, result & 0x8000);
         // Is zero if all bits are 0's.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_Z, result == 0);
        // RTL specifies zeroing out V, C bits.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_V, false);
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_C, false);
        break;

    case Enu::EMnemonic::LDBA:
        readOperandByteValue(opSpec, addrMode, tempByte);
        tempWord = a & 0xff00;
        tempWord |= tempByte;
        registerBank.writeRegisterWord(Enu::CPURegisters::A, tempWord);
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_N, false);
         // Is zero if all bits are 0's.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_Z, (tempWord & 0xff) == 0);

        break;

    case Enu::EMnemonic::LDBX:
        readOperandByteValue(opSpec, addrMode, tempByte);
        tempWord = x & 0xff00;
        tempWord |= tempByte;
        registerBank.writeRegisterWord(Enu::CPURegisters::X, tempWord);
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_N, false);
         // Is zero if all bits are 0's.
        registerBank.writeStatusBit(Enu::EStatusBit::STATUS_Z, (tempWord & 0xff) == 0);
        break;

    case Enu::EMnemonic::STBA:
        tempByte = static_cast<quint8>(0xff & registerBank.readRegisterWordCurrent(Enu::CPURegisters::A));
        writeOperandByte(opSpec, tempByte, addrMode);
        break;

    case Enu::EMnemonic::STBX:
        tempByte = static_cast<quint8>(0xff & registerBank.readRegisterWordCurrent(Enu::CPURegisters::X));
        writeOperandByte(opSpec, tempByte, addrMode);
        break;
    }
}

void IsaCpu::executeTrap(Enu::EMnemonic mnemon)
{
    quint16 pc;
    // The
    quint16 tempAddr, temp = manager->getOperatingSystem()->getBurnValue() - 9;
    memory->readWord(temp, tempAddr);
    quint16 pcAddr = manager->getOperatingSystem()->getBurnValue() - 1;
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
        // though not part of the specification, the Pep9 hardware must increment the program counter
        // in order for non-unary traps to function correctly
        pc = registerBank.readRegisterWordCurrent(Enu::CPURegisters::PC) + 2;
        registerBank.writeRegisterWord(Enu::CPURegisters::PC, pc);
#endif
        [[fallthrough]];
    // Unary traps
    case Enu::EMnemonic::NOP0:;
        [[fallthrough]];
    case Enu::EMnemonic::NOP1:;
        // Writes to mem[T-1].
        memory->writeByte(tempAddr - 1, registerBank.readRegisterByteCurrent(Enu::CPURegisters::IS) /*IS*/);
        // Writes to mem[T-2], mem[T-3].
        memory->writeWord(tempAddr - 3, registerBank.readRegisterWordCurrent(Enu::CPURegisters::SP) /*SP*/);
        // Writes to mem[T-4], mem[T-5].
        memory->writeWord(tempAddr - 5, registerBank.readRegisterWordCurrent(Enu::CPURegisters::PC) /*PC*/);
        // Writes to mem[T-6], mem[T-7].
        memory->writeWord(tempAddr - 7, registerBank.readRegisterWordCurrent(Enu::CPURegisters::X) /*X*/);
        // Writes to mem[T-8], mem[T-9].
        memory->writeWord(tempAddr - 9, registerBank.readRegisterWordCurrent(Enu::CPURegisters::A) /*A*/);
        // Writes to mem[T-10].
        memory->writeByte(tempAddr - 10, registerBank.readStatusBitsCurrent() /*NZVC*/);
        memory->readWord(pcAddr, pc);
        registerBank.writeRegisterWord(Enu::CPURegisters::SP, tempAddr - 10);
        registerBank.writeRegisterWord(Enu::CPURegisters::PC, pc);
#if performTrapFix
        // Though not part of the specification, clear out the index register to
        // prevent bug in OS where non-unary instructions fail due to junk
        // in the high order byte of the index register. The book is published,
        // so we have to fix it here.
        registerBank.writeRegisterWord(Enu::CPURegisters::X, 0);
#endif
    }
}

void IsaCpu::breakpointHandler()
{
    // If the CPU is not being debugged, breakpoints make no sense. Abort.
    if(!inDebug) return;
    else if(breakpointsISA.contains(registerBank.readRegisterWordCurrent(Enu::CPURegisters::PC))) {
        asmBreakpointHit = true;
        emit hitBreakpoint(Enu::BreakpointTypes::ASSEMBLER);
        return;
    }
    else {
        asmBreakpointHit = false;
    }
}
