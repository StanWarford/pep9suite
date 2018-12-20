#include "asmprogrammanager.h"
#include "asmprogram.h"
#include <QSharedPointer>
#include "asmcode.h"
AsmProgramManager* AsmProgramManager::instance = nullptr;
AsmProgramManager::AsmProgramManager(QObject *parent): QObject(parent), operatingSystem(nullptr), userProgram(nullptr)
{

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

void AsmProgramManager::setUserProgram(QSharedPointer<AsmProgram>prog)
{
    userProgram = prog;
}

const AsmProgram *AsmProgramManager::getProgramAtPC() const
{
#pragma message ("calculate PC")
    return getProgramAt(0);
}

AsmProgram *AsmProgramManager::getProgramAtPC()
{
#pragma message ("calculate PC")
    return getProgramAt(0);
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
