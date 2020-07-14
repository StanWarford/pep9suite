#include "pep9interfaceisacpu.h"

#include "assembler/asmprogram.h"
#include "assembler/asmprogrammanager.h"
#include "pep/enu.h"
#include "pep/pep.h"
#include "stack/typetags.h"
#include "symbol/symbolentry.h"

#include "pep9asmcode.h"

Pep9InterfaceISACPU::Pep9InterfaceISACPU(const AMemoryDevice *dev, const AsmProgramManager *manager) noexcept:
    InterfaceISACPU(dev, manager)
{

}

void Pep9InterfaceISACPU::calculateStackChangeStart(quint8 instr)
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

void Pep9InterfaceISACPU::calculateStackChangeEnd(quint8 instr, quint16 opspec, quint16 sp, quint16 pc, quint16 acc)
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
                QList<QPair<ESymbolFormat, QString>> primList;
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
                QList<QPair<ESymbolFormat,QString>> primList;
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
                QList<QPair<ESymbolFormat,QString>> primList;
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

// Convert a mnemonic into its string
QString mnemonDecode(Enu::EMnemonic instrSpec)
{
    static QMetaEnum metaenum = Enu::staticMetaObject.enumerator(Enu::staticMetaObject.indexOfEnumerator("EMnemonic"));
    return QString(metaenum.valueToKey((int)instrSpec)).toLower();
}
