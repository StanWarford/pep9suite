// File: cacheline.cpp
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
    // CRP tells us what item we need to evict next.
    quint16 evict_index = replacement_policy->evict();

    evicted_entry = entries[evict_index];

    // Print out what is being evicted, and what is swapping in.
    /*qDebug().noquote() << QString("Evicting bin %1 (containing %2) for index %3 in line %4")
                          .arg(evict_index)
                          .arg(entries[evict_index].is_present ? QString("%1").arg(entries[evict_index].index) : "NA")
                          .arg(address.index)
                          .arg(address.tag);*/
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
