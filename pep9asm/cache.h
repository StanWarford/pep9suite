#ifndef CACHE_H
#define CACHE_H

#include <QObject>
#include <QSharedPointer>

class AReplacementFactory;
namespace Cache {
    Q_NAMESPACE
    struct CacheAddress
    {
        quint16 tag, index, offset;
    };

    enum class WriteAllocationPolicy{
        WriteAllocate, NoWriteAllocate
    };
    Q_ENUM_NS(WriteAllocationPolicy);

    struct CacheConfiguration {
        // Number of addressing bits used for the tag, index and data.
        quint16 tag_bits, index_bits, data_bits;
        quint16 associativity;
        WriteAllocationPolicy write_allocation = WriteAllocationPolicy::NoWriteAllocate;
        QSharedPointer<AReplacementFactory> policy;
    };

}
#endif // CACHE_H
