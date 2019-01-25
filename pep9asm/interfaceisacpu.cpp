#include "interfaceisacpu.h"
#include <QDebug>
#include "pep.h"
#include "enu.h"
#include "asmprogrammanager.h"
#include "asmprogram.h"
#include "typetags.h"
#include "symbolentry.h"
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
#pragma message("TODO: stack checks")
    /*
     * Following sanity checks must be performed:
     *  x - Have static trace tag errors corrupted the stack (one time check)?
     *  x - Has a dynamic runtime operation corrupted the stack?
     *  x - Is the addressing mode immediate?
     *  x - Do the tags listed in comments match in length to the argument?
     *  x - Should trace tags be tracked? Or does the program lack tags?
     *  x - What if some lines have trace tags but others don't?
     *  Is alloc or free being called?
     *      How do we handle alloc?
     *
     * **StackTrace**
     *  x - After push / pop in StackTrace, verify that it returns true.
     *      x - If it returns false, we have a corrupt stack.
     *  x - If CallStack is ever exhausted before size is hit, return false.
     */
    if(!(*activeIntact)|| this->manager->getProgramAt(pc) == nullptr) return;
    Enu::EMnemonic mnemon = Pep::decodeMnemonic[instr];
    quint16 size = 0;
    switch(mnemon) {
    case Enu::EMnemonic::CALL:
        firstLineAfterCall = true;
        memTrace.activeStack->call(sp);
        activeActions->push(stackAction::call);
        qDebug() << "Called!" ;
        qDebug().noquote() << *(memTrace.activeStack);
        break;

    case Enu::EMnemonic::RET:
        if(activeActions->isEmpty()) break;
        switch(activeActions->pop()) {
        case stackAction::call:
            if(memTrace.activeStack->ret()) {
                firstLineAfterCall = true;
                qDebug() << "Returned!" ;
                qDebug().noquote() << *(memTrace.activeStack);
            }
            else {
                qDebug() <<"Unbalanced stack operation 7";
                *activeIntact = false;
            }
            break;
        default:
            qDebug() <<"Unbalanced stack operation 1";
            *activeIntact = false;
        }
        break;

    case Enu::EMnemonic::SUBSP:
        if(manager->getProgramAt(pc)->getTraceInfo()->instrToSymlist.contains(pc)) {
            quint16 size = 0;
            for(auto pair : manager->getProgramAt(pc)->getTraceInfo()->instrToSymlist[pc]) {
                size += pair->size();
            }
            if(size != opspec) {
                *activeIntact = false;
                qDebug() <<"Stack is wrong size";
                break;
            }
        }
        if(firstLineAfterCall) {
            if(manager->getProgramAt(pc)->getTraceInfo()->instrToSymlist.contains(pc)) {
                QList<QPair<Enu::ESymbolFormat,QString>> primList;
                for(auto item : manager->getProgramAt(pc)->getTraceInfo()->instrToSymlist[pc]) {
                    primList.append(item->toPrimitives());
                }
                memTrace.activeStack->pushLocals(sp, primList);
            }
            activeActions->push(stackAction::locals);
            qDebug() << "Alloc'ed Locals!" ;
        }
        else {
            if(manager->getProgramAt(pc)->getTraceInfo()->instrToSymlist.contains(pc)) {
                QList<QPair<Enu::ESymbolFormat,QString>> primList;
                for(auto item : manager->getProgramAt(pc)->getTraceInfo()->instrToSymlist[pc]) {
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
        if(manager->getProgramAt(pc)->getTraceInfo()->instrToSymlist.contains(pc)) {
            for(auto pair : manager->getProgramAt(pc)->getTraceInfo()->instrToSymlist[pc]) {
                size += pair->size();
            }
            if(size != opspec) {
                *activeIntact = false;
                qDebug() <<"Stack is wrong size";
                break;
            }
        }
        else if(activeActions->isEmpty()) {
            qDebug() << "Unbalanced stack operation 2";
            *activeIntact = false;
        }
        else break;
        switch(activeActions->pop()) {
        case stackAction::locals:
            if(memTrace.activeStack->popLocals(size)) {
                qDebug() << "Popped locals!" ;
                qDebug().noquote() << *(memTrace.activeStack);
            }
            else {
                qDebug() << "Unbalanced stack operation 5";
                *activeIntact = false;
            }
            break;
        case stackAction::params:
            if(memTrace.activeStack->getTOS().size()>size
                    && memTrace.activeStack->popAndOrphan(size)) {
                activeActions->push(stackAction::params);
                qDebug() << "Popped Params & orphaned!" ;
                qDebug().noquote() << *(memTrace.activeStack);
            }
            else if(memTrace.activeStack->getTOS().size() <= size) {
                bool success = true;
                activeActions->push(stackAction::params);
                while(size > 0 && success) {
                    size -= memTrace.activeStack->getTOS().size();
                    success &= memTrace.activeStack->popParams(memTrace.activeStack->getTOS().size());
                    activeActions->pop();
                }
                if(success) {
                    qDebug() << "Popped Params!" ;
                    qDebug().noquote() << *(memTrace.activeStack);
                }
                else {
                    qDebug() << "Unbalanced stack operation 6";
                    *activeIntact = false;
                }
            }
            else {
                qDebug() << "Unbalanced stack operation 8";
                *activeIntact = false;
            }
            break;
        default:
            qDebug() << "Unbalanced stack operation 3";
            *activeIntact = false;
            break;
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
    // Only trace the stack if trace tags are present, and no assembly time
    // errors occured.
    userStackIntact = this->manager->getUserProgram()->getTraceInfo()->hadTraceTags
            && !manager->getUserProgram()->getTraceInfo()->staticTraceError;
    // Never attempt to trace OS
    osStackIntact = false;
    // Store globals, if there were no trace tag errors
    if(userStackIntact) {
        QList<QPair<quint16,QPair<Enu::ESymbolFormat,QString>>> lst;
        QMap<QSharedPointer<const SymbolEntry>, QSharedPointer<AType>> map =
                manager->getUserProgram()->getTraceInfo()->staticAllocSymbolTypes;
        for(auto global : map.keys()) {
            QList<QPair<Enu::ESymbolFormat,QString>> innerPairs = map[global]->toPrimitives();
            quint16 addr = global->getValue();
            for(auto primitive : innerPairs) {
                lst.append({addr,{primitive}});
                addr += Enu::tagNumBytes(primitive.first);
            }

        }
        memTrace.globalTrace.setTags(lst);
    }

    userActions.clear();
    osActions.clear();
    activeActions = & userActions;
    activeIntact = &userStackIntact;
    isTrapped = false;
    firstLineAfterCall = false;
}
