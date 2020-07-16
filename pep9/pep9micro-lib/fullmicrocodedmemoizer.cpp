// File: fullmicrocodedmemoizer.cpp
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
#include "fullmicrocodedmemoizer.h"

#include <cassert>

#include <QString>
#include <QtCore>
#include <QDebug>
#include <QStack>


#include "assembler/asmprogram.h"
#include "assembler/asmprogrammanager.h"
#include "cpu/registerfile.h"
#include "memory/amemorydevice.h"
#include "pep/pep.h"

#include "cpudata.h"
#include "fullmicrocodedcpu.h"

const auto NBit_t = Pep9::Definition::getStatusBitOffset(Pep9::uarch::EStatusBit::STATUS_N);
const auto ZBit_t = Pep9::Definition::getStatusBitOffset(Pep9::uarch::EStatusBit::STATUS_Z);
const auto VBit_t = Pep9::Definition::getStatusBitOffset(Pep9::uarch::EStatusBit::STATUS_V);
const auto CBit_t = Pep9::Definition::getStatusBitOffset(Pep9::uarch::EStatusBit::STATUS_C);
const auto SBit_t = Pep9::Definition::getStatusBitOffset(Pep9::uarch::EStatusBit::STATUS_S);

FullMicrocodedMemoizer::FullMicrocodedMemoizer(FullMicrocodedCPU& item): cpu(item),
    inOS(false), cyclesLast(0), cyclesUser(0), cyclesOS(0), stateUser(CPUState()), stateOS(CPUState())
{

}

void FullMicrocodedMemoizer::clear()
{
    stateUser = CPUState();
    stateOS = CPUState();
    inOS = false;
    cyclesLast = 0;
    cyclesUser = 0;
    cyclesOS = 0;
}

void FullMicrocodedMemoizer::storeStateInstrEnd()
{
    using namespace Pep9::ISA;

    // Determine the number of cycles elapsed during the last instruction
    auto cyclesElapsed = cpu.microCycleCounter - cyclesLast;
    // Increment the correct counter for who "spent" the cycles
    if(inOS) cyclesOS += cyclesElapsed;
    else cyclesUser += cyclesElapsed;
    cyclesLast = cpu.microCycleCounter;

    // Determine if the subsequent instruction will enter/exit operating system.
    auto mnemon = decodeMnemonic[cpu.data->getRegisterBank().getIRCache()];
    if(isTrapMap[mnemon]) {
        inOS = true;
    }
    else if (mnemon == EMnemonic::RETTR) {
        inOS = false;
    }
    cpu.getMemoryDevice()->onInstructionFinished(cpu.data->getRegisterBank().getIRCache());

}

void FullMicrocodedMemoizer::storeStateInstrStart()
{
    using namespace Pep9::uarch;

    quint8 instr;
    // Fetch the instruction specifier, located at the memory address of PC
    cpu.getMemoryDevice()->getByte(cpu.getCPURegWordStart(CPURegisters::PC), instr);
    if(inOS) {
        stateOS.instructionsCalled[instr]++;
        stateOS.instructionsExecuted++;
    }
    else {
        stateUser.instructionsCalled[instr]++;
        stateUser.instructionsExecuted++;
    }
    cpu.data->getRegisterBank().setIRCache(instr);
    calculateOpVal();
}

QString FullMicrocodedMemoizer::memoize()
{
    using namespace Pep9::uarch;

    const RegisterFile& file = cpu.data->getRegisterBank();
    SymbolTable* symTable = nullptr;
    auto pc = cpu.getCPURegWordStart(CPURegisters::PC);
    if(cpu.manager->getProgramAt(pc) != nullptr) {
        symTable = cpu.manager->getProgramAt(pc)
                ->getSymbolTable().get();
    }
    QString build, AX, NZVC;
    AX = QString(" A=%1, X=%2, SP=%3")
            .arg(formatNum(cpu.getCPURegWordCurrent(CPURegisters::A)),
                 formatNum(cpu.getCPURegWordCurrent(CPURegisters::X)),
                 formatNum(cpu.getCPURegWordCurrent(CPURegisters::SP)));
    quint8 tempByte = 0;
    tempByte |= file.readStatusBitCurrent(NBit_t) * Pep9::uarch::EMask::NMask;
    tempByte |= file.readStatusBitCurrent(ZBit_t) * Pep9::uarch::EMask::ZMask;
    tempByte |= file.readStatusBitCurrent(VBit_t) * Pep9::uarch::EMask::VMask;
    tempByte |= file.readStatusBitCurrent(CBit_t) * Pep9::uarch::EMask::CMask;
    NZVC = QString(" SNZVC=") % QString("%1").arg(QString::number(tempByte, 2), 5, '0');
    build = (attemptAddrReplace(symTable, pc) + QString(":")).leftJustified(10) %
            formatInstr(cpu.pep_version.get(), symTable, file.getIRCache(), cpu.getCPURegWordCurrent(CPURegisters::OS));
    build += "  " + AX;
    build += NZVC;
    return build;
}

