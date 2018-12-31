#include "acpumodel.h"
#include "amemorydevice.h"
ACPUModel::ACPUModel(QSharedPointer<AMemoryDevice> memoryDev, QObject* parent) noexcept: QObject(parent), memory(memoryDev), callDepth(0), inDebug(false), inSimulation(false),
    executionFinished(false), controlError(false), errorMessage("")
{

}

ACPUModel::~ACPUModel()
{

}

AMemoryDevice *ACPUModel::getMemoryDevice() noexcept
{
    return memory.get();
}

const AMemoryDevice *ACPUModel::getMemoryDevice() const noexcept
{
    return memory.get();
}

void ACPUModel::setMemoryDevice(QSharedPointer<AMemoryDevice> newDevice)
{
    memory = newDevice;
}

bool ACPUModel::getExecutionFinished() const noexcept
{
    return executionFinished;
}

int ACPUModel::getCallDepth() const noexcept
{
    return callDepth;
}

void ACPUModel::onClearMemory()
{
    memory->clearErrors();
    memory->clearMemory();
}
