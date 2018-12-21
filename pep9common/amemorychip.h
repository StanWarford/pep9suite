// File: amemorychip.h
/*
    Pep9CPU is a CPU simulator for executing microcode sequences to
    implement instructions in the instruction set of the Pep/9 computer.

    Copyright (C) 2018  Matthew McRaven, Pepperdine University

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

#ifndef AMEMORYCHIP_H
#define AMEMORYCHIP_H

#include <QObject>
#include <exception>

/*
 * Error thrown when a user attempts to write to readonly memory chips or
 * access a Nil chip.
 */
class bad_chip_operation: public std::runtime_error{
public:
    explicit bad_chip_operation(const std::string& what_arg) : runtime_error(what_arg){
    }
    explicit bad_chip_operation (const char* what_arg): runtime_error(what_arg){
    }
};

/*
 * AMemoryChip represents a memory chip containing a number of bytes that is located in main memory.
 *
 * Each chip is inserted at a memory location starting at baseAddress, and represents a
 * contiguous block of memory up to baseAddress+size-1.
 *
 * Each chip is capable of input, output, input-output, or nothing.
 * If an illegal operation is performed (e.g. writing to a ROM chip) a bad_chip_operation is thrown
 * If an out of bounds access occurs, a range_error is thrown
 *
 * To access a value at an address, calculate the offset of the target address from the base address,
 * and pass this offset to the target read/write function. An example:
 *
 * Suppose a chip of length 0x1000 is inserted at 0x8000. It will span address 0x8000-0x8fff.
 * To access the value at 0x8caf, calculate the offset from the base address (0x8caf-0x8000=0x0caf),
 * and pass that offset to any desire IO function.
 *
 */
class AMemoryChip : public QObject
{
    Q_OBJECT
public:

    // Various kinds of memory chips supplied default
    enum class ChipTypes {
        CONST = 0, // Memory device that is read/writable, but its contents remain 0
        NIL = 1, // Memory device that cannot be read/written.
        IDEV = 10, ODEV = 11, // Memory mapped IO devices.
        RAM = 20, ROM = 21 // Typical random access / read only memory.
    };

    // Kinds of IO allowed by the chip
    enum IOFunctions: int {
        NONE = 0, READ = 1<<0, WRITE = 1<<1, MEMORY_MAPPED = 1<<2
    };

    explicit AMemoryChip(quint16 size, quint16 baseAddress, QObject *parent = nullptr);
    virtual ~AMemoryChip();

    // Number of bytes contained by this chip
    quint16 getSize() const;
    // Location in main memory where this chip is inserted
    quint16 getBaseAddres() const;

    // Returns the abilities of the chip |'ed together.
    // e.g. check if a chip is writable, (chip->getIOFunctions() & IOFunctions::WRITE) == true
    virtual IOFunctions getIOFunctions() const = 0;
    virtual ChipTypes getChipType() const = 0;
    // Read / Write functions that may generate signals or trap for IO.
    virtual bool readByte(quint8& output, quint16 offsetFromBase) const = 0;
    virtual bool writeByte(quint16 offsetFromBase, quint8 value) = 0;
    // Read / Write of words as two read / write byte operations and bitmath
    virtual bool readWord(quint16& output, quint16 offsetFromBase) const;
    virtual bool writeWord(quint16 offsetFromBase, quint16 value);

    // Get / Set functions that are guarenteed to not generate signals or trap for IO.
    virtual bool getByte(quint8& output, quint16 offsetFromBase) const = 0;
    virtual bool setByte(quint16 offsetFromBase, quint8 value) = 0;
    // Get / Set of words as two get / set byte operations and bitmath
    virtual bool getWord(quint16& output, quint16 offsetFromBase) const;
    virtual bool setWord(quint16 offsetFromBase, quint16 value);

protected:
    quint16 size, baseAddress;
    //Helpers that throw a stylized error message for an out of bounds error message.
    [[noreturn]] void outOfBoundsReadHelper(quint16 offsetFromBase) const;
    [[noreturn]] void outOfBoundsWriteHelper(quint16 offsetFromBase, quint8 value);
    [[noreturn]] void outOfBoundsWriteHelper(quint16 offsetFromBase, quint16 value);
};

#endif // AMEMORYCHIP_H
