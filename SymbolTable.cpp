#include "SymbolTable.h"
#include "SymbolEntry.h"
//#include "AbstractSymbolValue.h"
typedef qint32 SymbolID;
typedef std::shared_ptr<SymbolEntry> SymbolEntryPtr;
typedef std::shared_ptr<AbstractSymbolValue> AbstractSymbolValuePtr;
SymbolID SymbolTable::_nextUserSymbolID = 0;
SymbolID SymbolTable::getNextUserSymbolID()
{
	QAtomicInt newSymbolID = ++_nextUserSymbolID;
	return newSymbolID;
}

SymbolTable::SymbolTable():_symbolDictionary(), _symbolLookup()
{
}

SymbolTable::~SymbolTable()
{
}

SymbolEntryPtr SymbolTable::getValue(SymbolID symbolID) const
{

	return _symbolDictionary.find(symbolID).value();
}

SymbolEntryPtr SymbolTable::getValue(const QString & symbolName) const
{
	return getValue(_symbolLookup.find(symbolName).value());
}

SymbolEntryPtr SymbolTable::insertSymbol(const QString & symbolName)
{
	if (exists(symbolName))
	{
        //Handle multiple definitions
	}
	SymbolID id = SymbolTable::getNextUserSymbolID();
	_symbolLookup.insert(symbolName, id);
    auto val = _symbolDictionary.insert(id, SymbolEntryPtr::make_shared(id,symbolName));
	return val.value();
}

SymbolEntryPtr SymbolTable::setValue(SymbolID symbolID, AbstractSymbolValuePtr value)
{
    SymbolEntryPtr rval = _symbolDictionary[symbolID];
    if(rval->isDefined())
    {
        rval->setMultiplyDefined();
    }
    rval->setValue(value);
    return rval;
}

SymbolEntryPtr SymbolTable::setValue(const QString & symbolName, AbstractSymbolValuePtr value)
{
	return setValue(_symbolLookup.find(symbolName).value(), value);
}

bool SymbolTable::exists(const QString& symbolName) const
{
	return _symbolLookup.find(symbolName)!= _symbolLookup.end();
}

bool SymbolTable::exists(SymbolID symbolID) const
{
	return _symbolDictionary.find(symbolID) != _symbolDictionary.end();
}

quint32 SymbolTable::numMultiplyDefinedSymbols() const
{
	return quint32();
}

quint32 SymbolTable::numUndefinedSymbols() const
{
	return quint32();
}

QList<SymbolEntryPtr> SymbolTable::getSymbolEntries() const
{
	return _symbolDictionary.values();
}
