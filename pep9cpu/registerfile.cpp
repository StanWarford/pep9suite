#include "registerfile.h"

RegisterFile::RegisterFile(): registersStart(QVector<quint8>(32, 0)), registersCurrent(QVector<quint8>(32, 0)),
    statusBitsStart(0), statusBitsCurrent(0), irCache(0)
{

}

quint8 RegisterFile::readStatusBitsStart() const
{
    return statusBitsStart;
}

quint8 RegisterFile::readStatusBitsCurrent() const
{
    return statusBitsCurrent;
}

bool RegisterFile::readStatusBitCurrent(Enu::EStatusBit bit)
{
    return crackStatusBit(statusBitsCurrent, bit);
}

bool RegisterFile::readStatusBitStart(Enu::EStatusBit bit)
{
    return crackStatusBit(statusBitsStart, bit);
}

void RegisterFile::writeStatusBit(Enu::EStatusBit bit, bool val)
{
    int mask = 0;
    switch(bit)
    {
    case Enu::STATUS_N:
        mask = Enu::NMask;
        break;
    case Enu::STATUS_Z:
        mask = Enu::ZMask;
        break;
    case Enu::STATUS_V:
        mask = Enu::VMask;
        break;
    case Enu::STATUS_C:
        mask = Enu::CMask;
        break;
    case Enu::STATUS_S:
        mask = Enu::SMask;
        break;
    default:
        // Should never occur, but might happen if a bad status bit is passed
        return;
    }
    // Mask out the target bit, then or the new value in to the target position.
    // Explicitly narrow status bit calculation.
    statusBitsCurrent = static_cast<quint8>((statusBitsCurrent & ~mask) | ((val ? 1 : 0) * mask));
}

void RegisterFile::clearStatusBits()
{
   statusBitsStart = 0;
   statusBitsCurrent = 0;
}

quint8 RegisterFile::convertRegister(Enu::CPURegisters reg) const
{
    // The CPU register enum is defined such that the integer value of
    // each register in the enum is the location of the first byte of the reigster.
    return static_cast<quint8>(reg);
}

quint16 RegisterFile::readRegisterWordCurrent(quint8 reg) const
{
    if(reg + 1< Enu::maxRegisterNumber) return static_cast<quint16>(registersCurrent[reg] << 8 | registersCurrent[reg + 1]);
    else return 0;
}

quint16 RegisterFile::readRegisterWordCurrent(Enu::CPURegisters reg) const
{
    return readRegisterWordCurrent(convertRegister(reg));
}

quint16 RegisterFile::readRegisterWordStart(quint8 reg) const
{
    if(reg + 1< Enu::maxRegisterNumber) return static_cast<quint16>(registersStart[reg] << 8 | registersStart[reg + 1]);
    else return 0;
}

quint16 RegisterFile::readRegisterWordStart(Enu::CPURegisters reg) const
{
    return readRegisterWordStart(convertRegister(reg));
}

quint8 RegisterFile::readRegisterByteCurrent(quint8 reg) const
{
    if(reg <= Enu::maxRegisterNumber) return registersCurrent[reg];
    else return 0;
}

quint8 RegisterFile::readRegisterByteCurrent(Enu::CPURegisters reg) const
{
    return readRegisterByteCurrent(convertRegister(reg));
}

quint8 RegisterFile::readRegisterByteStart(quint8 reg) const
{
    if(reg <= Enu::maxRegisterNumber) return registersStart[reg];
    else return 0;
}

quint8 RegisterFile::readRegisterByteStart(Enu::CPURegisters reg) const
{
    return readRegisterByteStart(convertRegister(reg));
}

void RegisterFile::writeRegisterWord(quint8 reg, quint16 val)
{
    if(reg + 1 <= Enu::maxRegisterNumber) {
        registersCurrent[reg] = static_cast<quint8>(val >> 8);
        registersCurrent[reg + 1] = static_cast<quint8>(val & 0xff);
    }
}

void RegisterFile::writeRegisterWord(Enu::CPURegisters reg, quint16 val)
{
    writeRegisterWord(convertRegister(reg), val);
}

void RegisterFile::writeRegisterByte(quint8 reg, quint8 val)
{
    if(reg <= Enu::maxRegisterNumber) {
        registersCurrent[reg] = val;
    }
}

void RegisterFile::writeRegisterByte(Enu::CPURegisters reg, quint8 val)
{
    writeRegisterByte(convertRegister(reg), val);
}

void RegisterFile::clearRegisters()
{
    registersStart.fill(0);
    registersCurrent.fill(0);
}

void RegisterFile::writePCStart(quint16 val)
{
    quint8 reg = convertRegister(Enu::CPURegisters::PC);
    if(reg + 1 < Enu::maxRegisterNumber) {
        registersStart[reg] = static_cast<quint8>(val >> 8);
        registersStart[reg + 1] = static_cast<quint8>(val & 0xff);
    }
}

void RegisterFile::setIRCache(quint8 val)
{
    irCache = val;
}

quint8 RegisterFile::getIRCache() const
{
    return irCache;
}

void RegisterFile::flattenFile()
{
    if(registersStart.length() != registersCurrent.length()) {
        qErrnoWarning("Warning, register banks do not match in size");
        return;
    }
    // Compilers might complain about this, but this can be optimized more easily to use SIMD
    // or vector instructions, hopefully making the cost of copying registers neglibible
    memcpy(registersStart.data(), registersCurrent.data(), static_cast<std::size_t>(registersStart.length()));
    statusBitsStart = statusBitsCurrent;
}

bool RegisterFile::crackStatusBit(quint8 statusBits, Enu::EStatusBit bit)
{
    int mask = 0;
    switch(bit)
    {
    case Enu::STATUS_N:
        mask = Enu::NMask;
        break;
    case Enu::STATUS_Z:
        mask = Enu::ZMask;
        break;
    case Enu::STATUS_V:
        mask = Enu::VMask;
        break;
    case Enu::STATUS_C:
        mask = Enu::CMask;
        break;
    case Enu::STATUS_S:
        mask = Enu::SMask;
        break;
    default:
        // Should never occur, but might happen if a bad status bit is passed
        return false;
    }
    return statusBits & mask;
}
