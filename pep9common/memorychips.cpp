#include "memorychips.h"
#include <QApplication>
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
    if(offsetFromBase >= size) outOfBoundsReadHelper(offsetFromBase);
    output = 0;
    return true;
}

bool ConstChip::writeByte(quint16 offsetFromBase, quint8 value)
{
    if(offsetFromBase >= size) outOfBoundsWriteHelper(offsetFromBase, value);
    // Constant device's state cannot be changed from 0.
    return true;
}

bool ConstChip::getByte(quint16 offsetFromBase, quint8 &output) const
{
    if(offsetFromBase >= size) outOfBoundsReadHelper(offsetFromBase);
    output = 0;
    return true;
}

bool ConstChip::setByte(quint16 offsetFromBase, quint8 value)
{
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
    // A "dsiabled" chip can't be cleared
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
    std::string message = "Attempted to access nil chip at: " +
            QString("0x%1.").arg(offsetFromBase + baseAddress, 4, 16, QLatin1Char('0')).toStdString();
    throw bad_chip_write(message);
}

bool NilChip::setByte(quint16 offsetFromBase, quint8)
{
    std::string message = "Attempted to access nil chip at: " +
            QString("0x%1.").arg(offsetFromBase + baseAddress, 4, 16, QLatin1Char('0')).toStdString();
    throw bad_chip_write(message);
}



InputChip::InputChip(quint32 size, quint16 baseAddress, QObject *parent):
    AMemoryChip (size, baseAddress, parent), memory(QVector<quint8>(size, 0)), waiting(QVector<bool>(size, false)),
    requestCanceled(QVector<bool>(size, false)), requestAborted(QVector<bool>(size, false))
{

}

InputChip::~InputChip()
{

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
    for (unsigned int it = 0; it < size; it++) {
        memory[it] = 0;
        waiting[it] = false;
        requestCanceled[it] = false;
        requestAborted[it] = false;
    }
}

bool InputChip::isCachable() const noexcept
{
    return false;
}

bool InputChip::readByte(quint16 offsetFromBase, quint8 &output) const
{

    if(offsetFromBase >= size) outOfBoundsReadHelper(offsetFromBase);   
    waiting[offsetFromBase] = true;
    requestCanceled[offsetFromBase] = false;
    requestAborted[offsetFromBase] = false;
    emit inputRequested(baseAddress + offsetFromBase);
    // Let the UI handle I/O before returning to this device
    QApplication::processEvents();
    if(requestCanceled[offsetFromBase]) return false;
    else if(requestAborted[offsetFromBase]) {
        throw io_aborted("Requested input that was not received.");
    }
    output = memory[offsetFromBase];
    return true;
}

bool InputChip::writeByte(quint16, quint8)
{
    std::string const str("Attempted to write to read only memory.");
    throw bad_chip_write(str);
}

bool InputChip::getByte(quint16 offsetFromBase, quint8 &output) const
{
    if(offsetFromBase >= size) outOfBoundsReadHelper(offsetFromBase);
    output = memory[offsetFromBase];
    return true;
}

bool InputChip::setByte(quint16 offsetFromBase, quint8 value)
{
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


OutputChip::OutputChip(quint32 size, quint16 baseAddress, QObject *parent): AMemoryChip (size, baseAddress, parent), memory(QVector<quint8>(size, 0))
{

}

OutputChip::~OutputChip()
{

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
    for (unsigned int it = 0; it < size; it++) {
        memory[it] = 0;
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
    if(offsetFromBase >= size) outOfBoundsWriteHelper(offsetFromBase, value);
    memory[offsetFromBase] = value;
    emit this->outputGenerated(offsetFromBase+baseAddress, value);
    return true;
}

bool OutputChip::getByte(quint16 offsetFromBase, quint8 &output) const
{
    if(offsetFromBase >= size) outOfBoundsReadHelper(offsetFromBase);
    output = memory[offsetFromBase];
    return true;
}

bool OutputChip::setByte(quint16 offsetFromBase, quint8 value)
{
    if(offsetFromBase >= size) outOfBoundsWriteHelper(offsetFromBase, value);
    memory[offsetFromBase] = value;
    return true;
}



RAMChip::RAMChip(quint32 size, quint16 baseAddress, QObject *parent): AMemoryChip (size, baseAddress, parent), memory(QVector<quint8>(size, 0))
{

}

RAMChip::~RAMChip()
{

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
    for (unsigned int it = 0; it < size; it++) {
        memory[it] = 0;
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
    if(offsetFromBase >= size) outOfBoundsReadHelper(offsetFromBase);
    output = memory[offsetFromBase];
    return true;
}

bool RAMChip::setByte(quint16 offsetFromBase, quint8 value)
{
    if(offsetFromBase >= size) outOfBoundsWriteHelper(offsetFromBase, value);
    memory[offsetFromBase] = value;
    return true;
}



ROMChip::ROMChip(quint32 size, quint16 baseAddress, QObject *parent): AMemoryChip (size, baseAddress, parent), memory(QVector<quint8>(size, 0))
{

}

ROMChip::~ROMChip()
{

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
    for (unsigned int it = 0; it < size; it++) {
        memory[it] = 0;
    }
}

bool ROMChip::readByte(quint16 offsetFromBase, quint8 &output) const
{
    return getByte(offsetFromBase, output);
}

bool ROMChip::writeByte(quint16 offsetFromBase, quint8)
{
    QString format("Attempted to write to read only memory at: 0x%1.");
    std::string message = format.
            arg(offsetFromBase + baseAddress, 4, 16, QLatin1Char('0')).
            toStdString();
    throw bad_chip_write(message);
}

bool ROMChip::getByte(quint16 offsetFromBase, quint8 &output) const
{
    if(offsetFromBase >= size) outOfBoundsReadHelper(offsetFromBase);
    output = memory[offsetFromBase];
    return true;
}

bool ROMChip::setByte(quint16 offsetFromBase, quint8 value)
{
    if(offsetFromBase >= size) outOfBoundsWriteHelper(offsetFromBase, value);
    memory[offsetFromBase] = value;
    return true;
}
