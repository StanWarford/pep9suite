// File: acpumodel.cpp
/*
    The Pep/9 suite of applications (Pep9, Pep9CPU, Pep9Micro) are
    simulators for the Pep/9 virtual machine, and allow users to
    create, simulate, and debug across various levels of abstraction.

    Copyright (C) 2018  J. Stanley Warford & Matthew McRaven, Pepperdine University

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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

bool ACPUModel::getInSimulation() const noexcept
{
    return inSimulation;
}

bool ACPUModel::getInDebug() const noexcept
{
    return inDebug;
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
