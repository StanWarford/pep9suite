#ifndef LRUREPLACE_H
#define LRUREPLACE_H

#include <random>
#include <deque>

#include <QMetaObject>
#include <QMetaEnum>
#include <QVector>
#include <QSharedPointer>

#include "cachereplace.h"

/*
 * Algorithms Summary
 */
namespace CacheAlgorithms{
    Q_NAMESPACE
    enum CacheAlgorithms {
        // Count
        LRU, MRU,
        LFU, MFU,
        FIFO,
        Random,
    };
    Q_ENUM_NS(CacheAlgorithms);
    static QMap<CacheAlgorithms, AReplacementFactory*> factory;
}
/*
 * Least/Most Recently Used (LRU/MRU) Replacement
 */

class RecentReplace : public AReplacementPolicy
{

protected:
    quint32 count;
    QVector<quint32> last_access;

public:
    typedef QVector<quint32>::const_iterator iterator;
    typedef std::function<iterator(iterator, iterator)> SelectFunction;
public:
    RecentReplace(quint16 size, SelectFunction element_select);
    virtual ~RecentReplace() override = 0 ;


    // AReplacementPolicy interface
public:
    void reference(quint16 index) override;
    quint16 evict() override;
    quint16 supports_evicition_lookahead() const override;
    quint16 eviction_loohahead() const override;
    QVector<quint16> eviction_loohahead(quint16 count) const override;
    void clear() override;

protected:
   SelectFunction element_select;
};

class LRUReplace : public RecentReplace
{
public:
    LRUReplace(quint16 size);
    virtual ~LRUReplace() override = default;

    QString get_algorithm_name() const override;
};

class MRUReplace : public RecentReplace
{
public:
    MRUReplace(quint16 size);
    virtual ~MRUReplace() override = default;

    QString get_algorithm_name() const override;
};

class LRUFactory : public AReplacementFactory
{
public:
    LRUFactory(quint16 associativity);
    ~LRUFactory() override = default;

    QSharedPointer<AReplacementPolicy> create_policy() override;
    QString get_algorithm_name() const override {return algorithm();}
    static const QString algorithm();
    static CacheAlgorithms::CacheAlgorithms algorithm_enum();
};

class MRUFactory : public AReplacementFactory
{
public:
    MRUFactory(quint16 associativity);
    ~MRUFactory() override = default;

    QSharedPointer<AReplacementPolicy> create_policy() override;
    QString get_algorithm_name() const override {return algorithm();}
    static const QString algorithm();
    static CacheAlgorithms::CacheAlgorithms algorithm_enum();
};

/*
 * Least/Most Frequently Used (LFU/MFU) Replacement
 */

class FrequencyReplace : public AReplacementPolicy
{

protected:
    QVector<quint32> access_count;

public:
    typedef QVector<quint32>::const_iterator iterator;
    typedef std::function<iterator(iterator, iterator)> SelectFunction;
public:
    FrequencyReplace(quint16 size, SelectFunction element_select);
    virtual ~FrequencyReplace() override = 0 ;


    // AReplacementPolicy interface
public:
    void reference(quint16 index) override;
    quint16 evict() override;
    quint16 supports_evicition_lookahead() const override;
    quint16 eviction_loohahead() const override;
    QVector<quint16> eviction_loohahead(quint16 count) const override;
    void clear() override;

private:
   SelectFunction element_select;
};

class LFUReplace : public FrequencyReplace
{
public:
    LFUReplace(quint16 size);
    virtual ~LFUReplace() override = default;

    QString get_algorithm_name() const override;
};

class MFUReplace : public FrequencyReplace
{
public:
    MFUReplace(quint16 size);
    virtual ~MFUReplace() override = default;

    QString get_algorithm_name() const override;
};

class LFUFactory : public AReplacementFactory
{
public:
    LFUFactory(quint16 associativity);
    ~LFUFactory() override = default;

    QSharedPointer<AReplacementPolicy> create_policy() override;
    QString get_algorithm_name() const override {return algorithm();}
    static const QString algorithm();
    static CacheAlgorithms::CacheAlgorithms algorithm_enum();
};

class MFUFactory : public AReplacementFactory
{
public:
    MFUFactory(quint16 associativity);
    ~MFUFactory() override = default;

    QSharedPointer<AReplacementPolicy> create_policy() override;
    QString get_algorithm_name() const override {return algorithm();}
    static const QString algorithm();
    static CacheAlgorithms::CacheAlgorithms algorithm_enum();
};

/*
 * Order based replacement algorithms.
 */

class FIFOReplace : public AReplacementPolicy
{

protected:
    quint16 size, next_victim;
public:
    FIFOReplace(quint16 size);
    virtual ~FIFOReplace() override = default;

    // AReplacementPolicy interface
public:
    void reference(quint16 index) override;
    QString get_algorithm_name() const override;
    quint16 evict() override;
    quint16 supports_evicition_lookahead() const override;
    quint16 eviction_loohahead() const override;
    QVector<quint16> eviction_loohahead(quint16 count) const override;
    void clear() override;
};

class FIFOFactory : public AReplacementFactory
{
public:
    FIFOFactory(quint16 associativity);
    ~FIFOFactory() override = default;

    QSharedPointer<AReplacementPolicy> create_policy() override;
    QString get_algorithm_name() const override {return algorithm();}
    static const QString algorithm();
    static CacheAlgorithms::CacheAlgorithms algorithm_enum();
};

/*
 * Random Replacement
 */
class RandomReplace : public AReplacementPolicy
{
public:
    RandomReplace(std::function<quint16()> get_random);
    virtual ~RandomReplace() override = default;

    // AReplacementPolicy interface
public:
    void reference(quint16 index) override;
    quint16 evict() override;
    quint16 supports_evicition_lookahead() const override;
    quint16 eviction_loohahead() const override;
    QVector<quint16> eviction_loohahead(quint16 count) const override;
    void clear() override;

private:
    // Support a single look-ahead for eviction.
    mutable bool has_next_random = false;
    mutable quint16 next_random = 0;
    std::function<quint16()> get_random;
    QString get_algorithm_name() const override;

};

class RandomFactory : public AReplacementFactory
{
public:
    RandomFactory(quint16 associativity);
    ~RandomFactory() override = default;

    QSharedPointer<AReplacementPolicy> create_policy() override;
    QString get_algorithm_name() const override;
    static const QString algorithm();
    static CacheAlgorithms::CacheAlgorithms algorithm_enum();
private:
    std::default_random_engine generator;
    std::uniform_int_distribution<quint16> distribution;
    std::function<quint16()> random_function;
};

#endif // LRUREPLACE_H
