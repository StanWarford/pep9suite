// File: registerfile.h
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
#ifndef REGISTERFILE_H
#define REGISTERFILE_H

#include <vector>

#include <QtCore>

#include "pep/enu.h"
#include "pep/apepversion.h"
#include "pep/types.h"

/*
 * Provides access to all 32 CPU registers and 5 status bits with one backup history state.
 */
class RegisterFile
{
    const quint8 max_register_number, max_status_bit_number;
    // Use a fixed size array, so as to make memcpy safer. Profiled to be slightly slower than
    // QVector, but guarantee that both arrays will always have the same number of elements is
    // more valuable.
    std::vector<quint8> registersStart, registersCurrent;
    std::vector<bool> statusBitsStart, statusBitsCurrent;
    // Allocate storage for |'ed together bit flags, and provide a way to cahce the instruction register./
    quint8 irCache;
public:
    explicit RegisterFile(quint8 max_registers, quint8 max_status_bits);

    // While status bits are not in the register bank, the must also be preserved from
    // the start to the end of a cycle.

    // Querry a named status bit.
    bool readStatusBitCurrent(PepCore::CPUStatusBits_name_t bit) const;
    bool readStatusBitStart(PepCore::CPUStatusBits_name_t bit) const;
    // Set a named status bit.
    void writeStatusBit(PepCore::CPUStatusBits_name_t bit, bool val);

    // Set all status bits to 0 / false.
    void clearStatusBits();


    // Read a register word by number or name in either history state.
    quint16 readRegisterWordCurrent(PepCore::CPURegisters_number_t reg) const;
    quint16 readRegisterWordStart(PepCore::CPURegisters_number_t reg) const;

    // Read a register byte by number or name in either history state.
    quint8 readRegisterByteCurrent(PepCore::CPURegisters_number_t reg) const;
    quint8 readRegisterByteStart(PepCore::CPURegisters_number_t reg) const;

    // Write a new word to a register by number or name for the current state.
    void writeRegisterWord(quint8 reg, quint16 val);
    // Write a new byte to a register by number or name for the current state.
    void writeRegisterByte(quint8 reg, quint8 val);

    // Set the value of all CPU registers to 0.
    void clearRegisters();

    // Modifies the starting value of the register. Needed for correct PC highlighting.
    void overwriteRegisterWordStart(quint8 reg, quint16 val);

    // Since the value in the IR isn't correct at the start of a cycle,
    // implementations might choose to predict the correct value and cache it.
    // This prevents display bugs if an instruction modifies the memory address from
    // which it was fetched.
    void setIRCache(quint8 val);
    quint8 getIRCache() const;

    // Copy all current values to the starting values.
    void flattenFile();
};

#endif // REGISTERFILE_H
