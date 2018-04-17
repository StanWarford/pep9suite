#include "microcodeprogram.h"
#include "code.h"
#include "SymbolEntry.h"
#include "SymbolValue.h"
MicrocodeProgram::MicrocodeProgram(): programVec(),preconditionsVec(),postconditionsVec(),microcodeVec()
{

}

MicrocodeProgram::~MicrocodeProgram()
{
    for(int it = 0;it< programVec.length();it++)
    {
        delete programVec[it];
    }
}

MicrocodeProgram::MicrocodeProgram(QVector<Code*>objectCode,SymbolTable* symbolTable):
    symTable(symbolTable),programVec(objectCode),
    preconditionsVec(),postconditionsVec(),microcodeVec()
{
    Code* x;
    for(int it=0; it<objectCode.size();it++)
    {
        x=objectCode[it];
        if(x->hasUnitPre())preconditionsVec.append(it);
        else if(x->hasUnitPost())postconditionsVec.append(it);
        else if(x->isMicrocode())microcodeVec.append(it);
    }
    for(int it = 0;it < microcodeVec.length();it++)
    {
        MicroCode* line = ((MicroCode*)programVec[microcodeVec[it]]);
        if((line->getSymbol())==nullptr)
        {
            line->setSymbol(symTable->insertSymbol("_as"+QString::number(it)).get());
        }
        line->getSymbol()->setValue(std::shared_ptr<SymbolValueLocation>::make_shared(it));
    }
    for(int it=0;it<microcodeVec.length();it++)
    {
        MicroCode* line = ((MicroCode*)programVec[microcodeVec[it]]);
        if(line->getBranchFunction()==Enu::Assembler_Assigned&&it+1<microcodeVec.length())
        {
            line->setBranchFunction(Enu::Unconditional);
            line->setTrueTarget(((MicroCode*)programVec[microcodeVec[it+1]])->getSymbol());
            line->setFalseTarget(((MicroCode*)programVec[microcodeVec[it+1]])->getSymbol());
        }
        else if(line->getBranchFunction()==Enu::Assembler_Assigned&&it+1==microcodeVec.length())
        {
            line->setBranchFunction(Enu::Stop);
            line->setTrueTarget(((MicroCode*)programVec[microcodeVec[it]])->getSymbol());
            line->setFalseTarget(((MicroCode*)programVec[microcodeVec[it]])->getSymbol());
        }
        if(line->getTrueTarget()==nullptr)
        {
            line->setTrueTarget(((MicroCode*)programVec[microcodeVec[it]])->getSymbol());
        }
        if(line->getFalseTarget()==nullptr)
        {
            line->setFalseTarget(((MicroCode*)programVec[microcodeVec[it]])->getSymbol());
        }
    }
    for(auto a : symTable->getSymbolEntries())
    {
        qDebug()<<a->getName()<<","<<a->getValue();
    }
}

const QVector<Code*> MicrocodeProgram::getObjectCode() const
{
    return this->programVec;
}

const QString MicrocodeProgram::format() const
{
    QString output = "";
    for(Code* line : programVec)
    {
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
    if(codeLine<microcodeVec.size())
    {
        return (MicroCode*) programVec[microcodeVec[codeLine]];
    }
    return nullptr;
}

int MicrocodeProgram::codeLength() const
{
    return microcodeVec.length();
}

