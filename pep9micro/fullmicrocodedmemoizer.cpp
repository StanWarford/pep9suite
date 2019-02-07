#include "fullmicrocodedmemoizer.h"
#include "fullmicrocodedcpu.h"
#include "pep.h"
#include "amemorydevice.h"
#include "newcpudata.h"
#include "registerfile.h"
#include <assert.h>
#include <QString>
#include <QtCore>
#include <QDebug>
#include <QStack>
const QString stackFrameEnter("%1\n===CALL===\n");
const QString stackFrameLeave("%1\n===RET====\n");
const QString trapEnter("%1\n===TRAP===\n");
const QString trapLeave("%1\n===RETR===\n");
static quint8 max_symLen=0;
static quint8 inst_size=6;
static quint8 oper_addr_size=12;

FullMicrocodedMemoizer::FullMicrocodedMemoizer(FullMicrocodedCPU& item): cpu(item),
    state(CPUState()), level(Enu::DebugLevels::MINIMAL), OSSymTable()
{

}

Enu::DebugLevels FullMicrocodedMemoizer::getDebugLevel() const
{
    return level;
}

void FullMicrocodedMemoizer::clear()
{
    state = CPUState();
}

void FullMicrocodedMemoizer::storeStateInstrEnd()
{
    switch(level)
    {
    case Enu::DebugLevels::ALL:
        //Intentional fallthrough
        [[fallthrough]];
    case Enu::DebugLevels::MINIMAL:
        //Intentional fallthrough
        [[fallthrough]];
    case Enu::DebugLevels::NONE:
        #pragma message("This calculation is not quite right")
        break;
    }
}

void FullMicrocodedMemoizer::storeStateInstrStart()
{
    quint8 instr;
    switch(level)
    {
    case Enu::DebugLevels::ALL:
        //Intentional fallthrough
        [[fallthrough]];
    case Enu::DebugLevels::MINIMAL:
        //Intentional fallthrough
        [[fallthrough]];
    case Enu::DebugLevels::NONE:
        // Fetch the instruction specifier, located at the memory address of PC
        cpu.getMemoryDevice()->getByte(cpu.data->getRegisterBank()
                                       .readRegisterWordStart(Enu::CPURegisters::PC), instr);
        state.instructionsCalled[instr]++;
        cpu.data->getRegisterBank().setIRCache(instr);
        break;
    }
}

QString FullMicrocodedMemoizer::memoize()
{
    const RegisterFile& file = cpu.data->getRegisterBank();
    quint8 ir = 0;
    QString build, AX, NZVC;
    switch(level) {
    case Enu::DebugLevels::ALL:
        [[fallthrough]];
    case Enu::DebugLevels::MINIMAL:
        AX = QString(" A=%1, X=%2, SP=%3")

                .arg(formatNum(file.readRegisterWordCurrent(Enu::CPURegisters::A)),
                     formatNum(file.readRegisterWordCurrent(Enu::CPURegisters::X)),
                     formatNum(file.readRegisterWordCurrent(Enu::CPURegisters::SP)));
        NZVC = QString(" SNZVC=") % QString("%1").arg(QString::number(file.readStatusBitsCurrent(), 2), 5, '0');
        build = (attempSymAddrReplace(file.readRegisterWordStart(Enu::CPURegisters::PC)) + QString(":")).leftJustified(10) %
                formatInstr(file.getIRCache(), file.readRegisterWordCurrent(Enu::CPURegisters::OS));
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
        break;
    case Enu::DebugLevels::NONE:
        break;
    }
    return build;
}

QString FullMicrocodedMemoizer::finalStatistics()
{
    if(level == Enu::DebugLevels::NONE) return "";
    Enu::EMnemonic mnemon = Enu::EMnemonic::STOP;
    QList<Enu::EMnemonic> mnemonList = QList<Enu::EMnemonic>();
    mnemonList.append(mnemon);
    QList<quint32> tally = QList<quint32>();
    tally.append(0);
    int tallyIt=0;
    for(int it=0; it<256; it++)
    {
        if(mnemon == Pep::decodeMnemonic[it])
        {
            tally[tallyIt]+= state.instructionsCalled[it];
        }
        else
        {
            tally.append(state.instructionsCalled[it]);
            tallyIt++;
            mnemon = Pep::decodeMnemonic[it];
            mnemonList.append(mnemon);
        }
    }
    //qSort(tally);
    QString output="";
    for(int index = 0; index < tally.length(); index++)
    {
        if(tally[index] == 0) continue;
        output.append(QString("%1").arg(mnemonDecode(mnemonList[index]),5) % QString(": ") % QString::number(tally[index]) % QString("\n"));
    }
    return output;
}

void FullMicrocodedMemoizer::setDebugLevel(Enu::DebugLevels level)
{
    this->level = level;
}

// Properly formats a number as a 4 char hex
QString FullMicrocodedMemoizer::formatNum(quint16 number)
{
    return QString("%1").arg(QString::number(number,16),4,'0').toUpper();
}

