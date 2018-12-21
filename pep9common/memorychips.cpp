#include "memorychips.h"
#include <QApplication>
ConstChip::ConstChip(quint16 size, quint16 baseAddress, QObject *parent):
    AMemoryChip (size, baseAddress, parent)
{

}

ConstChip::~ConstChip()
{

}

AMemoryChip::IOFunctions ConstChip::getIOFunctions() const
{
    return AMemoryChip::NONE;
}

AMemoryChip::ChipTypes ConstChip::getChipType() const
{
    return AMemoryChip::ChipTypes::CONST;
}

bool ConstChip::readByte(quint8& output, quint16 offsetFromBase) const
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

bool ConstChip::getByte(quint8& output, quint16 offsetFromBase) const
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

NilChip::NilChip(quint16 size, quint16 baseAddress, QObject *parent): AMemoryChip (size, baseAddress, parent)
{

}

NilChip::~NilChip()
{

}

AMemoryChip::IOFunctions NilChip::getIOFunctions() const
{
    return IOFunctions::NONE;
}

AMemoryChip::ChipTypes NilChip::getChipType() const
{
    return ChipTypes::NIL;
}

bool NilChip::readByte(quint8 &output, quint16 offsetFromBase) const
{
    return getByte(output, offsetFromBase);
}

bool NilChip::writeByte(quint16 offsetFromBase, quint8 value)
{
    return setByte(offsetFromBase, value);
}

bool NilChip::getByte(quint8 &output, quint16 offsetFromBase) const
{
    std::string message = "Attempted to access nil chip at: " +
            QString("0x%1").arg(offsetFromBase + baseAddress, 4, 16, QLatin1Char('0')).toStdString();
    throw bad_chip_operation(message);
}

bool NilChip::setByte(quint16 offsetFromBase, quint8 value)
{
    std::string message = "Attempted to access nil chip at: " +
            QString("0x%1").arg(offsetFromBase + baseAddress, 4, 16, QLatin1Char('0')).toStdString();
    throw bad_chip_operation(message);
}

InputChip::InputChip(quint16 size, quint16 baseAddress, QObject *parent):
    AMemoryChip (size, baseAddress, parent), memory(QVector<quint8>(size)), inputRequestCanceled(false)
{

}

InputChip::~InputChip()
{

}

AMemoryChip::IOFunctions InputChip::getIOFunctions() const
{
    return static_cast<AMemoryChip::IOFunctions>(  static_cast<int>(AMemoryChip::READ)
                                                 | static_cast<int>(AMemoryChip::MEMORY_MAPPED));
}

AMemoryChip::ChipTypes InputChip::getChipType() const
{
    return AMemoryChip::ChipTypes::IDEV;
}

bool InputChip::readByte(quint8& output, quint16 offsetFromBase) const
{

    if(offsetFromBase >= size) outOfBoundsReadHelper(offsetFromBase);
    inputRequestCanceled = false;
    emit inputRequested(baseAddress + offsetFromBase);
    // Let the UI handle I/O before returning to this device
    QApplication::processEvents();
    output = memory[offsetFromBase];
    return !inputRequestCanceled;
}

bool InputChip::writeByte(quint16, quint8)
{
    std::string const str("Attempted to write to read only memory");
    throw bad_chip_operation(str);
}

bool InputChip::getByte(quint8& output, quint16 offsetFromBase) const
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

void InputChip::onInputReceived(quint16 offsetFromBase, quint8 value)
{
    if(offsetFromBase >= size) outOfBoundsWriteHelper(offsetFromBase, value);
    memory[offsetFromBase] = value;
}

void InputChip::onInputCanceled()
{
    inputRequestCanceled = true;
}

OutputDevice::OutputDevice(quint16 size, quint16 baseAddress, QObject *parent): AMemoryChip (size, baseAddress, parent), memory(QVector<quint8>(size))
{

}

OutputDevice::~OutputDevice()
{

}

AMemoryChip::IOFunctions OutputDevice::getIOFunctions() const
{
        return static_cast<AMemoryChip::IOFunctions>(  static_cast<int>(AMemoryChip::READ)
                                                     | static_cast<int>(AMemoryChip::WRITE)
                                                     | static_cast<int>(AMemoryChip::MEMORY_MAPPED));
}

