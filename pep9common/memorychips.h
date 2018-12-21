// File: memorychips.h
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
#ifndef MEMORYCHIPS_H
#define MEMORYCHIPS_H

#include <QVector>

#include "amemorychip.h"

/*
 * Memory Chip that is hardwired to 0. Useful as a filler.
 */
class ConstChip : public AMemoryChip {

public:
    explicit ConstChip(quint16 size, quint16 baseAddress, QObject *parent = nullptr);
    virtual ~ConstChip() override;
    // AMemoryChip interface
    IOFunctions getIOFunctions() const override;
    ChipTypes getChipType() const override;
    bool readByte(quint8& output, quint16 offsetFromBase) const override;
    bool writeByte(quint16 offsetFromBase, quint8 value) override;
    bool getByte(quint8& output, quint16 offsetFromBase) const override;
    bool setByte(quint16 offsetFromBase, quint8 value) override;
};

/*
 * Memory chip that errors when read / written. Useful for implementing deadzone in main meory
 */
class NilChip: public AMemoryChip {

public:
    explicit NilChip(quint16 size, quint16 baseAddress, QObject *parent = nullptr);
    virtual ~NilChip() override;
    // AMemoryChip interface
    IOFunctions getIOFunctions() const override;
    ChipTypes getChipType() const override;
    bool readByte(quint8 &output, quint16 offsetFromBase) const override;
    bool writeByte(quint16 offsetFromBase, quint8 value) override;
    bool getByte(quint8 &output, quint16 offsetFromBase) const override;
    bool setByte(quint16 offsetFromBase, quint8 value) override;
};

/*
 * Memory Chip that handles memory mapped input.
 */
class InputChip : public AMemoryChip {
private:
    mutable QVector<quint8> memory;
    mutable bool inputRequestCanceled;
public:
    explicit InputChip(quint16 size, quint16 baseAddress, QObject *parent = nullptr);
    virtual ~InputChip() override;
    // AMemoryChip interface
    IOFunctions getIOFunctions() const override;
    ChipTypes getChipType() const override;
    bool readByte(quint8& output, quint16 offsetFromBase) const override;
    bool writeByte(quint16 offsetFromBase, quint8 value) override;
    bool getByte(quint8& output, quint16 offsetFromBase) const override;
    bool setByte(quint16 offsetFromBase, quint8 value) override;

signals:
    void inputRequested(quint16 address) const;

public slots:
    // Called when input is successfully received
    void onInputReceived(quint16 offsetFromBase, quint8 value);
    // Called when input is not received or otherwise canceled
    void onInputCanceled();
};

/*
 * Memory Chip that handles memory mapped output.
 */
class OutputDevice : public AMemoryChip {
    QVector<quint8> memory;
public:
    explicit OutputDevice(quint16 size, quint16 baseAddress, QObject *parent = nullptr);
    virtual ~OutputDevice() override;
    // AMemoryChip interface
    IOFunctions getIOFunctions() const override;
    ChipTypes getChipType() const override;
    bool readByte(quint8 &output, quint16 offsetFromBase) const override;
    bool writeByte(quint16 offsetFromBase, quint8 value) override;
    bool getByte(quint8 &output, quint16 offsetFromBase) const override;
    bool setByte(quint16 offsetFromBase, quint8 value) override;

signals:
    void inputReceived(quint16 address, quint8 value);
};

/*
 * Memory Chip that is readable & writable. Used to implement the majority of main memory.
 */
class RAMDevice : public AMemoryChip {
    QVector<quint8> memory;
public:
    explicit RAMDevice(quint16 size, quint16 baseAddress, QObject *parent = nullptr);
    virtual ~RAMDevice() override;
    // AMemoryChip interface
    IOFunctions getIOFunctions() const override;
    ChipTypes getChipType() const override;
    bool readByte(quint8 &output, quint16 offsetFromBase) const override;
    bool writeByte(quint16 offsetFromBase, quint8 value) override;
    bool getByte(quint8 &output, quint16 offsetFromBase) const override;
    bool setByte(quint16 offsetFromBase, quint8 value) override;
};

/*
 * Memory Chip that is only readable. Used to store the operating system.
 */
class ROMDevice : public AMemoryChip {
    QVector<quint8> memory;
public:
    explicit ROMDevice(quint16 size, quint16 baseAddress, QObject *parent = nullptr);
    virtual ~ROMDevice() override;
    // AMemoryChip interface
    IOFunctions getIOFunctions() const override;
    ChipTypes getChipType() const override;
    bool readByte(quint8 &output, quint16 offsetFromBase) const override;
    bool writeByte(quint16 offsetFromBase, quint8 value) override;
    bool getByte(quint8 &output, quint16 offsetFromBase) const override;
    bool setByte(quint16 offsetFromBase, quint8 value) override;
};

#endif // MEMORYCHIPS_H
