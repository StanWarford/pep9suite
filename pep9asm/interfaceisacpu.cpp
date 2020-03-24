// File: interfaceisacpu.cpp
/*
    Pep9 is a virtual machine for writing machine language and assembly
    language programs.

    Copyright (C) 2019 J. Stanley Warford & Matthew McRaven, Pepperdine University

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

#include "interfaceisacpu.h"
#include <QDebug>
#include "pep.h"
#include "enu.h"
#include "asmprogrammanager.h"
#include "asmprogram.h"
#include "typetags.h"
#include "symbolentry.h"
#include "asmcode.h"
InterfaceISACPU::InterfaceISACPU(const AMemoryDevice* dev, const AsmProgramManager* manager) noexcept:
    manager(manager), opValCache(0),
    breakpointsISA(), asmStepCount(0), asmBreakpointHit(false), doDebug(false),
    firstLineAfterCall(false), isTrapped(false), memTrace(QSharedPointer<MemoryTrace>::create()),
    userActions(), osActions(), activeActions(&userActions)
{
    memTrace->activeStack = &memTrace->userStack;
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

QSharedPointer<const MemoryTrace> InterfaceISACPU::getMemoryTrace() const
{
    return memTrace;
}

quint16 InterfaceISACPU::getOperandValue() const
{
    return opValCache;
}

void InterfaceISACPU::setDebugBreakpoints(bool doDebug) noexcept
{
    this->doDebug = doDebug;
}

void InterfaceISACPU::doISAStepWhile(std::function<bool ()> condition)
{
    do{
        onISAStep();
    } while(condition());
}

void InterfaceISACPU::calculateStackChangeStart(quint8 instr)
{
    if(Pep::isTrapMap[Pep::decodeMnemonic[instr]]) {
        isTrapped = true;
        activeActions = &osActions;
    }
    else if(Pep::decodeMnemonic[instr] == Enu::EMnemonic::RETTR) {
        isTrapped = false;
        memTrace->activeStack = &memTrace->userStack;
        activeActions = &userActions;
    }
}

void InterfaceISACPU::calculateStackChangeEnd(quint8 instr, quint16 opspec, quint16 sp, quint16 pc, quint16 acc)
{
    /*
     * Following sanity checks must be performed:
     *  x - Have static trace tag errors corrupted the stack (one time check)?
     *  x - Has a dynamic runtime operation corrupted the stack?
     *  x - Is the addressing mode immediate?
     *  x - Do the tags listed in comments match in length to the argument?
     *  x - Should trace tags be tracked? Or does the program lack tags?
     *  x - What if some lines have trace tags but others don't?
     *  x - Is malloc being called?
     *
     * **StackTrace**
     *  x - After push / pop in StackTrace, verify that it returns true.
     *      x - If it returns false, we have a corrupt stack.
     *  x - If CallStack is ever exhausted before size is hit, return false.
     */

    if(!memTrace->activeStack->isStackIntact() || this->manager->getProgramAt(pc) == nullptr
            // For now, only allow tracing of user programs
            || this->manager->getUserProgram() != this->manager->getProgramAt(pc)) return;
    Enu::EMnemonic mnemon = Pep::decodeMnemonic[instr];
    quint16 size = 0;
    bool mallocPreError = false;
    switch(mnemon) {
    case Enu::EMnemonic::CALL:
        firstLineAfterCall = true;
        memTrace->activeStack->call(sp - 2);
        activeActions->push(stackAction::call);
        if(dynamic_cast<const NonUnaryInstruction*>(manager->getUserProgram()->memAddressToCode(pc)) != nullptr){
            const NonUnaryInstruction* instr = dynamic_cast<const NonUnaryInstruction*>(manager->getUserProgram()->memAddressToCode(pc));
            // If a previous call to malloc has corrupted the heap,
            // don't attempt any further processing.
            if(memTrace->heapTrace.canAddNew() == false) return;
            // A call to things other than malloc don't trigger heap changes.
            else if(instr->getSymbolicOperand()->getName() != "malloc") return;
            // In case a user wrote a self modifying program, and
            // give up on tracking futue heap changes.
            else if(instr->getMnemonic() != Enu::EMnemonic::CALL) mallocPreError = true;
            // Don't try to track calls to malloc via a literal address, and give up
            // on tracking future heap changes.
            else if(!instr->hasSymbolicOperand()) mallocPreError = true;

            // If there was an error, prevent any new heap adjustments from being made.
            if(mallocPreError == true) {
                memTrace->heapTrace.setCanAddNew(false);
                #pragma message("TODO: Add heap error messages")
                memTrace->heapTrace.setErrorMessage("Heap corrupted.");
                return;
            }
            // A call with no symbol traces listed is ignored.
            else if(manager->getUserProgram()->getTraceInfo()->instrToSymlist.contains(pc)) {
                memTrace->heapTrace.setInMalloc(true);
                QList<QPair<Enu::ESymbolFormat, QString>> primList;
                for(auto item : manager->getProgramAt(pc)->getTraceInfo()->instrToSymlist[pc]) {
                    primList.append(item->toPrimitives());
                }
                memTrace->heapTrace.pushHeap(heapPtr, primList);
                heapPtr += acc;
            }
            else {
                memTrace->heapTrace.setCanAddNew(false);
                memTrace->heapTrace.setErrorMessage("Added object to heap with no trace tags.");
            }


        }
        //qDebug() << "Called!" ;
        //qDebug().noquote() << *(memTrace->activeStack);
        break;

    case Enu::EMnemonic::RET:
        if(activeActions->isEmpty()) break;
        switch(activeActions->pop()) {
        case stackAction::call:
            if(memTrace->activeStack->ret()) {
                firstLineAfterCall = true;
                memTrace->heapTrace.setInMalloc(false);
                //qDebug() << "Returned!" ;
                //qDebug().noquote() << *(memTrace->activeStack);
            }
            else {
                //qDebug() <<"Unbalanced stack operation 7";
                memTrace->activeStack->setStackIntact(false);
                memTrace->activeStack->setErrorMessage("ERROR: Executed a return, expected a ADD- or SUBSP.");
            }
            break;
        default:
            memTrace->activeStack->setErrorMessage("ERROR: Unspecified error during return (e.g. stack was empty).");
            memTrace->activeStack->setStackIntact(false);
        }
        break;

    case Enu::EMnemonic::SUBSP:
        if(manager->getProgramAt(pc)->getTraceInfo()->instrToSymlist.contains(pc)) {
            quint16 size = 0;
            for(auto pair : manager->getProgramAt(pc)->getTraceInfo()->instrToSymlist[pc]) {
                size += pair->size();
            }
            if(size != opspec) {
                memTrace->activeStack->setStackIntact(false);
                memTrace->activeStack->setErrorMessage("ERROR: Operand of SUBSP does not match size of trace tags.");
                break;
            }
        }
        if(firstLineAfterCall) {
            if(manager->getProgramAt(pc)->getTraceInfo()->instrToSymlist.contains(pc)) {
                QList<QPair<Enu::ESymbolFormat,QString>> primList;
                for(auto item : manager->getProgramAt(pc)->getTraceInfo()->instrToSymlist[pc]) {
                    primList.append(item->toPrimitives());
                }
                memTrace->activeStack->pushLocals(sp, primList);
            }
            activeActions->push(stackAction::locals);
            //qDebug() << "Alloc'ed Locals!" ;
        }
        else {
            if(manager->getProgramAt(pc)->getTraceInfo()->instrToSymlist.contains(pc)) {
                QList<QPair<Enu::ESymbolFormat,QString>> primList;
                for(auto item : manager->getProgramAt(pc)->getTraceInfo()->instrToSymlist[pc]) {
                    primList.append(item->toPrimitives());
                }
                memTrace->activeStack->pushParams(sp, primList);
            }
            activeActions->push(stackAction::params);
            //qDebug() << "Alloc'ed params! " ;//<< activeStack->top();
        }
        //qDebug().noquote()<< *(memTrace->activeStack);
        break;

    case Enu::EMnemonic::ADDSP:
        if(manager->getProgramAt(pc)->getTraceInfo()->instrToSymlist.contains(pc)) {
            for(auto pair : manager->getProgramAt(pc)->getTraceInfo()->instrToSymlist[pc]) {
                size += pair->size();
            }
            if(size != opspec) {
                memTrace->activeStack->setStackIntact(false);
                memTrace->activeStack->setErrorMessage("ERROR: Operand of ADDSP does not match size of trace tags.");
                break;
            }
        }
        else if(activeActions->isEmpty()) {
            memTrace->activeStack->setErrorMessage("ERROR: Executed ADDSP, but no items are eligible to be popped.");
            memTrace->activeStack->setStackIntact(false);
        }
        else {
            memTrace->activeStack->setErrorMessage("ERROR: Executed ADDSP, but no trace info was available.");
            memTrace->activeStack->setStackIntact(false);
            break;
        }
        switch(activeActions->pop()) {
        case stackAction::locals:
            if(memTrace->activeStack->popLocals(size)) {
                //qDebug() << "Popped locals!" ;
                //qDebug().noquote() << *(memTrace->activeStack);
            }
            else {
                memTrace->activeStack->setErrorMessage("ERROR: Executed ADDSP when a return was expected.");
                memTrace->activeStack->setStackIntact(false);
            }
            break;
        case stackAction::params:
            if(memTrace->activeStack->getTOS().size()>size
                    && memTrace->activeStack->popAndOrphan(size)) {
                activeActions->push(stackAction::params);
                //qDebug() << "Popped Params & orphaned!" ;
                //qDebug().noquote() << *(memTrace->activeStack);
            }
            else if(memTrace->activeStack->getTOS().size() <= size) {
                bool success = true;
                activeActions->push(stackAction::params);
                while(size > 0 && success) {
                    size -= memTrace->activeStack->getTOS().size();
                    success &= memTrace->activeStack->popParams(memTrace->activeStack->getTOS().size());
                    activeActions->pop();
                }
                if(success) {
                    //qDebug() << "Popped Params!" ;
                    //qDebug().noquote() << *(memTrace->activeStack);
                }
                else {
                    memTrace->activeStack->setErrorMessage("ERROR: Failed to pop correct number of bytes in ADDSP.");
                    memTrace->activeStack->setStackIntact(false);
                }
            }
            else {
                memTrace->activeStack->setErrorMessage("ERROR: Failed to pop correct number of bytes in ADDSP.");
                memTrace->activeStack->setStackIntact(false);
            }
            break;
        default:
            memTrace->activeStack->setErrorMessage("ERROR: An unspecified error occured in ADDSP (e.g. the stack was empty).");
            memTrace->activeStack->setStackIntact(false);
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
    asmStepCount = 0;
    asmBreakpointHit = false;
    memTrace->clear();
    // Only trace the stack if trace tags are present, and no assembly time
    // errors occured.
    bool hadWarnings =  false;
    // If debugging an object code program, there is no user program
    // so don't bother rendering stack.
    if(this->manager->getUserProgram().isNull()) {
        return;
    }
    else if(!this->manager->getUserProgram().isNull() ){
        hadWarnings = !this->manager->getUserProgram()->getTraceInfo()->hadTraceTags
        || manager->getUserProgram()->getTraceInfo()->staticTraceError;
    }
    memTrace->setHasTraceWarnings(hadWarnings);
    memTrace->userStack.setStackIntact(!hadWarnings);

    // Store globals, if there were no trace tag errors
    if(!memTrace->hasTraceWarnings()) {
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

        memTrace->globalTrace.setTags(lst);

        // Handle the existence of the heap
        if(manager->getUserProgram()->getTraceInfo()->hasHeapMalloc) {
            heapPtr = manager->getUserProgram()->getTraceInfo()->heapPtr->getValue();
            memTrace->heapTrace.setHeapIntact(!hadWarnings);
            memTrace->heapTrace.setCanAddNew(!hadWarnings);
        }
    }
    else {
        memTrace->heapTrace.setHeapIntact(false);
        memTrace->heapTrace.setCanAddNew(false);
    }

    userActions.clear();
    osActions.clear();
    activeActions = & userActions;
    isTrapped = false;
    firstLineAfterCall = false;
}
