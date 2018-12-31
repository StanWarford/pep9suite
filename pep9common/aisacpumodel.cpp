#include "aisacpumodel.h"
#include <QDebug>
InterfaceISACPU::InterfaceISACPU() noexcept: breakpointsISA(), asmInstructionCounter(0), asmBreakpointHit(false), doDebug(false)
{

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

void InterfaceISACPU::reset() noexcept
{
    asmInstructionCounter = 0;
    asmBreakpointHit = false;
}
