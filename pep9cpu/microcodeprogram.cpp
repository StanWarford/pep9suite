#include "microcodeprogram.h"
#include "microcode.h"
#include "symbolentry.h"
#include "symbolvalue.h"
MicrocodeProgram::MicrocodeProgram(): programVec(), preconditionsVec(), postconditionsVec(), microcodeVec()
{

}

MicrocodeProgram::~MicrocodeProgram()
{
    // Since ever item in the program vector was allocated with new by the microcode pane, make sure to explicitly delete it.
    for(int it = 0;it< programVec.length();it++) {
        delete programVec[it];
    }
}

MicrocodeProgram::MicrocodeProgram(QVector<AMicroCode*>objectCode, QSharedPointer<SymbolTable> symbolTable):
    symTable(symbolTable), programVec(objectCode),
    preconditionsVec(), postconditionsVec(), microcodeVec()
{
    AMicroCode* item;
    for(int it=0; it<objectCode.size();it++) {
        item = objectCode[it];
        // If the item at the iterator is code or a test condition, put the index in the appropriate vector
        if(item->hasUnitPre()) preconditionsVec.append(it);
        else if(item->hasUnitPost()) postconditionsVec.append(it);
        else if(item->isMicrocode()) microcodeVec.append(it);
    }

    for(int it = 0; it < microcodeVec.length(); it++) {
        MicroCode* line = static_cast<MicroCode*>(programVec[microcodeVec[it]]);
        // If the line doesn't already have a symbol, create an assembler assigned one.
        if((line->getSymbol()) == nullptr) {
            line->setSymbol(symTable->insertSymbol("_as" + QString::number(it)).data());
        }
        symbolTable->setValue(line->getSymbol()->getSymbolID(), QSharedPointer<SymbolValueNumeric>::create(it));
    }
    // For each microcode line with an Assembler_Assigned branch function,
    // replace it with the appropriate Uncoditional branch to the next line of microcode.
    for(int it=0;it<microcodeVec.length();it++) {
        MicroCode* line = static_cast<MicroCode*>(programVec[microcodeVec[it]]);
        // If the line of microcode has no branch function, assume an uncoditional branch to the following line of microcode.
        if((line->getBranchFunction() == Enu::Assembler_Assigned) && (it + 1 <microcodeVec.length())) {
            line->setBranchFunction(Enu::Unconditional);
            line->setTrueTarget(static_cast<MicroCode*>(programVec[microcodeVec[it + 1]])->getSymbol());
            line->setFalseTarget(static_cast<MicroCode*>(programVec[microcodeVec[it + 1]])->getSymbol());
        }
        // If the final instruction has no explicit addressing mode, set the line's branch function to stop.
        else if((line->getBranchFunction() == Enu::Assembler_Assigned) && (it + 1 == microcodeVec.length())) {
            line->setBranchFunction(Enu::Stop);
            line->setTrueTarget(static_cast<MicroCode*>(programVec[microcodeVec[it]])->getSymbol());
            line->setFalseTarget(static_cast<MicroCode*>(programVec[microcodeVec[it]])->getSymbol());
        }
        // If either the true are false targets are null pointers, set the target to self.
        // This could prevent crashes if someone attempt to operate on either target without an explit null check.
        if(line->getTrueTarget() == nullptr) {
            line->setTrueTarget(static_cast<MicroCode*>(programVec[microcodeVec[it]])->getSymbol());
        }
        if(line->getFalseTarget() == nullptr) {
            line->setFalseTarget(static_cast<MicroCode*>(programVec[microcodeVec[it]])->getSymbol());
        }
    }
}

QSharedPointer<const SymbolTable> MicrocodeProgram::getSymTable() const
{
    return this->symTable;
}

const QVector<AMicroCode*> MicrocodeProgram::getObjectCode() const
{
    return this->programVec;
}

const QString MicrocodeProgram::format() const
{
    QString output = "";
    for(AMicroCode* line : programVec) {
        output.append(line->getSourceCode()  +"\n");
    }
    return output;
}

int MicrocodeProgram::codeLineToProgramLine(int codeLine) const
{
    return microcodeVec[codeLine];
}

bool MicrocodeProgram::hasMicrocode() const
{
    return !microcodeVec.isEmpty();
}

bool MicrocodeProgram::hasUnitPre() const
{
    return !preconditionsVec.empty();
}

const MicroCode *MicrocodeProgram::getCodeLine(quint16 codeLine) const
{
    if(codeLine<microcodeVec.size()) {
        return static_cast<MicroCode*>(programVec[microcodeVec[codeLine]]);
    }
    return nullptr;
}

MicroCode *MicrocodeProgram::getCodeLine(quint16 codeLine)
{
    if(codeLine<microcodeVec.size()) {
        return static_cast<MicroCode*>(programVec[microcodeVec[codeLine]]);
    }
    return nullptr;
}

int MicrocodeProgram::codeLength() const
{
    return microcodeVec.length();
}

