#include "cachememory.h"

#include <cmath>
#include <QDebug>

#include "cacheline.h"

CacheMemory::CacheMemory(QSharedPointer<MainMemory> memory_device, Cache::CacheConfiguration config,
                         QObject* parent):
    track_reads(true), AMemoryDevice(parent), memory_device(memory_device), replace_factory(config.policy),
    tag_size(config.tag_bits), index_size(config.index_bits),
    data_size(16 - tag_size - index_size), associativity(config.associativity),
    allocation_policy(config.write_allocation), cacheLinesTouched(), stats(256)
{

    // Make sure that cache won't accidentally go outside the range of accessible memory
    assert(safeConfiguration(maxAddress(), tag_size, index_size, data_size, associativity));

    // Resize the cache to fit the correct number of lines.
    //resize_cache(tag_size, index_size, associativity, replace_factory);
    cache.resize(1 << tag_size);
    for(auto& line : cache) {
        line = CacheLine(associativity, replace_factory->create_policy());
    }
    /*for(int it=0; it<65536; it++) {
        qDebug() << it;
        breakdown_address(it);
    }*/
}

bool CacheMemory::safeConfiguration(quint32 memory_size, quint16 tag_size,
                                     quint16 index_size, quint16 data_size, quint16 associativity)
{
    bool size_match = tag_size+index_size+data_size == (int)ceil(log2(memory_size));
    bool used_associativty = (1 << index_size) >= associativity;
    return size_match & used_associativty;
}

quint16 CacheMemory::getTagSize() const
{
    return tag_size;
}

quint16 CacheMemory::getIndexSize() const
{
    return index_size;
}

quint16 CacheMemory::getAssociativty() const
{
    return associativity;
}

quint16 CacheMemory::getDataSize() const
{
    return data_size;
}

Cache::WriteAllocationPolicy CacheMemory::getAllocationPolicy() const
{
    return allocation_policy;
}

bool CacheMemory::resizeCache(Cache::CacheConfiguration config)
{
    if(!safeConfiguration(maxAddress(), config.tag_bits, config.index_bits, config.data_bits, config.associativity)) {
        return false;
    }
    else {
        cache.resize(1 << tag_size);
        tag_size = config.tag_bits;
        index_size = config.index_bits;
        data_size = config.data_bits;
        associativity = config.associativity;
        allocation_policy = config.write_allocation;
        cacheLinesTouched.clear();

        clearStats();

        this->replace_factory = config.policy;
        for(auto& line : cache) {
            line = CacheLine(associativity, replace_factory->create_policy());
        }
        emit configurationChanged();
        return true;
    }
}

void CacheMemory::clearCache()
{
    for(auto& line : cache) {
        line.clear();
    }

    clearStats();

    clearCacheLinesTouched();
}

quint32 CacheMemory::maxAddress() const noexcept
{
    return memory_device->maxAddress();
}

void CacheMemory::clearMemory()
{
    clearCache();
    memory_device->clearMemory();
}

void CacheMemory::onCycleStarted()
{
    //TODO: Cycle based cache reporting.
    memory_device->onCycleStarted();
}

void CacheMemory::onCycleFinished()
{
    //TODO: Cycle based cache reporting.

    memory_device->onCycleFinished();
}

void CacheMemory::onInstructionFinished(quint8 instruction_spec)
{
    auto& at = stats[instruction_spec];
    for(auto tx : instruction_txs) {
        at.read_hit  += tx.read_hit;
        at.read_miss += tx.read_miss;
        at.write_hit += tx.write_hit;
        at.write_miss += tx.write_miss;
    }
    qDebug() << at.read_hit << at.read_miss <<at.write_hit << at.write_miss;
    instruction_txs.clear();
}

bool CacheMemory::getReadCachingEnabled() const noexcept
{
    return track_reads;
}

void CacheMemory::setReadCachingEnabled(bool value) noexcept
{
    this->track_reads = value;
}

bool CacheMemory::readByte(quint16 address, quint8 &output) const
{
    if(track_reads) {
        bytesRead.insert(address);
    }
    auto address_breakdown = breakdownAddress(address);

    // If address is present in line, notify CRP of a hit.
    // MUST capture line by reference, or Qt's COW will kick in, and no changes
    // will be saved.
    if(CacheLine& line = cache[address_breakdown.tag];
            line.contains_index(address_breakdown.index)) {
        //qDebug().noquote() << QString("Hit %1\t").arg(address);
        line.update(address_breakdown);
        // Only increment statistics if this transaction hasn't already "touched"
        // the current cache line.
        if(transactionLines.find({address_breakdown.tag, address_breakdown.index}) != transactionLines.end()) {
            tx.read_hit++;
        }
    }
    // Otherwise must insert index associated with line.
    else {
        //qDebug().noquote() << QString("Miss %1\t").arg(address);
        auto evicted = line.insert(address_breakdown);
        if(track_reads && evicted.is_present) {
            if(!evictedLines.contains(address_breakdown.tag)) evictedLines.insert(address_breakdown.tag, QList<CacheEntry>());
            evictedLines[address_breakdown.tag].append(evicted);
        }
        // Only increment statistics if this transaction hasn't already "touched"
        // the current cache line.
        if(transactionLines.find({address_breakdown.tag, address_breakdown.index}) != transactionLines.end()) {
            tx.read_miss++;
        }
    }
    // Either if the read is a hit or a miss, the cache line associated with
    // the current address must be updated.
    cacheLinesTouched.insert(address_breakdown.tag);
    // Prevent line cache entry from being counted in statistics again.
    transactionLines.insert({address_breakdown.tag, address_breakdown.index});
    return memory_device->readByte(address, output);
}

