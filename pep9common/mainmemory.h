// File: mainmemory.h
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
#ifndef MAINMEMORY_H
#define MAINMEMORY_H

#include <QMap>
#include <QObject>
#include <QSharedPointer>
#include <QVector>

#include "amemorychip.h"
#include "amemorydevice.h"
class AMemoryChip;
class NilChip;

/*
 * Structure that specifies the locatiocation & size
 * of a chip to be inserted into main memory.
 */
struct MemoryChipSpec {
    AMemoryChip::ChipTypes type;
    quint16 startAddr;
    quint32 size;
};

class MainMemory : public AMemoryDevice
{
    Q_OBJECT
    // Store whether or not the addressToChipLookupTable should be updated with
    // each insertion or removal.
    bool updateMemMap;
    // Pointer to the chip that will be used to "blank out" memory addresses
    // past the end alloacted memory (i.e. memory address after the argument
    // of a .BURN).
    QSharedPointer<NilChip> endChip;
    // For all 2^16 addresses in machine, create a look-up table that speeds up
    // translation of an address to the memory chip that contains it.
    QVector<AMemoryChip*> addressToChipLookupTable;
    // Starting address of each chip inserted into the memory system.
    QMap<quint16, QSharedPointer<AMemoryChip>> memoryChipMap;
    QMap<AMemoryChip*, QSharedPointer<AMemoryChip>> ptrLookup;
    // Buffer input for particular addresses (needed for batch character input).
    mutable QMap<quint16, QByteArray> inputBuffer;
    // A list of all memory locations that have a pending input request.
    mutable QSet<quint16> waitingOnInput;
    // Highest accessible address in memory.
    mutable quint32 maxAddr;

public:
    explicit MainMemory(QObject* parent = nullptr) noexcept;
    virtual ~MainMemory() override;

    // AMemoryDevice interface
    quint32 maxAddress() const noexcept override;
    void insertChip(QSharedPointer<AMemoryChip> chip, quint16 address);
    // Return the chip containing address. Will return nullptr
    // if the address is out-of-range of the current memory space (e.g. address
    // 0xff16 when only 0x8000 bytes of memory are installed).
    AMemoryChip* chipAt(quint16 address) noexcept;
    const AMemoryChip* chipAt(quint16 address) const noexcept;


    // Configure this memory device as described by the specifications in the
    // specList. Will attempt to reuse the currently installed memory chips,
    // and will only allocate new chips if absolutely necessary.
    // Currently installed chips that are not re-used will be eligible for
    // automatic garbage collection (via refrence counting in shared pointers).
    void constructMemoryDevice(QList<MemoryChipSpec> specList);

    // Remove the chip containing address and return it. Will return nullptr
    // if the address is out-of-range of the current memory space (e.g. address
    // 0xff16 when only 0x8000 bytes of memory are installed).
    QSharedPointer<AMemoryChip> removeChip(quint16 address);
    // Uninstall all chips from memory, and return them as a vector.
    // This will leave memory with no readable or writable addresses.
    QVector<QSharedPointer<AMemoryChip>> removeAllChips();

    // If multiple chip insertions / deletions will be performed, it is possible
    // to prevent the address cache from being updated spuriously for improved
    // performance.
    void autoUpdateMemoryMap(bool update) noexcept;

    // Copies the bytes from values into main memory starting at address.
    void loadValues(quint16 address, QVector<quint8> values) noexcept;

    // In main memory, address caching on reads is disabled.
    bool getReadCachingEnabled() const noexcept override {return false;}
    void setReadCachingEnabled(bool /*value*/) noexcept override {}

public slots:
    // Set the values in all memory chips to 0, clear all outstanding IO operations.
    void clearMemory() override;

    // Main memory doesn't need to provide dynamic aging of any memory contents,
    // so these methods do nothing.
    void onCycleStarted() override;
    void onCycleFinished() override;

    // In main memory, address caching on reads is disabled.
    bool readByte(quint16 address, quint8 &output) const override;
    bool writeByte(quint16 address, quint8 value) override;
    bool getByte(quint16 address, quint8 &output) const override;
    bool setByte(quint16 address, quint8 value) override;

    // Clear any saved input, and cancel any outstanding IO requests.
    void clearIO();

    // If no input is currently requested for the address, it will be
    // buffered internally for future usage.
    void onInputReceived(quint16 address, quint8 input);
    void onInputReceived(quint16 address, QChar input);
    void onInputReceived(quint16 address, QString input);

    void onInputCanceled(quint16 address);
    void onInputAborted(quint16 address);

signals:
    void inputRequested(quint16 address);
    void outputWritten(quint16 address, quint8 value);

protected slots:
    void onChipInputRequested(quint16 address);
    void onChipOutputWritten(quint16 address, quint8 value);

private:
    void calculateAddressToChip() noexcept;
};

#endif // MAINMEMORY_H
