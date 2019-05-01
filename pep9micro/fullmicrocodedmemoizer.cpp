#include "fullmicrocodedmemoizer.h"
#include "fullmicrocodedcpu.h"
#include "pep.h"
#include "amemorydevice.h"
#include "asmprogram.h"
#include "asmprogrammanager.h"
#include "cpudata.h"
#include "registerfile.h"
#include <assert.h>
#include <QString>
#include <QtCore>
#include <QDebug>
#include <QStack>

FullMicrocodedMemoizer::FullMicrocodedMemoizer(FullMicrocodedCPU& item): cpu(item),
    state(CPUState())
{

}

void FullMicrocodedMemoizer::clear()
{
    state = CPUState();
}

void FullMicrocodedMemoizer::storeStateInstrEnd()
{

}

void FullMicrocodedMemoizer::storeStateInstrStart()
{
    quint8 instr;
    // Fetch the instruction specifier, located at the memory address of PC
    cpu.getMemoryDevice()->getByte(cpu.data->getRegisterBank()
                                   .readRegisterWordStart(Enu::CPURegisters::PC), instr);
    state.instructionsCalled[instr]++;
    cpu.data->getRegisterBank().setIRCache(instr);
    calculateOpVal();
}

QString FullMicrocodedMemoizer::memoize()
{
    const RegisterFile& file = cpu.data->getRegisterBank();
    SymbolTable* symTable = nullptr;
    if(cpu.manager->getProgramAt(file.readRegisterWordStart(Enu::CPURegisters::PC)) != nullptr) {
        symTable = cpu.manager->getProgramAt(file.readRegisterWordStart(Enu::CPURegisters::PC))
                ->getSymbolTable().get();
    }
    quint8 ir = 0;
    QString build, AX, NZVC;
    AX = QString(" A=%1, X=%2, SP=%3")

            .arg(formatNum(file.readRegisterWordCurrent(Enu::CPURegisters::A)),
                 formatNum(file.readRegisterWordCurrent(Enu::CPURegisters::X)),
                 formatNum(file.readRegisterWordCurrent(Enu::CPURegisters::SP)));
    NZVC = QString(" SNZVC=") % QString("%1").arg(QString::number(file.readStatusBitsCurrent(), 2), 5, '0');
    build = (attemptAddrReplace(symTable, file.readRegisterWordStart(Enu::CPURegisters::PC)) + QString(":")).leftJustified(10) %
            formatInstr(symTable, file.getIRCache(), file.readRegisterWordCurrent(Enu::CPURegisters::OS));
    build += "  " + AX;
    build += NZVC;
    ir = cpu.data->getRegisterBank().getIRCache();
    if(Pep::isTrapMap[Pep::decodeMnemonic[ir]]) {
        build += generateTrapFrame(state);
    }
    else if(Pep::decodeMnemonic[ir] == Enu::EMnemonic::RETTR) {
        build += generateTrapFrame(state,false);
    }
    else if(Pep::decodeMnemonic[ir] == Enu::EMnemonic::CALL) {
        build += generateStackFrame(state);
    }
    else if(Pep::decodeMnemonic[ir] == Enu::EMnemonic::RET) {
        build += generateStackFrame(state,false);
    }
    return build;
}

