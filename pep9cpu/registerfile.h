#ifndef REGISTERFILE_H
#define REGISTERFILE_H
#include <QtCore>
#include "enu.h"

class RegisterFile
{
    QVector<quint8> registersStart, registersCurrent;
    quint8 statusBitsStart, statusBitsCurrent, irCache;
public:
    RegisterFile();

    // While status bits are not in the register bank, the must also be preserved from
    // the start to the end of a cycle
    quint8 readStatusBitsStart() const;
    quint8 readStatusBitsCurrent() const;
    bool readStatusBitCurrent(Enu::EStatusBit bit);
    bool readStatusBitStart(Enu::EStatusBit bit);
    void writeStatusBit(Enu::EStatusBit bit, bool val);

    void clearStatusBits();

    // Handle typical CPU registers
    inline quint8 convertRegister(Enu::CPURegisters reg) const;
    quint16 readRegisterWordCurrent(quint8 reg) const;
    quint16 readRegisterWordCurrent(Enu::CPURegisters reg) const;
    quint16 readRegisterWordStart(quint8 reg) const;
    quint16 readRegisterWordStart(Enu::CPURegisters reg) const;

    quint8 readRegisterByteCurrent(quint8 reg) const;
    quint8 readRegisterByteCurrent(Enu::CPURegisters reg) const;
    quint8 readRegisterByteStart(quint8 reg) const;
    quint8 readRegisterByteStart(Enu::CPURegisters reg) const;

    void writeRegisterWord(quint8 reg, quint16 val);
    void writeRegisterWord(Enu::CPURegisters reg, quint16 val);
    void writeRegisterByte(quint8 reg, quint8 val);
    void writeRegisterByte(Enu::CPURegisters reg, quint8 val);

    void clearRegisters();
    // Modifies the starting value of the program counter. Needed for correct highlighting.
    void writePCStart(quint16 val);
    // Since the value in the IR isn't correct at the start of a cycle,
    // implementations might choose to predict the correct value and cache it.
    // This prevents display bugs if an instruction modifies the memory address from
    // which it was fetched.
    void setIRCache(quint8 val);
    quint8 getIRCache() const;

    void flattenFile();

private:
    bool crackStatusBit(quint8 statusBits, Enu::EStatusBit bit);
};

#endif // REGISTERFILE_H
