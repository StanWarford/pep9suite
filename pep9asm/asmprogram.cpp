#include "asmprogram.h"
#include "asmcode.h"
#include "symboltable.h"
AsmProgram::AsmProgram(): program(), indexToMemAddress(), memAddressToIndex(), symTable(QSharedPointer<SymbolTable>(new SymbolTable())),
   burn(false), burnAddress(0), burnValue(0)
{

}

AsmProgram::AsmProgram(QList<QSharedPointer<AsmCode> > programList, QSharedPointer<SymbolTable> symbolTable): program(programList),
    indexToMemAddress(), memAddressToIndex(), symTable(symbolTable), burn(false), burnAddress(0), burnValue(0)
{
    programByteLength = 0;
    int start = -1;
    for(int it = 0; it < programList.length(); it++)
    {
        if(start == -1 && programList[it]->getMemoryAddress() >= 0) start = static_cast<quint16>(programList[it]->getMemoryAddress());
        indexToMemAddress.insert(it, static_cast<quint16>(programList[it]->getMemoryAddress()));
        memAddressToIndex.insert(static_cast<quint16>(programList[it]->getMemoryAddress()), it);
        programByteLength += programList[it]->objectCodeLength();
    }
    programBounds = {static_cast<quint16>(start), static_cast<quint16>(start-1+programByteLength)};
}

AsmProgram::AsmProgram(QList<QSharedPointer<AsmCode> > programList, QSharedPointer<SymbolTable> symbolTable, quint16 burnAddress, quint16 burnValue) : program(programList),
    indexToMemAddress(), memAddressToIndex(), symTable(symbolTable), burn(true), burnAddress(burnAddress), burnValue(burnValue)
{
    programByteLength = 0;
    int start = -1;
    for(int it = 0; it < programList.length(); it++)
    {
        if(start == -1 && programList[it]->getMemoryAddress() >= 0) start = static_cast<quint16>(programList[it]->getMemoryAddress());
        indexToMemAddress.insert(it, static_cast<quint16>(programList[it]->getMemoryAddress()));
        memAddressToIndex.insert(static_cast<quint16>(programList[it]->getMemoryAddress()), it);
        programByteLength += programList[it]->objectCodeLength();
    }
    programBounds = {static_cast<quint16>(start), static_cast<quint16>(start-1+programByteLength)};
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
