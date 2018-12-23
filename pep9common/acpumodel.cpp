#include "acpumodel.h"
#include "amemorydevice.h"
ACPUModel::ACPUModel(QSharedPointer<AMemoryDevice> memoryDev, QObject* parent): QObject(parent), memory(memoryDev), callDepth(0), inDebug(false), inSimulation(false),
    executionFinished(false), controlError(false), errorMessage("")
{

}

ACPUModel::~ACPUModel()
{

}

AMemoryDevice *ACPUModel::getMemoryDevice()
{
    return memory.get();
}

const AMemoryDevice *ACPUModel::getMemoryDevice() const
{
    return memory.get();
}

void ACPUModel::setMemoryDevice(QSharedPointer<AMemoryDevice> newDevice)
{
    memory = newDevice;
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
