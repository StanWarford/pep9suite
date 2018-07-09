#include "cpucontrolsection.h"
#include <QApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <qmutex.h>
#include <QMutexLocker>

#include "cpudatasection.h"
#include "code.h"
#include "microcodeprogram.h"
#include "symbolentry.h"
#include "pep.h"
#include "symbolentry.h"
#include "symboltable.h"
#include "symboltable.h"
#include "memorysection.h"
#include "cpumemoizer.h"

QMutex dontEnter;
QElapsedTimer timer;
CPUControlSection *CPUControlSection::_instance = nullptr;
CPUTester *CPUTester::_instance = nullptr;
CPUControlSection::CPUControlSection(CPUDataSection * data, MemorySection* memory): QObject(nullptr),memoizer(new CPUMemoizer(*this)), data(data), memory(memory),
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
    if(memory->hadErroronStep()) return memory->getErrorMessage();
    else if(data->hadErrorOnStep()) return data->getErrorMessage();
    else if(hadErrorOnStep()) return errorMessage;
    else return "";
}

bool CPUControlSection::hadErrorOnStep() const
{
    return hadControlError || data->hadErrorOnStep() || memory->hadErroronStep();
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
    memory->onCancelWaiting();
    executionFinished = true;
}

void CPUControlSection::onDebuggingStarted()
{
    onSimulationStarted();
    memoizer->clear();
}

void CPUControlSection::onDebuggingFinished()
{
    onSimulationFinished();
}

void CPUControlSection::onStep() noexcept
{
    QMutexLocker mutty(&dontEnter);
    //Do step logic
    if(microprogramCounter==0)
    {
        //Store PC at the start of the cycle, so that we know where the instruction started from
        memoizer->storePC();
        //memoizer->storeState();
    }
    const MicroCode* prog = program->getCodeLine(microprogramCounter);
    this->setSignalsFromMicrocode(prog);
    data->setSignalsFromMicrocode(prog);
    data->onStep();
    branchHandler();
    microCycleCounter++;
    if(microprogramCounter==0 ||hadErrorOnStep() ||executionFinished)
    {
        memoizer->storeState();
        updateAtInstructionEnd();
        emit simulationInstructionFinished();
        macroCycleCounter++;
        qDebug().noquote() << memoizer->memoize();
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
    while(!executionFinished)
    {
        if(microCycleCounter%500==0)
        {
            QApplication::processEvents();
        }
        onStep();
        //If there was an error on the control flow
        if(hadErrorOnStep())
        {
            if(memory->hadErroronStep())
            {
                qDebug() << "Memory section reporting an error";
                break;
            }
            else if(data->hadErrorOnStep())
            {
                qDebug() << "Data section reporting an error";
                break;
            }
            else
            {
                qDebug() << "The control section died";
                break;
            }
        }
    }
    auto value = timer.elapsed();
    qDebug().nospace().noquote() << memoizer->finalStatistics() << "\n";
    qDebug().nospace().noquote() <<"Executed "<<macroCycleCounter<<" instructions in "<<microCycleCounter<< " cycles.";
    qDebug().nospace().noquote() <<"Averaging "<<microCycleCounter/macroCycleCounter<<" cycles per instruction.";
    qDebug().nospace().noquote() <<"Execution time (ms): " << value;
    qDebug().nospace().noquote() <<"Cycles per second: "<< microCycleCounter / (((float)value/1000));
    qDebug().nospace().noquote() <<"Instructions per second: "<< macroCycleCounter / (((float)value/1000));
    emit simulationFinished();
}

void CPUControlSection::onClearCPU()noexcept
{
    data->onClearCPU();
    memory->clearMemory();
    memory->clearErrors();
    memoizer->clear();
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
        if((!data->getStatusBit(Enu::STATUS_N) && !data->getStatusBit(Enu::STATUS_Z)))
        {
            temp = prog->getTrueTarget()->getValue();
        }
        else
        {
            temp = prog->getFalseTarget()->getValue();
        }
        break;
    case Enu::uBRGE:
        if((!data->getStatusBit(Enu::STATUS_N)))
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

void CPUControlSection::updateAtInstructionEnd()
{
#pragma message("Todo: Update CPU state at start of instruction")
    memory->onInstructionFinished();
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

