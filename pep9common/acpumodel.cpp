#include "acpumodel.h"
#include "amemorydevice.h"
ACPUModel::ACPUModel(AMemoryDevice* memoryDev, QObject* parent): QObject(parent), memory(memoryDev)
{

}

ACPUModel::~ACPUModel()
{

}

AMemoryDevice *ACPUModel::getMemoryDevice()
{
    return memory;
}

const AMemoryDevice *ACPUModel::getMemoryDevice() const
{
    return memory;
}

void ACPUModel::setMemoryDevice(AMemoryDevice *newDevice)
{
    memory = newDevice;
}

void ACPUModel::setDebugLevel(Enu::DebugLevels level)
{
#pragma message("TODO: add debugging levels to base class")
}

bool ACPUModel::getExecutionFinished() const
{
    return executionFinished;
}

void ACPUModel::onClearMemory()
{
    memory->clearErrors();
    memory->clearMemory();
}
