#include "amemorychip.h"

AMemoryChip::AMemoryChip(quint32 size, quint16 baseAddress, QObject *parent): QObject(parent), size(size), baseAddress(baseAddress)
{

}

AMemoryChip::~AMemoryChip()
{

}

void AMemoryChip::setBaseAddress(quint16 newAddress)
{
    baseAddress = newAddress;
}

quint32 AMemoryChip::getSize() const
{
    return size;
}

quint16 AMemoryChip::getBaseAddress() const
{
    return baseAddress;
}

bool AMemoryChip::readWord(quint16& output, quint16 offsetFromBase) const
{
    if(offsetFromBase+1 >= size) outOfBoundsReadHelper(offsetFromBase);
    quint8 temp = 0;
    bool retVal = readByte(temp, offsetFromBase);
    // No need to upsize temp first, since shift yields int32
    output = static_cast<quint16>(temp<<8);
    retVal &= readByte(temp, offsetFromBase+1);
    output |= temp;
    return retVal;

}

bool AMemoryChip::writeWord(quint16 offsetFromBase, quint16 value)
{
    if(offsetFromBase+1 >= size) outOfBoundsWriteHelper(offsetFromBase, value);
    bool retVal = writeByte(offsetFromBase, value >> 8);
    retVal &= writeByte(offsetFromBase+1, value & 0xff);
    return retVal;
}

bool AMemoryChip::getWord(quint16& output, quint16 offsetFromBase) const
{
    if(offsetFromBase+1 >= size) outOfBoundsReadHelper(offsetFromBase);
    quint8 temp = 0;
    bool retVal = getByte(temp, offsetFromBase);
    // No need to upsize temp first, since shift yields int32
    output = static_cast<quint16>(temp<<8);
    retVal &= getByte(temp, offsetFromBase+1);
    output |= temp;
    return retVal;
}

bool AMemoryChip::setWord(quint16 offsetFromBase, quint16 value)
{
    if(offsetFromBase+1 >= size) outOfBoundsWriteHelper(offsetFromBase, value);
    bool retVal = writeByte(offsetFromBase, value >> 8);
    retVal &= writeByte(offsetFromBase+1, value & 0xff);
    return retVal;
}

[[noreturn]] void AMemoryChip::outOfBoundsReadHelper(quint16 offsetFromBase) const
{
    std::string message = "Out of range memory read at: " +
            QString("0x%1").arg(offsetFromBase + baseAddress, 4, 16, QLatin1Char('0')).toStdString();
    throw std::out_of_range(message);
}

[[noreturn]] void AMemoryChip::outOfBoundsWriteHelper(quint16 offsetFromBase, quint8 value)
{
    QString format("Out of range memory write (value = 0x%1) at: 0x%2");
    std::string message = format.
            arg(value, 2, 16, QLatin1Char('0')).
            arg(offsetFromBase + baseAddress, 4, 16, QLatin1Char('0')).
            toStdString();
    throw std::out_of_range(message);
}

[[noreturn]] void AMemoryChip::outOfBoundsWriteHelper(quint16 offsetFromBase, quint16 value)
{
    QString format("Out of range memory write (value = 0x%1) at: 0x%2");
    std::string message = format.
            arg(value, 4, 16, QLatin1Char('0')).
            arg(offsetFromBase + baseAddress, 4, 16, QLatin1Char('0')).
            toStdString();
    throw std::out_of_range(message);
}
