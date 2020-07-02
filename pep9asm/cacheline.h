// File: cacheline.h
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
#ifndef CACHELINE_H
#define CACHELINE_H

#include <QObject>
#include <QVector>
#include <QSharedPointer>


#include "cache.h"

#include <optional>

class AReplacementPolicy;

/*
 * Class that contains all information needed to track a single cache entry within
 * a cache line.
 */
struct CacheEntry
{
    bool is_present = false;
    quint16 tag = 0;
    quint32 hit_count;
};

/*
 * A cache line consists of one-or-more cache entries, and a method to replace
 * those cache entries, called a cache replacement policy (CRP).
 */
class CacheLine
{
public:
    CacheLine();
    CacheLine(quint8 associativity, QSharedPointer<AReplacementPolicy> replacement_policy);
    // Check if an entry with the corresponding index is present in the line.
    bool contains_index(quint16 tag);
    // Notify the CRP that an index has been hit.
    void update(Cache::CacheAddress& address);
    // Insert the new index into the cache. Evict using CRP if needed.
    CacheEntry insert(Cache::CacheAddress& address);
    // Mark all entries in line as not-present and update replacement policy.
    void clear();

    // Request the cache entry at the i'th position in the line
    std::optional<const CacheEntry*> get_entry(quint16 position) const;
    // Access the replacement policy used for the line
    QSharedPointer<AReplacementPolicy> get_replacement_policy();
    QSharedPointer<const AReplacementPolicy> get_replacement_policy() const;
private:
    QVector<CacheEntry> entries;
    QSharedPointer<AReplacementPolicy> replacement_policy = nullptr;
};

#endif // CACHELINE_H
