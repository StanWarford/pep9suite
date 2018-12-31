#include "mainmemory.h"
#include "amemorychip.h"
#include "memorychips.h"
#include <QDebug>
MainMemory::MainMemory(QObject* parent) noexcept: AMemoryDevice (parent), updateMemMap(true),
    endChip(new NilChip(0xfff, 0, this)), addressToChip(1<<16)
{

}

MainMemory::~MainMemory()
{

}

quint32 MainMemory::size() const noexcept
{
    quint32 size = 0;
    for(auto it : memoryChipMap) {
        size += it->getSize();
    }
    return size;
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
    if(addressToChip[address] != nullptr) {
        return ptrLookup[addressToChip[address]];
    }
    return QSharedPointer<AMemoryChip>();

}

QSharedPointer<const AMemoryChip> MainMemory::chipAt(quint16 address) const noexcept
{
    if(addressToChip[address] != nullptr) {
        return ptrLookup[addressToChip[address]];
    }
    return QSharedPointer<AMemoryChip>();
}

QSharedPointer<AMemoryChip> MainMemory::removeChip(quint16 address)
{
    QSharedPointer<AMemoryChip> retVal = chipAt(address);
    if(retVal == endChip) return QSharedPointer<AMemoryChip>(nullptr);
    if(updateMemMap) calculateAddressToChip();
    addressToChip.remove(retVal->getBaseAddress());
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
    if(updateMemMap) calculateAddressToChip();
}

void MainMemory::loadValues(quint16 address, QVector<quint8> values) noexcept
{
    for(int idx = 0; idx < values.length() && idx + address <= static_cast<int>(size()); idx++)
    {
        bytesSet.insert(idx+address);
        setByte(idx + address, values[idx]);
    }
}

void MainMemory::clearMemory()
{
    for(auto chip : memoryChipMap) {
        chip->clear();
    }
    bytesSet.clear();
    bytesWritten.clear();
    clearErrors();
    clearIO();
}

void MainMemory::onCycleStarted()
{

}

void MainMemory::onCycleFinished()
{
    // No clean up on cycle end, yet.
}

bool MainMemory::readByte(quint16 address, quint8 &output) const
{
    QSharedPointer<const AMemoryChip> chip = chipAt(address);
    try {
        bool retVal = chip->readByte(address - chip->getBaseAddress(), output);
        return retVal;
    } catch (std::range_error &e) {
        throw e;
    } catch (bad_chip_write &e){
        throw e;
    } catch(io_aborted &e) {
        // IO was aborted, so we have an error
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
        //qDebug().noquote().nospace() << QString("0x%1: %2").arg(address, 4, 16, QLatin1Char('0')).arg(value, 2, 16, QLatin1Char('0'));
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
        // If a memory address has cached input, it should be safe to assume
        // that it is an input chip
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
        throw std::invalid_argument("Expected address of an InputChip, given address of other type");
    }
    else {
        dynamic_cast<InputChip*>(chip.get())->onInputCanceled(address - chip->getBaseAddress());
    }
}

void MainMemory::onInputAborted(quint16 address)
{
    QSharedPointer<AMemoryChip> chip = chipAt(address);
    if(chip->getChipType() != AMemoryChip::ChipTypes::IDEV) {
        throw std::invalid_argument("Expected address of an InputChip, given address of other type");
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
    }
}

void MainMemory::onChipOutputWritten(quint16 address, quint8 value)
{
    emit outputWritten(address, value);
}

void MainMemory::calculateAddressToChip() noexcept
{
    for (int it=0; it<= 0xfff; it++) {
        addressToChip[it] = nullptr;
    }
    for(auto chip : memoryChipMap) {
        for(quint32 it = chip->getBaseAddress(); it < chip->getBaseAddress() + chip->getSize(); it++) {
            addressToChip[static_cast<int>(it)] = chip.get();
        }
    }
}
