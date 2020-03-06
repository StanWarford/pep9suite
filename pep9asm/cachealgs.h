#ifndef LRUREPLACE_H
#define LRUREPLACE_H

#include <random>

#include <QVector>
#include <QSharedPointer>

#include "cachereplace.h"

/*
 * Least/Most Recently Used (LRU/MRU) Replacement
 */

class RecentReplace : public AReplacementPolicy
{

protected:
    quint32 count;
    QVector<quint32> last_access;

public:
    typedef QVector<quint32>::iterator iterator;
    typedef std::function<iterator(iterator, iterator)> SelectFunction;
public:
    RecentReplace(quint16 size, SelectFunction element_select);
    virtual ~RecentReplace() override = 0 ;


    // AReplacementPolicy interface
public:
    void reference(quint16 index) override;
    quint16 evict() override;
    void clear() override;

private:
   SelectFunction element_select;
};

class LRUReplace : public RecentReplace
{
public:
    LRUReplace(quint16 size);
    virtual ~LRUReplace() override = default;

    QString get_algorithm_name() const override {return "LRU";}
};

class MRUReplace : public RecentReplace
{
public:
    MRUReplace(quint16 size);
    virtual ~MRUReplace() override = default;

    QString get_algorithm_name() const override {return "MRU";}
};

class LRUFactory : public AReplacementFactory
{
public:
    LRUFactory(quint16 associativity);
    ~LRUFactory() override = default;

    QSharedPointer<AReplacementPolicy> create_policy() override;
};

class MRUFactory : public AReplacementFactory
{
public:
    MRUFactory(quint16 associativity);
    ~MRUFactory() override = default;

    QSharedPointer<AReplacementPolicy> create_policy() override;
};

/*
 * Least/Most Frequently Used (LFU/MFU) Replacement
 */

class FrequencyReplace : public AReplacementPolicy
{

protected:
    QVector<quint32> access_count;

public:
    typedef QVector<quint32>::iterator iterator;
    typedef std::function<iterator(iterator, iterator)> SelectFunction;
public:
    FrequencyReplace(quint16 size, SelectFunction element_select);
    virtual ~FrequencyReplace() override = 0 ;


    // AReplacementPolicy interface
public:
    void reference(quint16 index) override;
    quint16 evict() override;
    void clear() override;

private:
   SelectFunction element_select;
};

class LFUReplace : public FrequencyReplace
{
public:
    LFUReplace(quint16 size);
    virtual ~LFUReplace() override = default;

    QString get_algorithm_name() const override {return "LFU";}
};

class MFUReplace : public FrequencyReplace
{
public:
    MFUReplace(quint16 size);
    virtual ~MFUReplace() override = default;

    QString get_algorithm_name() const override {return "MFU";}
};

class LFUFactory : public AReplacementFactory
{
public:
    LFUFactory(quint16 associativity);
    ~LFUFactory() override = default;

    QSharedPointer<AReplacementPolicy> create_policy() override;
};

class MFUFactory : public AReplacementFactory
{
public:
    MFUFactory(quint16 associativity);
    ~MFUFactory() override = default;

    QSharedPointer<AReplacementPolicy> create_policy() override;
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
    QString get_algorithm_name() const override {return "Random";}
    quint16 evict() override;
    void clear() override;

private:
    std::function<quint16()> get_random;

};

class RandomFactory : public AReplacementFactory
{
public:
    RandomFactory(quint16 associativity);
    ~RandomFactory() override = default;

    QSharedPointer<AReplacementPolicy> create_policy() override;
private:
    std::default_random_engine generator;
    std::uniform_int_distribution<quint16> distribution;
    std::function<quint16()> random_function;
};

#endif // LRUREPLACE_H