QString FullMicrocodedMemoizer::finalStatistics(bool includeOS)
{
    using namespace Pep9::ISA;
    auto mnemon = EMnemonic::STOP;
    auto mnemonList = QList<EMnemonic>();
    mnemonList.append(mnemon);
    auto instrVector = getInstructionHistogram(includeOS);
    QList<quint32> tally = QList<quint32>();
    tally.append(0);
    int tallyIt = 0;
    for(int it = 0; it < 256; it++) {
        if(mnemon == decodeMnemonic[it]) {
            tally[tallyIt]+= instrVector[it];
        }
        else {
            tally.append(instrVector[it]);
            tallyIt++;
            mnemon = decodeMnemonic[it];
            mnemonList.append(mnemon);
        }
    }
    //qSort(tally);
    QString output = "";
    for(int index = 0; index < tally.length(); index++) {
        if(tally[index] == 0) continue;
        output.append(QString("%1").arg(mnemonDecode(mnemonList[index]),5) % QString(": ") % QString::number(tally[index]) % QString("\n"));
    }
    return output;
}

quint64 FullMicrocodedMemoizer::getCycleCount(bool includeOS)
{
    return cyclesUser + (includeOS ? cyclesOS : 0);
}

quint64 FullMicrocodedMemoizer::getInstructionCount(bool includeOS)
{
    return stateUser.instructionsExecuted + (includeOS ? stateOS.instructionsExecuted : 0);
}

const QVector<quint32> FullMicrocodedMemoizer::getInstructionHistogram(bool includeOS)
{

    QVector<quint32> result(256);
    for(int it=0; it<256; it++) {
        result[it] = stateUser.instructionsCalled[it] + (includeOS ? stateOS.instructionsCalled[it]: 0);
    }
    return result;
}

void FullMicrocodedMemoizer::calculateOpVal() const
{
    using namespace Pep9::uarch;

    quint8 instr;
    cpu.memory->getByte(cpu.getCPURegWordStart(CPURegisters::PC), instr);
    auto instrToExecute = Pep9::ISA::decodeMnemonic[instr];
    auto addrMode = Pep9::ISA::decodeAddrMode[instr];
    if(Pep9::ISA::isUnaryMap[instrToExecute]) {
        cpu.opValCache = 0;
        return;
    }
    quint16 opSpec;
    cpu.memory->getWord(cpu.getCPURegWordStart(CPURegisters::PC) +1 , opSpec);
    if(Pep9::ISA::isStoreMnemonic(instrToExecute)) {
        calculateOpValStoreHelper(addrMode, opSpec);
    }
    else if(Pep9::ISA::operandDisplayFieldWidth(instrToExecute) == 2) {
        calculateOpValByteHelper(addrMode, opSpec);
    }
    else {
        calculateopValWordHelper(addrMode, opSpec);
    }
}

void FullMicrocodedMemoizer::calculateOpValStoreHelper(Pep9::ISA::EAddrMode addrMode, quint16 opSpec) const
{
    using namespace Pep9::uarch;

    quint16 effectiveAddress = 0;
    switch(addrMode) {
    case Pep9::ISA::EAddrMode::I:
        break;
    case Pep9::ISA::EAddrMode::D:
        effectiveAddress = opSpec;
        break;
    case Pep9::ISA::EAddrMode::S:
        effectiveAddress = opSpec + cpu.getCPURegWordCurrent(CPURegisters::SP);
        break;
    case Pep9::ISA::EAddrMode::X:
        effectiveAddress = opSpec + cpu.getCPURegWordCurrent(CPURegisters::X);
        break;
    case Pep9::ISA::EAddrMode::SX:
        effectiveAddress = opSpec
                + cpu.getCPURegWordCurrent(CPURegisters::SP)
                + cpu.getCPURegWordCurrent(CPURegisters::X);
        break;
    case Pep9::ISA::EAddrMode::N:
        effectiveAddress = opSpec;
        cpu.memory->getWord(effectiveAddress, effectiveAddress);
        break;
    case Pep9::ISA::EAddrMode::SF:
        effectiveAddress = opSpec + cpu.getCPURegWordCurrent(CPURegisters::SP);
        cpu.memory->readWord(effectiveAddress, effectiveAddress);
        break;
    case Pep9::ISA::EAddrMode::SFX:
        effectiveAddress = opSpec + cpu.getCPURegWordCurrent(CPURegisters::SP);
        cpu.memory->readWord(effectiveAddress, effectiveAddress);
        effectiveAddress += cpu.getCPURegWordCurrent(CPURegisters::X);
        break;
    default:
        break;
    }
    cpu.opValCache = effectiveAddress;
}

