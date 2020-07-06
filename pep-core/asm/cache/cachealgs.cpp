// File: cachealg.cpp
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

#include "cachealgs.h"
#include <algorithm>
#include <QDebug>

QSharedPointer<AReplacementFactory> getPolicyFactory(CacheAlgorithms::CacheAlgorithms alg, quint16 size)
{
    using algorithm = CacheAlgorithms::CacheAlgorithms;
    QSharedPointer<AReplacementFactory> retVal;
    switch(alg){
    case algorithm::LRU:
        retVal = QSharedPointer<LRUFactory>::create(size);
        break;
    case algorithm::MRU:
        retVal = QSharedPointer<MRUFactory>::create(size);
        break;
    case algorithm::BPLRU:
        retVal = QSharedPointer<BPLRUFactory>::create(size);
        break;
    case algorithm::LFU:
        retVal = QSharedPointer<LFUFactory>::create(size);
        break;
    case algorithm::LFUDA:
        retVal = QSharedPointer<LFUDAFactory>::create(size);
        break;
    case algorithm::MFU:
        retVal = QSharedPointer<MFUFactory>::create(size);
        break;
    case algorithm::FIFO:
        retVal = QSharedPointer<FIFOFactory>::create(size);
        break;
    case algorithm::Random:
        retVal = QSharedPointer<RandomFactory>::create(size);
        break;
    }
    return retVal;
}


#include <deque>
RecentReplace::RecentReplace(quint16 size, SelectFunction element_select)
    : count(0), last_access(size, {false, 0}), index_last(-1), element_select(element_select)
{

}

void RecentReplace::age()
{
    // Re-number cache entries is gaurenteed to preserve ordering of memory accesses.
    // For derived replacement algorithms, the magnitude of the difference in access times
    // is considered to be irrelevant. If relative magnitudes must be preserved,
    // subclasses must implement own aging algorithm.
    if(count > renumber_threshold) {
        // Last access time is unique, so no need to de-duplicate data first
        QList<quint32> asList;
        for(auto & [present, count] : last_access) {
            asList.append(count);
        }
        // Order access times from least to greatest.
        std::sort(asList.begin(), asList.end());
        // Construct a mapping from old times to new times, preserving order.
        quint32 start=0;
        std::map<quint32, quint32> renumber;
        for(auto val : asList) {
            renumber.insert({val, start++});
        }
        // Re-number access times, and preserve if the value is present or not.
        for(int it=0; it<last_access.size();it++) {
            last_access[it] = {std::get<0>(last_access[it]), renumber[std::get<1>(last_access[it])]};
        }
        // Reset counter.
        count = start;
    }

}

RecentReplace::~RecentReplace() = default;

void RecentReplace::reference(quint16 index)
{
    // If re-accessing MRU element, then there is no need to update
    // count, as MRU item will remain the MRU. This will help prevent overflow
    // when combined with aging.
    if(index != index_last) {
        last_access[index] = {true, count++};
        index_last=index;
    }
}

quint16 RecentReplace::evict()
{
    auto index = eviction_loohahead();
    last_access[index] = {true, count++};
    return index;
}

quint16 RecentReplace::supports_evicition_lookahead() const
{
    return 1;
}

quint16 RecentReplace::eviction_loohahead() const
{
    using item = std::tuple<bool, quint32>;

    // Determine if any "not present" items are still in the cache.
    // If so, evict the first one.
    auto index_absent = std::find_if(last_access.cbegin(), last_access.cend(), [](const item&it){return !std::get<0>(it);});
    // Notice the trick of subtracting the cbegin iterator. This converts from an iterator to an index.
    if(index_absent != last_access.cend()) return static_cast<quint16>(index_absent - last_access.cbegin());
    // Otherwise, evict the first element that matches our selection criteria.
    auto index = element_select(last_access.cbegin(), last_access.cend()) - last_access.cbegin();
    return static_cast<quint16>(index);
}

QVector<quint16> RecentReplace::eviction_loohahead(quint16 /*count*/) const
{
    return QVector<quint16>{eviction_loohahead()};
}

void RecentReplace::clear()
{
    count = 0;
    for(auto& index: last_access) {
        index = {false, 0};
    }
}

LRUReplace::LRUReplace(quint16 size):
    RecentReplace(size, RecentReplace::SelectFunction(std::min_element<RecentReplace::iterator>))
{

}

QString LRUReplace::get_algorithm_name() const
{
    return LRUFactory::algorithm();
}

MRUReplace::MRUReplace(quint16 size):
    RecentReplace(size, RecentReplace::SelectFunction(std::max_element<RecentReplace::iterator>))
{

}

