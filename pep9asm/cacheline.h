#ifndef CACHELINE_H
#define CACHELINE_H

#include <QObject>
#include <QVector>
#include <QSharedPointer>


#include "cache.h"

class AReplacementPolicy;

struct CacheEntry
{
    bool is_present = false;
    quint16 index = 0;
    quint32 hit_count;
};

class CacheLine
{
public:
    CacheLine();
    CacheLine(quint8 associativity, QSharedPointer<AReplacementPolicy> replacement_policy);
    bool contains_index(quint16 index);
    // Notify the CRP that an index has been hit.
    void update(Cache::CacheAddress& address);
    // Insert the new index into the cache. Evict using CRP if needed.
    CacheEntry insert(Cache::CacheAddress& address);
    // Mark all entries in line as not-present and update replacement policy.
    void clear();

    std::optional<const CacheEntry*> get_entry(quint16 position) const;
    QSharedPointer<const AReplacementPolicy> get_replacement_policy() const;
private:
    QVector<CacheEntry> entries;
    QSharedPointer<AReplacementPolicy> replacement_policy = nullptr;
};

#endif // CACHELINE_H
