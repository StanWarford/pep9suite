#include "amccpumodel.h"
#include "microcodeprogram.h"
InterfaceMCCPU::InterfaceMCCPU(Enu::CPUType type): microprogramCounter(0), microCycleCounter(0),
    microBreakpointHit(false), program(nullptr), type(type)
{

}

InterfaceMCCPU::~InterfaceMCCPU()
{

}

unsigned int InterfaceMCCPU::getCycleCounter() const
{
    return microCycleCounter;
}

unsigned int InterfaceMCCPU::getMicrocodeLineNumber() const
{
    return microprogramCounter;
}

const MicrocodeProgram *InterfaceMCCPU::getProgram() const
{
    return program;
}

const MicroCode *InterfaceMCCPU::getCurrentMicrocodeLine() const
{
    return program->getCodeLine(microprogramCounter);
}

void InterfaceMCCPU::setMicrocodeProgram(MicrocodeProgram *program)
{
    this->program = program;
    microprogramCounter = 0;
}

Enu::CPUType InterfaceMCCPU::getCPUType() const
{
    return type;
}

void InterfaceMCCPU::reset()
{
    microprogramCounter = 0;
    microCycleCounter = 0;
    microBreakpointHit = false;
}

void InterfaceMCCPU::clear()
{
    reset();
    program = nullptr;
}
