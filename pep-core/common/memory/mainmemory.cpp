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
    endChip(new NilChip(0xffff, 0, this)), addressToChipLookupTable(1 << 16),
    maxAddr(0), inTransaction(false)
{

}

MainMemory::~MainMemory()
{

}

quint32 MainMemory::maxAddress() const noexcept
{
    // The size of main memory is equal to the value of highest address + 1
    // ( + 1 since addresses start at 0, not 1). The highest address in a chip
    // is the address of the chip plus its size.
    maxAddr = 0;
    for(auto it : memoryChipMap) {
        if(it == endChip) continue;
        if(maxAddr > it->getSize() + it->getBaseAddress()) continue;
        else {
            maxAddr = it->getSize() + it->getBaseAddress();
        }
    }
    maxAddr -= 1;
    // Account for addresses starting at 0, not 1.
    return maxAddr;
}

void MainMemory::insertChip(QSharedPointer<AMemoryChip> chip, quint16 address)
{
    memoryChipMap.insert(address, chip);
    ptrLookup.insert(chip.get(), chip);
    // Chip being passed in might be re-used from previous executions,
    // so make sure to explicitly reset its address to prevent mapping errors.
    chip->setBaseAddress(address);
    if(chip->getChipType() == AMemoryChip::ChipTypes::IDEV) {
        connect(static_cast<InputChip*>(chip.get()), &InputChip::inputRequested, this,  &MainMemory::onChipInputRequested);
    }
    else if(chip->getChipType() == AMemoryChip::ChipTypes::ODEV) {
        connect(static_cast<OutputChip*>(chip.get()), &OutputChip::outputGenerated, this,  &MainMemory::onChipOutputWritten);
    }
    if(updateMemMap) calculateAddressToChip();
}

AMemoryChip* MainMemory::chipAt(quint16 address) noexcept
{
    // If there exists a chip in the lookup map, return it.
    if(addressToChipLookupTable[address] != nullptr) {
        return addressToChipLookupTable[address];
    }
    return endChip.get();
}

const AMemoryChip* MainMemory::chipAt(quint16 address) const noexcept
{
    // If there exists a chip in the lookup map, return it.
    if(addressToChipLookupTable[address] != nullptr) {
        return addressToChipLookupTable[address];
    }
    return endChip.get();
}

void MainMemory::constructMemoryDevice(QList<MemoryChipSpec> specList)
{
    // Prevent interim memory map updates, as many chips will be inserted and removed.
    autoUpdateMemoryMap(false);
    // Cache all old memory chips and use them to construct new memory specification.
    auto chipCache = removeAllChips();
    for(auto spec : specList) {
        QSharedPointer<AMemoryChip> targetItem = nullptr;
        // Try to find a chip of the same type as requested by spec and assign it
        // to targetItem. If an item is found, remove it from the chip cache.
        for(int idx=0; idx < chipCache.size(); idx++) {
            if(chipCache[idx]->getChipType() == spec.type) {
                targetItem = chipCache[idx];
                chipCache.remove(idx);
                break;
            }
        }

        // No chip matching the type of spec was found, a chip needs to be allocated.
        if(targetItem == nullptr) {
            switch(spec.type) {
            case AMemoryChip::ChipTypes::RAM:
                targetItem = QSharedPointer<RAMChip>::create(spec.size, spec.startAddr, this);
                break;
            case AMemoryChip::ChipTypes::ROM:
                targetItem = QSharedPointer<ROMChip>::create(spec.size, spec.startAddr, this);
                break;
            case AMemoryChip::ChipTypes::IDEV:
                targetItem = QSharedPointer<InputChip>::create(spec.size, spec.startAddr, this);
                break;
            case AMemoryChip::ChipTypes::ODEV:
                targetItem = QSharedPointer<OutputChip>::create(spec.size, spec.startAddr, this);
                break;
            case AMemoryChip::ChipTypes::CONST:
                targetItem = QSharedPointer<ConstChip>::create(spec.size, spec.startAddr, this);
                break;
            default:
                //targetItem = endChip;
                return;
            }
        }

        // If spec size doesn't match targets actual size, then the chip must
        // be resized (shrunk or gorwn) to match the speciifcation. Failing
        // to do this could cause odd behaviors when changing the effective size
        // of memory by burning in a new OS.
        if(targetItem->getSize() != spec.size) {
            targetItem->resize(spec.size);
        }
        insertChip(targetItem, spec.startAddr);
    }
    // Create lookup tables with new chips
    autoUpdateMemoryMap(true);
    // All remaining chips in oldChipVec will automatically be deleted
    // when the last reference to them is destroyed.
}