// Properly format a number as 2 char hex
QString FullMicrocodedMemoizer::formatNum(quint8 number)
{
    return QString("%1").arg(QString::number(number,16),2,'0').toUpper();
}

// Properly format a 16 bit address
QString FullMicrocodedMemoizer::formatAddress(quint16 address)
{
    return "0x"+formatNum(address);
}

// Convert a mnemonic into it's string
QString FullMicrocodedMemoizer::mnemonDecode(quint8 instrSpec)
{
    static QMetaEnum metaenum = Enu::staticMetaObject.enumerator(Enu::staticMetaObject.indexOfEnumerator("EMnemonic"));
    return QString(metaenum.valueToKey((int)Pep::decodeMnemonic[instrSpec])).toLower();
}

// Convert a mnemonic into its string
QString FullMicrocodedMemoizer::mnemonDecode(Enu::EMnemonic instrSpec)
{
    static QMetaEnum metaenum = Enu::staticMetaObject.enumerator(Enu::staticMetaObject.indexOfEnumerator("EMnemonic"));
    return QString(metaenum.valueToKey((int)instrSpec)).toLower();
}

QString FullMicrocodedMemoizer::formatIS(quint8 instrSpec)
{
    return QString(mnemonDecode(instrSpec)).leftJustified(inst_size,' ');
}

QString FullMicrocodedMemoizer::formatUnary(quint8 instrSpec)
{
    return formatIS(instrSpec).leftJustified(inst_size+max_symLen+2+4);
}

QString FullMicrocodedMemoizer::formatNonUnary(quint8 instrSpec,quint16 oprSpec)
{
    return formatIS(instrSpec).leftJustified(inst_size) %
            QString(attempSymOprReplace(oprSpec)).rightJustified(max_symLen) %
            ", " % Pep::intToAddrMode(Pep::decodeAddrMode[instrSpec]).leftJustified(4,' ');
}

QString FullMicrocodedMemoizer::formatInstr(quint8 instrSpec,quint16 oprSpec)
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

QString FullMicrocodedMemoizer::generateStackFrame(CPUState&, bool enter)
{
    return "";
    if(enter) {
        //return stackFrameEnter.arg(" SP=%1").arg(formatAddress(registers.regState.reg_SP));
    }
    else {
        //return stackFrameLeave.arg(" SP=%1").arg(formatAddress(registers.regState.reg_SP));
    }
}

QString FullMicrocodedMemoizer::generateTrapFrame(CPUState&, bool enter)
{
    return "";
    if(enter) {
        //return trapEnter.arg(" SP=%1").arg(formatAddress(registers.regState.reg_SP));
    }
    else {
        //return trapLeave.arg(" SP=%1").arg(formatAddress(registers.regState.reg_SP));
    }
}

QString FullMicrocodedMemoizer::attempSymOprReplace(quint16 number)
{
    if(OSSymTable.count(number) == 1) return OSSymTable.find(number).value();
    else return formatNum(number);
}

QString FullMicrocodedMemoizer::attempSymAddrReplace(quint16 number)
{
    if(OSSymTable.count(number) == 1) return OSSymTable.find(number).value();
    else return formatAddress(number);
}

void FullMicrocodedMemoizer::loadSymbols()
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

/*QString generateLine()
//{
    //QString out="";
    //1 is address of instruction, 2 is inst specifier, 3 is optional oprsndspc + , + addr.
    //4 is data block (AXSP) for all NZVCS for BR, PCB & stack tracefor call
    //const QString format("0x%1: %2 %3 %4");
    //QString RHS ="";



    if(Pep::isUnaryMap[Pep::decodeMnemonic[cur.ir]])
    {
        if(Pep::isTrapMap[Pep::decodeMnemonic[cur.ir]])
        {
            out.append(trapFramepu.arg(fhnum(cur.pc),fhnum(cur.sp),QString::number(cur.callDepth)));
        }
        else if(Pep::decodeMnemonic[cur.ir] == Enu::EMnemonic::RET)
        {
            out.append(stackFramepo.arg(fhnum(cur.pc),fhnum(cur.sp),QString::number(cur.callDepth)));
        }
        else if(Pep::decodeMnemonic[cur.ir] == Enu::EMnemonic::RETTR)
        {
            out.append(trapFramepo.arg(fhnum(cur.pc),fhnum(cur.sp),QString::number(cur.callDepth)));
        }
        //out.append(format.arg(fmtadr(cur.pc),fmtis(cur.ir),fmtu(),RHS));
    //}
    else
    {
        if(cur.ir>=36 && cur.ir<=37)
        {
            out.append(stackFramepu.arg(fhnum(cur.pc),fhnum(cur.sp),QString::number(cur.callDepth)));
        }
        else if(cur.ir>=20 && cur.ir <=35)
        {
            RHS = "NZVCS="+QString::number(cur.nzvcs,2).leftJustified(5,'0');
        }
        out.append(format.arg(fmtadr(cur.pc),fmtis(cur.ir),fmtn(cur.OS,cur.ir),RHS));
    }
    //if(1); //If call, ret, trp, rettr generate generateSF
    //return out;
//}*/
