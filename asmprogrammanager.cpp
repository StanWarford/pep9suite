#include "asmprogrammanager.h"
#include "asmprogram.h"
AsmProgramManager* AsmProgramManager::instance = nullptr;
AsmProgramManager::AsmProgramManager(QObject *parent): QObject(parent), operatingSystem(nullptr), userProgram(nullptr)
{

}

AsmProgramManager *AsmProgramManager::getInstance()
{
    if(instance == nullptr) instance = new AsmProgramManager();
    return instance;
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
    quint16 pc = 0;
    if(!userProgram.isNull()) {

        if(userProgram->getProgramBounds().first <= pc && userProgram->getProgramBounds().second >= pc){
            return userProgram.data();
        }
    }
    else if(!operatingSystem.isNull()) {

        if(operatingSystem->getProgramBounds().first <= pc && operatingSystem->getProgramBounds().second >= pc){
            return operatingSystem.data();
        }
    }
    return nullptr;
}

AsmProgram *AsmProgramManager::getProgramAtPC()
{
    quint16 pc = 0;
    if(!userProgram.isNull()) {

        if(userProgram->getProgramBounds().first <= pc && userProgram->getProgramBounds().second >= pc){
            return userProgram.data();
        }
    }
    else if(!operatingSystem.isNull()) {

        if(operatingSystem->getProgramBounds().first <= pc && operatingSystem->getProgramBounds().second >= pc){
            return operatingSystem.data();
        }
    }
    return nullptr;
}

void AsmProgramManager::onBreakpointAdded(quint16 address)
{
#pragma message("TODO: Respond to breakpoints being added")
}

void AsmProgramManager::onBreakpointRemoved(quint16 address)
{
#pragma message("TODO: Respond to breakpoints being removed")
}

void AsmProgramManager::onRemoveAllBreakpoints()
{
#pragma message("TODO: Respond to all breakpoints being removed")
}

void AsmProgramManager::onSetBreakpoints(QSet<quint16> addresses)
{
#pragma message("TODO: Respond to breakpoints being set")
}
