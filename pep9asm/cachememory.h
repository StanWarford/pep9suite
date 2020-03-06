#ifndef CACHEMEMORY_H
#define CACHEMEMORY_H

#include <QMap>
#include <QObject>
#include <QSet>
#include <QVector>

#include "amemorydevice.h"
#include "mainmemory.h"
#include "cacheline.h"
#include "cachereplace.h"

enum class ReplacementPolicy {
    LRU,
};

struct AddressBreakdown
{
    quint16 tag, index, offset;
};

class CacheMemory : public AMemoryDevice
{
    Q_OBJECT
public:
    CacheMemory(QSharedPointer<MainMemory> memory_device, quint16 tag_size,
                quint16 index_size, quint16 associativity, QSharedPointer<AReplacementFactory> policy,
                QObject* parent = nullptr);

    // Read/Change cache parameters
    bool safe_configuration(quint32 memory_size, quint16 tag_size, quint16 index_size,
                            quint16 data_size, quint16 associativity);
    quint16 get_tag_size() const;
    quint16 get_index_size() const;
    quint16 get_associativty() const;
    quint16 get_data_size() const;
    void resize_cache(quint16 tag_size, quint16 index_size, quint16 associativity,
                       QSharedPointer<AReplacementFactory> factory);

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
    const QSet<quint16> getBytesWritten() const noexcept override;
    const QSet<quint16> getBytesSet() const noexcept override;
    // Call after all components have (synchronously) had a chance
    // to access these fields. The set of written / set bytes will
    // continue to grow until explicitly reset.
    void clearBytesWritten() noexcept override;
    void clearBytesSet() noexcept override;
private:
    mutable std::vector<CacheLine> cache;
    QSharedPointer<MainMemory> memory_device;
    QSharedPointer<AReplacementFactory> replace_factory;
    AddressBreakdown breakdown_address(quint16 address) const;
    quint16 tag_size, index_size, data_size, associativity;

    mutable quint32 hit_read, miss_read, writes;

};

#endif // CACHEMEMORY_H
