// File: cachereplace.h
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
#ifndef AREPLACEMENTFACTORY_H
#define AREPLACEMENTFACTORY_H

#include <QObject>

/*
 * Abstract base class that allows you to implement a cache replacement policy for a line.
 * In order to create a new cache replacement policy,
 */
class AReplacementPolicy
{
public:
    AReplacementPolicy() = default;
    virtual ~AReplacementPolicy() = default;

    // Index is an index in the vector of cache entries.
    // It is NOT the index field in a memory address or cache entry.
    virtual void reference(quint16 index) = 0;

    virtual QString get_algorithm_name() const = 0;
    // Return the index of the entry in the cache line to be evicted.
    virtual quint16 evict() = 0;
    // How far ahead can eviction predicitions be made?
    // For example, random evicition has a lookahead of 1.
    virtual quint16 supports_evicition_lookahead() const = 0;
    // Return the next item to be evicted from the cache.
    virtual quint16 eviction_loohahead() const = 0;
    // Return the next "count" items to be evicted from the cache.
    // If count is greater than supports_evicition_lookahead(), the smaller of the two will be returned.
    // The returned vector is the indicies of the cache entries to be evicted, sorted
    // from soonesrt eviction to latest eviction.
    virtual QVector<quint16> eviction_loohahead(quint16 count) const = 0;

    // Report if the algorithm supports dynamic aging, and provide the interface to age (if applicable).
    virtual bool canAge() const {return false;}
    virtual void age() {}

    virtual void clear() = 0;
};

/*
 * Abstract factory to construct objects that manage the eviction policies for an individual cache line.
 */
class AReplacementFactory
{
public:
    AReplacementFactory(quint16 associativity);
    virtual ~AReplacementFactory() = default;
    // Create an object to manage eviction for a cache line.
    virtual QSharedPointer<AReplacementPolicy> create_policy() = 0;
    // Return a string name describing the caching algorithm used.
    virtual QString get_algorithm_name() const = 0;
    // How many entries are to be used be cache line?
    quint16 get_associativity() const;

protected:
    quint16 associativity;
};

#endif // AREPLACEMENTFACTORY_H
