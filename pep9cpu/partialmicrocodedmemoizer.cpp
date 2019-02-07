#include "partialmicrocodedmemoizer.h"
#include "partialmicrocodedcpu.h"
#include "pep.h"
#include "amemorydevice.h"
#include "newcpudata.h"
#include "registerfile.h"
#include <assert.h>
#include <QString>
#include <QtCore>
#include <QDebug>
const QString stackFrameEnter("%1\n===CALL===\n");
const QString stackFrameLeave("%1\n===RET====\n");
const QString trapEnter("%1\n===TRAP===\n");
const QString trapLeave("%1\n===RETR===\n");
static quint8 max_symLen=0;
static quint8 inst_size=6;
static quint8 oper_addr_size=12;

PartialMicrocodedMemoizer::PartialMicrocodedMemoizer(PartialMicrocodedCPU& item): cpu(item),
    level(Enu::DebugLevels::MINIMAL)
{
}

Enu::DebugLevels PartialMicrocodedMemoizer::getDebugLevel() const
{
    return level;
}

void PartialMicrocodedMemoizer::clear()
{
}

void PartialMicrocodedMemoizer::storeStateInstrEnd()
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
        break;
    }
}

void PartialMicrocodedMemoizer::storeStateInstrStart()
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
        break;
    }
}

QString PartialMicrocodedMemoizer::memoize()
{
    QString build, AX, NZVC;
    const RegisterFile& file = cpu.data->getRegisterBank();
    switch(level){
    case Enu::DebugLevels::ALL:
        [[fallthrough]];
    case Enu::DebugLevels::MINIMAL:
        AX = QString(" A=%1, X=%2, SP=%3, ")

                .arg(formatNum(file.readRegisterWordCurrent(Enu::CPURegisters::A)),
                     formatNum(file.readRegisterWordCurrent(Enu::CPURegisters::X)),
                     formatNum(file.readRegisterWordCurrent(Enu::CPURegisters::SP)));
        NZVC = QString(" SNZVC=") % QString("%1").arg(QString::number(file.readStatusBitsCurrent(), 2), 5, '0');
        build = AX;
        build += NZVC;
    case Enu::DebugLevels::NONE:
        break;
    }
    return build;
}

QString PartialMicrocodedMemoizer::finalStatistics()
{
    if(level == Enu::DebugLevels::NONE) return "";
    QString output="";
    return output;
}

void PartialMicrocodedMemoizer::setDebugLevel(Enu::DebugLevels level)
{
    this->level = level;
}

//Properly formats a number as a 4 char hex
QString PartialMicrocodedMemoizer::formatNum(quint16 number)
{
    return QString("%1").arg(QString::number(number,16),4,'0').toUpper();
}

//Properly format a number as 2 char hex
QString PartialMicrocodedMemoizer::formatNum(quint8 number)
{
    return QString("%1").arg(QString::number(number,16),2,'0').toUpper();
}




//QString generateLine()
//{
    //QString out="";
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
    //return out;
//}
