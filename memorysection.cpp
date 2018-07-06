#include "memorysection.h"
#include <QApplication>
MemorySection* MemorySection::instance = nullptr;
MemorySection *MemorySection::getInstance()
{
    if(MemorySection::instance == nullptr)
    {
        MemorySection::instance = new MemorySection();
    }
    return MemorySection::instance;
}

MemorySection::MemorySection(QObject *parent) : QObject(parent), waiting(true), inBuffer(), maxBytes(0xffff), iPort(1560), oPort(1562),
    hadMemoryError(false), errorMessage()
{
    initializeMemory();
}

void MemorySection::initializeMemory()
{
    memory = QVector<quint8>(maxBytes+1);
}

quint8 MemorySection::getMemoryByte(quint16 address, bool useIOPorts) const
{
    if(address == iPort )
    {
        int i=4;
    }
    if(address == iPort && useIOPorts && false )
    {
        quint8 value;
        waiting = true;
        emit this->charRequestedFromInput();
        while(waiting)
        {
            QApplication::processEvents();
        }
        if(waiting == false && inBuffer.isEmpty())
        {
            hadMemoryError = true;
            errorMessage = "Memory Error: Requested input but received no input.";
        }
        value = inBuffer[0].toLatin1();
        inBuffer.remove(0,1);
        return value;

    }
    return memory[address];
}

quint16 MemorySection::getMemoryWord(quint16 address, bool useIOPorts) const
{
    if(address>0xfffe) return 0;
    return ((quint16)getMemoryByte(address,useIOPorts)<<8)|getMemoryByte(address+1,useIOPorts);
}

const QVector<quint8> MemorySection::getMemory() const
{
    return memory;
}

bool MemorySection::hadError() const
{
    return hadMemoryError;
}

const QString MemorySection::getErrorMessage()
{
    return errorMessage;
}

void MemorySection::setMemoryByte(quint16 address, quint8 value)
{
    quint8 old= memory[address];
    //if(old == value)return; //Don't continue if the new value is the old value
    onSetMemoryByte(address,value);
}

void MemorySection::setMemoryWord(quint16 address, quint16 value)
{
    address&=0xFFFE; //Memory access ignores the lowest order bit
    quint8 hi=memory[address],lo=memory[address+1]; //Cache old memory values
    //if((((quint16)hi<<8)|lo)==value)return; //Don't continue if the new value is the old value
    onSetMemoryWord(address,value);
}

void MemorySection::clearMemory() noexcept
{
    //Set all memory values to 0
    for(int it=0;it<memory.length();it++)
    {
        memory[it]=0;
    }
}

void MemorySection::clearErrors() noexcept
{
    hadMemoryError = false;
    errorMessage = "";
}

void MemorySection::loadObjectCode(quint16 address, QVector<quint8> values)
{
    int idx = 0;
    for(; idx < values.length() && idx + address <= maxBytes; idx++)
    {
        memory[idx + address] = values[idx];
    }
}

void MemorySection::onMemorySizeChanged(quint16 maxBytes)
{
    this->maxBytes = maxBytes;
    initializeMemory();
}

void MemorySection::onIPortChanged(quint16 newIPort)
{
    iPort = newIPort;
}

void MemorySection::onOPortChanged(quint16 newOPort)
{
    oPort = newOPort;
}

void MemorySection::onAppendInBuffer(const QString &newData)
{
    inBuffer.append(newData);
}

void MemorySection::onCancelWaiting()
{
    waiting = false;
}

void MemorySection::onSetMemoryByte(quint16 address, quint8 val)
{
    quint8 old = memory[address];
    if(address == oPort)
    {
        emit this->charWrittenToOutput(val);
    }
    memory[address] = val;
    if(old != val) emit memoryChanged(address,old,val);
}

void MemorySection::onSetMemoryWord(quint16 address, quint16 val)
{
    //Do not enforce aligned memory access here, as precondition code in homeworks depends on access being unaligned.
    onSetMemoryByte(address,val/256);
    onSetMemoryByte(address+1, val%256);

}

void MemorySection::onClearMemory() noexcept
{
    //On memory reset, only clear RAM
    clearMemory();
    clearErrors();
}

void MemorySection::onInstructionFinished() noexcept
{
#pragma message ("TODO")
}
