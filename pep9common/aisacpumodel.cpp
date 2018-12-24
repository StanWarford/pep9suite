#include "aisacpumodel.h"
#include <QDebug>
InterfaceISACPU::InterfaceISACPU(): breakpointsISA(), asmInstructionCounter(0), asmBreakpointHit(false), doDebug(false)
{

}

InterfaceISACPU::~InterfaceISACPU()
{

}

const QSet<quint16> InterfaceISACPU::getPCBreakpoints() const
{
    return breakpointsISA;
}

void InterfaceISACPU::breakpointsSet(QSet<quint16> addresses)
{
    breakpointsISA = addresses;
    if(doDebug) qDebug() << "BP set " << breakpointsISA;
}

void InterfaceISACPU::breakpointsRemoveAll()
{
    breakpointsISA.clear();
    if(doDebug) qDebug() << "BP cleared";
}

void InterfaceISACPU::breakpointRemoved(quint16 address)
{
    breakpointsISA.remove(address);
    if(doDebug) qDebug() << "Removed breakpoint at: " << address;
}


void InterfaceISACPU::breakpointAdded(quint16 address)
{
    breakpointsISA.insert(address);
    if(doDebug)qDebug() << "Added breakpoint at: " << address;
}

void InterfaceISACPU::setDebugBreakpoints(bool doDebug)
{
    this->doDebug = doDebug;
}

void InterfaceISACPU::reset()
{
    asmInstructionCounter = 0;
    asmBreakpointHit = false;
}
