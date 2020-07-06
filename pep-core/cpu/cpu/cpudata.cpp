// File: cpudata.cpp
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
#include "cpudata.h"

#include <stdexcept>
#include <string>

#include "cpu/registerfile.h"
#include "memory/amemorydevice.h"
#include "microassembler/microcode.h"
#include "microassembler/microcodeprogram.h"
#include "pep/pep.h"
CPUDataSection::CPUDataSection(Enu::CPUType type, QSharedPointer<AMemoryDevice> memDev, QObject *parent): QObject(parent), memDevice(memDev),
    cpuFeatures(type), mainBusState(Enu::None),
    registerBank(QSharedPointer<RegisterFile>::create()), memoryRegisters(6), controlSignals(Pep::numControlSignals()),
    clockSignals(Pep::numClockSignals()), emitEvents(true), hadDataError(false), errorMessage(""),
    isALUCacheValid(false), ALUHasOutputCache(false), ALUOutputCache(0), ALUStatusBitCache(0)
{
    presetStaticRegisters();
}

CPUDataSection::~CPUDataSection()
{
    //This code should not be called during the normal lifetime of Pep9CPU
}

bool CPUDataSection::aluFnIsUnary() const
{
    //The only alu functions that are unary are 0 & 10..15
    return controlSignals[Enu::ALU] == 0 || controlSignals[Enu::ALU] >= 10;
}

bool CPUDataSection::getAMuxOutput(quint8& result) const
{
        if(controlSignals[Enu::AMux] == 0 && cpuFeatures == Enu::CPUType::TwoByteDataBus) {
            //Which could come from MDRE when EOMux is 0
            if(controlSignals[Enu::EOMux] == 0) {
                result = memoryRegisters[Enu::MEM_MDRE];
                return true;
            }
            //Or comes from MDRO if EOMux is 1
            else if(controlSignals[Enu::EOMux] == 1) {
                result = memoryRegisters[Enu::MEM_MDRO];
                return true;
            }
            //Or has no has no output when EOMux is disabled
            else return false;

        }
        else if(controlSignals[Enu::AMux] == 0 && cpuFeatures == Enu::CPUType::OneByteDataBus) {
            result = memoryRegisters[Enu::MEM_MDR];
            return true;
        }
        else if(controlSignals[Enu::AMux] == 1) {
            return valueOnABus(result);
        }
        else return false;
}

bool CPUDataSection::calculateCSMuxOutput(bool &result) const
{
    //CSMux either outputs C when CS is 0
    if(controlSignals[Enu::CSMux]==0) {
        result = registerBank->readStatusBitsCurrent() & Enu::CMask;
        return true;
    }
    //Or outputs S when CS is 1
    else if(controlSignals[Enu::CSMux] == 1)  {
        result = registerBank->readStatusBitsCurrent() & Enu::SMask;
        return true;
    }
    //Otherwise it does not have valid output
    else return false;
}

