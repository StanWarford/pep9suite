#include "typetags.h"
#include "symbolentry.h"
AType::~AType()
{

}

QList<QPair<Enu::ESymbolFormat, QString> > AType::toPrimitives(QString runningPrefix) const
{

}

PrimitiveType::PrimitiveType(QSharedPointer<const SymbolEntry> symbol, Enu::ESymbolFormat format):
    symbol(symbol), format(format)
{

}

PrimitiveType::~PrimitiveType()
{

}

QList<QPair<Enu::ESymbolFormat, QString> > PrimitiveType::toPrimitives(QString prefix) const
{
    return QList<QPair<Enu::ESymbolFormat, QString>>{{format, prefix+symbol->getName()}};
}

QString PrimitiveType::toString(QString prefix) const
{
    return QString("%1%2")
            .arg(prefix)
            .arg(QString(symbol->getName()),0);
}

quint16 PrimitiveType::size() const
{
    return Enu::tagNumBytes(format);
}

PrimitiveType::operator QString() const
{
    return toString();
}

StructType::StructType(QSharedPointer<const SymbolEntry> symbol, QList<QSharedPointer<AType> > members):
   symbol(symbol),  members(members)

{

}

StructType::~StructType()
{

}

QList<QPair<Enu::ESymbolFormat, QString> > StructType::toPrimitives(QString prefix) const
{
    auto out = QList<QPair<Enu::ESymbolFormat, QString>>();
    QString runningPrefix = QString("%1%2.")
            .arg(prefix)
            .arg(symbol->getName());
    for(auto member : members) {
        out.append(member->toPrimitives(runningPrefix));
    }
    return out;
}

QString StructType::toString(QString prefix) const
{
    QStringList entries;
    for (QSharedPointer<AType> ent : members) {
        entries.append(QString("%1")
                       .arg(ent->toString(prefix+symbol->getName()+".")));
    }
    return QString("struct %1%2{"+entries.join(", ")+"}")
            .arg(prefix)
            .arg(symbol->getName());
}

quint16 StructType::size() const
{
    quint16 size = 0;
    for(auto member : members) {
        size += member->size();
    }
    return size;
}

StructType::operator QString() const
{
    return toString("");
}

ArrayType::ArrayType(QSharedPointer<const SymbolEntry> symbol, Enu::ESymbolFormat format, quint16 len):
    symbol(symbol), format(format), len(len)
{

}

ArrayType::~ArrayType()
{

}

QList<QPair<Enu::ESymbolFormat, QString> > ArrayType::toPrimitives(QString prefix) const
{
    auto out = QList<QPair<Enu::ESymbolFormat, QString>>();
    QString runningPrefix = QString("%1%2")
            .arg(prefix)
            .arg(symbol->getName());
    for(int it=0; it< this->len; it++) {
        out.append({{format,QString("%1[%2]").arg(runningPrefix).arg(it)}});
    }
    return out;
}

QString ArrayType::toString(QString prefix) const
{
    return QString("%1%2[%3]")
            .arg(prefix)
            .arg(symbol->getName())
            .arg(len);
}

quint16 ArrayType::size() const
{
    return static_cast<quint16>(Enu::tagNumBytes(format) * len);
}

ArrayType::operator QString() const
{
    return toString();
}

QDebug operator<<(QDebug os, const QSharedPointer<AType> &item)
{
    return os.noquote().nospace()<< *item.get();
}

QDebug operator<<(QDebug os, const QSharedPointer<const AType> &item)
{
    return os.noquote().nospace()<< *item.get();
}

