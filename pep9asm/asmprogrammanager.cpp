#include "asmprogrammanager.h"
#include "asmprogram.h"
#include <QSharedPointer>
#include "asmcode.h"
#include "symbolentry.h"
AsmProgramManager* AsmProgramManager::instance = nullptr;
AsmProgramManager::AsmProgramManager(QObject *parent): QObject(parent), operatingSystem(nullptr), userProgram(nullptr)
{
    userProgram.clear();
    operatingSystem.clear();
}

AsmProgramManager *AsmProgramManager::getInstance()
{
    if(instance == nullptr) instance = new AsmProgramManager();
    return instance;
}

QSharedPointer<AsmProgram> AsmProgramManager::getOperatingSystem()
{
    return operatingSystem;
}

QSharedPointer<AsmProgram> AsmProgramManager::getUserProgram()
{
    return userProgram;
}

QSharedPointer<const AsmProgram> AsmProgramManager::getOperatingSystem() const
{
    return operatingSystem;
}

QSharedPointer<const AsmProgram> AsmProgramManager::getUserProgram() const
{
    return userProgram;
}

void AsmProgramManager::setOperatingSystem(QSharedPointer<AsmProgram> prog)
{
    operatingSystem = prog;
}

quint16 AsmProgramManager::getMemoryVectorValue(MemoryVectors vector) const
{
    quint16 baseAddress = operatingSystem->getBurnValue();
    quint16 offset = getMemoryVectorOffset(vector);
    quint16 actual = baseAddress-offset;
    auto asmCode = operatingSystem->memAddressToCode(actual);
    if(dynamic_cast<const DotAddrss*>(asmCode) != nullptr) {
        const DotAddrss* dAddr = static_cast<const DotAddrss*>(asmCode);
        // The value of a .ADDRSS is the value of it's symbolic operand
        return static_cast<quint16>(dAddr->getSymbolicOperand()->getValue());
    }
    // If the location at a memory vector is not a .ADDRSS command, then the value
    // is malformed, so return a distinct value that will be easy to spot.
    return 0xDEAD;
}

void AsmProgramManager::setUserProgram(QSharedPointer<AsmProgram>prog)
{
    userProgram = prog;
}

AsmProgram *AsmProgramManager::getProgramAt(quint16 address)
{
    if(!userProgram.isNull()) {

        if(userProgram->getProgramBounds().first <= address && userProgram->getProgramBounds().second >= address){
            return userProgram.data();
        }
    }
    else if(!operatingSystem.isNull()) {

        if(operatingSystem->getProgramBounds().first <= address && operatingSystem->getProgramBounds().second >= address){
            return operatingSystem.data();
        }
    }
    return nullptr;
}

QSet<quint16> AsmProgramManager::getBreakpoints() const
{
    QSet<quint16> breakpoints;
    QList<QSharedPointer<AsmProgram>> progsList;
    if(!userProgram.isNull()) progsList.append(userProgram);
    if(!operatingSystem.isNull()) progsList.append(operatingSystem);
    for(QSharedPointer<AsmProgram> prog : progsList) {
        for(QSharedPointer<AsmCode> code : prog->getProgram())
        {
            if(code->hasBreakpoint()) breakpoints.insert(code->getMemoryAddress());
        }
    }
    return breakpoints;
}

const AsmProgram *AsmProgramManager::getProgramAt(quint16 address) const
{
    if(!userProgram.isNull()) {

        if(userProgram->getProgramBounds().first <= address && userProgram->getProgramBounds().second >= address){
            return userProgram.data();
        }
    }
    else if(!operatingSystem.isNull()) {

        if(operatingSystem->getProgramBounds().first <= address && operatingSystem->getProgramBounds().second >= address){
            return operatingSystem.data();
        }
    }
    return nullptr;
}

void AsmProgramManager::onBreakpointAdded(quint16 address)
{
    #pragma message("TODO: Respond to breakpoints being added")
    emit breakpointAdded(address);
}

void AsmProgramManager::onBreakpointRemoved(quint16 address)
{
#pragma message("TODO: Respond to breakpoints being removed")
    emit breakpointRemoved(address);
}

void AsmProgramManager::onRemoveAllBreakpoints()
{
#pragma message("TODO: Respond to all breakpoints being removed")
    emit removeAllBreakpoints();
}

quint16 AsmProgramManager::getMemoryVectorOffset(MemoryVectors which)
{
    switch(which) {
    case UserStack:;
        return 11;
    case SystemStack:;
        return 9;
    case CharIn:;
        return 7;
    case CharOut:;
        return 5;
    case Loader:;
        return 3;
    case Trap:;
        return 1;
    }
}
