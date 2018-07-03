#include "cpucontrolsection.h"
#include "cpudatasection.h"
#include "code.h"
#include "microcodeprogram.h"
#include "symbolentry.h"
#include "pep.h"
#include <QDebug>
#include "symbolentry.h"
#include "symboltable.h"
#include "symboltable.h"
#include "memorysection.h"
#include <QElapsedTimer>
QElapsedTimer timer;
CPUControlSection *CPUControlSection::_instance = nullptr;
CPUTester *CPUTester::_instance = nullptr;
CPUControlSection::CPUControlSection(CPUDataSection * data, MemorySection* memory): QObject(nullptr), data(data), memory(memory),
    microprogramCounter(0), microCycleCounter(0), macroCycleCounter(0),
    inSimulation(false), hadControlError(false), isPrefetchValid(false)
{

}

CPUControlSection *CPUControlSection::getInstance()
{
    if(_instance == nullptr)
    {
        _instance = new CPUControlSection(CPUDataSection::getInstance(),MemorySection::getInstance());
    }
    return _instance;
}

CPUControlSection::~CPUControlSection()
{
    //This object should last the lifetime of the  program, it does not need to be cleaned up
}

void CPUControlSection::setMicrocodeProgram(MicrocodeProgram *program)
{
    this->program = program;
    microprogramCounter = 0;
}

int CPUControlSection::getLineNumber() const
{
    return microprogramCounter;
}

const MicrocodeProgram *CPUControlSection::getProgram() const
{
    return program;
}

const MicroCode *CPUControlSection::getCurrentMicrocodeLine() const
{
    return program->getCodeLine(microprogramCounter);
}

QString CPUControlSection::getErrorMessage() const
{
    if(memory->hadError()) return memory->getErrorMessage();
    else if(data->hadErrorOnStep()) return data->getErrorMessage();
    else if(hadErrorOnStep()) return errorMessage;
    else return "";
}

bool CPUControlSection::hadErrorOnStep() const
{
    return hadControlError || data->hadErrorOnStep() || memory->hadError();
}

bool CPUControlSection::getExecutionFinished() const
{
    return executionFinished;
}

void CPUControlSection::onSimulationStarted()
{
    inSimulation = true;
    executionFinished = false;
}

void CPUControlSection::onSimulationFinished()
{
    data->clearClockSignals();
    data->clearControlSignals();
    executionFinished = true;
}

void CPUControlSection::onDebuggingStarted()
{
    onSimulationStarted();
}

void CPUControlSection::onDebuggingFinished()
{
    onSimulationFinished();
}

QString fhnum(quint16 num){
    return QString("%1").arg(QString::number(num,16),4,'0');
}
QString fmtadr(quint16 addr){
    return fhnum(addr);}
