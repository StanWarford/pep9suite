#include "cachealgs.h"
#include <algorithm>
#include <QDebug>
RecentReplace::RecentReplace(quint16 size, SelectFunction element_select)
: last_access(size, 0), count(0), element_select(element_select)
{

}

RecentReplace::~RecentReplace() = default;

void RecentReplace::reference(quint16 index)
{
    last_access[index] = count++;
}

quint16 RecentReplace::evict()
{
    auto index = element_select(last_access.begin(), last_access.end()) - last_access.begin();
    last_access[index] = count++;
    return index;
}

void RecentReplace::clear()
{
    count = 0;
    for(auto& index: last_access) {
        index = 0;
    }
}

LRUReplace::LRUReplace(quint16 size):
    RecentReplace(size, RecentReplace::SelectFunction(std::min_element<RecentReplace::iterator>))
{

}

MRUReplace::MRUReplace(quint16 size):
    RecentReplace(size, RecentReplace::SelectFunction(std::max_element<RecentReplace::iterator>))
{

}

LRUFactory::LRUFactory(quint16 associativity): AReplacementFactory(associativity)
{

}

QSharedPointer<AReplacementPolicy> LRUFactory::create_policy()
{
    return QSharedPointer<LRUReplace>::create(get_associativity());
}


MRUFactory::MRUFactory(quint16 associativity): AReplacementFactory(associativity)
{

}

QSharedPointer<AReplacementPolicy> MRUFactory::create_policy()
{
    return QSharedPointer<MRUReplace>::create(get_associativity());
}

/*
 * Frequency Replacement
 */
FrequencyReplace::FrequencyReplace(quint16 size, SelectFunction element_select)
: access_count(size, 0), element_select(element_select)
{

}

FrequencyReplace::~FrequencyReplace() = default;

void FrequencyReplace::reference(quint16 index)
{
    access_count[index]++;
}

quint16 FrequencyReplace::evict()
{
    auto index = element_select(access_count.begin(), access_count.end()) - access_count.begin();
    auto init_value = *std::min(access_count.begin(), access_count.end());
    access_count[index] = init_value;
    return index;
}

void FrequencyReplace::clear()
{
    for(auto& index: access_count) {
        index = 0;
    }
}

LFUReplace::LFUReplace(quint16 size):
    FrequencyReplace(size, FrequencyReplace::SelectFunction(std::min_element<FrequencyReplace::iterator>))
{

}

MFUReplace::MFUReplace(quint16 size):
    FrequencyReplace(size, FrequencyReplace::SelectFunction(std::max_element<FrequencyReplace::iterator>))
{

}

LFUFactory::LFUFactory(quint16 associativity): AReplacementFactory(associativity)
{

}

QSharedPointer<AReplacementPolicy> LFUFactory::create_policy()
{
    return QSharedPointer<LFUReplace>::create(get_associativity());
}


MFUFactory::MFUFactory(quint16 associativity): AReplacementFactory(associativity)
{

}

QSharedPointer<AReplacementPolicy> MFUFactory::create_policy()
{
    return QSharedPointer<MFUReplace>::create(get_associativity());
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
    return 0;
    //return get_random();
}

void RandomReplace::clear()
{
    // No-operation
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
