// File: memorychips.cpp.h
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
#include <QApplication>

#include "memorychips.h"

ConstChip::ConstChip(quint32 size, quint16 baseAddress, QObject *parent):
    AMemoryChip (size, baseAddress, parent)
{

}

ConstChip::~ConstChip()
{

}

AMemoryChip::IOFunctions ConstChip::getIOFunctions() const noexcept
{
    return AMemoryChip::NONE;
}

AMemoryChip::ChipTypes ConstChip::getChipType() const noexcept
{
    return AMemoryChip::ChipTypes::CONST;
}

void ConstChip::clear() noexcept
{
    // A chip that can't change has nothing to clear
}

bool ConstChip::readByte(quint16 offsetFromBase, quint8& output) const
{
    // If the read would be out of bounds, throw an error.
    if(offsetFromBase >= size) outOfBoundsReadHelper(offsetFromBase);
    output = 0;
    return true;
}

bool ConstChip::writeByte(quint16 offsetFromBase, quint8 value)
{
    // If the write would be out of bounds, throw an error.
    if(offsetFromBase >= size) outOfBoundsWriteHelper(offsetFromBase, value);
    // Constant device's state cannot be changed from 0.
    return true;
}

bool ConstChip::getByte(quint16 offsetFromBase, quint8 &output) const
{
    // If the get would be out of bounds, throw an error.
    if(offsetFromBase >= size) outOfBoundsReadHelper(offsetFromBase);
    output = 0;
    return true;
}

bool ConstChip::setByte(quint16 offsetFromBase, quint8 value)
{
    // If the set would be out of bounds, throw an error.
    if(offsetFromBase >= size) outOfBoundsWriteHelper(offsetFromBase, value);
    // Zero device's state cannot be changed.
    return true;
}



NilChip::NilChip(quint32 size, quint16 baseAddress, QObject *parent): AMemoryChip (size, baseAddress, parent)
{

}

NilChip::~NilChip()
{

}

AMemoryChip::IOFunctions NilChip::getIOFunctions() const noexcept
{
    return IOFunctions::NONE;
}

AMemoryChip::ChipTypes NilChip::getChipType() const noexcept
{
    return ChipTypes::NIL;
}

void NilChip::clear() noexcept
{
    // A "disabled" chip can't be cleared
}

bool NilChip::isCachable() const noexcept
{
    return false;
}

bool NilChip::readByte(quint16 offsetFromBase, quint8 &output) const
{
    return getByte(offsetFromBase, output);
}

bool NilChip::writeByte(quint16 offsetFromBase, quint8 value)
{
    return setByte(offsetFromBase, value);
}

bool NilChip::getByte(quint16 offsetFromBase, quint8 &/*output*/) const
{
    std::string message = "Attempted to read from memory not installed in computer at: " +
            QString("0x%1.").arg(offsetFromBase + baseAddress, 4, 16, QLatin1Char('0')).toStdString();
    throw bad_chip_write(message);
}

bool NilChip::setByte(quint16 offsetFromBase, quint8)
{
    std::string message = "Attempted to write to memory not installed in computer at: " +
            QString("0x%1.").arg(offsetFromBase + baseAddress, 4, 16, QLatin1Char('0')).toStdString();
    throw bad_chip_write(message);
}



InputChip::InputChip(quint32 size, quint16 baseAddress, QObject *parent):
    AMemoryChip (size, baseAddress, parent), memory(QVector<quint8>(static_cast<qint32>(size), 0)),
    waiting(QVector<bool>(static_cast<qint32>(size), false)), requestCanceled(QVector<bool>(static_cast<qint32>(size), false)),
    requestAborted(QVector<bool>(static_cast<qint32>(size), false))
{

}

InputChip::~InputChip()
{

}

void InputChip::resize(quint32 newSize) noexcept
{
    this->size = newSize;
    memory.resize(static_cast<qint32>(size));
    waiting.resize(static_cast<qint32>(size));
    requestCanceled.resize(static_cast<qint32>(size));
    requestAborted.resize(static_cast<qint32>(size));
    clear(); // Reset all values to false / 0.
}

AMemoryChip::IOFunctions InputChip::getIOFunctions() const noexcept
{
    return static_cast<AMemoryChip::IOFunctions>(  static_cast<int>(AMemoryChip::READ)
                                                 | static_cast<int>(AMemoryChip::MEMORY_MAPPED));
}

AMemoryChip::ChipTypes InputChip::getChipType() const noexcept
{
    return AMemoryChip::ChipTypes::IDEV;
}

