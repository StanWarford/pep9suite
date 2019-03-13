// File: amemorydevice.cpp
/*
    The Pep/9 suite of applications (Pep9, Pep9CPU, Pep9Micro) are
    simulators for the Pep/9 virtual machine, and allow users to
    create, simulate, and debug across various levels of abstraction.

    Copyright (C) 2018  J. Stanley Warford & Matthew McRaven, Pepperdine University

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

#include "amemorydevice.h"

AMemoryDevice::AMemoryDevice(QObject *parent) noexcept: QObject(parent), bytesWritten(), bytesSet(),
    errorMessage(""), error(false)
{

}

bool AMemoryDevice::hadError() const noexcept
{
    return error;
}

QString AMemoryDevice::getErrorMessage() const noexcept
{
    return errorMessage;
}

void AMemoryDevice::clearErrors()
{
    error = false;
    errorMessage = "";
}

const QSet<quint16> AMemoryDevice::getBytesWritten() const noexcept
{
    return bytesWritten;
}

const QSet<quint16> AMemoryDevice::getBytesSet() const noexcept
{
    return bytesSet;
}

void AMemoryDevice::clearBytesWritten() noexcept
{
    bytesWritten.clear();
}

void AMemoryDevice::clearBytesSet() noexcept
{
    bytesSet.clear();
}

bool AMemoryDevice::readWord(quint16 offsetFromBase, quint16 &output) const
{
    quint8 temp = 0;
    bool retVal = readByte(offsetFromBase, temp);
    // Store the high order byte in output, but must first shift it over
    output = static_cast<quint16>(quint16{temp} << 8);
    // Read is succesful if the first and second bytes are fetched succesfully
    retVal &= readByte(offsetFromBase+1, temp);
    // Store the low order byte in output
    output |= temp;
    return retVal;
}

bool AMemoryDevice::writeWord(quint16 offsetFromBase, quint16 value)
{
    bool retVal = writeByte(offsetFromBase, value >> 8);
    retVal &= writeByte(offsetFromBase+1, value & 0xff);
    return retVal;
}

bool AMemoryDevice::getWord(quint16 offsetFromBase, quint16 &output) const
{
    quint8 temp = 0;
    bool retVal = getByte(offsetFromBase, temp);
    // Store the high order byte in output, but must first shift it over
    output = static_cast<quint16>(quint16{temp} << 8);
    // Get is succesful if the first and second bytes are fetched succesfully
    retVal &= getByte(offsetFromBase+1, temp);
    // Store the low order byte in output
    output |= temp;
    return retVal;
}

bool AMemoryDevice::setWord(quint16 offsetFromBase, quint16 value)
{
    bool retVal = setByte(offsetFromBase, value >> 8);
    retVal &= setByte(offsetFromBase+1, value & 0xff);
    return retVal;
}