#include <enu.h>
QString emn2str(quint8 ir)
{
    QMetaEnum metaenum = Enu::staticMetaObject.enumerator(Enu::staticMetaObject.indexOfEnumerator("EMnemonic"));
    return QString(metaenum.valueToKey((int)Pep::decodeMnemonic[ir]));
}
QString fmtis(quint8 ir){
    return QString(emn2str(ir)).toLower().leftJustified(5,' ');
}
QString fmtu(){
    return QString().leftJustified(9,' ');
}
QString fmtn (quint16 os, quint8 ir){
    return fhnum(os)+QString(", "+Pep::intToAddrMode(Pep::decodeAddrMode[ir])).leftJustified(5,' ');
}
QString CPUControlSection::generateLine()
{
    QString out="";
    //1 is address of instruction, 2 is inst specifier, 3 is optional oprsndspc + , + addr.
    //4 is data block (AXSP) for all NZVCS for BR, PCB & stack tracefor call
    const QString format("0x%1: %2 %3 %4");
    const QString stackFrame("\n===CALL===\nPC=0x%1, SP=0x%2, depth=%3\n===CALL===\n");
    const QString trapFrame("\n===TRAP===\nPC=0x%1, SP=0x%2, depth=%3\n===TRAP===\n");
    QString RHS ="";



    if(Pep::isUnaryMap[Pep::decodeMnemonic[cur.ir]])
    {
        if(Pep::isTrapMap[Pep::decodeMnemonic[cur.ir]])
        {
            out.append(trapFrame.arg(fhnum(cur.pc),fhnum(cur.sp),QString::number(cur.callDepth)));
        }
        else if(Pep::decodeMnemonic[cur.ir] == Enu::EMnemonic::RET)
        {
            out.append(stackFrame.arg(fhnum(cur.pc),fhnum(cur.sp),QString::number(cur.callDepth)));
        }
        else if(Pep::decodeMnemonic[cur.ir] == Enu::EMnemonic::RETTR)
        {
            out.append(trapFrame.arg(fhnum(cur.pc),fhnum(cur.sp),QString::number(cur.callDepth)));
        }
        out.append(format.arg(fmtadr(cur.pc),fmtis(cur.ir),fmtu(),RHS));
    }
    else
    {
        if(cur.ir>=36 && cur.ir<=37)
        {
            out.append(stackFrame.arg(fhnum(cur.pc),fhnum(cur.sp),QString::number(cur.callDepth)));
        }
        else if(cur.ir>=20 && cur.ir <=35)
        {
            RHS = "NZVCS="+QString::number(cur.nzvcs,2).leftJustified(5,'0');
        }
        out.append(format.arg(fmtadr(cur.pc),fmtis(cur.ir),fmtn(cur.OS,cur.ir),RHS));
    }
    if(1); //If call, ret, trp, rettr generate generateSF
    return out;
}
void CPUControlSection::onStep() noexcept
{
    //Do step logic
    if(microprogramCounter == 0)
    {
        captureState();
    }
    const MicroCode* prog = program->getCodeLine(microprogramCounter);
    this->setSignalsFromMicrocode(prog);
    data->setSignalsFromMicrocode(prog);
    data->onStep();
    branchHandler();
    microCycleCounter++;
    if(microprogramCounter==0)
    {
        macroCycleCounter++;
        cur.ir = data->getRegisterBankByte(8);
        cur.OS =  data->getRegisterBankWord(9);
        if(Pep::decodeMnemonic[cur.ir] == Enu::EMnemonic::CALL || Pep::isTrapMap[Pep::decodeMnemonic[cur.ir]])
        {
            cur.callDepth = prev.callDepth+1;
        }
        else if(Pep::decodeMnemonic[cur.ir] == Enu::EMnemonic::RET || Pep::decodeMnemonic[cur.ir] == Enu::EMnemonic::RETTR)
        {
            cur.callDepth = prev.callDepth-1;
        }
        else cur.callDepth = prev.callDepth;

        //qDebug()<<"Insturction #"<<macroCycleCounter<<" ; Cyle # "<<microCycleCounter;
        qDebug().noquote() << generateLine();
    }

}

void CPUControlSection::onClock()noexcept
{
    //Do clock logic
    if(!inSimulation)
    {
        data->onClock();
    }
    else
    {
        //One should not get here, otherwise that would mean that we clocked in a simulation
    }
}

void CPUControlSection::onRun()noexcept
{
    timer.start();
#pragma message ("This needs to be ammended for errors in execution")
    while(!executionFinished)
    {
        onStep();
        //If there was an error on the control flow
        if(this->hadErrorOnStep())
        {
            qDebug() << "The control section died";
            break;
        }
    }
    qDebug().noquote().nospace() << QString("%1").arg(Pep::enumToMnemonMap[Pep::decodeMnemonic[data->getRegisterBankByte(8)]].toLower(),6,' ');
    qDebug() <<"Execution time (ms): "<<timer.elapsed();
    qDebug() <<"Hz rating: "<< microCycleCounter / (((float)timer.elapsed())/1000);
}

void CPUControlSection::onClearCPU()noexcept
{
    data->onClearCPU();
    inSimulation = false;
    microprogramCounter = 0;
    microCycleCounter = 0;
    macroCycleCounter = 0;
    hadControlError = false;
    executionFinished = false;
    isPrefetchValid = false;
    errorMessage = "";
}

void CPUControlSection::onClearMemory() noexcept
{
    memory->onClearMemory();
}