QString MRUReplace::get_algorithm_name() const
{
    return MRUFactory::algorithm();
}


LRUFactory::LRUFactory(quint16 associativity): AReplacementFactory(associativity)
{

}

QSharedPointer<AReplacementPolicy> LRUFactory::create_policy()
{
    return QSharedPointer<LRUReplace>::create(get_associativity());
}

const QString LRUFactory::algorithm()
{
    QMetaObject meta = CacheAlgorithms::staticMetaObject;
    QMetaEnum metaEnum = meta.enumerator(meta.indexOfEnumerator("CacheAlgorithms"));
    return QString(metaEnum.key(CacheAlgorithms::LRU));
}

CacheAlgorithms::CacheAlgorithms LRUFactory::algorithm_enum()
{
    return CacheAlgorithms::LRU;
}

MRUFactory::MRUFactory(quint16 associativity): AReplacementFactory(associativity)
{

}

QSharedPointer<AReplacementPolicy> MRUFactory::create_policy()
{
    return QSharedPointer<MRUReplace>::create(get_associativity());
}

const QString MRUFactory::algorithm()
{
    QMetaObject meta = CacheAlgorithms::staticMetaObject;
    QMetaEnum metaEnum = meta.enumerator(meta.indexOfEnumerator("CacheAlgorithms"));
    return QString(metaEnum.key(CacheAlgorithms::MRU));
}

CacheAlgorithms::CacheAlgorithms MRUFactory::algorithm_enum()
{
    return CacheAlgorithms::MRU;
}

/*
 * Frequency Replacement
 */
FrequencyReplace::FrequencyReplace(quint16 size, SelectFunction element_select, InitFunction init_function)
: access_count(size, 0), element_select(element_select), init_function(init_function)
{

}

FrequencyReplace::~FrequencyReplace() = default;

void FrequencyReplace::reference(quint16 index)
{
    access_count[index]++;
}

quint16 FrequencyReplace::evict()
{
    quint16 location = eviction_loohahead();
    auto init_value = init_function(access_count.cbegin(), access_count.cend()); //*std::min(access_count.begin(), access_count.end());
    access_count[location] = init_value;
    return location;
}

quint16 FrequencyReplace::supports_evicition_lookahead() const
{
    return 1;
}

quint16 FrequencyReplace::eviction_loohahead() const
{
    quint16 location;
    // If an entry hasn't been referenced, then it is not present.
    if(auto zero_pos = std::find(access_count.cbegin(), access_count.cend(), 0);
            zero_pos != access_count.cend()) {
        location = zero_pos - access_count.cbegin();
    }
    // Otherwise we must evict a real cache entry instead of a not-present entry.
    else {
        location = element_select(access_count.cbegin(), access_count.cend()) - access_count.cbegin();
    }
    return location;
}

QVector<quint16> FrequencyReplace::eviction_loohahead(quint16 /*count*/) const
{

    return QVector<quint16>{eviction_loohahead()};
}

void FrequencyReplace::clear()
{
    for(auto& index: access_count) {
        index = 0;
    }
}

LFUReplace::LFUReplace(quint16 size):
    FrequencyReplace(size,
                     FrequencyReplace::SelectFunction(std::min_element<FrequencyReplace::iterator>),
                     FrequencyReplace::InitFunction([](FrequencyReplace::iterator, FrequencyReplace::iterator){return 0;}))
{

}

QString LFUReplace::get_algorithm_name() const
{
    return LFUFactory::algorithm();
}

LFUDAReplace::LFUDAReplace(quint16 size, quint16 age_steps):
    FrequencyReplace(size,
                     FrequencyReplace::SelectFunction(std::min_element<FrequencyReplace::iterator>),
                     FrequencyReplace::InitFunction([](FrequencyReplace::iterator start, FrequencyReplace::iterator end){
                        return *std::min_element(start, end);})),
    age_after(age_steps)
{

}

QString LFUDAReplace::get_algorithm_name() const
{
    return LFUDAFactory::algorithm();
}

void LFUDAReplace::age()
{
    timer++;

    // If the timer has not reached the next aging epoch, don't age yet.
    if(timer <= age_after) return;

    // Reset timer so it is below the aging threshold.
    timer = 0;
    auto min = *std::min_element(access_count.begin(), access_count.end());
    if(min == 0) return;
    // Access counts will tend toward infinity, since the min is constantly raised.
    // Subtraction preservers ordering, and normalizes the "least" accessed element to 0.
    for(auto & value : access_count) {
        value -= min;
    }
}

