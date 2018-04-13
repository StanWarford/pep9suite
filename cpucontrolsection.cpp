#include "cpucontrolsection.h"
#include "cpudatasection.h"
#include "code.h"
#include "microcodeprogram.h"
CPUControlSection *CPUControlSection::_instance = nullptr;
CPUTester *CPUTester::_instance = nullptr;


CPUControlSection *CPUControlSection::getInstance()
{
    if(_instance==nullptr)
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
    this->program=program;
    microprogramCounter=0;
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
#pragma message "todo"
}

void CPUControlSection::onSimulationFinished()
{
    data->clearClockSignals();
    data->clearControlSignals();
}

void CPUControlSection::onDebuggingStarted()
{
#pragma message "todo"
}

void CPUControlSection::onDebuggingFinished()
{
#pragma message "todo"
}

void CPUControlSection::onStep(quint8 mode) noexcept
{
    //Do step logic
    const MicroCode* prog = program->getCodeLine(microprogramCounter);
    data->setSignalsFromMicrocode(prog);
    branchHandler();
    data->onStep();
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
    while(prog->getBranchFunction()!=Enu::Stop)
    {
        /*
         * Handle address decoding of next instruction
         */
        if(prog==nullptr)
        {
            return;
        }
        //
        onStep(-1);
        //If there was a logical error on data operation
        if(data->hadErrorOnStep())
        {
            //Pass up the error
            qDebug()<<"The data section died";
            qDebug()<<data->errorMessage;
            break;
        }
        //If there was an error on the control flow
        else if(this->hadErrorOnStep())
        {
            qDebug()<<"The control section died";
            break;
        }
        prog = program->getCodeLine(microprogramCounter);

    }
}

void CPUControlSection::onClearCPU()noexcept
{
    data->onClearCPU();
    executionFinished=false;
    inSimulation=false;
    microprogramCounter=0;
    hadControlError=false;
    errorMessage="";
}

void CPUControlSection::onClearMemory() noexcept
{
    data->onClearMemory();
}

void CPUControlSection::onCPUFeaturesChanged(Enu::CPUType cpuType) noexcept
{
    data->onCPUFeaturesChanged(cpuType);
}

CPUControlSection::CPUControlSection(CPUDataSection * data): QObject(nullptr),data(data),microprogramCounter(0),hadControlError(false),inSimulation(false)
{

}

void CPUControlSection::branchHandler()
{
    const MicroCode* prog = program->getCodeLine(microprogramCounter);
    switch(prog->getBranchFunction())
    {
    case Enu::Unconditional:
        microprogramCounter = prog->getTrueTarget();
        break;
    case Enu::Stop:
        executionFinished=true;
        break;
    default:
        //This should never occur
        break;

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
    if(_instance==nullptr)
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