void CPUControlSection::branchHandler()
{
    const MicroCode* prog = program->getCodeLine(microprogramCounter);
    int temp = microprogramCounter;
    quint8 byte = 0;
    QString tempString;
    const SymbolTable* symTable = this->program->getSymTable();;
    std::shared_ptr<SymbolEntry> val;
    switch(prog->getBranchFunction())
    {
    case Enu::Unconditional:
        temp = prog->getTrueTarget()->getValue();
        break;
    case Enu::uBRGT:
        if((!data->getStatusBit(Enu::STATUS_N)))
        {
            temp = prog->getTrueTarget()->getValue();
        }
        else
        {
            temp = prog->getFalseTarget()->getValue();
        }
        break;
    case Enu::uBRGE:
        if((!data->getStatusBit(Enu::STATUS_N)) || data->getStatusBit(Enu::STATUS_Z))
        {
            temp = prog->getTrueTarget()->getValue();
        }
        else
        {
            temp = prog->getFalseTarget()->getValue();
        }
        break;
    case Enu::uBREQ:
        if(data->getStatusBit(Enu::STATUS_Z))
        {
            temp = prog->getTrueTarget()->getValue();
        }
        else
        {
            temp = prog->getFalseTarget()->getValue();
        }
        break;
    case Enu::uBRLE:
        if(data->getStatusBit(Enu::STATUS_N) || data->getStatusBit(Enu::STATUS_Z))
        {
            temp = prog->getTrueTarget()->getValue();
        }
        else
        {
            temp = prog->getFalseTarget()->getValue();
        }
        break;
    case Enu::uBRLT:
        if(data->getStatusBit(Enu::STATUS_N))
        {
            temp = prog->getTrueTarget()->getValue();
        }
        else
        {
            temp = prog->getFalseTarget()->getValue();
        }
        break;
    case Enu::uBRNE:
        if((!data->getStatusBit(Enu::STATUS_Z)))
        {
            temp = prog->getTrueTarget()->getValue();
        }
        else
        {
            temp = prog->getFalseTarget()->getValue();
        }
        break;
    case Enu::uBRV:
        if(data->getStatusBit(Enu::STATUS_V))
        {
            temp = prog->getTrueTarget()->getValue();
        }
        else
        {
            temp = prog->getFalseTarget()->getValue();
        }
        break;
    case Enu::uBRC:
        if(data->getStatusBit(Enu::STATUS_C))
        {
            temp = prog->getTrueTarget()->getValue();
        }
        else
        {
            temp = prog->getFalseTarget()->getValue();
        }
        break;
    case Enu::uBRS:
        if(data->getStatusBit(Enu::STATUS_S))
        {
            temp = prog->getTrueTarget()->getValue();
        }
        else
        {
            temp = prog->getFalseTarget()->getValue();
        }
        break;
    case Enu::IsPrefetchValid:
        if(isPrefetchValid)
        {
            temp = prog->getTrueTarget()->getValue();
        }
        else
        {
            temp = prog->getFalseTarget()->getValue();
        }
        break;
    case Enu::IsUnary:
        byte = data->getRegisterBankByte(8);
        if(Pep::isUnaryMap[Pep::decodeMnemonic[byte]])
        {
            temp = prog->getTrueTarget()->getValue();
        }
        else
        {
            temp = prog->getFalseTarget()->getValue();
        }
        break;
    case Enu::IsPCEven:
        if(data->getRegisterBankByte(7)%2 == 0)
        {
            temp = prog->getTrueTarget()->getValue();
        }
        else
        {
            temp = prog->getFalseTarget()->getValue();
        }
        break;
    case Enu::AddressingModeDecoder:
        temp = data->getRegisterBankByte(8);
        tempString = Pep::intToAddrMode(Pep::decodeAddrMode[temp]).toLower()+"Addr";
        if(symTable->exists(tempString))
        {
            val = symTable->getValue(tempString);
            if(val->isDefined())
            {
                temp = val->getValue();
            }
            else
            {
                executionFinished = true;
                hadControlError = true;
                errorMessage = "ERROR: AMD jumped to multiply defined instr - " + tempString;
            }

        }
        else
        {
            executionFinished = true;
            hadControlError = true;
            errorMessage = "ERROR: AMD looked for undefined inst - " + tempString;
        }
        break;
    case Enu::InstructionSpecifierDecoder:
        temp = data->getRegisterBankByte(8);
        tempString = Pep::enumToMnemonMap[Pep::decodeMnemonic[temp]].toLower();
        if(symTable->exists(tempString))
        {
            val = symTable->getValue(tempString);
            if(val->isDefined())
            {
                temp = val->getValue();
            }
            else
            {
                executionFinished = true;
                hadControlError = true;
                errorMessage = "ERROR: ISD jumped to multiply defined instr - " + tempString;
            }

        }
        else
        {
            executionFinished = true;
            hadControlError = true;
            errorMessage = "ERROR: ISD looked for undefined inst - " + tempString;
        }
        break;
    case Enu::Stop:
        executionFinished = true;
        break;
    default:
        //This should never occur
        break;

    }
    if(hadControlError) //If there was an error in the control section, make sure the CPU stops
    {
        executionFinished = true;
    }
    else if(temp == microprogramCounter&&prog->getBranchFunction()!=Enu::Stop)
    {
        hadControlError = true;
        errorMessage = "Don't branch to yourself";
        executionFinished  = true;
    }
    else
    {
        microprogramCounter = temp;
    }
}

