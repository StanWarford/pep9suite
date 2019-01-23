#include "interfaceisacpu.h"
#include <QDebug>
#include "pep.h"
#include "enu.h"
#include "asmprogrammanager.h"
#include "asmprogram.h"
#include "typetags.h"
InterfaceISACPU::InterfaceISACPU(const AsmProgramManager* manager) noexcept: manager(manager),
    breakpointsISA(), asmInstructionCounter(0), asmBreakpointHit(false), doDebug(false),
    firstLineAfterCall(false), isTrapped(false),userActions(), osActions(), activeActions(&userActions),
    userStackIntact(true), osStackIntact(true), activeIntact(&userStackIntact)
{
    memTrace.activeStack = &memTrace.userStack;
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

void InterfaceISACPU::calculateStackChangeStart(quint8 instr, quint16 sp)
{
    if(Pep::isTrapMap[Pep::decodeMnemonic[instr]]) {
        isTrapped = true;
        memTrace.activeStack = &memTrace.osStack;
        activeActions = &osActions;
        activeIntact = &osStackIntact;
    }
    else if(Pep::decodeMnemonic[instr] == Enu::EMnemonic::RETTR) {
        isTrapped = false;
        memTrace.activeStack = &memTrace.userStack;
        activeActions = &userActions;
        activeIntact = &userStackIntact;
    }
}

void InterfaceISACPU::calculateStackChangeEnd(quint8 instr, quint16 opspec, quint16 sp, quint16 pc)
{
    switch(Pep::decodeMnemonic[instr]) {
    case Enu::EMnemonic::CALL:
        if(isTrapped) break;
        firstLineAfterCall = true;
        memTrace.activeStack->call(sp);
        activeActions->push(stackAction::call);
        qDebug() << "Called!" ;
        qDebug().noquote() << *(memTrace.activeStack);
        break;

    case Enu::EMnemonic::RET:
        if(isTrapped) break;
        if(activeActions->isEmpty()) break;
        switch(activeActions->pop()) {
        case stackAction::call:
            memTrace.activeStack->ret();
            firstLineAfterCall = true;
            qDebug() << "Returned!" ;
            qDebug().noquote() << *(memTrace.activeStack);
            break;
        default:
            qDebug() <<"Unbalanced stack operation 1";
            *activeIntact = false;
        }
        break;

    case Enu::EMnemonic::SUBSP:
        if(isTrapped) break;
        if(firstLineAfterCall) {
#pragma message("TODO: Verify OS matches trace tag len. Verify trace tags exist.")
            if(manager->getProgramAt(pc)->getTraceInfo().instrToSymlist.contains(pc)) {
                //qDebug().noquote().nospace() << manager->getProgramAt(pc)->getTraceInfo().instrToSymlist[pc];
                QList<QPair<Enu::ESymbolFormat,QString>> primList;
                for(auto item : manager->getProgramAt(pc)->getTraceInfo().instrToSymlist[pc]) {
                    primList.append(item->toPrimitives());
                }
                memTrace.activeStack->pushLocals(sp, primList);
            }
            activeActions->push(stackAction::locals);
            qDebug() << "Alloc'ed Locals!" ;
        }
        else {
            if(manager->getProgramAt(pc)->getTraceInfo().instrToSymlist.contains(pc)) {
                //qDebug().noquote().nospace() << manager->getProgramAt(pc)->getTraceInfo().instrToSymlist[pc];
                QList<QPair<Enu::ESymbolFormat,QString>> primList;
                for(auto item : manager->getProgramAt(pc)->getTraceInfo().instrToSymlist[pc]) {
                    primList.append(item->toPrimitives());
                }
                memTrace.activeStack->pushParams(sp, primList);
            }
            activeActions->push(stackAction::params);
            qDebug() << "Alloc'ed params! " ;//<< activeStack->top();
        }
        qDebug().noquote()<< *(memTrace.activeStack);
        break;

    case Enu::EMnemonic::ADDSP:
        if(isTrapped) break;
        if(activeActions->isEmpty()) break;
        switch(activeActions->pop()) {
        case stackAction::locals:
            if(manager->getProgramAt(pc)->getTraceInfo().instrToSymlist.contains(pc)) {
                //qDebug().noquote().nospace() << manager->getProgramAt(pc)->getTraceInfo().instrToSymlist[pc];
                quint16 size = 0;
                for(auto item : manager->getProgramAt(pc)->getTraceInfo().instrToSymlist[pc]) {
                    size += item->size();
                }
                memTrace.activeStack->popLocals(size);
            }
            qDebug() << "Popped locals!" ;
            qDebug().noquote() << *(memTrace.activeStack);
            break;
        case stackAction::params:
#pragma message("TODO: Verify that sizes match")
            if(manager->getProgramAt(pc)->getTraceInfo().instrToSymlist.contains(pc)) {
                //qDebug().noquote().nospace() << manager->getProgramAt(pc)->getTraceInfo().instrToSymlist[pc];
                quint16 size = 0;
                for(auto item : manager->getProgramAt(pc)->getTraceInfo().instrToSymlist[pc]) {
                    size += item->size();
                }
                memTrace.activeStack->popParams(size);
            }
            qDebug() << "Popped Params! " ;//<< activeStack->top();
            qDebug().noquote() << *(memTrace.activeStack);
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
    memTrace.clear();
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
