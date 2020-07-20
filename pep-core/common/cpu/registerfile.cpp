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
#include "registerfile.h"

RegisterFile::RegisterFile(quint8 max_registers, quint8 max_status_bits):
    max_register_number(max_registers), max_status_bit_number(max_status_bits),
    registersStart(max_registers + 1, 0),
    registersCurrent(max_registers + 1, 0),
    statusBitsStart(max_status_bits + 1, false),
    statusBitsCurrent(max_status_bits + 1, false), irCache(0)
{

}

bool RegisterFile::readStatusBitCurrent(PepCore::CPUStatusBits_name_t bit) const
{
    if(bit < max_status_bit_number) {
        return statusBitsCurrent[bit];
    }
    else {
        throw  std::invalid_argument("Status bit index not in range");
    }
}

bool RegisterFile::readStatusBitStart(PepCore::CPUStatusBits_name_t bit) const
{
    if(bit < max_status_bit_number) {
        return statusBitsStart[bit];
    }
    else {
        throw  std::invalid_argument("Status bit index not in range");
    }
}

void RegisterFile::writeStatusBit(PepCore::CPUStatusBits_name_t bit, bool val)
{
    if(bit < statusBitsCurrent.size()) {
        statusBitsCurrent[bit] = val;
    }
    else {
        throw  std::invalid_argument("Status bit index not in range");
    }
}



void RegisterFile::clearStatusBits()
{
    for(size_t it = 0; it < statusBitsStart.size(); it++) {
        statusBitsStart[it] = false;
        statusBitsCurrent[it] = false;
    }
}

quint16 RegisterFile::readRegisterWordCurrent(PepCore::CPURegisters_number_t reg) const
{
    if(reg + 1 <= max_register_number) return static_cast<quint16>(registersCurrent[reg] << 8 | registersCurrent[reg + 1]);
    else return 0;
}

quint16 RegisterFile::readRegisterWordStart(PepCore::CPURegisters_number_t reg) const
{
    if(reg + 1 <= max_register_number) return static_cast<quint16>(registersStart[reg] << 8 | registersStart[reg + 1]);
    else return 0;
}

quint8 RegisterFile::readRegisterByteCurrent(PepCore::CPURegisters_number_t reg) const
{
    if(reg <= max_register_number) return registersCurrent[reg];
    else return 0;
}

quint8 RegisterFile::readRegisterByteStart(PepCore::CPURegisters_number_t reg) const
{
    if(reg <= max_register_number) return registersStart[reg];
    else return 0;
}

void RegisterFile::writeRegisterWord(quint8 reg, quint16 val)
{
    if(reg + 1 <= max_register_number) {
        registersCurrent[reg] = static_cast<quint8>(val >> 8);
        registersCurrent[reg + 1] = static_cast<quint8>(val & 0xff);
    }
}


void RegisterFile::writeRegisterByte(PepCore::CPURegisters_number_t reg, quint8 val)
{
    if(reg <= max_register_number) {
        registersCurrent[reg] = val;
    }
}

void RegisterFile::clearRegisters()
{
    for(size_t it = 0; it<max_register_number; it++) {
        registersStart[it] = 0;
        registersCurrent[it] = 0;
    }
}

void RegisterFile::overwriteRegisterWordStart(quint8 reg, quint16 val)
{
    if(reg + 1 <= max_register_number) {
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
    /*
     * Compilers might complain about this, but this can be optimized more easily to use SIMD
     * or vector instructions, hopefully making the cost of copying registers neglibible.
     *
     * Since the arrays are declared to be the same size using std::array, no need
     * to verify that both arrays are indeed the same length at runtime.
     */
    memcpy(registersStart.data(), registersCurrent.data(), static_cast<std::size_t>(registersStart.size()));
    for(size_t it=0; it<statusBitsCurrent.size(); it++) {
        statusBitsStart[it] = statusBitsCurrent[it];
    }
}
