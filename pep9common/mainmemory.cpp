// File: mainmemory.cpp
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
#include <QDebug>

#include "amemorychip.h"
#include "memorychips.h"
#include "mainmemory.h"

MainMemory::MainMemory(QObject* parent) noexcept: AMemoryDevice (parent), updateMemMap(true),
    endChip(new NilChip(0xffff, 0, this)), addressToChipLookupTable(1 << 16)
{

}

MainMemory::~MainMemory()
{

}

quint32 MainMemory::size() const noexcept
{
    // Main memory is not sparse, so the size of main memory is
    // the highest address seen (which is the address of the chip
    // plus the size.
    quint32 maxAddrSeen = 0;
    for(auto it : memoryChipMap) {
        if(maxAddrSeen > it->getSize() + it->getBaseAddress()) continue;
        else {
            maxAddrSeen = it->getSize() + it->getBaseAddress();
        }
    }
    return maxAddrSeen;
}

void MainMemory::insertChip(QSharedPointer<AMemoryChip> chip, quint16 address)
{
    memoryChipMap.insert(address, chip);
    ptrLookup.insert(chip.get(), chip);
    if(chip->getChipType() == AMemoryChip::ChipTypes::IDEV) {
        connect(static_cast<InputChip*>(chip.get()), &InputChip::inputRequested, this,  &MainMemory::onChipInputRequested);
    }
    else if(chip->getChipType() == AMemoryChip::ChipTypes::ODEV) {
        connect(static_cast<OutputChip*>(chip.get()), &OutputChip::outputGenerated, this,  &MainMemory::onChipOutputWritten);
    }
    if(updateMemMap) calculateAddressToChip();
}

QSharedPointer<AMemoryChip> MainMemory::chipAt(quint16 address) noexcept
{
    // If there exists a chip in the lookup map, return it.
    if(addressToChipLookupTable[address] != nullptr) {
        return ptrLookup[addressToChipLookupTable[address]];
    }
    // Otherwise return a nullptr.
    return QSharedPointer<AMemoryChip>();
}

QSharedPointer<const AMemoryChip> MainMemory::chipAt(quint16 address) const noexcept
{
    // If there exists a chip in the lookup map, return it.
    if(addressToChipLookupTable[address] != nullptr) {
        return ptrLookup[addressToChipLookupTable[address]];
    }
    // Otherwise return a nullptr.
    return QSharedPointer<AMemoryChip>();
}

QSharedPointer<AMemoryChip> MainMemory::removeChip(quint16 address)
{
    QSharedPointer<AMemoryChip> retVal = chipAt(address);
    if(retVal == endChip) return QSharedPointer<AMemoryChip>(nullptr);
    if(updateMemMap) calculateAddressToChip();
    addressToChipLookupTable.remove(retVal->getBaseAddress());
    ptrLookup.remove(retVal.get());
    return retVal;
}

QVector<QSharedPointer<AMemoryChip> > MainMemory::removeAllChips()
{
    auto temp = memoryChipMap.values();
    QVector<QSharedPointer<AMemoryChip> > retVal;
    for(auto it : temp) {
        retVal.append(it);
        if(it->getChipType() == AMemoryChip::ChipTypes::IDEV) {
            disconnect(static_cast<InputChip*>(it.get()), &InputChip::inputRequested, this,  &MainMemory::onChipInputRequested);
        }
        else if(it->getChipType() == AMemoryChip::ChipTypes::ODEV) {
            disconnect(static_cast<OutputChip*>(it.get()), &OutputChip::outputGenerated, this,  &MainMemory::onChipOutputWritten);
        }
    }
    memoryChipMap.clear();
    ptrLookup.clear();
    if(updateMemMap) calculateAddressToChip();
    return retVal;
}

void MainMemory::autoUpdateMemoryMap(bool update) noexcept
{
    updateMemMap = update;
    // If updates are re-enabled, refresh the address lookup table.
    if(updateMemMap) calculateAddressToChip();
}

void MainMemory::loadValues(quint16 address, QVector<quint8> values) noexcept
{
    for(quint16 idx = 0;idx < values.length()
        && idx + address <= static_cast<int>(size()); idx++) {
        bytesSet.insert(idx + address);
        setByte(idx + address, values[idx]);
    }
}

void MainMemory::clearMemory()
{
    // Inform each chip that it needs to be zero'ed out.
    for(auto chip : memoryChipMap) {
        chip->clear();
    }
    // Cleared memory has no written or set bytes.
    bytesSet.clear();
    bytesWritten.clear();
    // Remove pending error messages and pending IO.
    clearErrors();
    clearIO();
}

void MainMemory::onCycleStarted()
{
    // Main memory doesn't have any per-cycle internal updates.
}

void MainMemory::onCycleFinished()
{
    // Main memory doesn't have any per-cycle internal updates.
}

bool MainMemory::readByte(quint16 address, quint8 &output) const
{
    QSharedPointer<const AMemoryChip> chip = chipAt(address);
    // Since IO can fail, wrap it in a try-catch.
    try {
        bool retVal = chip->readByte(address - chip->getBaseAddress(), output);
        return retVal;
    }
    // Did the memory access fall out of range?
    catch (std::range_error &e) {
        throw e;
    }
    // Input was requested, but it was aborted in an error-inducing way.
    catch(io_aborted &e) {
        this->error = true;
        this->errorMessage = e.what();
        return false;
    }

}

