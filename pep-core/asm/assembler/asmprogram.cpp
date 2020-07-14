// File: asmprogram.cpp
/*
    Pep9 is a virtual machine for writing machine language and assembly
    language programs.

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

#include "asmprogram.h"

#include <optional>

#include "assembler/asmcode.h"
#include "symbol/symboltable.h"
#include "symbol/symbolentry.h"

StaticTraceInfo::StaticTraceInfo(): staticTraceError(false), hadTraceTags(false), dynamicAllocSymbolTypes(), staticAllocSymbolTypes(),
    instrToSymlist(), hasHeapMalloc(), heapPtr(), mallocPtr()
{

}

AsmProgram::AsmProgram(): program(), indexToMemAddress(), memAddressToIndex(), symTable(QSharedPointer<SymbolTable>(new SymbolTable())),
    traceInfo(), burn(false), burnAddress(0), burnValue(0)
{

}

AsmProgram::AsmProgram(QList<QSharedPointer<AsmCode> > programList, QSharedPointer<SymbolTable> symbolTable,
                       QSharedPointer<const StaticTraceInfo> traceInfo): program(programList),
    indexToMemAddress(), memAddressToIndex(), symTable(symbolTable), traceInfo(traceInfo), burn(false), burnAddress(0), burnValue(0)
{
    programByteLength = 0;
    std::optional<int> start = std::nullopt;
    for(int it = 0; it < programList.length(); it++) {
        if(!start.has_value() && programList[it]->hasMemoryAddress() ) {
            start = static_cast<quint16>(programList[it]->getMemoryAddress());
        }
        indexToMemAddress.insert(it, static_cast<quint16>(programList[it]->getMemoryAddress()));
        memAddressToIndex.insert(static_cast<quint16>(programList[it]->getMemoryAddress()), it);
        programByteLength += programList[it]->objectCodeLength();
    }
    assert(start != -1);
    programBounds = {static_cast<quint16>(start.value()), static_cast<quint16>(start.value()-1+programByteLength)};
}

AsmProgram::AsmProgram(QList<QSharedPointer<AsmCode> > programList, QSharedPointer<SymbolTable> symbolTable,
                       QSharedPointer<const StaticTraceInfo> traceInfo, quint16 burnAddress, quint16 burnValue) : program(programList),
    indexToMemAddress(), memAddressToIndex(), symTable(symbolTable), traceInfo(traceInfo),
    burn(true), burnAddress(burnAddress), burnValue(burnValue)
{
    programByteLength = burnValue - burnAddress;

    // We are given program bounds by the burn address and burn val, so no need
    // to calculate like above constructor.
    programBounds = {static_cast<quint16>(burnAddress), static_cast<quint16>(burnValue)};
}

AsmProgram::~AsmProgram()
{

}

quint16 AsmProgram::getProgramLength() const
{
 return programByteLength;
}

const QVector<quint8> AsmProgram::getObjectCode() const
{
    QVector<quint8> vect;
    QList<int> objCode;
    for(QSharedPointer<AsmCode> line : program) {
        // If a line of code occurs before a burn, it can't emit object code
        if(hasBurn() && line->getMemoryAddress()<getBurnAddress()) continue;
        line->appendObjectCode(objCode);
        for(int val : objCode) {
            vect.append(static_cast<quint8>(val));
        }
        objCode.clear();
    }
    return vect;
}

const QList<QSharedPointer<AsmCode> > AsmProgram::getProgram() const
{
    return program;
}

QSharedPointer<SymbolTable> AsmProgram::getSymbolTable()
{
    return symTable;
}

const QSharedPointer<SymbolTable> AsmProgram::getSymbolTable() const
{
    return symTable;
}

bool AsmProgram::hasBurn() const
{
    return  burn;
}

quint16 AsmProgram::getBurnAddress() const
{
    return burnAddress;
}

quint16 AsmProgram::getBurnValue() const
{
    return burnValue;
}

AsmCode *AsmProgram::getCodeAtIndex(quint32 line)
{
    if(line >= static_cast<quint32>(program.length())) return nullptr;
    else return program[static_cast<int>(line)].data();
}

const AsmCode *AsmProgram::getCodeAtIndex(quint32 line) const
{
    if(line >= static_cast<quint32>(program.length())) return nullptr;
    else return program[static_cast<int>(line)].data();
}

const AsmCode *AsmProgram::memAddressToCode(quint16 memAddress) const
{
    if(memAddressToIndex.contains(memAddress)) return program[memAddressToIndex[memAddress]].data();
    else return nullptr;
}

int AsmProgram::numberOfLines() const
{
    return program.length();
}

QPair<quint16, quint16> AsmProgram::getProgramBounds() const
{
    return programBounds;
}

QSharedPointer<const StaticTraceInfo> AsmProgram::getTraceInfo() const
{
    return traceInfo;
}

QString AsmProgram::getFormattedSourceCode() const
{
    QStringList retVal;
    for (auto codeLine : program) {
        retVal.append(codeLine->getAssemblerSource());
    }
    // Make sure the last line ends in "\n", so concat a newline
    return retVal.join("\n")%"\n";
}

QString AsmProgram::getProgramListingCode() const
{
    QStringList retVal;
    for (auto codeLine : program) {
        retVal.append(codeLine->getAssemblerListing());
    }
        // Make sure the last line ends in "\n", so concat a newline
    return retVal.join("\n")%"\n";
}

QString AsmProgram::getProgramListing() const
{
    QString line = "-------------------------------------------------------------------------------\n";
    QString header = "      Object\nAddr  code   Symbol   Mnemon  Operand     Comment\n";
    return line % header % line % getProgramListingCode() % line % "\n";
}