void InputChip::clear() noexcept
{
    for (quint32 it = 0; it < size; it++) {
        memory[static_cast<qint32>(it)] = 0;
        waiting[static_cast<qint32>(it)] = false;
        requestCanceled[static_cast<qint32>(it)] = false;
        requestAborted[static_cast<qint32>(it)] = false;
    }
}

bool InputChip::isCachable() const noexcept
{
    return false;
}

bool InputChip::readByte(quint16 offsetFromBase, quint8 &output) const
{
    // If the read would be out of bounds, throw an error.
    if(offsetFromBase >= size) outOfBoundsReadHelper(offsetFromBase);   
    waiting[offsetFromBase] = true;
    requestCanceled[offsetFromBase] = false;
    requestAborted[offsetFromBase] = false;
    emit inputRequested(baseAddress + offsetFromBase);
    // Let the UI handle I/O before returning to this device
    QApplication::processEvents();
    if(requestCanceled[offsetFromBase]) return false;
    else if(requestAborted[offsetFromBase]) {
        memory[offsetFromBase] = errorChar;
    }
    output = memory[offsetFromBase];
    return true;
}

bool InputChip::writeByte(quint16 offsetFromBase, quint8 value)
{
    // If the set would be out of bounds, throw an error.
    if(offsetFromBase >= size) outOfBoundsWriteHelper(offsetFromBase, value);
    memory[offsetFromBase] = value;
    return true;
}

bool InputChip::getByte(quint16 offsetFromBase, quint8 &output) const
{
    // If the get would be out of bounds, throw an error.
    if(offsetFromBase >= size) outOfBoundsReadHelper(offsetFromBase);
    output = memory[offsetFromBase];
    return true;
}

bool InputChip::setByte(quint16 offsetFromBase, quint8 value)
{
    // If the set would be out of bounds, throw an error.
    if(offsetFromBase >= size) outOfBoundsWriteHelper(offsetFromBase, value);
    memory[offsetFromBase] = value;
    return true;
}

bool InputChip::waitingForInput(quint16 offsetFromBase) const
{
    return waiting[offsetFromBase];
}

void InputChip::onInputReceived(quint16 offsetFromBase, quint8 value)
{
    if(offsetFromBase >= size) outOfBoundsWriteHelper(offsetFromBase, value);
    memory[offsetFromBase] = value;
    waiting[offsetFromBase] = false;
}

void InputChip::onInputCanceled(quint16 offsetFromBase)
{
    waiting[offsetFromBase] = false;
    requestCanceled[offsetFromBase] = true;
}

void InputChip::onInputAborted(quint16 offsetFromBase)
{
    waiting[offsetFromBase] = false;
    requestAborted[offsetFromBase] = true;
}


OutputChip::OutputChip(quint32 size, quint16 baseAddress, QObject *parent): AMemoryChip (size, baseAddress, parent),
    memory(QVector<quint8>(static_cast<qint32>(size), 0))
{

}

OutputChip::~OutputChip()
{

}

void OutputChip::resize(quint32 newSize) noexcept
{
    this->size = newSize;
    memory.resize(static_cast<qint32>(size));
    clear(); // Reset all values to false / 0.
}

AMemoryChip::IOFunctions OutputChip::getIOFunctions() const noexcept
{
        return static_cast<AMemoryChip::IOFunctions>(  static_cast<int>(AMemoryChip::READ)
                                                     | static_cast<int>(AMemoryChip::WRITE)
                                                     | static_cast<int>(AMemoryChip::MEMORY_MAPPED));
}

AMemoryChip::ChipTypes OutputChip::getChipType() const noexcept
{
    return ChipTypes::ODEV;
}

void OutputChip::clear() noexcept
{
    for (quint32 it = 0; it < size; it++) {
        memory[static_cast<qint32>(it)] = 0;
    }
}

bool OutputChip::isCachable() const noexcept
{
    return false;
}

bool OutputChip::readByte(quint16 offsetFromBase, quint8 &output) const
{
    return getByte(offsetFromBase, output);
}

bool OutputChip::writeByte(quint16 offsetFromBase, quint8 value)
{
    // If the write would be out of bounds, throw an error.
    if(offsetFromBase >= size) outOfBoundsWriteHelper(offsetFromBase, value);
    memory[offsetFromBase] = value;
    // Generate an I/O event.
    emit this->outputGenerated(offsetFromBase + baseAddress, value);
    return true;
}

