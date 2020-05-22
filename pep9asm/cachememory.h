#ifndef CACHEMEMORY_H
#define CACHEMEMORY_H

#include <QMap>
#include <QObject>
#include <QSet>
#include <QVector>

#include <optional>
#include <set>

#include "amemorydevice.h"
#include "mainmemory.h"
#include "memoizerhelper.h"
#include "cachereplace.h"
#include "cacheline.h"
#include "cache.h"
#pragma message("TODO: Make eviction tracking optional for performance improvement.")

struct Transaction : public MemoryAccessStatistics{
    AMemoryDevice::AccessType transaction_mode;
};

class CacheMemory : public AMemoryDevice
{
    Q_OBJECT
public:
    CacheMemory(QSharedPointer<MainMemory> memory_device, Cache::CacheConfiguration config,
                QObject* parent = nullptr);

    // Read/Change cache parameters
    bool safeConfiguration(quint32 memory_size, quint16 tag_size, quint16 index_size,
                            quint16 data_size, quint16 associativity);
    quint16 getTagSize() const;
    quint16 getIndexSize() const;
    quint16 getAssociativty() const;
    quint16 getDataSize() const;
    Cache::WriteAllocationPolicy getAllocationPolicy() const;
    bool resizeCache(Cache::CacheConfiguration config);

    // Reset cache without affecting main memory.
    void clearCache();

    // Get information about cache lines.
    Cache::CacheAddress breakdownAddress(quint16 address) const;
    std::optional<const CacheLine*> getCacheLine(quint16 tag) const;
    QString getCacheAlgorithm() const;

    // Get transaction info
    QList<Transaction> getTransactions();
    void clearTransactionInfo();

signals:
    void configurationChanged();

// AMemoryDevice interface
public:
    quint32 maxAddress() const noexcept override;

public slots:
    void clearMemory() override;

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
    // TODO: Must find a way to clear bytesRead from ISA simulator, or
    // this set will grow forever. May need to move to base class.
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

    mutable QSet<quint16> cacheLinesTouched;

    // Transaction tracking.

    mutable bool in_tx = false;
    mutable Transaction tx;
    mutable std::set<std::tuple<quint16, quint16>> transactionLines;
    // At the end of every instruction, clear transaction lines.
    mutable QList<Transaction> instruction_txs;

    // Eviction tracking

    mutable QMap<quint16, QList<CacheEntry>> evictedLines;

};
#endif // CACHEMEMORY_H
