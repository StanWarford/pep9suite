// File: cache.h
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