bool MainMemory::writeByte(quint16 address, quint8 value)
{
    QSharedPointer<AMemoryChip> chip = chipAt(address);
    try {
        bool retVal = chip->writeByte(address - chip->getBaseAddress(), value);
        bytesWritten.insert(address);
        emit changed(address, value);
        return retVal;
    } catch (std::range_error& e) {
        error = true;
        errorMessage = e.what();
        return false;
    } catch (bad_chip_write& e){
        error = true;
        errorMessage = e.what();
        return false;
    }
}

bool MainMemory::getByte(quint16 address, quint8 &output) const
{
    QSharedPointer<const AMemoryChip> chip = chipAt(address);
    try {
        bool retVal = chip->getByte(address - chip->getBaseAddress(), output);
        return retVal;
    } catch (std::range_error& e) {
        throw e;
    } catch (bad_chip_write& e){
        throw e;
    }
}

bool MainMemory::setByte(quint16 address, quint8 value)
{
    QSharedPointer<AMemoryChip> chip = chipAt(address);
    try {
        bool retVal = chip->setByte(address - chip->getBaseAddress(), value);
        bytesSet.insert(address);
        emit changed(address, value);
        return retVal;
    } catch (std::range_error& e) {
        error = true;
        errorMessage = e.what();
        return false;
    } catch (bad_chip_write& e){
        error = true;
        errorMessage = e.what();
        return false;
    }
}

void MainMemory::clearIO()
{
    for(auto key : inputBuffer.keys()) {
        // Only pay attention to bufferedValues if they are input devices.
        if(chipAt(key)->getChipType() != AMemoryChip::ChipTypes::IDEV) continue;
        InputChip* in = dynamic_cast<InputChip*>(chipAt(key).get());
        quint16 offsetFromBase = key - in->getBaseAddress();
        if(in->waitingForInput(offsetFromBase)) {
            in->onInputCanceled(key);
        }
    }
    inputBuffer.clear();
}

void MainMemory::onInputReceived(quint16 address, quint8 input)
{
    onInputReceived(address, QString(input));
}

void MainMemory::onInputReceived(quint16 address, QChar input)
{
    onInputReceived(address, QString(input));
}

void MainMemory::onInputReceived(quint16 address, QString input)
{
    QSharedPointer<AMemoryChip> temp = chipAt(address);
    InputChip* chip;
    if(temp->getChipType() != AMemoryChip::ChipTypes::IDEV) {
        throw std::invalid_argument("Expected address of an InputChip, given address of other type");
    }
    else {
        chip = dynamic_cast<InputChip*>(temp.get());
    }
    quint16 offsetFromBase = address - chip->getBaseAddress();
    if(chip->waitingForInput(offsetFromBase)) {
        quint8 first = static_cast<quint8>(input.front().toLatin1());
        QByteArray rest = input.mid(1,-1).toLatin1();
        inputBuffer.insert(address, rest);
        chip->onInputReceived(offsetFromBase, first);
    } else if(inputBuffer.contains(address)) {
        inputBuffer[address].append(input.toLatin1());
    } else {
        inputBuffer.insert(address, input.toLatin1());
    }
}

void MainMemory::onInputCanceled(quint16 address)
{
    QSharedPointer<AMemoryChip> chip = chipAt(address);
    if(chip->getChipType() != AMemoryChip::ChipTypes::IDEV) {
        throw std::invalid_argument("Expected address of an InputChip, given address of other type.");
    }
    else {
        dynamic_cast<InputChip*>(chip.get())->onInputCanceled(address - chip->getBaseAddress());
    }
}

void MainMemory::onInputAborted(quint16 address)
{
    QSharedPointer<AMemoryChip> chip = chipAt(address);
    if(chip->getChipType() != AMemoryChip::ChipTypes::IDEV) {
        throw std::invalid_argument("Expected address of an InputChip, given address of other type.");
    }
    else {
        dynamic_cast<InputChip*>(chip.get())->onInputAborted(address - chip->getBaseAddress());
    }
}

void MainMemory::onChipInputRequested(quint16 address)
{
    if(inputBuffer.contains(address)) {
        quint8 first = inputBuffer[address].front();
        quint16 offsetFromBase = address - chipAt(address)->getBaseAddress();
        inputBuffer[address].remove(0, 1);
        if(inputBuffer[address].length() == 0) {
            inputBuffer.remove(address);
        }
        dynamic_cast<InputChip*>(chipAt(address).get())->onInputReceived(offsetFromBase, first);
    }
    else {
        emit inputRequested(address);
        // Make sure the signal is handled by the UI immediately
        QApplication::processEvents();
    }
}

void MainMemory::onChipOutputWritten(quint16 address, quint8 value)
{
    emit outputWritten(address, value);
}

void MainMemory::calculateAddressToChip() noexcept
{
    for (int it = 0; it <= 0xffff; it++) {
        addressToChipLookupTable[it] = nullptr;
    }
    for(auto chip : memoryChipMap) {
        for(quint32 it = chip->getBaseAddress(); it < chip->getBaseAddress() + chip->getSize(); it++) {
            addressToChipLookupTable[static_cast<int>(it)] = chip.get();
        }
    }
}