void CPUControlSection::setSignalsFromMicrocode(const MicroCode *line)
{
    int val;
    if(line->getClockSignal(Enu::EClockSignals::PValidCk))
    {
        val = line->getControlSignal(Enu::EControlSignals::PValid);
        if(val == Enu::signalDisabled)
        {
            errorMessage = "Error: Asserted PValidCk, but PValid was disabled.";
            hadControlError = true;
        }
        else
        {
            isPrefetchValid = val;
        }
    }
}

void CPUControlSection::captureState()
{
    this->prev=cur;
    cur.a = data->getRegisterBankWord(0);
    cur.x = data->getRegisterBankWord(2);
    cur.sp = data->getRegisterBankWord(4);
    cur.pc = data->getRegisterBankWord(6);
    cur.nzvcs =
            data->getStatusBit(Enu::STATUS_N) * Enu::NMask
          | data->getStatusBit(Enu::STATUS_Z) * Enu::ZMask
          | data->getStatusBit(Enu::STATUS_V) * Enu::VMask
          | data->getStatusBit(Enu::STATUS_C) * Enu::CMask
          | data->getStatusBit(Enu::STATUS_S) * Enu::SMask;
}

void CPUControlSection::initCPUStateFromPreconditions()
{
    onClearCPU();
    QList<UnitPreCode*> preCode;
    microprogramCounter=0;
    if(program == nullptr)
    {
        qDebug()<<"Can't init from null program";
    }
    for(Code* x : program->getObjectCode())
    {
        if(x->hasUnitPre())preCode.append((UnitPreCode*)x);
    }
    //Handle data section logic
    for(auto x : preCode)
    {
        x->setUnitPre(data);
    }

}

bool CPUControlSection::testPost()
{
    QList<UnitPreCode*> preCode;
    if(program == nullptr)
    {
        qDebug()<<"Can't init from null program";
    }
    for(Code* x : program->getObjectCode())
    {
        if(x->hasUnitPost())preCode.append((UnitPreCode*)x);
    }
    QString err;
    bool t=false;
    for(auto x : preCode)
    {
       ((UnitPostCode*) x)->testPostcondition(data,err);
        if(err!="")t=true;
    }
    qDebug()<<"The postcondtions were:"<<!t;
    return !t;
}

CPUTester *CPUTester::getInstance()
{
    if(_instance == nullptr)
    {
        _instance = new CPUTester(CPUControlSection::getInstance(),CPUDataSection::getInstance());
    }
    return _instance;
}

CPUTester::CPUTester(CPUControlSection *control, CPUDataSection *data): QObject(nullptr),control(control),data(data)
{

}

CPUTester::~CPUTester()
{

}

