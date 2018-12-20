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

SymbolValueNumeric::SymbolValueNumeric(quint16 value): _value(value)
{

}

SymbolValueNumeric::~SymbolValueNumeric()
{

}

void SymbolValueNumeric::setValue(quint16 value)
{
    _value = value;
}

qint32 SymbolValueNumeric::getValue() const
{
    return _value;
}

SymbolType SymbolValueNumeric::getSymbolType() const
{
    return SymbolType::NUMERIC_CONSTANT;
}

SymbolValueLocation::SymbolValueLocation(quint16 value):AbstractSymbolValue(), _base(value), _offset(0)
{
}

SymbolValueLocation::~SymbolValueLocation()
{
}

void SymbolValueLocation::setBase(quint16 value)
{
    _base = value;
}

void SymbolValueLocation::setOffset(quint16 value)
{
    _offset = value;
}

qint32 SymbolValueLocation::getValue() const
{
    return _base + _offset;

}

SymbolType SymbolValueLocation::getSymbolType() const
{
    return SymbolType::ADDRESS;
}

bool SymbolValueLocation::canRelocate() const
{
    return true;
}

quint16 SymbolValueLocation::getOffset() const
{
    return _offset;
}

quint16 SymbolValueLocation::getBase() const
{
    return _base;
}
