#include "symbolentry.h"
#include "symbolvalue.h"


SymbolEntry::SymbolEntry(SymbolTable* parent, SymbolTable::SymbolID symbolID, QString name):symbolID(symbolID), name(name),
symbolValue(QSharedPointer<SymbolValueEmpty>::create()), definedState(DefStates::UNDEFINED), parent(parent), _format()
{
}

SymbolEntry::SymbolEntry(SymbolTable* parent, SymbolTable::SymbolID symbolID, QString name,
                         SymbolTable::AbstractSymbolValuePtr value): symbolID(symbolID), name(name), parent(parent),
    symbolValue(nullptr), _format()
{
    setValue(value);
}

SymbolEntry::~SymbolEntry()
{
}

const SymbolTable* SymbolEntry::getParentTable() const
{
    return parent;
}


void SymbolEntry::setValue(SymbolTable::AbstractSymbolValuePtr value)
{
    //This function will not try to decide if a value is multiply defined based on if the value is already singlely defined.
    //This is because the owning SymbolTable might need to update the value of a symbol (code re-alignment, code re-ordering),
    //and so it doesn't make sense for the decision for this symbol to be multiply defined or not to be made here.
	symbolValue = value;
    //If given an empty value, then the symbol is undefined
    if (dynamic_cast<SymbolValueEmpty*>(value.data()))
	{
        definedState = DefStates::UNDEFINED;
	}
    //If the symbol is multiply defined, it remains multiply defined
    else if(definedState == DefStates::MULTIPLE)
    {
        definedState = DefStates::MULTIPLE;
    }
	else
	{
        definedState = DefStates::SINGLE;
	}
}

QString SymbolEntry::getName() const
{
	return name;
}

bool SymbolEntry::isDefined() const
{
    return definedState == DefStates::SINGLE;
}

bool SymbolEntry::isUndefined() const
{
    return definedState == DefStates::UNDEFINED;
}

bool SymbolEntry::isMultiplyDefined() const
{
	return definedState == DefStates::MULTIPLE;
}

void SymbolEntry::setMultiplyDefined()
{
    definedState = DefStates::MULTIPLE;
}

SymbolTable::SymbolID SymbolEntry::getSymbolID() const
{
	return symbolID;
}

qint32 SymbolEntry::getValue() const
{
	return symbolValue->getValue();
}

SymbolTable::AbstractSymbolValuePtr SymbolEntry::getRawValue()
{
    return symbolValue;
}

void SymbolEntry::setSymbolFormat(SymbolFormat format)
{
    _format = std::move(format);
}

const SymbolFormat &SymbolEntry::getSymbolFormat() const
{
    return _format;
}
