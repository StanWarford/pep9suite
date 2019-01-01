#include "amccpumodel.h"
#include "microcodeprogram.h"
InterfaceMCCPU::InterfaceMCCPU(Enu::CPUType type) noexcept: microprogramCounter(0), microCycleCounter(0),
    microBreakpointHit(false), sharedProgram(nullptr), type(type)
{

}

InterfaceMCCPU::~InterfaceMCCPU()
{

}

quint64 InterfaceMCCPU::getCycleCounter() const noexcept
{
    return microCycleCounter;
}

quint16 InterfaceMCCPU::getMicrocodeLineNumber() const noexcept
{
    return microprogramCounter;
}

QSharedPointer<const MicrocodeProgram> InterfaceMCCPU::getProgram() const noexcept
{
    return sharedProgram;
}

const MicroCode* InterfaceMCCPU::getCurrentMicrocodeLine() const noexcept
{
    return sharedProgram->getCodeLine(microprogramCounter);
}

void InterfaceMCCPU::setMicrocodeProgram(QSharedPointer<MicrocodeProgram> program)
{
    this->sharedProgram = program;
    microprogramCounter = 0;
}

Enu::CPUType InterfaceMCCPU::getCPUType() const noexcept
{
    return type;
}

void InterfaceMCCPU::reset() noexcept
{
    microprogramCounter = 0;
    microCycleCounter = 0;
    microBreakpointHit = false;
}
