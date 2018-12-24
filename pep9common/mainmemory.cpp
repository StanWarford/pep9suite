#include "mainmemory.h"
#include "amemorychip.h"
#include "memorychips.h"
MainMemory::MainMemory(QObject* parent): AMemoryDevice (parent),
    endChip(new NilChip(0xfff, 0, this)), addressToChip(1<<16)
{

}

quint16 MainMemory::size() const
{
    int size = 0;
    for(auto it : memoryChipMap) {
        size += it->getSize();
    }
    return size;
}

void MainMemory::insertChip(QSharedPointer<AMemoryChip> chip, quint16 address)
{
    memoryChipMap.insert(address, chip);
    if(chip->getChipType() == AMemoryChip::ChipTypes::IDEV) {
        connect((InputChip*)chip.get(), &InputChip::inputRequested, this,  &MainMemory::onChipInputRequested);
    }
    else if(chip->getChipType() == AMemoryChip::ChipTypes::ODEV) {
        connect((OutputChip*)chip.get(), &OutputChip::outputGenerated, this,  &MainMemory::onChipOutputWritten);
    }
    calculateAddressToChip();
}

QSharedPointer<AMemoryChip> MainMemory::chipAt(quint16 address)
{
    quint16 prevKey = memoryChipMap.firstKey();
    for(auto i = memoryChipMap.constBegin(); i != memoryChipMap.constEnd(); ++i){
        if(i.key()> address) break;
        else prevKey = i.key();
    }
    if(memoryChipMap[prevKey] == nullptr) return endChip;
    else return memoryChipMap[prevKey];
}

QSharedPointer<const AMemoryChip> MainMemory::chipAt(quint16 address) const
{
    quint16 prevKey = memoryChipMap.firstKey();
    for(auto i = memoryChipMap.constBegin(); i != memoryChipMap.constEnd(); ++i){
        if(i.key()> address) break;
        else prevKey = i.key();
    }
    return memoryChipMap[prevKey];
}

QSharedPointer<AMemoryChip> MainMemory::removeChip(quint16 address)
{
    QSharedPointer<AMemoryChip> retVal = chipAt(address);
    if(retVal == endChip) return QSharedPointer<AMemoryChip>(nullptr);
    calculateAddressToChip();
    return retVal;
}

QVector<QSharedPointer<AMemoryChip> > MainMemory::removeAllChips()
{
    auto temp = memoryChipMap.values();
    QVector<QSharedPointer<AMemoryChip> > retVal;
    for(auto it : temp) {
        retVal.append(it);
        if(it->getChipType() == AMemoryChip::ChipTypes::IDEV) {
            disconnect((InputChip*)it.get(), &InputChip::inputRequested, this,  &MainMemory::onChipInputRequested);
        }
        else if(it->getChipType() == AMemoryChip::ChipTypes::ODEV) {
            disconnect((OutputChip*)it.get(), &OutputChip::outputGenerated, this,  &MainMemory::onChipOutputWritten);
        }
    }
    memoryChipMap.clear();
    calculateAddressToChip();
    return retVal;
}

void MainMemory::clearMemory()
{
    for(auto chip : memoryChipMap) {
        chip->clear();
    }
    bytesSet.clear();
    bytesWritten.clear();
    clearErrors();
}

void MainMemory::onCycleStarted()
{
    bytesWritten.clear();
    bytesSet.clear();

}

void MainMemory::onCycleFinished()
{
    // No clean up on cycle end, yet.
}
bool MainMemory::readByte(quint8 &output, quint16 address) const
{
    QSharedPointer<const AMemoryChip> chip = chipAt(address);
    try {
        bool retVal = chip->readByte(output, address - chip->getBaseAddress());
        return retVal;
    } catch (std::range_error& e) {
        throw e;
    } catch (bad_chip_operation& e){
        throw e;
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
        throw e;
    } catch (bad_chip_operation& e){
        throw e;
    }
}

bool MainMemory::getByte(quint8 &output, quint16 address) const
{
    QSharedPointer<const AMemoryChip> chip = chipAt(address);
    try {
        bool retVal = chip->getByte(output, address - chip->getBaseAddress());
        return retVal;
    } catch (std::range_error& e) {
        throw e;
    } catch (bad_chip_operation& e){
        throw e;
    }
}

bool MainMemory::setByte(quint16 address, quint8 value)
{
    QSharedPointer<AMemoryChip> chip = chipAt(address);
    try {
        bool retVal = chip->setByte(address - chip->getBaseAddress(), value);
        bytesSet.insert(address);
        return retVal;
    } catch (std::range_error& e) {
        throw e;
    } catch (bad_chip_operation& e){
        throw e;
    }
}

void MainMemory::onInputReceived(quint16 address, quint8 input)
{
    #pragma message("TODO: Queue memory mapped IO")
    throw -1;
}

void MainMemory::onInputReceived(quint16 address, QChar input)
{
    #pragma message("TODO: Queue memory mapped IO")
    throw -1;
}

void MainMemory::onInputReceived(quint16 address, QString input)
{
    #pragma message("TODO: Queue memory mapped IO")
    throw -1;
}

void MainMemory::onInputCanceled(quint16 address)
{
    QSharedPointer<AMemoryChip> chip = chipAt(address);
    if(chip->getChipType() != AMemoryChip::ChipTypes::IDEV) {
        throw std::invalid_argument("Expected address of IDev, given other address");
    }
    else {
        dynamic_cast<InputChip*>(chip.get())->onInputCanceled();
    }
}

void MainMemory::onChipInputRequested(quint16 address)
{
    emit inputRequested(address);
}

void MainMemory::onChipOutputWritten(quint16 address, quint8 value)
{
    emit outputWritten(address, value);
}

void MainMemory::calculateAddressToChip()
{
    for (int it=0; it<= 0xfff; it++) {
        addressToChip[it] = nullptr;
    }
    for(auto chip : memoryChipMap) {
        for(int it = chip->getBaseAddress(); it< chip->getBaseAddress() + chip->getSize(); it++) {
            addressToChip[it] = chip.get();
        }
    }
}