QString FullMicrocodedMemoizer::finalStatistics()
{
    Enu::EMnemonic mnemon = Enu::EMnemonic::STOP;
    QList<Enu::EMnemonic> mnemonList = QList<Enu::EMnemonic>();
    mnemonList.append(mnemon);
    QList<quint32> tally = QList<quint32>();
    tally.append(0);
    int tallyIt = 0;
    for(int it = 0; it < 256; it++) {
        if(mnemon == Pep::decodeMnemonic[it]) {
            tally[tallyIt]+= state.instructionsCalled[it];
        }
        else {
            tally.append(state.instructionsCalled[it]);
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

void FullMicrocodedMemoizer::calculateOpVal() const
{
    quint8 instr;
    cpu.memory->getByte(cpu.getCPURegWordStart(Enu::CPURegisters::PC), instr);
    Enu::EMnemonic instrToExecute = Pep::decodeMnemonic[instr];
    Enu::EAddrMode addrMode = Pep::decodeAddrMode[instr];
    if(Pep::isUnaryMap[instrToExecute]) {
        cpu.opValCache = 0;
        return;
    }
    quint16 opSpec;
    cpu.memory->getWord(cpu.getCPURegWordStart(Enu::CPURegisters::PC) +1 , opSpec);
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
        effectiveAddress = opSpec + cpu.getCPURegWordCurrent(Enu::CPURegisters::SP);
        break;
    case Enu::EAddrMode::X:
        effectiveAddress = opSpec + cpu.getCPURegWordCurrent(Enu::CPURegisters::X);
        break;
    case Enu::EAddrMode::SX:
        effectiveAddress = opSpec
                + cpu.getCPURegWordCurrent(Enu::CPURegisters::SP)
                + cpu.getCPURegWordCurrent(Enu::CPURegisters::X);
        break;
    case Enu::EAddrMode::N:
        effectiveAddress = opSpec;
        cpu.memory->getWord(effectiveAddress, effectiveAddress);
        break;
    case Enu::EAddrMode::SF:
        effectiveAddress = opSpec + cpu.getCPURegWordCurrent(Enu::CPURegisters::SP);
        cpu.memory->readWord(effectiveAddress, effectiveAddress);
        break;
    case Enu::EAddrMode::SFX:
        effectiveAddress = opSpec + cpu.getCPURegWordCurrent(Enu::CPURegisters::SP);
        cpu.memory->readWord(effectiveAddress, effectiveAddress);
        effectiveAddress += cpu.getCPURegWordCurrent(Enu::CPURegisters::X);
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
        effectiveAddress = opSpec + cpu.getCPURegWordCurrent(Enu::CPURegisters::SP);
        cpu.memory->getByte(effectiveAddress, opVal);
        break;
    case Enu::EAddrMode::X:
        effectiveAddress = opSpec + cpu.getCPURegWordCurrent(Enu::CPURegisters::X);
        cpu.memory->getByte(effectiveAddress, opVal);
        break;
    case Enu::EAddrMode::SX:
        effectiveAddress = opSpec
                + cpu.getCPURegWordCurrent(Enu::CPURegisters::SP)
                + cpu.getCPURegWordCurrent(Enu::CPURegisters::X);
        cpu.memory->getByte(effectiveAddress, opVal);
        break;
    case Enu::EAddrMode::N:
        effectiveAddress = opSpec;
        cpu.memory->getWord(effectiveAddress, effectiveAddress);
        cpu.memory->getByte(effectiveAddress, opVal);
        break;
    case Enu::EAddrMode::SF:
        effectiveAddress = opSpec + cpu.getCPURegWordCurrent(Enu::CPURegisters::SP);
        cpu.memory->getWord(effectiveAddress, effectiveAddress);
        cpu.memory->getByte(effectiveAddress, opVal);
        break;
    case Enu::EAddrMode::SFX:
        effectiveAddress = opSpec + cpu.getCPURegWordCurrent(Enu::CPURegisters::SP);
        cpu.memory->getWord(effectiveAddress, effectiveAddress);
        effectiveAddress += cpu.getCPURegWordCurrent(Enu::CPURegisters::X);
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
        effectiveAddress = opSpec + cpu.getCPURegWordCurrent(Enu::CPURegisters::SP);
        cpu.memory->getWord(effectiveAddress, opVal);
        break;
    case Enu::EAddrMode::X:
        effectiveAddress = opSpec + cpu.getCPURegWordCurrent(Enu::CPURegisters::X);
        cpu.memory->getWord(effectiveAddress, opVal);
        break;
    case Enu::EAddrMode::SX:
        effectiveAddress = opSpec
                + cpu.getCPURegWordCurrent(Enu::CPURegisters::SP)
                + cpu.getCPURegWordCurrent(Enu::CPURegisters::X);
        cpu.memory->getWord(effectiveAddress, opVal);
        break;
    case Enu::EAddrMode::N:
        effectiveAddress = opSpec;
        cpu.memory->getWord(effectiveAddress, effectiveAddress);
        cpu.memory->getWord(effectiveAddress, opVal);
        break;
    case Enu::EAddrMode::SF:
        effectiveAddress = opSpec + cpu.getCPURegWordCurrent(Enu::CPURegisters::SP);
        cpu.memory->getWord(effectiveAddress, effectiveAddress);
        cpu.memory->getWord(effectiveAddress, opVal);
        break;
    case Enu::EAddrMode::SFX:
        effectiveAddress = opSpec + cpu.getCPURegWordCurrent(Enu::CPURegisters::SP);
        cpu.memory->getWord(effectiveAddress, effectiveAddress);
        effectiveAddress += cpu.getCPURegWordCurrent(Enu::CPURegisters::X);
        cpu.memory->getWord(effectiveAddress, opVal);
        break;
    default:
        break;
    }
    cpu.opValCache = opVal;
}
