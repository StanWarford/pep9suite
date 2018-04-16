#include "microcodeprogram.h"
#include "code.h"

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

MicrocodeProgram::MicrocodeProgram(QVector<Code*>objectCode):
    programVec(objectCode),preconditionsVec(),postconditionsVec(),microcodeVec()
{
    Code* x;
    for(int it=0; it<objectCode.size();it++)
    {
        x=objectCode[it];
        if(x->hasUnitPre())preconditionsVec.append(it);
        else if(x->hasUnitPost())postconditionsVec.append(it);
        else if(x->isMicrocode())microcodeVec.append(it);
    }
    for(int it=0;it<microcodeVec.length();it++)
    {
        MicroCode* line = ((MicroCode*)programVec[microcodeVec[it]]);
        if(line->getBranchFunction()==Enu::Assembler_Assigned&&it+1<microcodeVec.length())
        {
            line->setBranchFunction(Enu::Unconditional);
            line->setTrueTarget(it+1);
        }
        else if(line->getBranchFunction()==Enu::Assembler_Assigned&&it+1==microcodeVec.length())
        {
            line->setBranchFunction(Enu::Stop);
        }
    }
}

const QVector<Code*> MicrocodeProgram::getObjectCode() const
{
    return this->programVec;
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

