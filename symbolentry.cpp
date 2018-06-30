#include "symbolentry.h"
#include "symbolvalue.h"


SymbolEntry::SymbolEntry(SymbolTable::SymbolID symbolID, QString name) :_symbolID(symbolID), _name(name),
_value(std::make_shared<SymbolValueEmpty>()), _state(DefStates::UNDEFINED)
{
}

SymbolEntry::SymbolEntry(SymbolTable::SymbolID symbolID, QString name,
                         SymbolTable::AbstractSymbolValuePtr value) : _symbolID(symbolID), _name(name),
_value(nullptr)
{
    setValue(value);
}

SymbolEntry::~SymbolEntry()
{
}


void SymbolEntry::setValue(SymbolTable::AbstractSymbolValuePtr value)
{
	_value = value;
    if (dynamic_cast<SymbolValueEmpty*>(value.get()))
	{
        _state = DefStates::UNDEFINED;
	}
    else if(_state == DefStates::MULTIPLE)
    {
        _state = DefStates::MULTIPLE;
    }
	else
	{
        _state = DefStates::SINGLE;
	}
}

QString SymbolEntry::getName() const
{
	return _name;
}

bool SymbolEntry::isDefined() const
{
    return _state == DefStates::SINGLE;
}

bool SymbolEntry::isUndefined() const
{
    return _state == DefStates::UNDEFINED;
}

bool SymbolEntry::isMultiplyDefined() const
{
	return _state == DefStates::MULTIPLE;
}

void SymbolEntry::setMultiplyDefined()
{
    _state = DefStates::MULTIPLE;
}

SymbolTable::SymbolID SymbolEntry::getSymbolID() const
{
	return _symbolID;
}

qint32 SymbolEntry::getValue() const
{
	return _value->getValue();
}

SymbolTable::AbstractSymbolValuePtr SymbolEntry::getRawValue()
{
	return _value;
}
