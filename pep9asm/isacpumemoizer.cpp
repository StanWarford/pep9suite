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

IsaCpuMemoizer::IsaCpuMemoizer(IsaCpu &cpu): cpu(cpu), state(CPUState())
{

}

IsaCpuMemoizer::~IsaCpuMemoizer()
{

}

void IsaCpuMemoizer::clear()
{
    state = CPUState();
}

void IsaCpuMemoizer::storeStateInstrEnd()
{

}

void IsaCpuMemoizer::storeStateInstrStart()
{
    quint8 instr;
    // Fetch the instruction specifier, located at the memory address of PC
    cpu.getMemoryDevice()->getByte(cpu.registerBank.readRegisterWordStart(Enu::CPURegisters::PC), instr);
    state.instructionsCalled[instr]++;
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
    ir = file.getIRCache();
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

QString IsaCpuMemoizer::finalStatistics()
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