void FullMicrocodedMemoizer::calculateOpValByteHelper(Pep9::ISA::EAddrMode addrMode, quint16 opSpec) const
{
    using namespace Pep9::uarch;

    quint16 effectiveAddress = 0;
    quint8 opVal = 0;
    switch(addrMode) {
    case Pep9::ISA::EAddrMode::I:
        // Only store the low order byte of the operand specifier.
        opVal = static_cast<quint8>(opSpec);
        break;
    case Pep9::ISA::EAddrMode::D:
        effectiveAddress = opSpec;
        cpu.memory->getByte(effectiveAddress, opVal);
        break;
    case Pep9::ISA::EAddrMode::S:
        effectiveAddress = opSpec + cpu.getCPURegWordCurrent(CPURegisters::SP);
        cpu.memory->getByte(effectiveAddress, opVal);
        break;
    case Pep9::ISA::EAddrMode::X:
        effectiveAddress = opSpec + cpu.getCPURegWordCurrent(CPURegisters::X);
        cpu.memory->getByte(effectiveAddress, opVal);
        break;
    case Pep9::ISA::EAddrMode::SX:
        effectiveAddress = opSpec
                + cpu.getCPURegWordCurrent(CPURegisters::SP)
                + cpu.getCPURegWordCurrent(CPURegisters::X);
        cpu.memory->getByte(effectiveAddress, opVal);
        break;
    case Pep9::ISA::EAddrMode::N:
        effectiveAddress = opSpec;
        cpu.memory->getWord(effectiveAddress, effectiveAddress);
        cpu.memory->getByte(effectiveAddress, opVal);
        break;
    case Pep9::ISA::EAddrMode::SF:
        effectiveAddress = opSpec + cpu.getCPURegWordCurrent(CPURegisters::SP);
        cpu.memory->getWord(effectiveAddress, effectiveAddress);
        cpu.memory->getByte(effectiveAddress, opVal);
        break;
    case Pep9::ISA::EAddrMode::SFX:
        effectiveAddress = opSpec + cpu.getCPURegWordCurrent(CPURegisters::SP);
        cpu.memory->getWord(effectiveAddress, effectiveAddress);
        effectiveAddress += cpu.getCPURegWordCurrent(CPURegisters::X);
        cpu.memory->getByte(effectiveAddress, opVal);
        break;
    default:
        break;
    }
    cpu.opValCache = opVal;
}

void FullMicrocodedMemoizer::calculateopValWordHelper(Pep9::ISA::EAddrMode addrMode, quint16 opSpec) const
{
    using namespace Pep9::uarch;

    quint16 effectiveAddress = 0;
    quint16 opVal = 0;
    switch(addrMode) {
    case Pep9::ISA::EAddrMode::I:
        opVal = opSpec;
        break;
    case Pep9::ISA::EAddrMode::D:
        effectiveAddress = opSpec;
        cpu.memory->getWord(effectiveAddress, opVal);
        break;
    case Pep9::ISA::EAddrMode::S:
        effectiveAddress = opSpec + cpu.getCPURegWordCurrent(CPURegisters::SP);
        cpu.memory->getWord(effectiveAddress, opVal);
        break;
    case Pep9::ISA::EAddrMode::X:
        effectiveAddress = opSpec + cpu.getCPURegWordCurrent(CPURegisters::X);
        cpu.memory->getWord(effectiveAddress, opVal);
        break;
    case Pep9::ISA::EAddrMode::SX:
        effectiveAddress = opSpec
                + cpu.getCPURegWordCurrent(CPURegisters::SP)
                + cpu.getCPURegWordCurrent(CPURegisters::X);
        cpu.memory->getWord(effectiveAddress, opVal);
        break;
    case Pep9::ISA::EAddrMode::N:
        effectiveAddress = opSpec;
        cpu.memory->getWord(effectiveAddress, effectiveAddress);
        cpu.memory->getWord(effectiveAddress, opVal);
        break;
    case Pep9::ISA::EAddrMode::SF:
        effectiveAddress = opSpec + cpu.getCPURegWordCurrent(CPURegisters::SP);
        cpu.memory->getWord(effectiveAddress, effectiveAddress);
        cpu.memory->getWord(effectiveAddress, opVal);
        break;
    case Pep9::ISA::EAddrMode::SFX:
        effectiveAddress = opSpec + cpu.getCPURegWordCurrent(CPURegisters::SP);
        cpu.memory->getWord(effectiveAddress, effectiveAddress);
        effectiveAddress += cpu.getCPURegWordCurrent(CPURegisters::X);
        cpu.memory->getWord(effectiveAddress, opVal);
        break;
    default:
        break;
    }
    cpu.opValCache = opVal;
}