bool CacheMemory::writeByte(quint16 address, quint8 value)
{
    // No need to add to bytesWritten, this will be handled in main memory device.

    auto address_breakdown = breakdownAddress(address);

    // If address is present in line, notify CRP of a hit.
    // MUST capture line by reference, or Qt's COW will kick in, and no changes
    // will be saved.
    if(CacheLine& line = cache[address_breakdown.tag];
            line.contains_index(address_breakdown.index)) {
        // Update reference counts for the current line.
        cacheLinesTouched.insert(address_breakdown.tag);
        line.update(address_breakdown);
        // Only increment statistics if this transaction hasn't already "touched"
        // the current cache line.
        if(transactionLines.find({address_breakdown.tag, address_breakdown.index}) != transactionLines.end()) {
            tx.write_hit++;
        }
    }
    else if(allocation_policy == Cache::WriteAllocationPolicy::WriteAllocate) {
        // Perform eviction in memory dump pane.
        auto evicted = line.insert(address_breakdown);
        if(track_reads && evicted.is_present) {
            if(!evictedLines.contains(address_breakdown.tag)) evictedLines.insert(address_breakdown.tag, QList<CacheEntry>());
            evictedLines[address_breakdown.tag].append(evicted);
        }

        // Catch line evictions caused by write allocation.
        cacheLinesTouched.insert(address_breakdown.tag);

        // Only increment statistics if this transaction hasn't already "touched"
        // the current cache line.
        if(transactionLines.find({address_breakdown.tag, address_breakdown.index}) != transactionLines.end()) {
            tx.write_miss++;
        }

    }
    // We perform writeback without demand paging, so no need to update for
    // entries that are not present.
    else {
        // If write is a miss with a No Write Allocation policy, do nothing
        // other than document there was a miss.

        // Only increment statistics if this transaction hasn't already "touched"
        // the current cache line.
        if(transactionLines.find({address_breakdown.tag, address_breakdown.index}) != transactionLines.end()) {
            tx.write_miss++;
        }
    }
    // Prevent line cache entry from being counted in statistics again.
    transactionLines.insert({address_breakdown.tag, address_breakdown.index});
    return memory_device->writeByte(address, value);
}

bool CacheMemory::getByte(quint16 address, quint8 &output) const
{
    return memory_device->getByte(address, output);
}

bool CacheMemory::setByte(quint16 address, quint8 value)
{
    return memory_device->setByte(address, value);
}

const QSet<quint16> CacheMemory::getCacheLinesTouched() const noexcept
{
    return cacheLinesTouched;
}

const QList<CacheEntry> CacheMemory::getEvictedEntry(quint16 line) const noexcept
{
    if(auto item = evictedLines.find(line);
            item != evictedLines.end()) {
        return *item;
    }
    return QList<CacheEntry>();
}

const QMap<quint16, QList<CacheEntry> > CacheMemory::getAllEvictedEntries() const noexcept
{
    return evictedLines;
}

const QSet<quint16> CacheMemory::getBytesWritten() const noexcept
{
    return memory_device->getBytesWritten();
}

const QSet<quint16> CacheMemory::getBytesSet() const noexcept
{
    return memory_device->getBytesSet();
}

void CacheMemory::clearCacheLinesTouched() noexcept
{
    cacheLinesTouched.clear();
}

void CacheMemory::clearEvictedEntry(quint16 line) noexcept
{
    evictedLines.remove(line);
}

void CacheMemory::clearAllEvictedEntries() noexcept
{
    evictedLines.clear();
}

void CacheMemory::clearBytesWritten() noexcept
{
    memory_device->clearBytesWritten();
}

void CacheMemory::clearBytesSet() noexcept
{
    memory_device->clearBytesSet();
}

void CacheMemory::clearAllByteCaches() noexcept
{
    clearBytesRead();
    clearBytesWritten();
    clearBytesSet();
    clearCacheLinesTouched();
    clearAllEvictedEntries();
}

void CacheMemory::clearStats()
{
    for(auto& a : stats) {
        a.clear();
    }
}

void CacheMemory::beginTransaction(AMemoryDevice::ACCESS_MODE mode) const
{
    assert(in_tx == false);
    in_tx = true;
    tx.clear();
    tx.transaction_mode = mode;
}

void CacheMemory::endTransaction() const
{
    assert(in_tx == true);
    in_tx = false;
    instruction_txs.append(tx);
    transactionLines.clear();
}


Cache::CacheAddress CacheMemory::breakdownAddress(quint16 address) const
{
    Cache::CacheAddress ret;
    // Offset is the lowest order bits of the address.
    // Do not compute statically, since these values depend on cache configuration.
    int data_mask = (0x1 << data_size) - 1, data_shift = 0;
    int index_mask = (0x1 << index_size) - 1, index_shift = data_size;
    int tag_mask = (0x1 << tag_size) - 1, tag_shift = (data_size + index_size);

    ret.offset = (address >> data_shift) & (data_mask);
    // Index is middle bits of address.
    ret.index = (address >> index_shift) & (index_mask);
    // Tag is highest order bits of address.
    ret.tag = (address >> tag_shift) & tag_mask;
    //qDebug().noquote() << QString("T%1 I%2 O%3").arg(ret.tag).arg(ret.index).arg(ret.offset);
    return ret;
}

std::optional<const CacheLine *> CacheMemory::getCacheLine(quint16 tag) const
{
    if(tag >= (1 << tag_size)) return std::nullopt;
    else return &cache[tag];

}

QString CacheMemory::getCacheAlgorithm() const
{
    return replace_factory->get_algorithm_name();
}

void Stat_Me::clear()
{
    this->read_hit = 0;
    this->read_miss = 0;
    this->write_hit = 0;
    this->write_miss = 0;
}
