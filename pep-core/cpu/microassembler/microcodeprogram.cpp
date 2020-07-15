// File: microcodeprogram.cpp
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
#include "microcodeprogram.h"

#include "microassembler/microcode.h"
#include "symbol/symbolentry.h"
#include "symbol/symbolvalue.h"

MicrocodeProgram::MicrocodeProgram(): programVec(), preconditionsVec(), postconditionsVec(), microcodeVec()
{

}

MicrocodeProgram::~MicrocodeProgram()
{
    // Since ever item in the program vector was allocated with new by the microcode pane, make sure to explicitly delete it.
    for(int it = 0;it< programVec.length();it++) {
        delete programVec[it];
    }
}

MicrocodeProgram::MicrocodeProgram(QVector<AMicroCode*>objectCode, QSharedPointer<SymbolTable> symbolTable):
    symTable(symbolTable), programVec(objectCode),
    preconditionsVec(), postconditionsVec(), microcodeVec()
{
    AMicroCode* item;
    for(int it=0; it<objectCode.size();it++) {
        item = objectCode[it];
        // If the item at the iterator is code or a test condition, put the index in the appropriate vector
        if(item->hasUnitPre()) preconditionsVec.append(it);
        else if(item->hasUnitPost()) postconditionsVec.append(it);
        else if(item->isMicrocode()) microcodeVec.append(it);
    }
}

QSharedPointer<const SymbolTable> MicrocodeProgram::getSymTable() const
{
    return this->symTable;
}

const QVector<AMicroCode*> MicrocodeProgram::getObjectCode() const
{
    return this->programVec;
}

const QString MicrocodeProgram::format() const
{
    QString output = "";
    for(AMicroCode* line : programVec) {
        output.append(line->getSourceCode()  +"\n");
    }
    return output;
}

int MicrocodeProgram::codeLineToProgramLine(int codeLine) const
{
    return microcodeVec[codeLine];
}

bool MicrocodeProgram::hasMicrocode() const
{
    return !microcodeVec.isEmpty();
}

bool MicrocodeProgram::hasUnitPre() const
{
    return !preconditionsVec.empty();
}

const AExecutableMicrocode *MicrocodeProgram::getCodeLine(quint16 codeLine) const
{
    if(codeLine<microcodeVec.size()) {
        auto rval = static_cast<AExecutableMicrocode*>(programVec[microcodeVec[codeLine]]);
        return rval;
    }
    return nullptr;
}

AExecutableMicrocode *MicrocodeProgram::getCodeLine(quint16 codeLine)
{
    if(codeLine<microcodeVec.size()) {
        auto rval = static_cast<AExecutableMicrocode*>(programVec[microcodeVec[codeLine]]);
        return rval;
    }
    return nullptr;
}

int MicrocodeProgram::codeLength() const
{
    return microcodeVec.length();
}

