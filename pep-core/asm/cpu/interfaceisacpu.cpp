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

#include "assembler/asmcode.h"
#include "assembler/asmprogram.h"
#include "assembler/asmprogrammanager.h"
#include "stack/typetags.h"
#include "symbol/symbolentry.h"

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
        QList<QPair<quint16,QPair<ESymbolFormat,QString>>> lst;
        QMap<QSharedPointer<const SymbolEntry>, QSharedPointer<AType>> map =
                manager->getUserProgram()->getTraceInfo()->staticAllocSymbolTypes;
        for(auto global : map.keys()) {
            QList<QPair<ESymbolFormat,QString>> innerPairs = map[global]->toPrimitives();
            quint16 addr = global->getValue();
            for(auto primitive : innerPairs) {
                lst.append({addr,{primitive}});
                addr += tagNumBytes(primitive.first);
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
