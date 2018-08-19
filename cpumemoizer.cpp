#include "cpumemoizer.h"
#include <cpucontrolsection.h>
#include "enu.h"
#include "pep.h"
#include "cpudatasection.h"
#include "memorysection.h"
#include <QDebug>
const QString stackFrameEnter("%1\n===CALL===\n");
const QString stackFrameLeave("%1\n===RET====\n");
const QString trapEnter("%1\n===TRAP===\n");
const QString trapLeave("%1\n===RETR===\n");
static quint8 max_symLen=0;
static quint8 inst_size=6;
static quint8 oper_addr_size=12;

CPUMemoizer::CPUMemoizer(CPUControlSection& item):item(item), registers(CPUState()), OSSymTable(), level(Enu::DebugLevels::MINIMAL)
{
    loadSymbols();
}

Enu::DebugLevels CPUMemoizer::getDebugLevel() const
{
    return level;
}

void CPUMemoizer::clear()
{
    registers = CPUState();
}

void CPUMemoizer::storeStateInstrEnd()
{
    switch(level)
    {
    case Enu::DebugLevels::ALL:
        //Intentional fallthrough
        [[fallthrough]];
    case Enu::DebugLevels::MINIMAL:
        registers.regState.reg_A = item.data->getRegisterBankWord(0);
        registers.regState.reg_X = item.data->getRegisterBankWord(2);
        registers.regState.reg_SP = item.data->getRegisterBankWord(4);
        registers.regState.reg_PC_end = item.data->getRegisterBankWord(6);
        registers.regState.reg_IR = item.memory->getMemoryByte(registers.regState.reg_PC_start,false);
        if(!Pep::isUnaryMap[Pep::decodeMnemonic[registers.regState.reg_IR]])
        {
            registers.regState.reg_OS = item.memory->getMemoryWord(registers.regState.reg_PC_start +1,false);
        }

        registers.regState.bits_NZVCS =
                item.data->getStatusBit(Enu::STATUS_N) * Enu::NMask
              | item.data->getStatusBit(Enu::STATUS_Z) * Enu::ZMask
              | item.data->getStatusBit(Enu::STATUS_V) * Enu::VMask
              | item.data->getStatusBit(Enu::STATUS_C) * Enu::CMask
              | item.data->getStatusBit(Enu::STATUS_S) * Enu::SMask;
        //Intentional fallthrough
        [[fallthrough]];
    case Enu::DebugLevels::NONE:
        break;
    }
}

void CPUMemoizer::storeStateInstrStart()
{
    switch(level)
    {
    case Enu::DebugLevels::ALL:
        //Intentional fallthrough
        [[fallthrough]];
    case Enu::DebugLevels::MINIMAL:
        registers.instructionsCalled[item.memory->getMemoryByte(registers.regState.reg_PC_start,false)]++;
        //Intentional fallthrough
        [[fallthrough]];
    case Enu::DebugLevels::NONE:
        registers.regState.reg_PC_start = item.data->getRegisterBankWord(6);
        break;
    }
}

QString CPUMemoizer::memoize()
{
    QString AX = QString(" A=%1, X=%2,").arg(formatNum(registers.regState.reg_A),formatNum(registers.regState.reg_X));
    QString NZVC = QString(" NZVC=") % QString("%1").arg(QString::number(registers.regState.bits_NZVCS & ~Enu::SMask,2), 4, '0') % ",";
    QString build = (attempSymAddrReplace(registers.regState.reg_PC_start) + QString(":")).leftJustified(10) %
            formatInstr(registers.regState.reg_IR,registers.regState.reg_OS);
    build += "  " + AX;
    build += NZVC;
    if(Pep::isTrapMap[Pep::decodeMnemonic[registers.regState.reg_IR]])
    {
        build += generateTrapFrame(registers);
    }
    else if(Pep::decodeMnemonic[registers.regState.reg_IR]==Enu::EMnemonic::RETTR)
    {
        build += generateTrapFrame(registers,false);
    }
    else if(Pep::decodeMnemonic[registers.regState.reg_IR]==Enu::EMnemonic::CALL)
    {
        build += generateStackFrame(registers);
    }
    else if(Pep::decodeMnemonic[registers.regState.reg_IR]==Enu::EMnemonic::RET)
    {
        build += generateStackFrame(registers,false);
    }

    return build;
}