MFUReplace::MFUReplace(quint16 size):
    FrequencyReplace(size,
                     FrequencyReplace::SelectFunction(std::max_element<FrequencyReplace::iterator>),
                     FrequencyReplace::InitFunction([](FrequencyReplace::iterator, FrequencyReplace::iterator){return 0;}))
{

}

QString MFUReplace::get_algorithm_name() const
{
    return MFUFactory::algorithm();
}

LFUFactory::LFUFactory(quint16 associativity): AReplacementFactory(associativity)
{

}

QSharedPointer<AReplacementPolicy> LFUFactory::create_policy()
{
    return QSharedPointer<LFUReplace>::create(get_associativity());
}

const QString LFUFactory::algorithm()
{
    QMetaObject meta = CacheAlgorithms::staticMetaObject;
    QMetaEnum metaEnum = meta.enumerator(meta.indexOfEnumerator("CacheAlgorithms"));
    return QString(metaEnum.key(CacheAlgorithms::LFU));
}

CacheAlgorithms::CacheAlgorithms LFUFactory::algorithm_enum()
{
    return CacheAlgorithms::LFU;
}

LFUDAFactory::LFUDAFactory(quint16 associativity, quint16 age_after): AReplacementFactory(associativity),
    age_after(age_after)
{

}

QSharedPointer<AReplacementPolicy> LFUDAFactory::create_policy()
{
    return QSharedPointer<LFUDAReplace>::create(get_associativity(), age_after);
}

const QString LFUDAFactory::algorithm()
{
    QMetaObject meta = CacheAlgorithms::staticMetaObject;
    QMetaEnum metaEnum = meta.enumerator(meta.indexOfEnumerator("CacheAlgorithms"));
    return QString(metaEnum.key(CacheAlgorithms::LFUDA));
}

CacheAlgorithms::CacheAlgorithms LFUDAFactory::algorithm_enum()
{
    return CacheAlgorithms::LFUDA;
}


MFUFactory::MFUFactory(quint16 associativity): AReplacementFactory(associativity)
{

}

QSharedPointer<AReplacementPolicy> MFUFactory::create_policy()
{
    return QSharedPointer<MFUReplace>::create(get_associativity());
}

const QString MFUFactory::algorithm()
{
    QMetaObject meta = CacheAlgorithms::staticMetaObject;
    QMetaEnum metaEnum = meta.enumerator(meta.indexOfEnumerator("CacheAlgorithms"));
    return QString(metaEnum.key(CacheAlgorithms::MFU));
}

CacheAlgorithms::CacheAlgorithms MFUFactory::algorithm_enum()
{
    return CacheAlgorithms::MFU;
}

/*
 * Order Replacement
 */

FIFOReplace::FIFOReplace(quint16 size)
: size(size), next_victim(0)
{

}

void FIFOReplace::reference(quint16 /*index*/)
{
    // NOP
}

quint16 FIFOReplace::evict()
{
    auto victim = next_victim;
    next_victim = (next_victim + 1) % size;
    return victim;
}

quint16 FIFOReplace::supports_evicition_lookahead() const
{
    return size;
}

quint16 FIFOReplace::eviction_loohahead() const
{
    return next_victim;
}

QVector<quint16> FIFOReplace::eviction_loohahead(quint16 count) const
{
    auto items = next_victim;

    if(count>size) count = size;
    auto retVal = QVector<quint16>();
    retVal.resize(count);

    for(int it=0; it<count; it++)
    {
        retVal[it] = items;
        items = (items+1) % size;
    }
    return retVal;
}

void FIFOReplace::clear()
{
    next_victim = 0;
}

QString FIFOReplace::get_algorithm_name() const
{
    return FIFOFactory::algorithm();
}


FIFOFactory::FIFOFactory(quint16 associativity): AReplacementFactory(associativity)
{

}

QSharedPointer<AReplacementPolicy> FIFOFactory::create_policy()
{
    return QSharedPointer<FIFOReplace>::create(get_associativity());
}

const QString FIFOFactory::algorithm()
{
    QMetaObject meta = CacheAlgorithms::staticMetaObject;
    QMetaEnum metaEnum = meta.enumerator(meta.indexOfEnumerator("CacheAlgorithms"));
    return QString(metaEnum.key(CacheAlgorithms::FIFO));
}

CacheAlgorithms::CacheAlgorithms FIFOFactory::algorithm_enum()
{
    return CacheAlgorithms::FIFO;
}

/*
 * Random Replacement Policies
 */
RandomReplace::RandomReplace(std::function<quint16 ()> get_random):
    get_random(get_random)
{
}

void RandomReplace::reference(quint16 /*index*/)
{
    // No-operation
}

quint16 RandomReplace::evict()
{
    if(has_next_random) {
        has_next_random = false;
        return next_random;
    }
    else return get_random();
}

