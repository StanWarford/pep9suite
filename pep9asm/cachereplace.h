#ifndef AREPLACEMENTFACTORY_H
#define AREPLACEMENTFACTORY_H

#include <QObject>

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
    // Return the next "count" items to be evicted from the cache.
    // If count is greater than supports_evicition_lookahead(), the smaller of the two will be returned.
    // The returned vector is the indicies of the cache entries to be evicted, sorted
    // from soonesrt eviction to latest eviction.
    virtual QVector<quint16> eviction_loohahead(quint16 count) const = 0;

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
