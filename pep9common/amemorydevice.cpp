#include "amemorydevice.h"

AMemoryDevice::AMemoryDevice(QObject *parent) : QObject(parent)
{

}

bool AMemoryDevice::hadError() const
{
    return error;
}

QString AMemoryDevice::getErrorMessage() const
{
    return errorMessage;
}

void AMemoryDevice::clearErrors()
{
    error = false;
    errorMessage.clear();
}

const QSet<quint16> AMemoryDevice::getBytesWritten() const
{
    return bytesWritten;
}

const QSet<quint16> AMemoryDevice::getBytesSet() const
{
    return bytesSet;
}

void AMemoryDevice::clearBytesWritten()
{
    bytesWritten.clear();
}

void AMemoryDevice::clearBytesSet()
{
    bytesSet.clear();
}

bool AMemoryDevice::readWord(quint16 &output, quint16 offsetFromBase) const
{
    quint8 temp = 0;
    bool retVal = readByte(temp, offsetFromBase);
    // No need to upsize temp first, since shift yields int32
    output = static_cast<quint16>(temp<<8);
    retVal &= readByte(temp, offsetFromBase+1);
    output |= temp;
    return retVal;
}

bool AMemoryDevice::writeWord(quint16 offsetFromBase, quint16 value)
{
    bool retVal = writeByte(offsetFromBase, value >> 8);
    retVal &= writeByte(offsetFromBase+1, value & 0xff);
    return retVal;
}

bool AMemoryDevice::getWord(quint16 &output, quint16 offsetFromBase) const
{
    quint8 temp = 0;
    bool retVal = getByte(temp, offsetFromBase);
    // No need to upsize temp first, since shift yields int32
    output = static_cast<quint16>(temp<<8);
    retVal &= getByte(temp, offsetFromBase+1);
    output |= temp;
    return retVal;
}

bool AMemoryDevice::setWord(quint16 offsetFromBase, quint16 value)
{
    bool retVal = setByte(offsetFromBase, value >> 8);
    retVal &= setByte(offsetFromBase+1, value & 0xff);
    return retVal;
}
