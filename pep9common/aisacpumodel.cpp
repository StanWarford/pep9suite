#include "aisacpumodel.h"

InterfaceISACPU::InterfaceISACPU()
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
    //qDebug() << "BP set " << breakpointsISA;
}

void InterfaceISACPU::breakpointsRemoveAll()
{
    breakpointsISA.clear();
    //qDebug() << "BP cleared";
}

void InterfaceISACPU::breakpointRemoved(quint16 address)
{
    breakpointsISA.remove(address);
    //qDebug() << "Removed breakpoint at: " << address;
}


void InterfaceISACPU::breakpointAdded(quint16 address)
{
    breakpointsISA.insert(address);
   //qDebug() << "Added breakpoint at: " << address;
}
