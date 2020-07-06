// File: cachememory.h
/*
    Pep9 is a virtual machine for writing machine language and assembly
    language programs.

    Copyright (C) 2020  Matthew McRaven & J. Stanley Warford, Pepperdine University

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
#ifndef CACHEMEMORY_H
#define CACHEMEMORY_H
#include <optional>
#include <set>

#include <QMap>
#include <QObject>
#include <QSet>
#include <QVector>

#include "cpu/memoizerhelper.h"
#include "cache/cache.h"
#include "cache/cacheline.h"
#include "cache/cachereplace.h"
#include "memory/amemorydevice.h"
#include "memory/mainmemory.h"


// Print debug messages to the console.
static const bool DEBUG_MESSAGES = true;

/*
 * This struct is used to track data / instruction memory accesses over the
 * course of an instruction.
 */
struct Transaction : public MemoryAccessStatistics{
    AMemoryDevice::AccessType transaction_mode;
};

/*
 * This class implements a highly-configurable form of cache memory.
 * It is a write-through cache with configurable replacement policies as well
 * as address tag breakdowns. It comes with minimal performance cost over MainMemory/
 */
class CacheMemory : public AMemoryDevice
{
    Q_OBJECT
public:
    CacheMemory(QSharedPointer<MainMemory> memory_device, Cache::CacheConfiguration config,
                QObject* parent = nullptr);

    // Determine if a given cache configuration creates a working cache.
    bool safeConfiguration(quint32 memory_size, quint16 tag_size, quint16 index_size,
                            quint16 data_size, quint16 associativity);
    // Get the number of bits allocated to each part of an address.
    quint16 getTagSize() const;
    quint16 getIndexSize() const;
    quint16 getDataSize() const;

    quint16 getAssociativty() const;

    // Cache supports write-allocate or no-write-allocate, but should default to
    // no-allocate.
    Cache::WriteAllocationPolicy getAllocationPolicy() const;
    // If given a valid cache configuration, the cache will be changed in place, and
    // true is returned. If given an invalid configuration, then false is returned,
    // and the cache configuration is not changed.
    bool resizeCache(Cache::CacheConfiguration config);

    // Reset cache without affecting main memory.
    void clearCache();

    // Get information about cache lines.
    Cache::CacheAddress breakdownAddress(quint16 address) const;
    std::optional<const CacheLine*> getCacheLine(quint16 tag) const;
    QString getCacheAlgorithm() const;

    // Return the list of emmory transactions that have occurred this cycle.
    QList<Transaction> getTransactions();
    void clearTransactionInfo();

signals:
    void configurationChanged();

// AMemoryDevice interface
public:
    quint32 maxAddress() const noexcept override;

public slots:
    void clearMemory() override;

    // Track operations that happen each CPU cycle. Useful for dyanmic aging.
    void onCycleStarted() override;
    void onCycleFinished() override;

    // In the future, when there is a cache implementation in hardware, dynamic aging
    // will need to be based on cycles rather than instructions. However, the Pep/9 CPU specification cannot be ammended.
    void onInstructionFinished(quint8 instruction_spec) override;

    // Read tracking mantains a list of all addresses that have been read during the
    // current instruction. This is helpful for rendering changes in mutable memory devices, like
    // caches. Disabling read caching will improve performance, but will make visualizations
    // of mutable memory devices unreliable.
    bool getReadTrackingEnabled() const noexcept override;
    void setReadTrackingEnabled(bool value) noexcept override;

    // Transaction tracking.
    void beginTransaction(AccessType mode) const override;
    void endTransaction() const override;

    // Update existing items in cache.
    bool readByte(quint16 address, quint8 &output) const override;
    bool writeByte(quint16 address, quint8 value) override;
    // GET/SET bypass cache access, since they aren't "real" operations.
    bool getByte(quint16 address, quint8 &output) const override;
    bool setByte(quint16 address, quint8 value) override;

    // Returns the set of bytes the have been written / set.
    // since the last clear.
    const QSet<quint16> getCacheLinesTouched() const noexcept;
    const QList<CacheEntry> getEvictedEntry(quint16 line) const noexcept;
    const QMap<quint16, QList<CacheEntry>> getAllEvictedEntries() const noexcept;
    const QSet<quint16> getBytesWritten() const noexcept override;
    const QSet<quint16> getBytesSet() const noexcept override;
    // Call after all components have (synchronously) had a chance
    // to access these fields. The set of written / set bytes will
    // continue to grow until explicitly reset.
    void clearCacheLinesTouched() noexcept;
    void clearEvictedEntry(quint16 line) noexcept;
    void clearAllEvictedEntries() noexcept;
    void clearBytesWritten() noexcept override;
    void clearBytesSet() noexcept override;
    void clearAllByteCaches() noexcept override;

private:
    bool track_reads{true};
    // It is assumed that all CacheLines use the same size / replacement policy.
    // Violating this assumption will cause dynamic aging to fail.
    mutable std::vector<CacheLine> cache;
    QSharedPointer<MainMemory> memory_device;
    QSharedPointer<AReplacementFactory> replace_factory;
    quint16 tag_size, index_size, data_size, associativity;
    Cache::WriteAllocationPolicy allocation_policy;

    // Cache lines touched indicates which cache tags were read / written this cycle.
    // Must be manually cleared with ::clearCacheLinesTouched().
    mutable QSet<quint16> cacheTagsTouched;

    // Transaction tracking.
    mutable bool in_tx = false;
    mutable Transaction tx;
    // Contains {tag, index} pairs. These uniquely identify each line in the cache.
    mutable std::set<std::tuple<quint16, quint16>> transactionLines;
    // At the end of every instruction, clear transaction lines.
    mutable QList<Transaction> instruction_txs;

    // Eviction tracking. Disabled if tracke_reads is false.
    // For a given cache tag, contains the list of all
    mutable QMap<quint16, QList<CacheEntry>> evictedLines;

};
#endif // CACHEMEMORY_H
