#include "symbolvalue.h"



AbstractSymbolValue::AbstractSymbolValue()
{
}

AbstractSymbolValue::~AbstractSymbolValue()
{
}

SymbolValueEmpty::SymbolValueEmpty(): AbstractSymbolValue()
{
}

SymbolValueEmpty::~SymbolValueEmpty()
{
}

qint32 SymbolValueEmpty::getValue() const
{
    return qint32();
}

SymbolType SymbolValueEmpty::getSymbolType() const
{
    return SymbolType::EMPTY;
}

SymbolValueLocation::SymbolValueLocation(quint16 value):AbstractSymbolValue(),_value(value)
{
}


SymbolValueLocation::~SymbolValueLocation()
{
}

qint32 SymbolValueLocation::getValue() const
{
    return _value;
    
}

SymbolType SymbolValueLocation::getSymbolType() const
{
    return SymbolType::ADDRESS;
}

