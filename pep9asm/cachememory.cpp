#include "cachememory.h"
#include <cmath>
#include <QDebug>
CacheMemory::CacheMemory(QSharedPointer<MainMemory> memory_device, CacheConfiguration config,
                         QObject* parent):
    AMemoryDevice(parent), memory_device(memory_device), replace_factory(config.policy),
    tag_size(config.tag_bits), index_size(config.index_bits),
    data_size(16 - tag_size - index_size), associativity(config.associativity)
{
    // Make sure that cache won't accidentally go outside the range of accessible memory
    assert(safe_configuration(maxAddress(), tag_size, index_size, data_size, associativity));

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

bool CacheMemory::safe_configuration(quint32 memory_size, quint16 tag_size,
                                     quint16 index_size, quint16 data_size, quint16 associativity)
{
    bool size_match = tag_size+index_size+data_size == (int)ceil(log2(memory_size));
    bool used_associativty = (1 << index_size) >= associativity;
    return size_match & used_associativty;
}

quint16 CacheMemory::get_tag_size() const
{
    return tag_size;
}

quint16 CacheMemory::get_index_size() const
{
    return index_size;
}

quint16 CacheMemory::get_associativty() const
{
    return associativity;
}

quint16 CacheMemory::get_data_size() const
{
    return data_size;
}

void CacheMemory::resize_cache(CacheConfiguration config)
{
    // No operation for now.
    //cache.resize()
}

void CacheMemory::clearCache()
{
    for(auto& line : cache) {
        line.clear();
    }

    hit_read = 0;
    miss_read = 0;
    writes = 0;
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

bool CacheMemory::readByte(quint16 address, quint8 &output) const
{
    addressesTouched.insert(address);
    auto addr = breakdown_address(address);

    // If address is present in line, notify CRP of a hit.
    if(CacheLine& line = cache[addr.tag];
            line.contains_index(addr.index)) {
        qDebug().noquote() << QString("Hit %1\t").arg(address);
        line.update(addr.index);
        hit_read++;
    }
    // Otherwise must insert index associated with line.
    else {
        qDebug().noquote() << QString("Miss %1\t").arg(address);
        line.insert(addr.index);
        miss_read++;
    }

    return memory_device->readByte(address, output);
}

bool CacheMemory::writeByte(quint16 address, quint8 value)
{
    auto addr = breakdown_address(address);

    // If address is present in line, notify CRP of a hit.
    if(auto line = cache[addr.tag];
            line.contains_index(addr.index)) {
        line.update(addr.index);
        writes++;
    }
    // We perform writeback without demand paging, so no need to update for
    // entries that are not present.
    else {
        writes++;
    }

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

const QSet<quint16> CacheMemory::getBytesRead() const noexcept
{
    return addressesTouched;
}

const QSet<quint16> CacheMemory::getBytesWritten() const noexcept
{
    return memory_device->getBytesWritten();
}

const QSet<quint16> CacheMemory::getBytesSet() const noexcept
{
    return memory_device->getBytesSet();
}

void CacheMemory::clearBytesRead() noexcept
{
    addressesTouched.clear();
}

void CacheMemory::clearBytesWritten() noexcept
{
    memory_device->clearBytesWritten();
}

void CacheMemory::clearBytesSet() noexcept
{
    memory_device->clearBytesSet();
}

AddressBreakdown CacheMemory::breakdown_address(quint16 address) const
{
    AddressBreakdown ret;
    // Offset is the lowest order bits of the address
    static int data_mask = (0x1<<data_size) - 1, data_shift=0;
    static int index_mask = (0x1 << index_size) - 1, index_shift=data_size;
    static int tag_mask = (0x1 << tag_size) - 1, tag_shift=(data_size + index_size);

    ret.offset = (address >> data_shift) & (data_mask);
    // Index is middle bits of address.
    ret.index = (address >> index_shift) & (index_mask);
    // Tag is highest order bits of address.
    ret.tag = (address >> tag_shift) & tag_mask;
    //qDebug().noquote() << QString("T%1 I%2 O%3").arg(ret.tag).arg(ret.index).arg(ret.offset);
    return ret;
}

std::optional<const CacheLine *> CacheMemory::get_cache_line(quint16 tag) const
{
    if(tag >= (1 << tag_size)) return std::nullopt;
    else return &cache[tag];

}

QString CacheMemory::get_cache_algorithm() const
{
    return replace_factory->get_algorithm_name();
}
