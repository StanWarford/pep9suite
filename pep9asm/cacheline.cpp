#include "cacheline.h"

#include <QDebug>

#include "cachealgs.h"
#include "cachememory.h"

CacheLine::CacheLine(): entries(0, CacheEntry()), replacement_policy(nullptr)
{

}

CacheLine::CacheLine(quint8 associativity, QSharedPointer<AReplacementPolicy> replacement_policy):
    entries(associativity, CacheEntry()), replacement_policy(replacement_policy)
{

}

bool CacheLine::contains_index(quint16 index)
{
    for(auto entry : entries) {
        if(entry.is_present && entry.index == index) return true;
    }
    return false;
}

void CacheLine::update(Cache::CacheAddress &address)
{
    for(int it = 0; it < entries.size(); it++) {
        auto& entry = entries[it];
        if(entry.is_present && entry.index == address.index) {
            // TODO: Perform cache replacement policy update on present entry.
            replacement_policy->reference(it);
            entry.hit_count++;
        }
    }
}

CacheEntry CacheLine::insert(Cache::CacheAddress &address)
{
    CacheEntry evicted_entry;
    // TODO: get replacement index for CRP.
    quint16 evict_index = replacement_policy->evict();

    evicted_entry = entries[evict_index];

    qDebug().noquote() << QString("Evicting bin %1 (containing %2) for index %3 in line %4")
                          .arg(evict_index)
                          .arg(entries[evict_index].is_present ? QString("%1").arg(entries[evict_index].index) : "NA")
                          .arg(address.index)
                          .arg(address.tag);
    entries[evict_index].is_present = true;
    entries[evict_index].index = address.index;
    entries[evict_index].hit_count = 1;

    return evicted_entry;
}

void CacheLine::clear()
{
    replacement_policy->clear();
    for(auto& entry : entries) {
        entry.is_present = false;
        entry.index = 0;
        entry.hit_count = 0;
    }
}

std::optional<const CacheEntry *> CacheLine::get_entry(quint16 position) const
{
    if(position >= (1 << entries.size())) return std::nullopt;
    else return &entries[position];
}

QSharedPointer<AReplacementPolicy> CacheLine::get_replacement_policy()
{
    return replacement_policy;
}


QSharedPointer<const AReplacementPolicy> CacheLine::get_replacement_policy() const
{
    return replacement_policy;
}
