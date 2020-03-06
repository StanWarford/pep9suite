#ifndef CACHELINE_H
#define CACHELINE_H

#include <QObject>
#include <QVector>
#include <QSharedPointer>

class AReplacementPolicy;

struct CacheEntry
{
    bool is_present = false;
    quint16 index = 0;
};

class CacheLine
{
public:
    CacheLine();
    CacheLine(quint8 associativity, QSharedPointer<AReplacementPolicy> replacement_policy);
    bool contains_index(quint16 index);
    // Notify the CRP that an index has been hit.
    void update(quint16 index);
    // Insert the new index into the cache. Evict using CRP if needed.
    void insert(quint16 index);
    // Mark all entries in line as not-present and update replacement policy.
    void clear();
private:
    QVector<CacheEntry> entries;
    QSharedPointer<AReplacementPolicy> replacement_policy = nullptr;
};

#endif // CACHELINE_H
