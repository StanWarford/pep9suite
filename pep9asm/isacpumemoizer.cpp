#include "isacpumemoizer.h"
#include "isacpu.h"
#include "pep.h"
#include "amemorydevice.h"
#include "registerfile.h"
#include <assert.h>
#include <QString>
#include <QtCore>
#include <QDebug>
#include <QStack>

IsaCpuMemoizer::IsaCpuMemoizer(IsaCpu &cpu): cpu(cpu), state(IsaCpuMemoizer::CPUState()), OSSymTable()
{

}

IsaCpuMemoizer::~IsaCpuMemoizer()
{

}

const QString stackFrameEnter("%1\n===CALL===\n");
const QString stackFrameLeave("%1\n===RET====\n");
const QString trapEnter("%1\n===TRAP===\n");
const QString trapLeave("%1\n===RETR===\n");
static quint8 max_symLen=0;
static quint8 inst_size=6;
static quint8 oper_addr_size=12;

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
    quint8 ir = 0;
    QString build, AX, NZVC;
    AX = QString(" A=%1, X=%2, SP=%3")

            .arg(formatNum(file.readRegisterWordCurrent(Enu::CPURegisters::A)),
                 formatNum(file.readRegisterWordCurrent(Enu::CPURegisters::X)),
                 formatNum(file.readRegisterWordCurrent(Enu::CPURegisters::SP)));
    NZVC = QString(" SNZVC=") % QString("%1").arg(QString::number(file.readStatusBitsCurrent(), 2), 5, '0');
    build = (attempSymAddrReplace(file.readRegisterWordStart(Enu::CPURegisters::PC)) + QString(":")).leftJustified(10) %
            formatInstr(file.getIRCache(), file.readRegisterWordCurrent(Enu::CPURegisters::OS));
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

// Properly formats a number as a 4 char hex
QString IsaCpuMemoizer::formatNum(quint16 number)
{
    return QString("%1").arg(QString::number(number,16),4,'0').toUpper();
}

// Properly format a number as 2 char hex
QString IsaCpuMemoizer::formatNum(quint8 number)
{
    return QString("%1").arg(QString::number(number,16),2,'0').toUpper();
}

// Properly format a 16 bit address
QString IsaCpuMemoizer::formatAddress(quint16 address)
{
    return "0x"+formatNum(address);
}

// Convert a mnemonic into it's string
QString IsaCpuMemoizer::mnemonDecode(quint8 instrSpec)
{
    static QMetaEnum metaenum = Enu::staticMetaObject.enumerator(Enu::staticMetaObject.indexOfEnumerator("EMnemonic"));
    return QString(metaenum.valueToKey((int)Pep::decodeMnemonic[instrSpec])).toLower();
}

// Convert a mnemonic into its string
QString IsaCpuMemoizer::mnemonDecode(Enu::EMnemonic instrSpec)
{
    static QMetaEnum metaenum = Enu::staticMetaObject.enumerator(Enu::staticMetaObject.indexOfEnumerator("EMnemonic"));
    return QString(metaenum.valueToKey((int)instrSpec)).toLower();
}

QString IsaCpuMemoizer::formatIS(quint8 instrSpec)
{
    return QString(mnemonDecode(instrSpec)).leftJustified(inst_size,' ');
}

QString IsaCpuMemoizer::formatUnary(quint8 instrSpec)
{
    return formatIS(instrSpec).leftJustified(inst_size+max_symLen+2+4);
}

QString IsaCpuMemoizer::formatNonUnary(quint8 instrSpec,quint16 oprSpec)
{
    return formatIS(instrSpec).leftJustified(inst_size) %
            QString(attempSymOprReplace(oprSpec)).rightJustified(max_symLen) %
            ", " % Pep::intToAddrMode(Pep::decodeAddrMode[instrSpec]).leftJustified(4,' ');
}

QString IsaCpuMemoizer::formatInstr(quint8 instrSpec,quint16 oprSpec)
{
    if(Pep::isUnaryMap[Pep::decodeMnemonic[instrSpec]])
    {
        return formatUnary(instrSpec);
    }
    else
    {
        return formatNonUnary(instrSpec,oprSpec);
    }
}

QString IsaCpuMemoizer::generateStackFrame(CPUState&, bool enter)
{
    return "";
    if(enter) {
        //return stackFrameEnter.arg(" SP=%1").arg(formatAddress(registers.regState.reg_SP));
    }
    else {
        //return stackFrameLeave.arg(" SP=%1").arg(formatAddress(registers.regState.reg_SP));
    }
}

QString IsaCpuMemoizer::generateTrapFrame(CPUState&, bool enter)
{
    return "";
    if(enter) {
        //return trapEnter.arg(" SP=%1").arg(formatAddress(registers.regState.reg_SP));
    }
    else {
        //return trapLeave.arg(" SP=%1").arg(formatAddress(registers.regState.reg_SP));
    }
}

QString IsaCpuMemoizer::attempSymOprReplace(quint16 number)
{
    if(OSSymTable.count(number) == 1) return OSSymTable.find(number).value();
    else return formatNum(number);
}

QString IsaCpuMemoizer::attempSymAddrReplace(quint16 number)
{
    if(OSSymTable.count(number) == 1) return OSSymTable.find(number).value();
    else return formatAddress(number);
}

void IsaCpuMemoizer::loadSymbols()
{
    QString  osFileString;
    //In the future, have a switch between loading the aligned and unaligned code
    if(true) osFileString = (":/help/osunalignedsymbols.txt");
    else osFileString = ("nope");
    QFile osFile(osFileString);
    quint8 msylen=  0;
    if(osFile.open(QFile::ReadOnly)) {
        bool temp = 0;
        QTextStream file(&osFile);
        while(!file.atEnd()) {
            QString text = file.readLine();
            QList<QString> parts =text.split(' ',QString::SplitBehavior::SkipEmptyParts );
            quint16 key = parts[1].toUShort(&temp,16);
            OSSymTable.insert(key,parts[0]);
            msylen = msylen<parts[0].length() ? parts[0].length() : msylen;
        }
    }
    oper_addr_size = msylen+3;
    max_symLen = msylen;
}
