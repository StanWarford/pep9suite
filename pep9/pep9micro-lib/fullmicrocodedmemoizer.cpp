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
#include "cpu/cpudata.h"
#include "cpu/registerfile.h"
#include "memory/amemorydevice.h"
#include "pep/pep.h"

#include "fullmicrocodedcpu.h"

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

    // Determine the number of cycles elapsed during the last instruction
    auto cyclesElapsed = cpu.microCycleCounter - cyclesLast;
    // Increment the correct counter for who "spent" the cycles
    if(inOS) cyclesOS += cyclesElapsed;
    else cyclesUser += cyclesElapsed;
    cyclesLast = cpu.microCycleCounter;

    // Determine if the subsequent instruction will enter/exit operating system.
    auto mnemon = Pep::decodeMnemonic[cpu.data->getRegisterBank().getIRCache()];
    if(Pep::isTrapMap[mnemon]) {
        inOS = true;
    }
    else if (mnemon == Enu::EMnemonic::RETTR) {
        inOS = false;
    }
    cpu.getMemoryDevice()->onInstructionFinished(cpu.data->getRegisterBank().getIRCache());

}

void FullMicrocodedMemoizer::storeStateInstrStart()
{
    quint8 instr;
    // Fetch the instruction specifier, located at the memory address of PC
    cpu.getMemoryDevice()->getByte(cpu.getCPURegWordStart(Pep9::CPURegisters::PC), instr);
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
    const RegisterFile& file = cpu.data->getRegisterBank();
    SymbolTable* symTable = nullptr;
    auto pc = cpu.getCPURegWordStart(Pep9::CPURegisters::PC);
    if(cpu.manager->getProgramAt(pc) != nullptr) {
        symTable = cpu.manager->getProgramAt(pc)
                ->getSymbolTable().get();
    }
    QString build, AX, NZVC;
    AX = QString(" A=%1, X=%2, SP=%3")
            .arg(formatNum(cpu.getCPURegWordCurrent(Pep9::CPURegisters::A)),
                 formatNum(cpu.getCPURegWordCurrent(Pep9::CPURegisters::X)),
                 formatNum(cpu.getCPURegWordCurrent(Pep9::CPURegisters::SP)));
    NZVC = QString(" SNZVC=") % QString("%1").arg(QString::number(file.readStatusBitsCurrent(), 2), 5, '0');
    build = (attemptAddrReplace(symTable, pc) + QString(":")).leftJustified(10) %
            formatInstr(symTable, file.getIRCache(), cpu.getCPURegWordCurrent(Pep9::CPURegisters::OS));
    build += "  " + AX;
    build += NZVC;
    return build;
}

