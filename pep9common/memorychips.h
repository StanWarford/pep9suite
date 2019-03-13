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
    Q_OBJECT
public:
    explicit ConstChip(quint32 size, quint16 baseAddress, QObject *parent = nullptr);
    virtual ~ConstChip() override;

    // AMemoryChip interface
    IOFunctions getIOFunctions() const noexcept override;
    ChipTypes getChipType() const noexcept override;
    void clear() noexcept override;

    bool readByte(quint16 offsetFromBase, quint8 &output) const override;
    bool writeByte(quint16 offsetFromBase, quint8 value) override;
    bool getByte(quint16 offsetFromBase, quint8 &output) const override;
    bool setByte(quint16 offsetFromBase, quint8 value) override;
};

/*
 * Memory chip that errors when read / written. Useful for implementing deadzone in main meory
 */
class NilChip: public AMemoryChip {
    Q_OBJECT
public:
    explicit NilChip(quint32 size, quint16 baseAddress, QObject *parent = nullptr);
    virtual ~NilChip() override;

    // AMemoryChip interface
    IOFunctions getIOFunctions() const noexcept override;
    ChipTypes getChipType() const noexcept override;
    void clear() noexcept override;
    bool isCachable() const noexcept override;

    bool readByte(quint16 offsetFromBase, quint8 &output) const override;
    bool writeByte(quint16 offsetFromBase, quint8 value) override;
    bool getByte(quint16 offsetFromBase, quint8 &output) const override;
    bool setByte(quint16 offsetFromBase, quint8 value) override;
};

/*
 * Memory Chip that handles memory mapped input.
 */
class InputChip : public AMemoryChip {
    Q_OBJECT
    mutable QVector<quint8> memory;
    mutable QVector<bool> waiting, requestCanceled, requestAborted;
public:
    explicit InputChip(quint32 size, quint16 baseAddress, QObject *parent = nullptr);
    virtual ~InputChip() override;

    // AMemoryChip interface
    IOFunctions getIOFunctions() const noexcept override;
    ChipTypes getChipType() const noexcept override;
    void clear() noexcept override;
    bool isCachable() const noexcept override;

    bool readByte(quint16 offsetFromBase, quint8 &output) const override;
    bool writeByte(quint16 offsetFromBase, quint8 value) override;
    bool getByte(quint16 offsetFromBase, quint8 &output) const override;
    bool setByte(quint16 offsetFromBase, quint8 value) override;
    bool waitingForInput(quint16 offsetFromBase) const;

signals:
    void inputRequested(quint16 address) const;

public slots:
    // Called when input is successfully received
    void onInputReceived(quint16 offsetFromBase, quint8 value);
    // Called when input is no longer needed
    void onInputCanceled(quint16 offsetFromBase);
    // Called when input is requested, but cannot be served
    void onInputAborted(quint16 offsetFromBase);
};

/*
 * Memory Chip that handles memory mapped output.
 */
class OutputChip : public AMemoryChip {
    Q_OBJECT
    QVector<quint8> memory;
public:
    explicit OutputChip(quint32 size, quint16 baseAddress, QObject *parent = nullptr);
    virtual ~OutputChip() override;

    // AMemoryChip interface
    IOFunctions getIOFunctions() const noexcept override;
    ChipTypes getChipType() const noexcept override;
    void clear() noexcept override;
    bool isCachable() const noexcept override;

    bool readByte(quint16 offsetFromBase, quint8 &output) const override;
    bool writeByte(quint16 offsetFromBase, quint8 value) override;
    bool getByte(quint16 offsetFromBase, quint8 &output) const override;
    bool setByte(quint16 offsetFromBase, quint8 value) override;

signals:
    void outputGenerated(quint16 address, quint8 value);
};

/*
 * Memory Chip that is readable & writable. Used to implement the majority of main memory.
 */
class RAMChip : public AMemoryChip {
    Q_OBJECT
    QVector<quint8> memory;
public:
    explicit RAMChip(quint32 size, quint16 baseAddress, QObject *parent = nullptr);
    virtual ~RAMChip() override;

    // AMemoryChip interface
    IOFunctions getIOFunctions() const noexcept override;
    ChipTypes getChipType() const noexcept override;
    void clear() noexcept override;

    bool readByte(quint16 offsetFromBase, quint8 &output) const override;
    bool writeByte(quint16 offsetFromBase, quint8 value) override;
    bool getByte(quint16 offsetFromBase, quint8 &output) const override;
    bool setByte(quint16 offsetFromBase, quint8 value) override;
};

/*
 * Memory Chip that is only readable. Used to store the operating system.
 */
class ROMChip : public AMemoryChip {
    Q_OBJECT
    QVector<quint8> memory;
public:
    explicit ROMChip(quint32 size, quint16 baseAddress, QObject *parent = nullptr);
    virtual ~ROMChip() override;

    // AMemoryChip interface
    IOFunctions getIOFunctions() const noexcept override;
    ChipTypes getChipType() const noexcept override;
    void clear() noexcept override;

    bool readByte(quint16 offsetFromBase, quint8 &output) const override;
    bool writeByte(quint16 offsetFromBase, quint8 value) override;
    bool getByte(quint16 offsetFromBase, quint8 &output) const override;
    bool setByte(quint16 offsetFromBase, quint8 value) override;
};

#endif // MEMORYCHIPS_H