bool CPUDataSection::calculateALUOutput(quint8 &res, quint8 &NZVC) const
{
    /*
     * Profiling determined calculateALUOutput(...) to be the most computationally intensive part of the program.
     * This function is used multiple times per cycle, so we cache the result for increased performance.
     */
    if(isALUCacheValid) {
        res = ALUOutputCache;
        NZVC = ALUStatusBitCache;
        return ALUHasOutputCache;
    }
    // This function should not set any errors.
    // Errors will be handled by step(..)
    quint8 a, b;
    bool carryIn = false;
    bool hasA = getAMuxOutput(a), hasB = valueOnBBus(b);
    bool hasCIn = calculateCSMuxOutput(carryIn);
    if(!((aluFnIsUnary() && hasA) || (hasA && hasB))) {
        // The ALU output calculation would not be meaningful given its current function and inputs
        isALUCacheValid = true;
        ALUHasOutputCache = false;
        return ALUHasOutputCache;
    }
    // Unless otherwise noted, do not return true (sucessfully) early, or the calculation for the NZ bits will be skipped.
    switch(controlSignals[Enu::ALU]) {
    case Enu::A_func: // A
        res = a;
        break;
    case Enu::ApB_func: // A plus B
        res = a + b;
        NZVC |= Enu::CMask * quint8{res<a||res<b}; // Carry out if result is unsigned less than a or b.
        // There is a signed overflow iff the high order bits of the input are the same,
        // and the inputs & output differs in sign.
        // Shifts in 0's (unsigned chars), so after shift, only high order bit remain.
        NZVC |= Enu::VMask * ((~(a ^ b) & (a ^ res)) >> 7) ;
        break;
    case Enu::ApnBp1_func: // A plus ~B plus 1
        hasCIn = true;
        carryIn = 1;
        [[fallthrough]];
    case Enu::ApnBpCin_func: // A plus ~B plus Cin
        // Clang thinks this is a garbage value. It isn't.
        // Lots of "errors" spawn from this, but this is well-defined behavior.
        b = ~b;
        [[fallthrough]];
    case Enu::ApBpCin_func: // A plus B plus Cin
        // Expected carry in, none was provided, so ALU calculation yeilds a meaningless result
        if (!hasCIn) return false;
        // Might cause overflow, but overflow is well defined for unsigned ints
        res = a + b + quint8{carryIn};
        NZVC |= Enu::CMask * quint8{res<a||res<b}; // Carry out if result is unsigned less than a or b.
        // There is a signed overflow iff the high order bits of the input are the same,
        // and the inputs & output differs in sign.
        // Shifts in 0's (unsigned chars), so after shift, only high order bit remain.
        NZVC |= Enu::VMask * ((~(a ^ b) & (a ^ res)) >> 7) ;
        break;
    case Enu::AandB_func: // A * B
        res = a & b;
        break;
    case Enu::nAandB_func: // ~(A * B)
        res = ~(a & b);
        break;
    case Enu::AorB_func: // A + B
        res = a | b;
        break;
    case Enu::nAorB_func: // ~(A + B)
        res = ~(a | b);
        break;
    case Enu::AxorB_func: // A xor B
        res = a ^ b;
        break;
    case Enu::nA_func: // ~A
        res = ~a;
        break;
    case Enu::ASLA_func: // ASL A
        res = static_cast<quint8>(a<<1);
        NZVC |= Enu::CMask * ((a & 0x80) >> 7); // Carry out equals the hi order bit
        NZVC |= Enu::VMask * (((a << 1) ^ a) >>7); // Signed overflow if a<hi> doesn't match a<hi-1>
        break;
    case Enu::ROLA_func: // ROL A
        if (!hasCIn) return false;
        res = static_cast<quint8>(a<<1 | quint8{carryIn});
        NZVC |= Enu::CMask * ((a & 0x80) >> 7); // Carry out equals the hi order bit
        NZVC |= Enu::VMask * (((a << 1) ^a) >>7); // Signed overflow if a<hi> doesn't match a<hi-1>
        break;
    case Enu::ASRA_func: // ASR A
        hasCIn = true;
        carryIn = a & 128; // RORA and ASRA only differ by how the carryIn is calculated
        [[fallthrough]];
    case Enu::RORA_func: // ROR a
        if (!hasCIn) return false;
        // A will not be sign extended since it is unsigned.
        // Widen carryIn so that << yields a meaningful result.
        res = static_cast<quint8>(a >> 1 | static_cast<quint8>(carryIn) << 7);
        // Carry out is lowest order bit of a
        NZVC |= Enu::CMask * (a & 1);
        break;
    case Enu::NZVCA_func: // Move A to NZVC
        res = 0;
        NZVC |= Enu::NMask & a;
        NZVC |= Enu::ZMask & a;
        NZVC |= Enu::VMask & a;
        NZVC |= Enu::CMask & a;
        return true; // Must return early to avoid NZ calculation
    default: // If the default has been hit, then an invalid function was selected
        return false;
    }
    // Calculate N, then shift to correct position
    NZVC |= (res & 0x80) ? Enu::NMask : 0; // Result is negative if high order bit is 1
    // Calculate Z, then shift to correct position
    NZVC |= (res == 0) ? Enu::ZMask : 0;
    // Save the result of the ALU calculation
    ALUOutputCache = res;
    ALUStatusBitCache = NZVC;
    isALUCacheValid = true;
    ALUHasOutputCache = true;
    return ALUHasOutputCache;

}

