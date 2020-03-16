#ifndef CACHEMEMORY_H
#define CACHEMEMORY_H

#include <QMap>
#include <QObject>
#include <QSet>
#include <QVector>

#include <optional>

#include "amemorydevice.h"
#include "mainmemory.h"
#include "cacheline.h"
#include "cachereplace.h"

namespace Cache {
    Q_NAMESPACE
    struct CacheAddress
    {
        quint16 tag, index, offset;
    };

    enum class WriteAllocationPolicy{
        WriteAllocate, NoWriteAllocate
    };
    Q_ENUM_NS(WriteAllocationPolicy);

    struct CacheConfiguration {
        // Number of addressing bits used for the tag, index and data.
        quint16 tag_bits, index_bits, data_bits;
        quint16 associativity;
        WriteAllocationPolicy write_allocation = WriteAllocationPolicy::NoWriteAllocate;
        QSharedPointer<AReplacementFactory> policy;
    };

}
class CacheMemory : public AMemoryDevice
{
    Q_OBJECT
public:
    CacheMemory(QSharedPointer<MainMemory> memory_device, Cache::CacheConfiguration config,
                QObject* parent = nullptr);

    // Read/Change cache parameters
    bool safeConfiguration(quint32 memory_size, quint16 tag_size, quint16 index_size,
                            quint16 data_size, quint16 associativity);
    quint16 geTagSize() const;
    quint16 getIndexSize() const;
    quint16 getAssociativty() const;
    quint16 getDataSize() const;
    Cache::WriteAllocationPolicy getAllocationPolicy() const;
    void resizeCache(Cache::CacheConfiguration config);

    // Reset cache without affecting main memory.
    void clearCache();

    // Get information about cache lines.
    Cache::CacheAddress breakdownAddress(quint16 address) const;
    std::optional<const CacheLine*> getCacheLine(quint16 tag) const;
    QString getCacheAlgorithm() const;

    // Get/Reset cache statistics
    //quint32 get_hits() const;
    //quint32 get_misses() const;
    //quint32 get_accesses() const;
    //void reset_statistics();
    // TODO


    // AMemoryDevice interface
public:
    quint32 maxAddress() const noexcept override;
public slots:
    void clearMemory() override;
    void onCycleStarted() override;
    void onCycleFinished() override;
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
    const QSet<quint16> getBytesWritten() const noexcept override;
    const QSet<quint16> getBytesSet() const noexcept override;
    // Call after all components have (synchronously) had a chance
    // to access these fields. The set of written / set bytes will
    // continue to grow until explicitly reset.
    void clearCacheLinesTouched() noexcept;
    void clearBytesWritten() noexcept override;
    void clearBytesSet() noexcept override;
    void clearAllByteCaches() noexcept override;
private:
    mutable std::vector<CacheLine> cache;
    QSharedPointer<MainMemory> memory_device;
    QSharedPointer<AReplacementFactory> replace_factory;
    quint16 tag_size, index_size, data_size, associativity;
    Cache::WriteAllocationPolicy allocation_policy;

    mutable quint32 hit_read, miss_read, hit_writes, miss_writes;
    mutable QSet<quint16> cacheLinesTouched;

};
#endif // CACHEMEMORY_H
