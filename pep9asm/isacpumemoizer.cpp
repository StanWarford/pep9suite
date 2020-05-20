// File: isacpumemoizer.cpp
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
#include "isacpumemoizer.h"
#include "isacpu.h"
#include "pep.h"
#include "asmprogram.h"
#include "asmprogrammanager.h"
#include "amemorydevice.h"
#include "registerfile.h"
#include <assert.h>
#include <QString>
#include <QtCore>
#include <QDebug>
#include <QStack>

IsaCpuMemoizer::IsaCpuMemoizer(IsaCpu &cpu): cpu(cpu), inOS(false), stateUser(CPUState()), stateOS(CPUState())
{

}

IsaCpuMemoizer::~IsaCpuMemoizer()
{

}

void IsaCpuMemoizer::clear()
{
    stateUser = CPUState();
    stateOS = CPUState();
}

void IsaCpuMemoizer::storeStateInstrEnd()
{
    auto mnemon = Pep::decodeMnemonic[cpu.registerBank.getIRCache()];
    if(Pep::isTrapMap[mnemon]) {
        inOS = true;
    }
    else if (mnemon == Enu::EMnemonic::RETTR) {
        inOS = false;
    }
    cpu.getMemoryDevice()->onInstructionFinished(cpu.registerBank.getIRCache());
}

void IsaCpuMemoizer::storeStateInstrStart()
{
    quint8 instr;

    // Fetch the instruction specifier, located at the memory address of PC
    cpu.getMemoryDevice()->getByte(cpu.registerBank.readRegisterWordStart(Enu::CPURegisters::PC), instr);
    if(inOS) {
        stateOS.instructionsCalled[instr]++;
        stateOS.instructionsExecuted++;
    }
    else {
        stateUser.instructionsCalled[instr]++;
        stateUser.instructionsExecuted++;
    }
    cpu.registerBank.setIRCache(instr);
}

QString IsaCpuMemoizer::memoize()
{
    const RegisterFile& file = cpu.registerBank;
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
    build += "  " + AX;
    build += NZVC;
    return build;
}

QString IsaCpuMemoizer::finalStatistics(bool includeOS)
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

quint64 IsaCpuMemoizer::getCycleCount(bool includeOS)
{
    return stateUser.instructionsExecuted + (includeOS ? stateOS.instructionsExecuted : 0);
}

quint64 IsaCpuMemoizer::getInstructionCount(bool includeOS)
{
    return stateUser.instructionsExecuted + (includeOS ? stateOS.instructionsExecuted : 0);
}

const QVector<quint32> IsaCpuMemoizer::getInstructionHistogram(bool includeOS)
{
    QVector<quint32> result(256);
    for(int it=0; it<256; it++) {
        result[it] = stateUser.instructionsCalled[it] + (includeOS ? stateOS.instructionsCalled[it]: 0);
    }
    return result;
}