Enu::CPUType CPUDataSection::getCPUType() const
{
    return cpuFeatures;
}

RegisterFile &CPUDataSection::getRegisterBank()
{
    return *registerBank.get();
}

const RegisterFile &CPUDataSection::getRegisterBank() const
{
    return *registerBank.get();
}

quint8 CPUDataSection::getRegisterBankByte(quint8 registerNumber) const
{
    return registerBank->readRegisterByteCurrent(registerNumber);
}

quint16 CPUDataSection::getRegisterBankWord(quint8 registerNumber) const
{
    return registerBank->readRegisterWordCurrent(registerNumber);
}

quint8 CPUDataSection::getRegisterBankByte(Enu::CPURegisters registerNumber) const
{
    return registerBank->readRegisterByteCurrent(registerNumber);
}

quint16 CPUDataSection::getRegisterBankWord(Enu::CPURegisters registerNumber) const
{
    return registerBank->readRegisterWordCurrent(registerNumber);
}

quint8 CPUDataSection::getMemoryRegister(Enu::EMemoryRegisters registerNumber) const
{
    return memoryRegisters[registerNumber];
}

bool CPUDataSection::valueOnABus(quint8 &result) const
{
    if(controlSignals[Enu::A] == Enu::signalDisabled) return false;
    result = getRegisterBankByte(controlSignals[Enu::A]);
    return true;
}

bool CPUDataSection::valueOnBBus(quint8 &result) const
{
    if(controlSignals[Enu::B] == Enu::signalDisabled) return false;
    result = getRegisterBankByte(controlSignals[Enu::B]);
    return true;
}

bool CPUDataSection::valueOnCBus(quint8 &result) const
{
    if(controlSignals[Enu::CMux] == 0) {
        // If CMux is 0, then the NZVC bits (minus S) are directly routed to result
        result = (registerBank->readStatusBitsCurrent() & (~Enu::SMask));
        return true;
    }
    else if(controlSignals[Enu::CMux] == 1) {
        quint8 temp = 0; // Discard NZVC bits for this calculation, they are unecessary for calculating C's output
        // Otherwise the value of C depends solely on the ALU
        return calculateALUOutput(result, temp);
    }
    else return false;
}

Enu::MainBusState CPUDataSection::getMainBusState() const
{
    return mainBusState;
}

