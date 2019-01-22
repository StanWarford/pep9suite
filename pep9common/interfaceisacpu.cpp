#include "interfaceisacpu.h"
#include <QDebug>
#include "pep.h"
#include "enu.h"
#include "asmprogrammanager.h"
#include "asmprogram.h"
InterfaceISACPU::InterfaceISACPU(const AsmProgramManager* manager) noexcept: manager(manager),
    breakpointsISA(), asmInstructionCounter(0), asmBreakpointHit(false), doDebug(false),
    firstLineAfterCall(false), isTrapped(false),userActions(), osActions(), activeActions(&userActions),
    userStackIntact(true), osStackIntact(true), activeIntact(&userStackIntact)
{

}

InterfaceISACPU::~InterfaceISACPU()
{

}

const QSet<quint16> InterfaceISACPU::getPCBreakpoints() const noexcept
{
    return breakpointsISA;
}

void InterfaceISACPU::breakpointsSet(QSet<quint16> addresses) noexcept
{
    breakpointsISA = addresses;
    if(doDebug) qDebug() << "BP set " << breakpointsISA;
}

void InterfaceISACPU::breakpointsRemoveAll() noexcept
{
    breakpointsISA.clear();
    if(doDebug) qDebug() << "BP cleared";
}

void InterfaceISACPU::breakpointRemoved(quint16 address) noexcept
{
    breakpointsISA.remove(address);
    if(doDebug) qDebug() << "Removed breakpoint at: " << address;
}


void InterfaceISACPU::breakpointAdded(quint16 address) noexcept
{
    breakpointsISA.insert(address);
    if(doDebug)qDebug() << "Added breakpoint at: " << address;
}

void InterfaceISACPU::setDebugBreakpoints(bool doDebug) noexcept
{
    this->doDebug = doDebug;
}

void InterfaceISACPU::calculateStackChangeStart(quint8 instr)
{
    if(Pep::isTrapMap[Pep::decodeMnemonic[instr]]) {
        isTrapped = true;
        activeActions = &osActions;
        activeIntact = &osStackIntact;
    }
    else if(Pep::decodeMnemonic[instr] == Enu::EMnemonic::RETTR) {
        isTrapped = false;
        activeActions = &userActions;
        activeIntact = &userStackIntact;
    }
}

void InterfaceISACPU::calculateStackChangeEnd(quint8 instr, quint16 opspec)
{
    switch(Pep::decodeMnemonic[instr]) {
    case Enu::EMnemonic::CALL:
        if(isTrapped) break;
        firstLineAfterCall = true;
        //activeStack->top() += 2;
        activeActions->push(stackAction::call);
        qDebug() << "Called! " ;//<< activeStack->top();
        break;
    case Enu::EMnemonic::RET:
        if(isTrapped) break;
        if(activeActions->isEmpty()) break;
        switch(activeActions->pop()) {
        case stackAction::call:
            //activeStack->top() -= 2;
            qDebug() << "Returned! " ;//<< activeStack->top();
            firstLineAfterCall = true;
            break;
        default:
            qDebug() <<"Unbalanced stack operation 1";
            *activeIntact = false;
        }
        break;
    case Enu::EMnemonic::SUBSP:
        if(isTrapped) break;
        if(firstLineAfterCall) {
            activeActions->push(stackAction::locals);
            //activeStack->top() += cpu.getCPURegWordCurrent(Enu::CPURegisters::OS);
            qDebug() << "Alloc'ed Locals! " ;//<< activeStack->top();
        }
        else {
            activeActions->push(stackAction::params);
            //activeStack->push(registers.regState.reg_OS);
            qDebug() << "Alloc'ed params! " ;//<< activeStack->top();
        }
        break;
    case Enu::EMnemonic::ADDSP:
        if(isTrapped) break;
        if(activeActions->isEmpty()) break;
        switch(activeActions->pop()) {
        case stackAction::locals:
            //activeStack->pop();
            qDebug() << "Popped locals! SS:" ;//<< activeStack->size();
            break;
        case stackAction::params:
            //activeStack->top() -= cpu.getCPURegWordCurrent(Enu::CPURegisters::OS);
            qDebug() << "Popped Params! " ;//<< activeStack->top();
            break;
        default:
            qDebug() << "Unbalance stack operation 2";
            *activeIntact = false;
        }
        break;
    case Enu::EMnemonic::BR:
        [[fallthrough]];
    case Enu::EMnemonic::BRC:
        [[fallthrough]];
    case Enu::EMnemonic::BREQ:
        [[fallthrough]];
    case Enu::EMnemonic::BRGE:
        [[fallthrough]];
    case Enu::EMnemonic::BRGT:
        [[fallthrough]];
    case Enu::EMnemonic::BRLE:
        [[fallthrough]];
    case Enu::EMnemonic::BRLT:
        [[fallthrough]];
    case Enu::EMnemonic::BRNE:
        [[fallthrough]];
    case Enu::EMnemonic::BRV:
        firstLineAfterCall = true;
        break;
    default:
        firstLineAfterCall = false;
        break;
    }
}

void InterfaceISACPU::reset() noexcept
{
    asmInstructionCounter = 0;
    asmBreakpointHit = false;

    // Dependant upon trace info of program
    userStackIntact = false;
    osStackIntact = false;

    userActions.clear();
    osActions.clear();
    activeActions = & userActions;
    activeIntact = &userStackIntact;
    isTrapped = false;
    firstLineAfterCall = false;
}
