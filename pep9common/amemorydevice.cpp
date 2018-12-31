#include "amemorydevice.h"

AMemoryDevice::AMemoryDevice(QObject *parent) noexcept: QObject(parent), bytesWritten(), bytesSet(), error(false),
    errorMessage("")
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
