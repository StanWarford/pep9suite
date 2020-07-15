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

#include "microassembler/microcodeprogram.h"
#include "microassembler/microcode.h"

InterfaceMCCPU::InterfaceMCCPU(PepCore::CPUType type) noexcept: microprogramCounter(0), microCycleCounter(0),
    microBreakpointHit(false), sharedProgram(nullptr), type(type)
{

}

InterfaceMCCPU::~InterfaceMCCPU()
{

}

void InterfaceMCCPU::breakpointsSet(QSet<quint16> addressess)
{
    breakpointsRemoveAll();
    for(auto line : addressess) {
        breakpointAdded(line);
    }
}

void InterfaceMCCPU::breakpointsRemoveAll()
{
    if(!sharedProgram.isNull()) {
        for(int it=0;it<sharedProgram->codeLength();it++) {
            sharedProgram->getCodeLine(it)->setBreakpoint(false);
        }
    }
}

void InterfaceMCCPU::breakpointRemoved(quint16 address)
{
    if(!sharedProgram.isNull()) {
        auto assumed_code_line = sharedProgram->getCodeLine(address);
        if(!assumed_code_line->isMicrocode()) {
            throw std::invalid_argument(std::to_string(address) + " does not correspond to a line of executable microcode.");
        }
        if(auto ptr = dynamic_cast<AExecutableMicrocode*>(assumed_code_line);
                ptr != nullptr) {
            ptr->setBreakpoint(false);
        }
    }
}

void InterfaceMCCPU::breakpointAdded(quint16 address)
{
    if(!sharedProgram.isNull()) {
        auto assumed_code_line = sharedProgram->getCodeLine(address);
        if(!assumed_code_line->isMicrocode()) {
            throw std::invalid_argument(std::to_string(address) + " does not correspond to a line of executable microcode.");
        }
        if(auto ptr = dynamic_cast<AExecutableMicrocode*>(assumed_code_line);
                ptr != nullptr) {
            ptr->setBreakpoint(true);
        }
    }
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

const AExecutableMicrocode* InterfaceMCCPU::getCurrentMicrocodeLine() const noexcept
{
    return sharedProgram->getCodeLine(microprogramCounter);
}

void InterfaceMCCPU::setMicrocodeProgram(QSharedPointer<MicrocodeProgram> program)
{
    this->sharedProgram = program;
    microprogramCounter = 0;
}

PepCore::CPUType InterfaceMCCPU::getCPUType() const noexcept
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