QString CPUMemoizer::finalStatistics()
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
            tally[tallyIt]+= registers.instructionsCalled[it];
        }
        else
        {
            tally.append(registers.instructionsCalled[it]);
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

void CPUMemoizer::setDebugLevel(Enu::DebugLevels level)
{
    this->level = level;
}
#include "assert.h"
quint16 CPUMemoizer::getRegisterStart(CPURegisters reg) const
{
    if(reg != CPURegisters::PC) assert(false); // Attempted to access register that was not cached
    else return registers.regState.reg_PC_start;
    return -1;
}

//Properly formats a number as a 4 char hex
QString CPUMemoizer::formatNum(quint16 number)
{
    return QString("%1").arg(QString::number(number,16),4,'0').toUpper();
}

//Properly format a number as 2 char hex
QString CPUMemoizer::formatNum(quint8 number)
{
    return QString("%1").arg(QString::number(number,16),2,'0').toUpper();
}

//Properly format a 16 bit address
QString CPUMemoizer::formatAddress(quint16 address)
{
    return "0x"+formatNum(address);
}

//Convert a mnemonix into it's string
QString CPUMemoizer::mnemonDecode(quint8 instrSpec)
{
    static QMetaEnum metaenum = Enu::staticMetaObject.enumerator(Enu::staticMetaObject.indexOfEnumerator("EMnemonic"));
    return QString(metaenum.valueToKey((int)Pep::decodeMnemonic[instrSpec])).toLower();
}
//Convert a mnemonix into it's string
QString CPUMemoizer::mnemonDecode(Enu::EMnemonic instrSpec)
{
    static QMetaEnum metaenum = Enu::staticMetaObject.enumerator(Enu::staticMetaObject.indexOfEnumerator("EMnemonic"));
    return QString(metaenum.valueToKey((int)instrSpec)).toLower();
}

QString CPUMemoizer::formatIS(quint8 instrSpec)
{
    return QString(mnemonDecode(instrSpec)).leftJustified(inst_size,' ');
}

QString CPUMemoizer::formatUnary(quint8 instrSpec)
{
    return formatIS(instrSpec).leftJustified(inst_size+max_symLen+2+4);
}

QString CPUMemoizer::formatNonUnary(quint8 instrSpec,quint16 oprSpec)
{
    return formatIS(instrSpec).leftJustified(inst_size) %
            QString(attempSymOprReplace(oprSpec)).rightJustified(max_symLen) %
            ", " % Pep::intToAddrMode(Pep::decodeAddrMode[instrSpec]).leftJustified(4,' ');
}

QString CPUMemoizer::formatInstr(quint8 instrSpec,quint16 oprSpec)
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

QString CPUMemoizer::generateStackFrame(CPUState&, bool enter)
{
    if(enter)
    {
        return stackFrameEnter.arg(" SP=%1").arg(formatAddress(registers.regState.reg_SP));
    }
    else
    {
        return stackFrameLeave.arg(" SP=%1").arg(formatAddress(registers.regState.reg_SP));
    }
}

QString CPUMemoizer::generateTrapFrame(CPUState&, bool enter)
{
    if(enter)
    {
        return trapEnter.arg(" SP=%1").arg(formatAddress(registers.regState.reg_SP));
    }
    else
    {
        return trapLeave.arg(" SP=%1").arg(formatAddress(registers.regState.reg_SP));
    }
}

QString CPUMemoizer::attempSymOprReplace(quint16 number)
{
    if(OSSymTable.count(number)==1) return OSSymTable.find(number).value();
    else return formatNum(number);
}

QString CPUMemoizer::attempSymAddrReplace(quint16 number)
{
    if(OSSymTable.count(number)==1) return OSSymTable.find(number).value();
    else return formatAddress(number);
}

void CPUMemoizer::loadSymbols()
{
    QString  osFileString;
    //In the future, have a switch between loading the aligned and unaligned code
    if(true) osFileString = (":/help/osunalignedsymbols.txt");
    else osFileString = ("nope");
    QFile osFile(osFileString);
    quint8 msylen=0;
    if(osFile.open(QFile::ReadOnly))
    {
        bool temp=0;
        QTextStream file(&osFile);
        while(!file.atEnd())
        {
            QString text = file.readLine();
            QList<QString> parts =text.split(' ',QString::SplitBehavior::SkipEmptyParts );
            quint16 key = parts[1].toUShort(&temp,16);
            OSSymTable.insert(key,parts[0]);
            msylen = msylen<parts[0].length() ? parts[0].length() : msylen;
        }
    }
    oper_addr_size=msylen+3;
    max_symLen=msylen;
}

QString generateLine()
{
    QString out="";
    //1 is address of instruction, 2 is inst specifier, 3 is optional oprsndspc + , + addr.
    //4 is data block (AXSP) for all NZVCS for BR, PCB & stack tracefor call
    //const QString format("0x%1: %2 %3 %4");
    //QString RHS ="";



    /*if(Pep::isUnaryMap[Pep::decodeMnemonic[cur.ir]])
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
        }*/
        //out.append(format.arg(fmtadr(cur.pc),fmtis(cur.ir),fmtu(),RHS));
    //}
    /*else
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
    }*/
    //if(1); //If call, ret, trp, rettr generate generateSF
    return out;
}
