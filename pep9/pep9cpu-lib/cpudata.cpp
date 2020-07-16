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
#include "pep/constants.h"
#include "pep/pep.h"

#include "pep9microcode.h"
const auto LoadCk_t = to_uint8_t(Pep9::uarch::EClockSignals::LoadCk);
const auto C_t = to_uint8_t(Pep9::uarch::EControlSignals::C);
const auto B_t = to_uint8_t(Pep9::uarch::EControlSignals::B);
const auto A_t = to_uint8_t(Pep9::uarch::EControlSignals::A);
const auto MARCk_t = to_uint8_t(Pep9::uarch::EClockSignals::MARCk);
const auto MDRCk_t = to_uint8_t(Pep9::uarch::EClockSignals::MDRCk);
const auto AMux_t = to_uint8_t(Pep9::uarch::EControlSignals::AMux);
const auto MDRMux_t = to_uint8_t(Pep9::uarch::EControlSignals::MDRMux);
const auto CMux_t = to_uint8_t(Pep9::uarch::EControlSignals::CMux);
const auto ALU_t = to_uint8_t(Pep9::uarch::EControlSignals::ALU);
const auto CSMux_t = to_uint8_t(Pep9::uarch::EControlSignals::CSMux);
const auto SCk_t = to_uint8_t(Pep9::uarch::EClockSignals::SCk);
const auto CCk_t = to_uint8_t(Pep9::uarch::EClockSignals::CCk);
const auto VCk_t = to_uint8_t(Pep9::uarch::EClockSignals::VCk);
const auto AndZ_t = to_uint8_t(Pep9::uarch::EControlSignals::AndZ);
const auto ZCk_t = to_uint8_t(Pep9::uarch::EClockSignals::ZCk);
const auto NCk_t = to_uint8_t(Pep9::uarch::EClockSignals::NCk);
const auto MemWrite_t = to_uint8_t(Pep9::uarch::EControlSignals::MemWrite);
const auto MemRead_t = to_uint8_t(Pep9::uarch::EControlSignals::MemRead);
const auto MARMux_t = to_uint8_t(Pep9::uarch::EControlSignals::MARMux);
const auto MDROCk_t = to_uint8_t(Pep9::uarch::EClockSignals::MDROCk);
const auto MDROMux_t = to_uint8_t(Pep9::uarch::EControlSignals::MDROMux);
const auto MDRECk_t = to_uint8_t(Pep9::uarch::EClockSignals::MDRECk);
const auto MDREMux_t = to_uint8_t(Pep9::uarch::EControlSignals::MDREMux);
const auto EOMux_t = to_uint8_t(Pep9::uarch::EControlSignals::EOMux);

const auto PValidCk_t = to_uint8_t(Pep9::uarch::EClockSignals::PValidCk);
const auto PValid_t = to_uint8_t(Pep9::uarch::EControlSignals::PValid);

const auto NBit_t = Pep9::Definition::getStatusBitOffset(Pep9::uarch::EStatusBit::STATUS_N);
const auto ZBit_t = Pep9::Definition::getStatusBitOffset(Pep9::uarch::EStatusBit::STATUS_Z);
const auto VBit_t = Pep9::Definition::getStatusBitOffset(Pep9::uarch::EStatusBit::STATUS_V);
const auto CBit_t = Pep9::Definition::getStatusBitOffset(Pep9::uarch::EStatusBit::STATUS_C);
const auto SBit_t = Pep9::Definition::getStatusBitOffset(Pep9::uarch::EStatusBit::STATUS_S);
CPUDataSection::CPUDataSection(PepCore::CPUType type, QSharedPointer<const Pep9::Definition> pep_version,
                               QSharedPointer<AMemoryDevice> memDev, QObject *parent): QObject(parent),
    pep_version(pep_version), memDevice(memDev),
    cpuFeatures(type), mainBusState(Pep9::uarch::MainBusState::None),
    registerBank(QSharedPointer<RegisterFile>::create(pep_version->maxRegisterNumber(), pep_version->maxStatusBitNumber())),
    memoryRegisters(6), controlSignals(Pep9::uarch::numControlSignals()),
    clockSignals(Pep9::uarch::numClockSignals()), emitEvents(true), hadDataError(false), errorMessage(""),
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
    using namespace Pep9::uarch;
    //The only alu functions that are unary are 0 & 10..15
    return controlSignals[ALU_t] == 0 || controlSignals[ALU_t] >= 10;
}

