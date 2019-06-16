// File: interfacemccpu.h
/*
    Pep9CPU is a CPU simulator for executing microcode sequences to
    implement instructions in the instruction set of the Pep/9 computer.

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
#include "interfacemccpu.h"
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

void InterfaceMCCPU::doMCStepWhile(std::function<bool ()> condition)
{
    do{
        onMCStep();
    } while(condition());
}
