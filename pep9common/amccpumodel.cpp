#include "amccpumodel.h"
#include "microcodeprogram.h"
InterfaceMCCPU::InterfaceMCCPU(Enu::CPUType type): type(type)
{

}

InterfaceMCCPU::~InterfaceMCCPU()
{

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