AMemoryChip::ChipTypes OutputDevice::getChipType() const
{
    return ChipTypes::ODEV;
}

bool OutputDevice::readByte(quint8 &output, quint16 offsetFromBase) const
{
    return getByte(output, offsetFromBase);
}

bool OutputDevice::writeByte(quint16 offsetFromBase, quint8 value)
{
    if(offsetFromBase >= size) outOfBoundsWriteHelper(offsetFromBase, value);
    memory[offsetFromBase] = value;
    emit this->inputReceived(offsetFromBase+baseAddress, value);
    return true;
}

bool OutputDevice::getByte(quint8 &output, quint16 offsetFromBase) const
{
    if(offsetFromBase >= size) outOfBoundsReadHelper(offsetFromBase);
    output = memory[offsetFromBase];
    return true;
}

bool OutputDevice::setByte(quint16 offsetFromBase, quint8 value)
{
    if(offsetFromBase >= size) outOfBoundsWriteHelper(offsetFromBase, value);
    memory[offsetFromBase] = value;
    return true;
}

RAMDevice::RAMDevice(quint16 size, quint16 baseAddress, QObject *parent): AMemoryChip (size, baseAddress, parent), memory(QVector<quint8>(size))
{

}

RAMDevice::~RAMDevice()
{

}

AMemoryChip::IOFunctions RAMDevice::getIOFunctions() const
{
    return static_cast<AMemoryChip::IOFunctions>(  static_cast<int>(AMemoryChip::READ)
                                                   | static_cast<int>(AMemoryChip::WRITE));
}

AMemoryChip::ChipTypes RAMDevice::getChipType() const
{
    return AMemoryChip::ChipTypes::RAM;
}

bool RAMDevice::readByte(quint8 &output, quint16 offsetFromBase) const
{
    return getByte(output, offsetFromBase);
}

bool RAMDevice::writeByte(quint16 offsetFromBase, quint8 value)
{
    return setByte(offsetFromBase, value);
}

bool RAMDevice::getByte(quint8 &output, quint16 offsetFromBase) const
{
    if(offsetFromBase >= size) outOfBoundsReadHelper(offsetFromBase);
    output = memory[offsetFromBase];
    return true;
}

bool RAMDevice::setByte(quint16 offsetFromBase, quint8 value)
{
    if(offsetFromBase >= size) outOfBoundsWriteHelper(offsetFromBase, value);
    memory[offsetFromBase] = value;
    return true;
}



ROMDevice::ROMDevice(quint16 size, quint16 baseAddress, QObject *parent): AMemoryChip (size, baseAddress, parent), memory(QVector<quint8>(size))
{

}

ROMDevice::~ROMDevice()
{

}

AMemoryChip::IOFunctions ROMDevice::getIOFunctions() const
{
    return static_cast<AMemoryChip::IOFunctions>(static_cast<int>(AMemoryChip::READ));
}

AMemoryChip::ChipTypes ROMDevice::getChipType() const
{
    return AMemoryChip::ChipTypes::RAM;
}

bool ROMDevice::readByte(quint8 &output, quint16 offsetFromBase) const
{
    return getByte(output, offsetFromBase);
}

bool ROMDevice::writeByte(quint16 offsetFromBase, quint8)
{
    QString format("Attempted to write to read only memory at: 0x1%");
    std::string message = format.
            arg(offsetFromBase + baseAddress, 4, 16, QLatin1Char('0')).
            toStdString();
    throw bad_chip_operation(message);
}

bool ROMDevice::getByte(quint8 &output, quint16 offsetFromBase) const
{
    if(offsetFromBase >= size) outOfBoundsReadHelper(offsetFromBase);
    output = memory[offsetFromBase];
    return true;
}

bool ROMDevice::setByte(quint16 offsetFromBase, quint8 value)
{
    if(offsetFromBase >= size) outOfBoundsWriteHelper(offsetFromBase, value);
    memory[offsetFromBase] = value;
    return true;
}