quint16 RandomReplace::supports_evicition_lookahead() const
{
    return  1;
}

quint16 RandomReplace::eviction_loohahead() const
{
    if(!has_next_random) {
        has_next_random = true;
        next_random = get_random();
    }
    return next_random;
}

QVector<quint16> RandomReplace::eviction_loohahead(quint16 /*count*/) const
{
    return QVector<quint16>{eviction_loohahead()};
}

void RandomReplace::clear()
{
    has_next_random = false;
    // No-operation
}

QString RandomReplace::get_algorithm_name() const {
    return RandomFactory::algorithm();
}

RandomFactory::RandomFactory(quint16 associativity): AReplacementFactory(associativity)
{
    distribution = std::uniform_int_distribution<quint16>(0, associativity -1);
    random_function = std::function<quint16()>([&](){return distribution(generator);});
}

QSharedPointer<AReplacementPolicy> RandomFactory::create_policy()
{
    return QSharedPointer<RandomReplace>::create(random_function);
}

QString RandomFactory::get_algorithm_name() const
{
    return algorithm();
}

const QString RandomFactory::algorithm()
{
    QMetaObject meta = CacheAlgorithms::staticMetaObject;
    QMetaEnum metaEnum = meta.enumerator(meta.indexOfEnumerator("CacheAlgorithms"));
    return QString(metaEnum.key(CacheAlgorithms::Random));
}

CacheAlgorithms::CacheAlgorithms RandomFactory::algorithm_enum()
{
    return CacheAlgorithms::Random;
}


BPLRU::BPLRU(quint16 associativity, std::function<quint16 ()> &evict_left, std::function<quint16 ()> evict_right):
    AReplacementPolicy(), associativity(associativity), side(MRUSide::LEFT),
    evict_left(evict_left), evict_right(evict_right)
{

}

void BPLRU::reference(quint16 index)
{
    // If the index is on the left side, protect the right side.
    auto old_side = side;
    if(index < associativity/2) {
        side = MRUSide::LEFT;
    } else {
        side = MRUSide::RIGHT;
    }
    if(old_side != side) has_next_random = false;
}

QString BPLRU::get_algorithm_name() const
{
    return BPLRUFactory::algorithm();
}

quint16 BPLRU::evict()
{
    quint16 index = eviction_loohahead();
    has_next_random = false;

    // If the index is on the left side, protect the right side.
    if(index < associativity/2) {
        side = MRUSide::LEFT;
    } else {
        side = MRUSide::RIGHT;
    }
    return index;
}

quint16 BPLRU::supports_evicition_lookahead() const
{
    return 1;
}

quint16 BPLRU::eviction_loohahead() const
{
    if(has_next_random) return next_random;
    else if(side == MRUSide::RIGHT) {
        has_next_random = true;
        next_random = evict_left();
    } else if(side == MRUSide::LEFT) {
        has_next_random = true;
        next_random = evict_right();
    }
    return next_random;
}

QVector<quint16> BPLRU::eviction_loohahead(quint16 /*count*/) const
{
    return QVector<quint16>{eviction_loohahead()};
}

void BPLRU::clear()
{
    side = MRUSide::RIGHT;
    has_next_random = false;
}

BPLRUFactory::BPLRUFactory(quint16 associativity): AReplacementFactory(associativity)
{
    // Use two separate random functions, since there is no guarantee that there is
    // symetry between the number of cache entries on the right half and the left half.
    evict_left = std::uniform_int_distribution<quint16>(0, (associativity/2) -1);
    qDebug() << 0 << (associativity/2) -1;
    evict_right = std::uniform_int_distribution<quint16>((associativity/2), associativity -1);
    qDebug() << (associativity/2) << associativity - 1;

    rand_left = std::function<quint16()>([&](){return evict_left(generator);});
    rand_right = std::function<quint16()>([&](){return evict_right(generator);});
}

QSharedPointer<AReplacementPolicy> BPLRUFactory::create_policy()
{
    return QSharedPointer<BPLRU>::create(get_associativity(), rand_left, rand_right);
}

QString BPLRUFactory::get_algorithm_name() const
{
    return algorithm();
}

const QString BPLRUFactory::algorithm()
{
    QMetaObject meta = CacheAlgorithms::staticMetaObject;
    QMetaEnum metaEnum = meta.enumerator(meta.indexOfEnumerator("CacheAlgorithms"));
    return QString(metaEnum.key(algorithm_enum()));
}

CacheAlgorithms::CacheAlgorithms BPLRUFactory::algorithm_enum()
{
    return CacheAlgorithms::BPLRU;
}
