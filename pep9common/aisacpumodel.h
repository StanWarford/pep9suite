#ifndef AISACPUMODEL_H
#define AISACPUMODEL_H

#include "ACPUModel.h"
#include <QSet>
#include <QtCore>
class AMemoryDevice;
class InterfaceISACPU
{
public:
    explicit InterfaceISACPU();
    virtual ~InterfaceISACPU();
    const QSet<quint16> getPCBreakpoints() const;

    virtual void onISAStep() = 0;
    void breakpointsSet(QSet<quint16> addresses);
    void breakpointsRemoveAll();
    void breakpointRemoved(quint16 address);
    void breakpointAdded(quint16 address);

protected:
    QSet<quint16> breakpointsISA;
    int asmInstructionCounter;
    bool asmBreakpointHit;
    virtual void updateAtInstructionEnd() = 0; // Update simulation state at the start of a assembly level instruction
};

#endif // AISACPU_H