bool OutputChip::getByte(quint16 offsetFromBase, quint8 &output) const
{
    // If the get would be out of bounds, throw an error.
    if(offsetFromBase >= size) outOfBoundsReadHelper(offsetFromBase);
    output = memory[offsetFromBase];
    return true;
}

bool OutputChip::setByte(quint16 offsetFromBase, quint8 value)
{
    // If the set would be out of bounds, throw an error.
    if(offsetFromBase >= size) outOfBoundsWriteHelper(offsetFromBase, value);
    memory[offsetFromBase] = value;
    return true;
}

RAMChip::RAMChip(quint32 size, quint16 baseAddress, QObject *parent): AMemoryChip (size, baseAddress, parent),
    memory(QVector<quint8>(static_cast<qint32>(size), 0))
{

}

RAMChip::~RAMChip()
{

}

void RAMChip::resize(quint32 newSize) noexcept
{
    this->size = newSize;
    memory.resize(static_cast<qint32>(size));
    clear(); // Reset all values to false / 0.
}

AMemoryChip::IOFunctions RAMChip::getIOFunctions() const noexcept
{
    return static_cast<AMemoryChip::IOFunctions>(  static_cast<int>(AMemoryChip::READ)
                                                   | static_cast<int>(AMemoryChip::WRITE));
}

AMemoryChip::ChipTypes RAMChip::getChipType() const noexcept
{
    return AMemoryChip::ChipTypes::RAM;
}

void RAMChip::clear() noexcept
{
    for (quint32 it = 0; it < size; it++) {
        memory[static_cast<qint32>(it)] = 0;
    }
}

bool RAMChip::readByte(quint16 offsetFromBase, quint8 &output) const
{
    return getByte(offsetFromBase, output);
}

bool RAMChip::writeByte(quint16 offsetFromBase, quint8 value)
{
    return setByte(offsetFromBase, value);
}

bool RAMChip::getByte(quint16 offsetFromBase, quint8 &output) const
{
    // If the get would be out of bounds, throw an error.
    if(offsetFromBase >= size) outOfBoundsReadHelper(offsetFromBase);
    output = memory[offsetFromBase];
    return true;
}

bool RAMChip::setByte(quint16 offsetFromBase, quint8 value)
{
    // If the set would be out of bounds, throw an error.
    if(offsetFromBase >= size) outOfBoundsWriteHelper(offsetFromBase, value);
    memory[offsetFromBase] = value;
    return true;
}



ROMChip::ROMChip(quint32 size, quint16 baseAddress, QObject *parent): AMemoryChip (size, baseAddress, parent),
    memory(QVector<quint8>(static_cast<qint32>(size), 0))
{

}

ROMChip::~ROMChip()
{

}

void ROMChip::resize(quint32 newSize) noexcept
{
    this->size = newSize;
    memory.resize(static_cast<qint32>(size));
    clear(); // Reset all values to false / 0.
}

AMemoryChip::IOFunctions ROMChip::getIOFunctions() const noexcept
{
    return static_cast<AMemoryChip::IOFunctions>(static_cast<int>(AMemoryChip::READ));
}

AMemoryChip::ChipTypes ROMChip::getChipType() const noexcept
{
    return AMemoryChip::ChipTypes::RAM;
}

void ROMChip::clear() noexcept
{
    for (quint32 it = 0; it < size; it++) {
        memory[static_cast<qint32>(it)] = 0;
    }
}

bool ROMChip::readByte(quint16 offsetFromBase, quint8 &output) const
{
    return getByte(offsetFromBase, output);
}

bool ROMChip::writeByte(quint16 /*offsetFromBase*/, quint8)
{
    // Don't allow users to change (write to) read only memory.
    return true;
    /*QString format("Attempted to write to read only memory at: 0x%1.");
    std::string message = format.
            arg(offsetFromBase + baseAddress, 4, 16, QLatin1Char('0')).
            toStdString();
    throw bad_chip_write(message);*/
}

bool ROMChip::getByte(quint16 offsetFromBase, quint8 &output) const
{
    // If the get would be out of bounds, throw an error.
    if(offsetFromBase >= size) outOfBoundsReadHelper(offsetFromBase);
    output = memory[offsetFromBase];
    return true;
}

bool ROMChip::setByte(quint16 offsetFromBase, quint8 value)
{
    // If the set would be out of bounds, throw an error.
    if(offsetFromBase >= size) outOfBoundsWriteHelper(offsetFromBase, value);
    memory[offsetFromBase] = value;
    return true;
}

