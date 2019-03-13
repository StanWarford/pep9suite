// File: amemorychip.cpp
/*
    The Pep/9 suite of applications (Pep9, Pep9CPU, Pep9Micro) are
    simulators for the Pep/9 virtual machine, and allow users to
    create, simulate, and debug across various levels of abstraction.

    Copyright (C) 2018 J. Stanley Warford & Matthew McRaven, Pepperdine University

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "amemorychip.h"

AMemoryChip::AMemoryChip(quint32 size, quint16 baseAddress, QObject *parent) noexcept: QObject(parent), size(size), baseAddress(baseAddress)
{

}

AMemoryChip::~AMemoryChip()
{

}

void AMemoryChip::setBaseAddress(quint16 newAddress)
{
    baseAddress = newAddress;
}

quint32 AMemoryChip::getSize() const noexcept
{
    return size;
}

quint16 AMemoryChip::getBaseAddress() const noexcept
{
    return baseAddress;
}

bool AMemoryChip::readWord(quint16 offsetFromBase, quint16& output) const
{
    if(static_cast<quint32>(offsetFromBase+1) >= size) outOfBoundsReadHelper(offsetFromBase);
    quint8 temp = 0;
    bool retVal = readByte(offsetFromBase, temp);
    // No need to upsize temp first, since shift yields int32
    output = static_cast<quint16>(temp<<8);
    retVal &= readByte(offsetFromBase+1, temp);
    output |= temp;
    return retVal;

}

bool AMemoryChip::writeWord(quint16 offsetFromBase, quint16 value)
{
    if(static_cast<quint32>(offsetFromBase+1) >= size) outOfBoundsWriteHelper(offsetFromBase, value);
    bool retVal = writeByte(offsetFromBase, value >> 8);
    retVal &= writeByte(offsetFromBase+1, value & 0xff);
    return retVal;
}

bool AMemoryChip::getWord(quint16 offsetFromBase, quint16& output) const
{
    if(static_cast<quint32>(offsetFromBase + 1) >= size) outOfBoundsReadHelper(offsetFromBase);
    quint8 temp = 0;
    bool retVal = getByte(offsetFromBase, temp);
    // No need to upsize temp first, since shift yields int32
    output = static_cast<quint16>(temp<<8);
    retVal &= getByte(offsetFromBase + 1, temp);
    output |= temp;
    return retVal;
}

bool AMemoryChip::setWord(quint16 offsetFromBase, quint16 value)
{
    if(static_cast<quint32>(offsetFromBase+1) >= size) outOfBoundsWriteHelper(offsetFromBase, value);
    bool retVal = writeByte(offsetFromBase, value >> 8);
    retVal &= writeByte(offsetFromBase+1, value & 0xff);
    return retVal;
}

[[noreturn]] void AMemoryChip::outOfBoundsReadHelper(quint16 offsetFromBase) const
{
    std::string message = "Out of range memory read at: " +
            QString("0x%1.").arg(offsetFromBase + baseAddress, 4, 16, QLatin1Char('0')).toStdString();
    throw std::out_of_range(message);
}

[[noreturn]] void AMemoryChip::outOfBoundsWriteHelper(quint16 offsetFromBase, quint8 value)
{
    QString format("Out of range memory write (value = 0x%1) at: 0x%2.");
    std::string message = format.
            arg(value, 2, 16, QLatin1Char('0')).
            arg(offsetFromBase + baseAddress, 4, 16, QLatin1Char('0')).
            toStdString();
    throw std::out_of_range(message);
}

[[noreturn]] void AMemoryChip::outOfBoundsWriteHelper(quint16 offsetFromBase, quint16 value)
{
    QString format("Out of range memory write (value = 0x%1) at: 0x%2.");
    std::string message = format.
            arg(value, 4, 16, QLatin1Char('0')).
            arg(offsetFromBase + baseAddress, 4, 16, QLatin1Char('0')).
            toStdString();
    throw std::out_of_range(message);
}

bad_chip_write::~bad_chip_write()
{

}

io_aborted::~io_aborted()
{

}
