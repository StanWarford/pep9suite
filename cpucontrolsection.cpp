#include "cpucontrolsection.h"
#include "cpudatasection.h"
#include "code.h"
#include "microcodeprogram.h"
#include "SymbolEntry.h"
#include "pep.h"
#include <QDebug>
CPUControlSection *CPUControlSection::_instance = nullptr;
CPUTester *CPUTester::_instance = nullptr;


CPUControlSection *CPUControlSection::getInstance()
{
    if(_instance == nullptr)
    {
        _instance = new CPUControlSection(CPUDataSection::getInstance());
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
    if(data->hadErrorOnStep()) return data->getErrorMessage();
    else if(hadErrorOnStep()) return errorMessage;
    else return "";
}

bool CPUControlSection::hadErrorOnStep() const
{
    return hadControlError || data->hadErrorOnStep();
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


void CPUControlSection::onStep() noexcept
{
    //Do step logic
    const MicroCode* prog = program->getCodeLine(microprogramCounter);
    this->setSignalsFromMicrocode(prog);
    data->setSignalsFromMicrocode(prog);
    data->onStep();
    branchHandler();
    cycleCounter++;
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
    const MicroCode* prog = program->getCodeLine(microprogramCounter);
#pragma message ("This needs to be ammended for errors in execution");
    while(prog->getBranchFunction() != Enu::Stop)
    {
        /*
         * Handle address decoding of next instruction
         */
        if(prog == nullptr)
        {
            return;
        }
        //
        onStep();
        //If there was a logical error on data operation
        if(data->hadErrorOnStep())
        {
            //Pass up the error
            qDebug() << "The data section died";
            qDebug() << data->errorMessage;
            break;
        }
        //If there was an error on the control flow
        else if(this->hadErrorOnStep())
        {
            qDebug() << "The control section died";
            break;
        }
        prog = program->getCodeLine(microprogramCounter);

    }
}

void CPUControlSection::onClearCPU()noexcept
{
    data->onClearCPU();
    inSimulation = false;
    microprogramCounter = 0;
    cycleCounter = 0;
    hadControlError = false;
    executionFinished = false;
    isPrefetchValid = false;
    errorMessage = "";
}

void CPUControlSection::onClearMemory() noexcept
{
    data->onClearMemory();
}

void CPUControlSection::onCPUFeaturesChanged(Enu::CPUType cpuType) noexcept
{
    data->onCPUFeaturesChanged(cpuType);
}

CPUControlSection::CPUControlSection(CPUDataSection * data): QObject(nullptr), data(data),microprogramCounter(0), cycleCounter(0),
    inSimulation(false), hadControlError(false), isPrefetchValid(false)
{

}

void CPUControlSection::branchHandler()
{
    const MicroCode* prog = program->getCodeLine(microprogramCounter);
    int temp = microprogramCounter;
    char byte = 0;
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
        if(byte < 18|| (byte == 38|| byte == 39))
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
        qDebug() << Pep::decodeAddrMode[temp];
        executionFinished = true; //For now, the instruction jump table is unimplmented
        hadControlError = true;
        errorMessage = "Error: Addressing Mode Decoder not implemented.";
        break;
    case Enu::InstructionSpecifierDecoder:
        executionFinished = true; //For now, the instruction jump table is unimplmented
        hadControlError = true;
        errorMessage = "Error: Instruction Specifier Decoder not implemented.";
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
    for(auto x : preCode)
    {
        x->setUnitPre(data);
    }
    //Handle any control section logic
    //None at the moment
    //Handle data section logic


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

