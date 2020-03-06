#ifndef AREPLACEMENTFACTORY_H
#define AREPLACEMENTFACTORY_H

#include <QObject>

class AReplacementPolicy
{
public:
    AReplacementPolicy() = default;
    virtual ~AReplacementPolicy() = default;

    // Index is an index in the vector of cache entires.
    // It is NOT the index field in a memory address or cache entry.
    virtual void reference(quint16 index) = 0;

    virtual QString get_algorithm_name() const = 0;
    virtual quint16 evict() = 0;

    virtual void clear() = 0;
};

class AReplacementFactory
{
public:
    AReplacementFactory(quint16 associativity);
    virtual ~AReplacementFactory() = default;
    virtual QSharedPointer<AReplacementPolicy> create_policy() = 0;
    quint16 get_associativity() const;

private:
    quint16 associativity;
};

#endif // AREPLACEMENTFACTORY_H