bool CPUDataSection::getAMuxOutput(quint8& result) const
{
        if(controlSignals[AMux_t] == 0 && cpuFeatures == PepCore::CPUType::TwoByteDataBus) {
            //Which could come from MDRE when EOMux is 0
            if(controlSignals[EOMux_t] == 0) {
                result = memoryRegisters[static_cast<int>(Pep9::uarch::EMemoryRegisters::MEM_MDRE)];
                return true;
            }
            //Or comes from MDRO if EOMux is 1
            else if(controlSignals[EOMux_t] == 1) {
                result = memoryRegisters[static_cast<int>(Pep9::uarch::EMemoryRegisters::MEM_MDRO)];
                return true;
            }
            //Or has no has no output when EOMux is disabled
            else return false;

        }
        else if(controlSignals[AMux_t] == 0 && cpuFeatures == PepCore::CPUType::OneByteDataBus) {
            result = memoryRegisters[static_cast<int>(Pep9::uarch::EMemoryRegisters::MEM_MDR)];
            return true;
        }
        else if(controlSignals[AMux_t] == 1) {
            return valueOnABus(result);
        }
        else return false;
}

bool CPUDataSection::calculateCSMuxOutput(bool &result) const
{
    //CSMux either outputs C when CS is 0
    if(controlSignals[CSMux_t]==0) {
        result = registerBank->readStatusBitCurrent(CBit_t);
        return true;
    }
    //Or outputs S when CS is 1
    else if(controlSignals[CSMux_t] == 1)  {
        result = registerBank->readStatusBitCurrent(SBit_t);
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
    switch(static_cast<Pep9::uarch::EALUFunc>(controlSignals[ALU_t])) {
    case Pep9::uarch::EALUFunc::A_func: // A
        res = a;
        break;
    case Pep9::uarch::EALUFunc::ApB_func: // A plus B
        res = a + b;
        NZVC |= Pep9::uarch::CMask * quint8{res<a||res<b}; // Carry out if result is unsigned less than a or b.
        // There is a signed overflow iff the high order bits of the input are the same,
        // and the inputs & output differs in sign.
        // Shifts in 0's (unsigned chars), so after shift, only high order bit remain.
        NZVC |= Pep9::uarch::VMask * ((~(a ^ b) & (a ^ res)) >> 7) ;
        break;
    case Pep9::uarch::EALUFunc::ApnBp1_func: // A plus ~B plus 1
        hasCIn = true;
        carryIn = 1;
        [[fallthrough]];
    case Pep9::uarch::EALUFunc::ApnBpCin_func: // A plus ~B plus Cin
        // Clang thinks this is a garbage value. It isn't.
        // Lots of "errors" spawn from this, but this is well-defined behavior.
        b = ~b;
        [[fallthrough]];
    case Pep9::uarch::EALUFunc::ApBpCin_func: // A plus B plus Cin
        // Expected carry in, none was provided, so ALU calculation yeilds a meaningless result
        if (!hasCIn) return false;
        // Might cause overflow, but overflow is well defined for unsigned ints
        res = a + b + quint8{carryIn};
        NZVC |= Pep9::uarch::CMask * quint8{res<a||res<b}; // Carry out if result is unsigned less than a or b.
        // There is a signed overflow iff the high order bits of the input are the same,
        // and the inputs & output differs in sign.
        // Shifts in 0's (unsigned chars), so after shift, only high order bit remain.
        NZVC |= Pep9::uarch::VMask * ((~(a ^ b) & (a ^ res)) >> 7) ;
        break;
    case Pep9::uarch::EALUFunc::AandB_func: // A * B
        res = a & b;
        break;
    case Pep9::uarch::EALUFunc::nAandB_func: // ~(A * B)
        res = ~(a & b);
        break;
    case Pep9::uarch::EALUFunc::AorB_func: // A + B
        res = a | b;
        break;
    case Pep9::uarch::EALUFunc::nAorB_func: // ~(A + B)
        res = ~(a | b);
        break;
    case Pep9::uarch::EALUFunc::AxorB_func: // A xor B
        res = a ^ b;
        break;
    case Pep9::uarch::EALUFunc::nA_func: // ~A
        res = ~a;
        break;
    case Pep9::uarch::EALUFunc::ASLA_func: // ASL A
        res = static_cast<quint8>(a<<1);
        NZVC |= Pep9::uarch::CMask * ((a & 0x80) >> 7); // Carry out equals the hi order bit
        NZVC |= Pep9::uarch::VMask * (((a << 1) ^ a) >>7); // Signed overflow if a<hi> doesn't match a<hi-1>
        break;
    case Pep9::uarch::EALUFunc::ROLA_func: // ROL A
        if (!hasCIn) return false;
        res = static_cast<quint8>(a<<1 | quint8{carryIn});
        NZVC |= Pep9::uarch::CMask * ((a & 0x80) >> 7); // Carry out equals the hi order bit
        NZVC |= Pep9::uarch::VMask * (((a << 1) ^a) >>7); // Signed overflow if a<hi> doesn't match a<hi-1>
        break;
    case Pep9::uarch::EALUFunc::ASRA_func: // ASR A
        hasCIn = true;
        carryIn = a & 128; // RORA and ASRA only differ by how the carryIn is calculated
        [[fallthrough]];
    case Pep9::uarch::EALUFunc::RORA_func: // ROR a
        if (!hasCIn) return false;
        // A will not be sign extended since it is unsigned.
        // Widen carryIn so that << yields a meaningful result.
        res = static_cast<quint8>(a >> 1 | static_cast<quint8>(carryIn) << 7);
        // Carry out is lowest order bit of a
        NZVC |= Pep9::uarch::CMask * (a & 1);
        break;
    case Pep9::uarch::EALUFunc::NZVCA_func: // Move A to NZVC
        res = 0;
        NZVC |= Pep9::uarch::NMask & a;
        NZVC |= Pep9::uarch::ZMask & a;
        NZVC |= Pep9::uarch::VMask & a;
        NZVC |= Pep9::uarch::CMask & a;
        return true; // Must return early to avoid NZ calculation
    default: // If the default has been hit, then an invalid function was selected
        return false;
    }
    // Calculate N, then shift to correct position
    NZVC |= (res & 0x80) ? Pep9::uarch::NMask : 0; // Result is negative if high order bit is 1
    // Calculate Z, then shift to correct position
    NZVC |= (res == 0) ? Pep9::uarch::ZMask : 0;
    // Save the result of the ALU calculation
    ALUOutputCache = res;
    ALUStatusBitCache = NZVC;
    isALUCacheValid = true;
    ALUHasOutputCache = true;
    return ALUHasOutputCache;

}

PepCore::CPUType CPUDataSection::getCPUType() const
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

quint8 CPUDataSection::getMemoryRegister(Pep9::uarch::EMemoryRegisters registerNumber) const
{
    return memoryRegisters[static_cast<int>(registerNumber)];
}

bool CPUDataSection::valueOnABus(quint8 &result) const
{
    if(controlSignals[A_t] == Enu::signalDisabled) return false;
    result = getRegisterBankByte(controlSignals[A_t]);
    return true;
}

bool CPUDataSection::valueOnBBus(quint8 &result) const
{
    if(controlSignals[B_t] == Enu::signalDisabled) return false;
    result = getRegisterBankByte(controlSignals[B_t]);
    return true;
}

bool CPUDataSection::valueOnCBus(quint8 &result) const
{
    if(controlSignals[CMux_t] == 0) {
        // If CMux is 0, then the NZVC bits (minus S) are directly routed to result
        result = 0;
        result |= registerBank->readStatusBitCurrent(NBit_t) * Pep9::uarch::EMask::NMask;
        result |= registerBank->readStatusBitCurrent(ZBit_t) * Pep9::uarch::EMask::ZMask;
        result |= registerBank->readStatusBitCurrent(VBit_t) * Pep9::uarch::EMask::VMask;
        result |= registerBank->readStatusBitCurrent(CBit_t) * Pep9::uarch::EMask::CMask;
        return true;
    }
    else if(controlSignals[CMux_t] == 1) {
        quint8 temp = 0; // Discard NZVC bits for this calculation, they are unecessary for calculating C's output
        // Otherwise the value of C depends solely on the ALU
        return calculateALUOutput(result, temp);
    }
    else return false;
}

Pep9::uarch::MainBusState CPUDataSection::getMainBusState() const
{
    return mainBusState;
}

bool CPUDataSection::getStatusBit(Pep9::uarch::EStatusBit statusBit) const
{
    using namespace Pep9::uarch;
    switch(statusBit)
    {
    // Mask out bit of interest, then convert to bool
    case EStatusBit::STATUS_N:
        return registerBank->readStatusBitCurrent(NBit_t);
    case EStatusBit::STATUS_Z:
        return registerBank->readStatusBitCurrent(ZBit_t);
    case EStatusBit::STATUS_V:
        return registerBank->readStatusBitCurrent(VBit_t);
    case EStatusBit::STATUS_C:
        return registerBank->readStatusBitCurrent(CBit_t);
    case EStatusBit::STATUS_S:
        return registerBank->readStatusBitCurrent(SBit_t);
    default:
        // Should never occur, but might happen if a bad status bit is passed
        return false;
    }
}

void CPUDataSection::onSetStatusBit(Pep9::uarch::EStatusBit statusBit, bool val)
{
    using namespace Pep9::uarch;

    bool oldVal = false;

    // Mask out the original value, then or it with the properly shifted bit
    oldVal = getStatusBit(statusBit);
    switch(statusBit)
    {
    // Mask out bit of interest, then convert to bool
    case EStatusBit::STATUS_N:
        registerBank->writeStatusBit(NBit_t, val);
        break;
    case EStatusBit::STATUS_Z:
        registerBank->writeStatusBit(ZBit_t, val);
        break;
    case EStatusBit::STATUS_V:
        registerBank->writeStatusBit(VBit_t, val);
        break;
    case EStatusBit::STATUS_C:
        registerBank->writeStatusBit(CBit_t, val);
        break;
    case EStatusBit::STATUS_S:
        registerBank->writeStatusBit(SBit_t, val);
        break;
    }
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

void CPUDataSection::onSetMemoryRegister(Pep9::uarch::EMemoryRegisters reg, quint8 val)
{
    auto reg_num = static_cast<int>(reg);
    quint8 oldVal = memoryRegisters[reg_num];
    memoryRegisters[reg_num] = val;
    if(emitEvents) {
    if(oldVal != val) emit memoryRegisterChanged(reg, oldVal, val);
    }
}

void CPUDataSection::onSetClock(Pep9::uarch::EClockSignals clock, bool value)
{
    clockSignals[to_uint8_t(clock)] = value;
}

void CPUDataSection::onSetControlSignal(Pep9::uarch::EControlSignals control, quint8 value)
{
    controlSignals[to_uint8_t(control)] = value;
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
    using namespace Pep9::uarch;
    bool marChanged = false;
    quint8 a, b;
    if(clockSignals[MARCk_t] && valueOnABus(a) && valueOnBBus(b)) {
        marChanged = !(a == memoryRegisters[static_cast<int>(EMemoryRegisters::MEM_MARA)] &&
                       b == memoryRegisters[static_cast<int>(EMemoryRegisters::MEM_MARB)]);
    }
    switch(mainBusState)
    {
    case Pep9::uarch::MainBusState::None:
        //One cannot change MAR contents and initiate a R/W on same cycle
        if(!marChanged) {
            if(controlSignals[MemRead_t] == 1) mainBusState = MainBusState::MemReadFirstWait;
            else if(controlSignals[MemWrite_t] == 1) mainBusState = MainBusState::MemWriteFirstWait;
        }
        break;
    case Pep9::uarch::MainBusState::MemReadFirstWait:
        if(!marChanged && controlSignals[MemRead_t] == 1) mainBusState = MainBusState::MemReadSecondWait;
        else if(marChanged && controlSignals[MemRead_t] == 1); //Initiating a new read brings us back to first wait
        else if(controlSignals[MemWrite_t] == 1) mainBusState = MainBusState::MemWriteFirstWait; //Switch from read to write.
        else mainBusState = Pep9::uarch::MainBusState::None; //If neither are check, bus goes back to doing nothing
        break;
    case Pep9::uarch::MainBusState::MemReadSecondWait:
        if(!marChanged && controlSignals[MemRead_t] == 1) mainBusState = MainBusState::MemReadReady;
        else if(marChanged && controlSignals[MemRead_t] == 1) mainBusState = MainBusState::MemReadFirstWait;
        else if(controlSignals[MemWrite_t] == 1) mainBusState = MainBusState::MemWriteFirstWait;
        else mainBusState = MainBusState::None; //If neither are check, bus goes back to doing nothing
        break;
    case Pep9::uarch::MainBusState::MemReadReady:
        if(controlSignals[MemRead_t] == 1) mainBusState = MainBusState::MemReadFirstWait; //Another MemRead will bring us back to first MemRead, regardless of it MarChanged
        else if(controlSignals[MemWrite_t] == 1) mainBusState = MainBusState::MemWriteFirstWait;
        else mainBusState = MainBusState::None; //If neither are check, bus goes back to doing nothing
        break;
    case Pep9::uarch::MainBusState::MemWriteFirstWait:
        if(!marChanged && controlSignals[MemWrite_t] == 1) mainBusState = MainBusState::MemWriteSecondWait;
        else if(marChanged && controlSignals[MemWrite_t] == 1); //Initiating a new write brings us back to first wait
        else if(controlSignals[MemRead_t] == 1) mainBusState = MainBusState::MemReadFirstWait; //Switch from write to read.
        else mainBusState = MainBusState::None; //If neither are check, bus goes back to doing nothing
        break;
    case Pep9::uarch::MainBusState::MemWriteSecondWait:
        if(!marChanged && controlSignals[MemWrite_t] == 1) mainBusState = MainBusState::MemWriteReady;
        else if(marChanged && controlSignals[MemWrite_t] == 1) mainBusState = MainBusState::MemWriteFirstWait; //Initiating a new write brings us back to first wait
        else if(controlSignals[MemRead_t] == 1) mainBusState = MainBusState::MemReadFirstWait; //Switch from write to read.
        else mainBusState = MainBusState::None; //If neither are check, bus goes back to doing nothing
        break;
    case Pep9::uarch::MainBusState::MemWriteReady:
        if(controlSignals[MemWrite_t]==1)mainBusState = MainBusState::MemWriteFirstWait; //Another MemWrite will reset the bus state back to first MemWrite
        else if(controlSignals[MemRead_t]==1) mainBusState = MainBusState::MemReadFirstWait; //Switch from write to read.
        else mainBusState = MainBusState::None; //If neither are check, bus goes back to doing nothing
        break;
    default:
        mainBusState = MainBusState::None;
        break;
    }
}

void CPUDataSection::stepOneByte() noexcept
{
    using namespace Pep9::uarch;

    auto constexpr mara_reg = static_cast<size_t>(Pep9::uarch::EMemoryRegisters::MEM_MARA);
    auto constexpr marb_reg = static_cast<size_t>(Pep9::uarch::EMemoryRegisters::MEM_MARB);
    auto constexpr mdr_reg = static_cast<size_t>(Pep9::uarch::EMemoryRegisters::MEM_MDR);
    //Update the bus state first, as the rest of the read / write functionality depends on it
    handleMainBusState();
    if(hadErrorOnStep()) return; //If the bus had an error, give up now

    isALUCacheValid = false;
    //Set up all variables needed by stepping calculation
    auto aluFunc = static_cast<Pep9::uarch::EALUFunc>(controlSignals[ALU_t]);
    quint8 a = 0, b = 0, c = 0, alu = 0, NZVC = 0;
    bool hasA = valueOnABus(a), hasB = valueOnBBus(b), hasC = valueOnCBus(c), statusBitError = false;
    bool hasALUOutput = calculateALUOutput(alu,NZVC);

    //Handle write to memory
    if(mainBusState == Pep9::uarch::MainBusState::MemWriteReady) {
        // << upcasts from quint8 to int32, must explicitly narrow.
        quint16 address = static_cast<quint16>( ( memoryRegisters[mara_reg]<<8) | memoryRegisters[marb_reg]);
        memDevice->writeByte(address, memoryRegisters[mdr_reg]);
    }

    //MARCk
    if(clockSignals[MARCk_t] && hasA && hasB) {
        onSetMemoryRegister(Pep9::uarch::EMemoryRegisters::MEM_MARA, a);
        onSetMemoryRegister(Pep9::uarch::EMemoryRegisters::MEM_MARB, b);
    }
    else if(clockSignals[MARCk_t]) {//Handle error where no data is present
        hadDataError = true;
        errorMessage = "No values on A & B during MARCk.";
        return;
    }

    //LoadCk
    if(clockSignals[LoadCk_t]) {
        if(controlSignals[C_t] == Enu::signalDisabled) {
            hadDataError = true;
            errorMessage = "No destination register specified for LoadCk.";
        }
        else if(!hasC) {
            hadDataError = true;
            errorMessage = "No value on C Bus to clock in.";
        }
        else onSetRegisterByte(controlSignals[C_t], c);
    }
    quint16 address;
    quint8 value;

    //MDRCk
    if(clockSignals[MDRCk_t]) {
        switch(controlSignals[MDRMux_t]) {
        case 0: //Pick memory
            address = static_cast<quint16>(memoryRegisters[mara_reg]<<8) + memoryRegisters[marb_reg];
            if(mainBusState != Pep9::uarch::MainBusState::MemReadReady) {
                hadDataError = true;
                errorMessage = "No value from data bus to write to MDR.";
            }
            else {
                memDevice->getByte(address, value);
                onSetMemoryRegister(Pep9::uarch::EMemoryRegisters::MEM_MDR, value);
            }
            break;
        case 1: //Pick C Bus;
            if(!hasC) {
                hadDataError = true;
                errorMessage = "No value on C bus to write to MDR.";
            }
            else onSetMemoryRegister(Pep9::uarch::EMemoryRegisters::MEM_MDR,c);
            break;
        default:
            hadDataError = true;
            errorMessage = "No value to clock into MDR.";
            break;
        }

    }

    //NCk
    if(clockSignals[NCk_t]) {
        if(aluFunc!=Pep9::uarch::EALUFunc::UNDEFINED_func && hasALUOutput) onSetStatusBit(EStatusBit::STATUS_N, Pep9::uarch::NMask & NZVC);
        else statusBitError = true;
    }

    //ZCk
    if(clockSignals[ZCk_t]) {
        if(aluFunc!=Pep9::uarch::EALUFunc::UNDEFINED_func && hasALUOutput)
        {
            if(controlSignals[AndZ_t] == 0) {
                onSetStatusBit(EStatusBit::STATUS_Z, Pep9::uarch::ZMask & NZVC);
            }
            else if(controlSignals[AndZ_t] == 1) {
                onSetStatusBit(EStatusBit::STATUS_Z, static_cast<bool>((Pep9::uarch::ZMask & NZVC) && getStatusBit(EStatusBit::STATUS_Z)));
            }
            else statusBitError = true;
        }
        else statusBitError = true;
    }

    //VCk
    if(clockSignals[VCk_t]) {
        if(aluFunc != Pep9::uarch::EALUFunc::UNDEFINED_func && hasALUOutput) onSetStatusBit(EStatusBit::STATUS_V, Pep9::uarch::VMask & NZVC);
        else statusBitError = true;
    }

    //CCk
    if(clockSignals[CCk_t]) {
        if(aluFunc!=Pep9::uarch::EALUFunc::UNDEFINED_func && hasALUOutput) onSetStatusBit(EStatusBit::STATUS_C, Pep9::uarch::CMask & NZVC);
        else statusBitError = true;
    }

    //SCk
    if(clockSignals[SCk_t]) {
        if(aluFunc!=Pep9::uarch::EALUFunc::UNDEFINED_func && hasALUOutput) onSetStatusBit(EStatusBit::STATUS_S, Pep9::uarch::CMask & NZVC);
        else statusBitError = true;
    }

    if(statusBitError) {
        hadDataError = true;
        errorMessage = "ALU Error: No output from ALU to clock into status bits.";
    }

}

void CPUDataSection::stepTwoByte() noexcept
{
    using namespace Pep9::uarch;

    auto constexpr mara_reg = static_cast<size_t>(Pep9::uarch::EMemoryRegisters::MEM_MARA);
    auto constexpr marb_reg = static_cast<size_t>(Pep9::uarch::EMemoryRegisters::MEM_MARB);
    auto constexpr mdre_reg = static_cast<size_t>(Pep9::uarch::EMemoryRegisters::MEM_MDRE);
    auto constexpr mdro_reg = static_cast<size_t>(Pep9::uarch::EMemoryRegisters::MEM_MDRO);

    //Update the bus state first, as the rest of the read / write functionality depends on it
    handleMainBusState();
    if(hadErrorOnStep()) return; //If the bus had an error, give up now

    isALUCacheValid = false;
    // Set up all variables needed by stepping calculation
    auto aluFunc = static_cast<Pep9::uarch::EALUFunc>(controlSignals[ALU_t]);
    quint8 a = 0, b = 0, c = 0, alu = 0, NZVC = 0, temp = 0;
    quint16 address;
    bool memSigError = false, hasA = valueOnABus(a), hasB = valueOnBBus(b), hasC = valueOnCBus(c);
    bool statusBitError = false, hasALUOutput = calculateALUOutput(alu, NZVC);

    // Handle write to memory
    if(mainBusState == Pep9::uarch::MainBusState::MemWriteReady) {
        // << widens quint8 to int32, must explictly narrow.
        quint16 address = static_cast<quint16>((memoryRegisters[mara_reg]<<8)| memoryRegisters[marb_reg]);
        address&=0xFFFE; // Memory access ignores lowest order bit
        memDevice->writeWord(address, memoryRegisters[mdre_reg]*256 + memoryRegisters[mdro_reg]);
    }

    // MARCk
    if(clockSignals[MARCk_t]) {
        if(controlSignals[MARMux_t] == 0) {
            // If MARMux is 0, route MDRE, MDRO to MARA, MARB
            onSetMemoryRegister(Pep9::uarch::EMemoryRegisters::MEM_MARA, memoryRegisters[mdre_reg]);
            onSetMemoryRegister(Pep9::uarch::EMemoryRegisters::MEM_MARB, memoryRegisters[mdro_reg]);
        }
        else if(controlSignals[MARMux_t] == 1 && hasA && hasB) {
            // If MARMux is 1, route A, B to MARA, MARB
            onSetMemoryRegister(Pep9::uarch::EMemoryRegisters::MEM_MARA, a);
            onSetMemoryRegister(Pep9::uarch::EMemoryRegisters::MEM_MARB,b );
        }
        else {  // Otherwise MARCk is high, but no data flows through MARMux
            hadDataError = true;
            errorMessage = "MARMux has no output but MARCk.";
            return;
        }
    }

    // LoadCk
    if(clockSignals[LoadCk_t]) {
        if(controlSignals[C_t] == Enu::signalDisabled) {
            hadDataError = true;
            errorMessage = "No destination register specified for LoadCk.";
        }
        else if(!hasC) {
            hadDataError = true;
            errorMessage = "No value on C Bus to clock in.";
        }
        else onSetRegisterByte(controlSignals[C_t], c);
    }

#pragma message("TODO: Determine the fate of completed + ignored memreads.")
    // Determining the fate of memreads would transacting with memory.
    bool in_tx = false;
    // MDRECk
    if(clockSignals[MDRECk_t]) {
        switch(controlSignals[MDREMux_t])
        {
        case 0: // Pick memory
            // << widens quint8 to int32, must explictly narrow.
            address = static_cast<quint16>((memoryRegisters[mara_reg]<<8) | memoryRegisters[marb_reg]);
            address &= 0xFFFE; // Memory access ignores lowest order bit
            if(mainBusState != Pep9::uarch::MainBusState::MemReadReady){
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
                onSetMemoryRegister(Pep9::uarch::EMemoryRegisters::MEM_MDRE, temp);
            }
            break;
        case 1: // Pick C Bus;
            if(!hasC) {
                hadDataError=true;
                errorMessage = "No value on C bus to write to MDRE.";
                return;
            }
            else onSetMemoryRegister(Pep9::uarch::EMemoryRegisters::MEM_MDRE,c);
            break;
        default:
            hadDataError = true;
            errorMessage = "No value to clock into MDRE.";
            break;
        }

    }

    //MDRECk
    if(clockSignals[MDROCk_t]) {
        switch(controlSignals[MDROMux_t])
        {
        case 0: //Pick memory
            // << widens to quint8 to int32, must explictly narrow.
            address = static_cast<quint16>((memoryRegisters[mara_reg]<<8) | memoryRegisters[marb_reg]);
            address &= 0xFFFE; //Memory access ignores lowest order bit
            address += 1;
            if(mainBusState != Pep9::uarch::MainBusState::MemReadReady){
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
                onSetMemoryRegister(Pep9::uarch::EMemoryRegisters::MEM_MDRO, temp);
            }
            break;
        case 1: //Pick C Bus;
            if(!hasC) {
                hadDataError = true;
                errorMessage = "No value on C bus to write to MDRO.";
                return;
            }
            else onSetMemoryRegister(Pep9::uarch::EMemoryRegisters::MEM_MDRO, c);
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
    if(clockSignals[NCk_t]) {
        if(aluFunc != Pep9::uarch::EALUFunc::UNDEFINED_func && hasALUOutput) onSetStatusBit(EStatusBit::STATUS_N, Pep9::uarch::NMask & NZVC);
        else statusBitError = true;
    }

    //If no ALU output, don't set flags.
    //ZCk
    if(clockSignals[ZCk_t]) {
        if(aluFunc != Pep9::uarch::EALUFunc::UNDEFINED_func && hasALUOutput) {
            if(controlSignals[AndZ_t] == 0) {
                onSetStatusBit(EStatusBit::STATUS_Z, Pep9::uarch::ZMask & NZVC);
            }
            else if(controlSignals[AndZ_t] == 1) {
                onSetStatusBit(EStatusBit::STATUS_Z, static_cast<bool>((Pep9::uarch::ZMask & NZVC) && getStatusBit(EStatusBit::STATUS_Z)));
            }
            else statusBitError = true;
        }
        else statusBitError = true;
    }

    //VCk
    if(clockSignals[VCk_t]) {
        if(aluFunc != Pep9::uarch::EALUFunc::UNDEFINED_func && hasALUOutput) onSetStatusBit(EStatusBit::STATUS_V, Pep9::uarch::VMask & NZVC);
        else statusBitError = true;
    }

    //CCk
    if(clockSignals[CCk_t]) {
        if(aluFunc != Pep9::uarch::EALUFunc::UNDEFINED_func && hasALUOutput) onSetStatusBit(EStatusBit::STATUS_C, Pep9::uarch::CMask & NZVC);
        else statusBitError = true;
    }

    //SCk
    if(clockSignals[SCk_t]) {
        if(aluFunc != Pep9::uarch::EALUFunc::UNDEFINED_func && hasALUOutput) onSetStatusBit(EStatusBit::STATUS_S, Pep9::uarch::CMask & NZVC);
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
    if(cpuFeatures == PepCore::CPUType::OneByteDataBus) {
        stepOneByte();
    }
    else if(cpuFeatures == PepCore::CPUType::TwoByteDataBus) {
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
    mainBusState = Pep9::uarch::MainBusState::None;
    clearErrors();
    clearRegisters();
    clearClockSignals();
    clearControlSignals();
}

void CPUDataSection::onSetCPUType(PepCore::CPUType type)
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