bool CPUDataSection::getStatusBit(Enu::EStatusBit statusBit) const
{
    quint8 NZVCSbits = registerBank->readStatusBitsCurrent();
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

void CPUDataSection::onSetStatusBit(Enu::EStatusBit statusBit, bool val)
{
    bool oldVal = false;

    // Mask out the original value, then or it with the properly shifted bit
    oldVal = registerBank->readStatusBitCurrent(statusBit);
    registerBank->writeStatusBit(statusBit, val);
    if(emitEvents) {
        if(oldVal != val) emit statusBitChanged(statusBit, val);
    }
}

void CPUDataSection::onSetRegisterByte(quint8 reg, quint8 val)
{
    if(reg > 21) return; // Don't allow static registers to be written to
    quint8 oldVal = getRegisterBankByte(reg);
    registerBank->writeRegisterByte(reg, val);
    if(emitEvents) {
        if(oldVal != val) emit registerChanged(reg, oldVal, val);
    }
}

void CPUDataSection::onSetRegisterWord(quint8 reg, quint16 val)
{
   if(reg + 1 >21) return; // Don't allow static registers to be written to
   quint8 oldHigh = getRegisterBankByte(reg), oldLow = getRegisterBankByte(reg+1);
   quint8 newHigh = val / 256, newLow = val & quint8{255};
   registerBank->writeRegisterByte(reg, newHigh);
   registerBank->writeRegisterByte(reg + 1, newLow);
   if(emitEvents) {
       if(oldHigh != val) emit registerChanged(reg, oldHigh, newHigh);
       if(oldLow != val) emit registerChanged(reg, oldLow, newLow);
   }
}

void CPUDataSection::onSetMemoryRegister(Enu::EMemoryRegisters reg, quint8 val)
{
    quint8 oldVal = memoryRegisters[reg];
    memoryRegisters[reg] = val;
    if(emitEvents) {
    if(oldVal != val) emit memoryRegisterChanged(reg, oldVal, val);
    }
}

void CPUDataSection::onSetClock(Enu::EClockSignals clock, bool value)
{
    clockSignals[clock] = value;
}

void CPUDataSection::onSetControlSignal(Enu::EControlSignals control, quint8 value)
{
    controlSignals[control] = value;
}

bool CPUDataSection::setSignalsFromMicrocode(const MicroCode *line)
{ 
    /*
     * Verify that both arrays of control signals are the same length,
     * so that memcpy is safe. Otherwise, raise an exception that must be dealt with
     * informing the simulation that the CPU has an unrecoverable error.
     *
     * Memcpy was used instead of a for loop, as the original loop was profiled
     * to be the most time consuming piece of the simulation. Memcpy yielded a ~30%
     * increase in perfomance.
     */
    if(controlSignals.length() == line->getControlSignals().length()) {
        // Memcpy is safe as long as both arrays match in size.
        memcpy(controlSignals.data(),
               line->getControlSignals().data(),
               static_cast<std::size_t>(controlSignals.length()));
    }
    else {
        hadDataError = true;
        errorMessage = "Control signals did not match in length";
        throw std::invalid_argument("Argument's # of control signals did not match internal \
        number of control signals");
    }

    // Same verification as described above, except for clock signals.
    if(clockSignals.length() == line->getClockSignals().length()) {
        // Memcpy is safe as long as both arrays match in size.
        memcpy(clockSignals.data(),
               line->getClockSignals().data(),
               static_cast<std::size_t>(clockSignals.length()));
    }
    else {
        hadDataError = true;
        errorMessage = "Clock signals did not match in length";
        throw std::invalid_argument("Argument's # of clock signals did not match internal \
        number of clock signals");
    }
    return true;
}

void CPUDataSection::setEmitEvents(bool b)
{
    emitEvents = b;
}

bool CPUDataSection::hadErrorOnStep() const
{
    return hadDataError;
}

QString CPUDataSection::getErrorMessage() const
{
    return errorMessage;
}

void CPUDataSection::handleMainBusState() noexcept
{
    bool marChanged = false;
    quint8 a, b;
    if(clockSignals[Enu::MARCk] && valueOnABus(a) && valueOnBBus(b)) {
        marChanged = !(a == memoryRegisters[Enu::MEM_MARA] && b == memoryRegisters[Enu::MEM_MARB]);
    }
    switch(mainBusState)
    {
    case Enu::None:
        //One cannot change MAR contents and initiate a R/W on same cycle
        if(!marChanged) {
            if(controlSignals[Enu::MemRead] == 1) mainBusState = Enu::MemReadFirstWait;
            else if(controlSignals[Enu::MemWrite] == 1) mainBusState = Enu::MemWriteFirstWait;
        }
        break;
    case Enu::MemReadFirstWait:
        if(!marChanged && controlSignals[Enu::MemRead] == 1) mainBusState = Enu::MemReadSecondWait;
        else if(marChanged && controlSignals[Enu::MemRead] == 1); //Initiating a new read brings us back to first wait
        else if(controlSignals[Enu::MemWrite] == 1) mainBusState = Enu::MemWriteFirstWait; //Switch from read to write.
        else mainBusState = Enu::None; //If neither are check, bus goes back to doing nothing
        break;
    case Enu::MemReadSecondWait:
        if(!marChanged && controlSignals[Enu::MemRead] == 1) mainBusState = Enu::MemReadReady;
        else if(marChanged && controlSignals[Enu::MemRead] == 1) mainBusState = Enu::MemReadFirstWait;
        else if(controlSignals[Enu::MemWrite] == 1) mainBusState = Enu::MemWriteFirstWait;
        else mainBusState = Enu::None; //If neither are check, bus goes back to doing nothing
        break;
    case Enu::MemReadReady:
        if(controlSignals[Enu::MemRead] == 1) mainBusState = Enu::MemReadFirstWait; //Another MemRead will bring us back to first MemRead, regardless of it MarChanged
        else if(controlSignals[Enu::MemWrite] == 1) mainBusState = Enu::MemWriteFirstWait;
        else mainBusState = Enu::None; //If neither are check, bus goes back to doing nothing
        break;
    case Enu::MemWriteFirstWait:
        if(!marChanged && controlSignals[Enu::MemWrite] == 1) mainBusState = Enu::MemWriteSecondWait;
        else if(marChanged && controlSignals[Enu::MemWrite] == 1); //Initiating a new write brings us back to first wait
        else if(controlSignals[Enu::MemRead] == 1) mainBusState = Enu::MemReadFirstWait; //Switch from write to read.
        else mainBusState = Enu::None; //If neither are check, bus goes back to doing nothing
        break;
    case Enu::MemWriteSecondWait:
        if(!marChanged && controlSignals[Enu::MemWrite] == 1) mainBusState = Enu::MemWriteReady;
        else if(marChanged && controlSignals[Enu::MemWrite] == 1) mainBusState = Enu::MemWriteFirstWait; //Initiating a new write brings us back to first wait
        else if(controlSignals[Enu::MemRead] == 1) mainBusState = Enu::MemReadFirstWait; //Switch from write to read.
        else mainBusState = Enu::None; //If neither are check, bus goes back to doing nothing
        break;
    case Enu::MemWriteReady:
        if(controlSignals[Enu::MemWrite]==1)mainBusState = Enu::MemWriteFirstWait; //Another MemWrite will reset the bus state back to first MemWrite
        else if(controlSignals[Enu::MemRead]==1) mainBusState = Enu::MemReadFirstWait; //Switch from write to read.
        else mainBusState = Enu::None; //If neither are check, bus goes back to doing nothing
        break;
    default:
        mainBusState = Enu::None;
        break;
    }
}

void CPUDataSection::stepOneByte() noexcept
{
    //Update the bus state first, as the rest of the read / write functionality depends on it
    handleMainBusState();
    if(hadErrorOnStep()) return; //If the bus had an error, give up now

    isALUCacheValid = false;
    //Set up all variables needed by stepping calculation
    Enu::EALUFunc aluFunc = static_cast<Enu::EALUFunc>(controlSignals[Enu::ALU]);
    quint8 a = 0, b = 0, c = 0, alu = 0, NZVC = 0;
    bool hasA = valueOnABus(a), hasB = valueOnBBus(b), hasC = valueOnCBus(c), statusBitError = false;
    bool hasALUOutput = calculateALUOutput(alu,NZVC);

    //Handle write to memory
    if(mainBusState == Enu::MemWriteReady) {
        // << upcasts from quint8 to int32, must explicitly narrow.
        quint16 address = static_cast<quint16>((memoryRegisters[Enu::MEM_MARA]<<8)
                | memoryRegisters[Enu::MEM_MARB]);
        memDevice->writeByte(address, memoryRegisters[Enu::MEM_MDR]);
    }

    //MARCk
    if(clockSignals[Enu::MARCk] && hasA && hasB) {
        onSetMemoryRegister(Enu::MEM_MARA, a);
        onSetMemoryRegister(Enu::MEM_MARB, b);
    }
    else if(clockSignals[Enu::MARCk]) {//Handle error where no data is present
        hadDataError = true;
        errorMessage = "No values on A & B during MARCk.";
        return;
    }

    //LoadCk
    if(clockSignals[Enu::LoadCk]) {
        if(controlSignals[Enu::C] == Enu::signalDisabled) {
            hadDataError = true;
            errorMessage = "No destination register specified for LoadCk.";
        }
        else if(!hasC) {
            hadDataError = true;
            errorMessage = "No value on C Bus to clock in.";
        }
        else onSetRegisterByte(controlSignals[Enu::C], c);
    }
    quint16 address;
    quint8 value;

    //MDRCk
    if(clockSignals[Enu::MDRCk]) {
        switch(controlSignals[Enu::MDRMux]) {
        case 0: //Pick memory
            address = static_cast<quint16>(memoryRegisters[Enu::MEM_MARA]<<8) + memoryRegisters[Enu::MEM_MARB];
            if(mainBusState != Enu::MemReadReady) {
                hadDataError = true;
                errorMessage = "No value from data bus to write to MDR.";
            }
            else {
                memDevice->getByte(address, value);
                onSetMemoryRegister(Enu::MEM_MDR, value);
            }
            break;
        case 1: //Pick C Bus;
            if(!hasC) {
                hadDataError = true;
                errorMessage = "No value on C bus to write to MDR.";
            }
            else onSetMemoryRegister(Enu::MEM_MDR,c);
            break;
        default:
            hadDataError = true;
            errorMessage = "No value to clock into MDR.";
            break;
        }

    }

    //NCk
    if(clockSignals[Enu::NCk]) {
        if(aluFunc!=Enu::UNDEFINED_func && hasALUOutput) onSetStatusBit(Enu::STATUS_N,Enu::NMask & NZVC);
        else statusBitError = true;
    }

    //ZCk
    if(clockSignals[Enu::ZCk]) {
        if(aluFunc!=Enu::UNDEFINED_func && hasALUOutput)
        {
            if(controlSignals[Enu::AndZ] == 0) {
                onSetStatusBit(Enu::STATUS_Z,Enu::ZMask & NZVC);
            }
            else if(controlSignals[Enu::AndZ] == 1) {
                onSetStatusBit(Enu::STATUS_Z, static_cast<bool>((Enu::ZMask & NZVC) && getStatusBit(Enu::STATUS_Z)));
            }
            else statusBitError = true;
        }
        else statusBitError = true;
    }

    //VCk
    if(clockSignals[Enu::VCk]) {
        if(aluFunc != Enu::UNDEFINED_func && hasALUOutput) onSetStatusBit(Enu::STATUS_V,Enu::VMask & NZVC);
        else statusBitError = true;
    }

    //CCk
    if(clockSignals[Enu::CCk]) {
        if(aluFunc!=Enu::UNDEFINED_func && hasALUOutput) onSetStatusBit(Enu::STATUS_C,Enu::CMask & NZVC);
        else statusBitError = true;
    }

    //SCk
    if(clockSignals[Enu::SCk]) {
        if(aluFunc!=Enu::UNDEFINED_func && hasALUOutput) onSetStatusBit(Enu::STATUS_S,Enu::CMask & NZVC);
        else statusBitError = true;
    }

    if(statusBitError) {
        hadDataError = true;
        errorMessage = "ALU Error: No output from ALU to clock into status bits.";
    }

}

void CPUDataSection::stepTwoByte() noexcept
{
    //Update the bus state first, as the rest of the read / write functionality depends on it
    handleMainBusState();
    if(hadErrorOnStep()) return; //If the bus had an error, give up now

    isALUCacheValid = false;
    // Set up all variables needed by stepping calculation
    Enu::EALUFunc aluFunc = static_cast<Enu::EALUFunc>(controlSignals[Enu::ALU]);
    quint8 a = 0, b = 0, c = 0, alu = 0, NZVC = 0, temp = 0;
    quint16 address;
    bool memSigError = false, hasA = valueOnABus(a), hasB = valueOnBBus(b), hasC = valueOnCBus(c);
    bool statusBitError = false, hasALUOutput = calculateALUOutput(alu, NZVC);

    // Handle write to memory
    if(mainBusState == Enu::MemWriteReady) {
        // << widens quint8 to int32, must explictly narrow.
        quint16 address = static_cast<quint16>((memoryRegisters[Enu::MEM_MARA]<<8)
                | memoryRegisters[Enu::MEM_MARB]);
        address&=0xFFFE; // Memory access ignores lowest order bit
        memDevice->writeWord(address, memoryRegisters[Enu::MEM_MDRE]*256 + memoryRegisters[Enu::MEM_MDRO]);
    }

    // MARCk
    if(clockSignals[Enu::MARCk]) {
        if(controlSignals[Enu::MARMux] == 0) {
            // If MARMux is 0, route MDRE, MDRO to MARA, MARB
            onSetMemoryRegister(Enu::MEM_MARA, memoryRegisters[Enu::MEM_MDRE]);
            onSetMemoryRegister(Enu::MEM_MARB, memoryRegisters[Enu::MEM_MDRO]);
        }
        else if(controlSignals[Enu::MARMux] == 1 && hasA && hasB) {
            // If MARMux is 1, route A, B to MARA, MARB
            onSetMemoryRegister(Enu::MEM_MARA, a);
            onSetMemoryRegister(Enu::MEM_MARB,b );
        }
        else {  // Otherwise MARCk is high, but no data flows through MARMux
            hadDataError = true;
            errorMessage = "MARMux has no output but MARCk.";
            return;
        }
    }

    // LoadCk
    if(clockSignals[Enu::LoadCk]) {
        if(controlSignals[Enu::C] == Enu::signalDisabled) {
            hadDataError = true;
            errorMessage = "No destination register specified for LoadCk.";
        }
        else if(!hasC) {
            hadDataError = true;
            errorMessage = "No value on C Bus to clock in.";
        }
        else onSetRegisterByte(controlSignals[Enu::C], c);
    }

#pragma message("TODO: Determine the fate of completed + ignored memreads.")
    // Determining the fate of memreads would transacting with memory.
    bool in_tx = false;
    // MDRECk
    if(clockSignals[Enu::MDRECk]) {
        switch(controlSignals[Enu::MDREMux])
        {
        case 0: // Pick memory
            // << widens quint8 to int32, must explictly narrow.
            address = static_cast<quint16>((memoryRegisters[Enu::MEM_MARA]<<8)
                    | memoryRegisters[Enu::MEM_MARB]);
            address &= 0xFFFE; // Memory access ignores lowest order bit
            if(mainBusState != Enu::MemReadReady){
                hadDataError = true;
                errorMessage = "No value from data bus to write to MDRE.";
                return;
            }
            else {
                memDevice->beginTransaction(AMemoryDevice::AccessType::NONE);
                in_tx = true;
                memSigError = memDevice->readByte(address, temp);
                if(!memSigError) {
                    hadDataError = true;
                    errorMessage = "Unable to read from memory into MDRE.";
                    return;
                }
                onSetMemoryRegister(Enu::MEM_MDRE, temp);
            }
            break;
        case 1: // Pick C Bus;
            if(!hasC) {
                hadDataError=true;
                errorMessage = "No value on C bus to write to MDRE.";
                return;
            }
            else onSetMemoryRegister(Enu::MEM_MDRE,c);
            break;
        default:
            hadDataError = true;
            errorMessage = "No value to clock into MDRE.";
            break;
        }

    }

    //MDRECk
    if(clockSignals[Enu::MDROCk]) {
        switch(controlSignals[Enu::MDROMux])
        {
        case 0: //Pick memory
            // << widens to quint8 to int32, must explictly narrow.
            address = static_cast<quint16>((memoryRegisters[Enu::MEM_MARA]<<8)
                    | memoryRegisters[Enu::MEM_MARB]);
            address &= 0xFFFE; //Memory access ignores lowest order bit
            address += 1;
            if(mainBusState != Enu::MemReadReady){
                hadDataError = true;
                errorMessage = "No value from data bus to write to MDRO.";
                return;
            }
            else {
                if(!in_tx) {
                    memDevice->beginTransaction(AMemoryDevice::AccessType::NONE);
                    in_tx = true;
                }
                memSigError = memDevice->readByte(address, temp);
                if(!memSigError) {
                    hadDataError = true;
                    errorMessage = "Unable to read from memory into MDRE.";
                    return;
                }
                onSetMemoryRegister(Enu::MEM_MDRO, temp);
            }
            break;
        case 1: //Pick C Bus;
            if(!hasC) {
                hadDataError = true;
                errorMessage = "No value on C bus to write to MDRO.";
                return;
            }
            else onSetMemoryRegister(Enu::MEM_MDRO, c);
            break;
        default:
            hadDataError = true;
            errorMessage = "No value to clock into MDRO.";
            break;
        }

    }
#pragma message("TODO: Determine transact fate.")
    if(in_tx) {
        memDevice->endTransaction();
    }

    //NCk
    if(clockSignals[Enu::NCk]) {
        if(aluFunc != Enu::UNDEFINED_func && hasALUOutput) onSetStatusBit(Enu::STATUS_N, Enu::NMask & NZVC);
        else statusBitError = true;
    }

    //If no ALU output, don't set flags.
    //ZCk
    if(clockSignals[Enu::ZCk]) {
        if(aluFunc != Enu::UNDEFINED_func && hasALUOutput) {
            if(controlSignals[Enu::AndZ] == 0) {
                onSetStatusBit(Enu::STATUS_Z,Enu::ZMask & NZVC);
            }
            else if(controlSignals[Enu::AndZ] == 1) {
                onSetStatusBit(Enu::STATUS_Z, static_cast<bool>((Enu::ZMask & NZVC) && getStatusBit(Enu::STATUS_Z)));
            }
            else statusBitError = true;
        }
        else statusBitError = true;
    }

    //VCk
    if(clockSignals[Enu::VCk]) {
        if(aluFunc != Enu::UNDEFINED_func && hasALUOutput) onSetStatusBit(Enu::STATUS_V, Enu::VMask & NZVC);
        else statusBitError = true;
    }

    //CCk
    if(clockSignals[Enu::CCk]) {
        if(aluFunc != Enu::UNDEFINED_func && hasALUOutput) onSetStatusBit(Enu::STATUS_C, Enu::CMask & NZVC);
        else statusBitError = true;
    }

    //SCk
    if(clockSignals[Enu::SCk]) {
        if(aluFunc != Enu::UNDEFINED_func && hasALUOutput) onSetStatusBit(Enu::STATUS_S, Enu::CMask & NZVC);
        else statusBitError = true;
    }

    if(statusBitError) {
        hadDataError = true;
        errorMessage = "ALU Error: No output from ALU to clock into status bits.";
    }
}

void CPUDataSection::presetStaticRegisters() noexcept
{
    // Pre-assign static registers according to CPU diagram
    registerBank->writeRegisterByte(22, 0x00);
    registerBank->writeRegisterByte(23, 0x01);
    registerBank->writeRegisterByte(24, 0x02);
    registerBank->writeRegisterByte(25, 0x03);
    registerBank->writeRegisterByte(26, 0x04);
    registerBank->writeRegisterByte(27, 0x08);
    registerBank->writeRegisterByte(28, 0xF0);
    registerBank->writeRegisterByte(29, 0xF6);
    registerBank->writeRegisterByte(30, 0xFE);
    registerBank->writeRegisterByte(31, 0xFF);
}

void CPUDataSection::clearControlSignals() noexcept
{
    //Set all control signals to disabled
    for(int it = 0; it < controlSignals.length(); it++) {
        controlSignals[it] = Enu::signalDisabled;
    }
}

void CPUDataSection::clearClockSignals() noexcept
{
    //Set all clock signals to low
    for(int it = 0; it < clockSignals.length(); it++) {
        clockSignals[it]=false;
    }
}

void CPUDataSection::clearRegisters() noexcept
{
    // Clear all registers in register bank, then restore the static values
    registerBank->clearRegisters();
    registerBank->clearStatusBits();
    presetStaticRegisters();

     // Clear all values from memory registers
    for(int it = 0; it < memoryRegisters.length(); it++) {
        memoryRegisters[it] = 0;
    }
}

void CPUDataSection::clearErrors() noexcept
{
    hadDataError = false;
    errorMessage.clear();
}

void CPUDataSection::onStep() noexcept
{
    //If the error hasn't been handled by now, clear it
    clearErrors();
    if(cpuFeatures == Enu::OneByteDataBus) {
        stepOneByte();
    }
    else if(cpuFeatures == Enu::TwoByteDataBus) {
        stepTwoByte();
    }
}

void CPUDataSection::onClock() noexcept
{
    //When the clock button is pushed, execute whatever control signals are set, and the clear their values
    onStep();
    clearClockSignals();
    clearControlSignals();
}

void CPUDataSection::onClearCPU() noexcept
{
    //Reset evey value associated with the CPU
    mainBusState = Enu::None;
    clearErrors();
    clearRegisters();
    clearClockSignals();
    clearControlSignals();
}

void CPUDataSection::onSetCPUType(Enu::CPUType type)
{
    if(cpuFeatures != type) {
       cpuFeatures = type;
       emit CPUTypeChanged(type);
    }

}

void CPUDataSection::setMemoryDevice(QSharedPointer<AMemoryDevice> newDevice)
{
    memDevice = newDevice;
}
