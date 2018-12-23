#ifndef AMCCPUMODEL_H
#define AMCCPUMODEL_H

#include "ACPUModel.h"
#include "enu.h"
class MicroCode;
class MicrocodeProgram;
class InterfaceMCCPU
{
public:
    explicit InterfaceMCCPU(Enu::CPUType type);
    virtual ~InterfaceMCCPU();

    unsigned int getMicrocodeLineNumber() const;
    const MicrocodeProgram* getProgram() const;
    const MicroCode* getCurrentMicrocodeLine() const;
    void setMicrocodeProgram(MicrocodeProgram* program);
    virtual void setCPUType() = 0;
    Enu::CPUType getCPUType() const;

    virtual void onMCStep() = 0;
    virtual void onClock() = 0;

protected:
    virtual void branchHandler() = 0; // Based on the current instruction, set the µPC correctly
    unsigned int microprogramCounter, microCycleCounter;
    bool microBreakpointHit;
    MicrocodeProgram* program;
    Enu::CPUType type;
};

#endif // AMCCPUMODEL_H