QSharedPointer<AMemoryChip> MainMemory::removeChip(quint16 address)
{
    AMemoryChip* chip = chipAt(address);
    // If the user requested out internal "end chip" that blanks out unused
    // addresses, just return a nullptr.
    if(chip == endChip.get()) return QSharedPointer<AMemoryChip>(nullptr);
    // Remove chip from lookup tables.
    // addressToChipLookupTable.remove(retVal->getBaseAddress());
    auto retVal = ptrLookup[chip];
    ptrLookup.remove(chip);
    if(updateMemMap) calculateAddressToChip();
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
    if(temp.contains(endChip)) {
        auto isNilChip = [this](QSharedPointer<AMemoryChip> chip) {return chip == endChip;};
        // We already hold a pointer to the Nil/terminator chip.
        // However, we can't discard return value of remove_if.
        // Therefore, catch the return value and cast it into the void.
        auto _ = std::remove_if(temp.begin(),temp.begin(), isNilChip);
        (void)_;
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
    inTransaction = false;
}

void MainMemory::loadValues(quint16 address, QVector<quint8> values) noexcept
{
    // Block signals being omitted, as it was causing issues with large heap sizes.
    bool block = signalsBlocked();
    blockSignals(true);
    // For ever value in the values array that falls in range of the memory module.
    for(quint16 idx = 0;idx < values.length()
        && idx + address <= static_cast<qint32>(maxAddress()); idx++) {
        bytesSet.insert(idx + address);
        setByte(idx + address, values[idx]);
    }
    blockSignals(block);
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
    inTransaction = false;
}

void MainMemory::onCycleStarted()
{
    // Main memory doesn't have any per-cycle internal updates.
}

void MainMemory::onCycleFinished()
{
    // Main memory doesn't have any per-cycle internal updates.
}

void MainMemory::onInstructionFinished(quint8 /*instruction_spec*/)
{
    // Main memory provides no per-instruction statistics.
}

bool MainMemory::readByte(quint16 address, quint8 &output) const
{
    const AMemoryChip *chip = chipAt(address);
    // Since IO can fail, wrap it in a try-catch.
    try {
        bool retVal = chip->readByte(address - chip->getBaseAddress(), output);
        return retVal;
    }
    // Did the memory access fall out of range?
    catch (std::range_error &e) {
        this->error = true;
        this->errorMessage = e.what();
        return false;
    }
    // Input was requested, but it was aborted in an error-inducing way.
    catch(io_aborted &e) {
        this->error = true;
        this->errorMessage = e.what();
        return false;
    }
    // An invalid chip (such as the nil chip) was read from.
    catch (bad_chip_write& e){
        error = true;
        errorMessage = e.what();
        return false;
    }

}

bool MainMemory::writeByte(quint16 address, quint8 value)
{
    AMemoryChip *chip = chipAt(address);
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
    const AMemoryChip *chip = chipAt(address);
    try {
        bool retVal = chip->getByte(address - chip->getBaseAddress(), output);
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

bool MainMemory::setByte(quint16 address, quint8 value)
{
    AMemoryChip *chip = chipAt(address);
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
        InputChip* in = dynamic_cast<InputChip*>(chipAt(key));
        quint16 offsetFromBase = key - in->getBaseAddress();
        if(in->waitingForInput(offsetFromBase)) {
            in->onInputCanceled(key);
        }
    }
    for(auto address : waitingOnInput) {
        onInputCanceled(address);
    }
    inputBuffer.clear();
    waitingOnInput.clear();
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
    if(input.isEmpty()) return;
    AMemoryChip *temp = chipAt(address);
    InputChip *chip;
    if(temp->getChipType() != AMemoryChip::ChipTypes::IDEV) {
        throw std::invalid_argument("Expected address of an InputChip, given address of other type.");
    }
    else {
        chip = dynamic_cast<InputChip*>(temp);
    }
    quint16 offsetFromBase = address - chip->getBaseAddress();
    if(chip->waitingForInput(offsetFromBase)) {
        quint8 first = static_cast<quint8>(input.front().toLatin1());
        QByteArray rest = input.mid(1,-1).toLatin1();
        inputBuffer.insert(address, rest);
        chip->onInputReceived(offsetFromBase, first);
        // Now that the address has been served IO, it is not waiting anymore.
        waitingOnInput.remove(address);
    } else if(inputBuffer.contains(address)) {
        inputBuffer[address].append(input.toLatin1());
    } else {
        inputBuffer.insert(address, input.toLatin1());
    }
}

void MainMemory::onInputCanceled(quint16 address)
{
    AMemoryChip *chip = chipAt(address);
    if(chip->getChipType() != AMemoryChip::ChipTypes::IDEV) {
        throw std::invalid_argument("Expected address of an InputChip, given address of other type.");
    }
    else {
        dynamic_cast<InputChip*>(chip)->onInputCanceled(address - chip->getBaseAddress());
    }
}

void MainMemory::onInputAborted(quint16 address)
{
    AMemoryChip *chip = chipAt(address);
    if(chip->getChipType() != AMemoryChip::ChipTypes::IDEV) {
        throw std::invalid_argument("Expected address of an InputChip, given address of other type.");
    }
    else {
        dynamic_cast<InputChip*>(chip)->onInputAborted(address - chip->getBaseAddress());
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
        dynamic_cast<InputChip*>(chipAt(address))->onInputReceived(offsetFromBase, first);
    }
    else {
        waitingOnInput.insert(address);
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
    for (int it = 0; it < addressToChipLookupTable.size(); it++) {
        addressToChipLookupTable[it] = nullptr;
    }
    ptrLookup[endChip.get()] = endChip;
    for(auto chip : memoryChipMap) {
        ptrLookup[chip.get()] = chip;
        for(quint32 it = chip->getBaseAddress(); it < chip->getBaseAddress() + chip->getSize(); it++) {
            addressToChipLookupTable[static_cast<int>(it)] = chip.get();
        }
    }
    maxAddress();
}