QString FullMicrocodedMemoizer::finalStatistics(bool includeOS)
{
    Enu::EMnemonic mnemon = Enu::EMnemonic::STOP;
    QList<Enu::EMnemonic> mnemonList = QList<Enu::EMnemonic>();
    mnemonList.append(mnemon);
    auto instrVector = getInstructionHistogram(includeOS);
    QList<quint32> tally = QList<quint32>();
    tally.append(0);
    int tallyIt = 0;
    for(int it = 0; it < 256; it++) {
        if(mnemon == Pep::decodeMnemonic[it]) {
            tally[tallyIt]+= instrVector[it];
        }
        else {
            tally.append(instrVector[it]);
            tallyIt++;
            mnemon = Pep::decodeMnemonic[it];
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
    quint8 instr;
    cpu.memory->getByte(cpu.getCPURegWordStart(Pep9::CPURegisters::PC), instr);
    Enu::EMnemonic instrToExecute = Pep::decodeMnemonic[instr];
    Enu::EAddrMode addrMode = Pep::decodeAddrMode[instr];
    if(Pep::isUnaryMap[instrToExecute]) {
        cpu.opValCache = 0;
        return;
    }
    quint16 opSpec;
    cpu.memory->getWord(cpu.getCPURegWordStart(Pep9::CPURegisters::PC) +1 , opSpec);
    if(Pep::isStoreMnemonic(instrToExecute)) {
        calculateOpValStoreHelper(addrMode, opSpec);
    }
    else if(Pep::operandDisplayFieldWidth(instrToExecute) == 2) {
        calculateOpValByteHelper(addrMode, opSpec);
    }
    else {
        calculateopValWordHelper(addrMode, opSpec);
    }
}

void FullMicrocodedMemoizer::calculateOpValStoreHelper(Enu::EAddrMode addrMode, quint16 opSpec) const
{
    quint16 effectiveAddress = 0;
    switch(addrMode) {
    case Enu::EAddrMode::I:
        break;
    case Enu::EAddrMode::D:
        effectiveAddress = opSpec;
        break;
    case Enu::EAddrMode::S:
        effectiveAddress = opSpec + cpu.getCPURegWordCurrent(Pep9::CPURegisters::SP);
        break;
    case Enu::EAddrMode::X:
        effectiveAddress = opSpec + cpu.getCPURegWordCurrent(Pep9::CPURegisters::X);
        break;
    case Enu::EAddrMode::SX:
        effectiveAddress = opSpec
                + cpu.getCPURegWordCurrent(Pep9::CPURegisters::SP)
                + cpu.getCPURegWordCurrent(Pep9::CPURegisters::X);
        break;
    case Enu::EAddrMode::N:
        effectiveAddress = opSpec;
        cpu.memory->getWord(effectiveAddress, effectiveAddress);
        break;
    case Enu::EAddrMode::SF:
        effectiveAddress = opSpec + cpu.getCPURegWordCurrent(Pep9::CPURegisters::SP);
        cpu.memory->readWord(effectiveAddress, effectiveAddress);
        break;
    case Enu::EAddrMode::SFX:
        effectiveAddress = opSpec + cpu.getCPURegWordCurrent(Pep9::CPURegisters::SP);
        cpu.memory->readWord(effectiveAddress, effectiveAddress);
        effectiveAddress += cpu.getCPURegWordCurrent(Pep9::CPURegisters::X);
        break;
    default:
        break;
    }
    cpu.opValCache = effectiveAddress;
}

void FullMicrocodedMemoizer::calculateOpValByteHelper(Enu::EAddrMode addrMode, quint16 opSpec) const
{
    quint16 effectiveAddress = 0;
    quint8 opVal = 0;
    switch(addrMode) {
    case Enu::EAddrMode::I:
        // Only store the low order byte of the operand specifier.
        opVal = static_cast<quint8>(opSpec);
        break;
    case Enu::EAddrMode::D:
        effectiveAddress = opSpec;
        cpu.memory->getByte(effectiveAddress, opVal);
        break;
    case Enu::EAddrMode::S:
        effectiveAddress = opSpec + cpu.getCPURegWordCurrent(Pep9::CPURegisters::SP);
        cpu.memory->getByte(effectiveAddress, opVal);
        break;
    case Enu::EAddrMode::X:
        effectiveAddress = opSpec + cpu.getCPURegWordCurrent(Pep9::CPURegisters::X);
        cpu.memory->getByte(effectiveAddress, opVal);
        break;
    case Enu::EAddrMode::SX:
        effectiveAddress = opSpec
                + cpu.getCPURegWordCurrent(Pep9::CPURegisters::SP)
                + cpu.getCPURegWordCurrent(Pep9::CPURegisters::X);
        cpu.memory->getByte(effectiveAddress, opVal);
        break;
    case Enu::EAddrMode::N:
        effectiveAddress = opSpec;
        cpu.memory->getWord(effectiveAddress, effectiveAddress);
        cpu.memory->getByte(effectiveAddress, opVal);
        break;
    case Enu::EAddrMode::SF:
        effectiveAddress = opSpec + cpu.getCPURegWordCurrent(Pep9::CPURegisters::SP);
        cpu.memory->getWord(effectiveAddress, effectiveAddress);
        cpu.memory->getByte(effectiveAddress, opVal);
        break;
    case Enu::EAddrMode::SFX:
        effectiveAddress = opSpec + cpu.getCPURegWordCurrent(Pep9::CPURegisters::SP);
        cpu.memory->getWord(effectiveAddress, effectiveAddress);
        effectiveAddress += cpu.getCPURegWordCurrent(Pep9::CPURegisters::X);
        cpu.memory->getByte(effectiveAddress, opVal);
        break;
    default:
        break;
    }
    cpu.opValCache = opVal;
}

void FullMicrocodedMemoizer::calculateopValWordHelper(Enu::EAddrMode addrMode, quint16 opSpec) const
{
    quint16 effectiveAddress = 0;
    quint16 opVal = 0;
    switch(addrMode) {
    case Enu::EAddrMode::I:
        opVal = opSpec;
        break;
    case Enu::EAddrMode::D:
        effectiveAddress = opSpec;
        cpu.memory->getWord(effectiveAddress, opVal);
        break;
    case Enu::EAddrMode::S:
        effectiveAddress = opSpec + cpu.getCPURegWordCurrent(Pep9::CPURegisters::SP);
        cpu.memory->getWord(effectiveAddress, opVal);
        break;
    case Enu::EAddrMode::X:
        effectiveAddress = opSpec + cpu.getCPURegWordCurrent(Pep9::CPURegisters::X);
        cpu.memory->getWord(effectiveAddress, opVal);
        break;
    case Enu::EAddrMode::SX:
        effectiveAddress = opSpec
                + cpu.getCPURegWordCurrent(Pep9::CPURegisters::SP)
                + cpu.getCPURegWordCurrent(Pep9::CPURegisters::X);
        cpu.memory->getWord(effectiveAddress, opVal);
        break;
    case Enu::EAddrMode::N:
        effectiveAddress = opSpec;
        cpu.memory->getWord(effectiveAddress, effectiveAddress);
        cpu.memory->getWord(effectiveAddress, opVal);
        break;
    case Enu::EAddrMode::SF:
        effectiveAddress = opSpec + cpu.getCPURegWordCurrent(Pep9::CPURegisters::SP);
        cpu.memory->getWord(effectiveAddress, effectiveAddress);
        cpu.memory->getWord(effectiveAddress, opVal);
        break;
    case Enu::EAddrMode::SFX:
        effectiveAddress = opSpec + cpu.getCPURegWordCurrent(Pep9::CPURegisters::SP);
        cpu.memory->getWord(effectiveAddress, effectiveAddress);
        effectiveAddress += cpu.getCPURegWordCurrent(Pep9::CPURegisters::X);
        cpu.memory->getWord(effectiveAddress, opVal);
        break;
    default:
        break;
    }
    cpu.opValCache = opVal;
}
