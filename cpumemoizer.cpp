#include "cpumemoizer.h"
#include <cpucontrolsection.h>
#include "enu.h"
#include "pep.h"
#include "cpudatasection.h"
#include "memorysection.h"
#include <QDebug>
const QString stackFrameEnter("\n===CALL===\n%1\n===CALL===\n");
const QString stackFrameLeave("\n===RET====\n%1\n===RET====\n");
const QString trapEnter("\n===TRAP===\n%1\n===TRAP===\n");
const QString trapLeave("\n===RETR===\n%1\n===RETR===\n");
static quint8 max_symLen=0;
static quint8 inst_size=6;
static quint8 oper_addr_size=12;

CPUMemoizer::CPUMemoizer(CPUControlSection& item):item(item),registers(CPUState({0,})),OSSymTable()
{
    loadSymbols();
}

void CPUMemoizer::clear()
{
    registers = CPUState({0,});
}

void CPUMemoizer::storeState()
{
    registers.regState.reg_A = item.data->getRegisterBankWord(0);
    registers.regState.reg_X = item.data->getRegisterBankWord(2);
    registers.regState.reg_SP = item.data->getRegisterBankWord(4);
    registers.regState.reg_PC_end = item.data->getRegisterBankWord(6);
    registers.regState.reg_IR = item.data->getRegisterBankByte(8);
    registers.regState.reg_OS = item.data->getRegisterBankWord(9);
    registers.regState.bits_NZVCS =
            item.data->getStatusBit(Enu::STATUS_N) * Enu::NMask
          | item.data->getStatusBit(Enu::STATUS_Z) * Enu::ZMask
          | item.data->getStatusBit(Enu::STATUS_V) * Enu::VMask
          | item.data->getStatusBit(Enu::STATUS_C) * Enu::CMask
            | item.data->getStatusBit(Enu::STATUS_S) * Enu::SMask;
}

void CPUMemoizer::storePC()
{
    registers.regState.reg_PC_start = item.data->getRegisterBankWord(6);
}

QString CPUMemoizer::memoize()
{
    QString build = (attempSymAddrReplace(registers.regState.reg_PC_start) + QString(":")).leftJustified(10) %
            formatInstr(registers.regState.reg_IR,registers.regState.reg_OS);
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
    else if(registers.regState.reg_IR <= 35 && registers.regState.reg_IR >= 14)
    {
        build += QString(" SNZVC=") % QString::number(registers.regState.bits_NZVCS,2).leftJustified(5,'0');
    }
    return build;
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

QString CPUMemoizer::formatIS(quint8 instrSpec)
{
    return QString(mnemonDecode(instrSpec)).leftJustified(inst_size,' ');
}

QString CPUMemoizer::formatUnary(quint8 instrSpec)
{
    return formatIS(instrSpec).leftJustified(oper_addr_size);
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

QString CPUMemoizer::generateStackFrame(CPUState& state, bool enter)
{
    if(enter)
    {
        return stackFrameEnter.arg("Enter");
    }
    else
    {
        return stackFrameLeave.arg("Leave");
    }
}

QString CPUMemoizer::generateTrapFrame(CPUState& state, bool enter)
{
    if(enter)
    {
        return trapEnter.arg("Enter");
    }
    else
    {
        return trapLeave.arg("Leave");
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
    qDebug()<<max_symLen;
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
