#include "symboltable.h"
#include "symbolentry.h"
#include "symbolvalue.h"
typedef QAtomicInt SymbolID;
typedef QSharedPointer<SymbolEntry> SymbolEntryPtr;
typedef QSharedPointer<AbstractSymbolValue> AbstractSymbolValuePtr;
SymbolID SymbolTable::nextUserSymbolID = 0;
SymbolID SymbolTable::getNextUserSymbolID()
{
    QAtomicInt newSymbolID = ++nextUserSymbolID;
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

    auto index = _symbolDictionary.find(symbolID);
    if(index == _symbolDictionary.end()) return QSharedPointer<SymbolEntry>();
    return index.value();
}

SymbolEntryPtr SymbolTable::getValue(const QString & symbolName) const
{
    auto index = _symbolLookup.find(symbolName);
    if(index == _symbolLookup.end())  return nullptr;
    return getValue(index.value());
}

SymbolEntryPtr SymbolTable::insertSymbol(const QString & symbolName)
{
    //Multiple definitions handled by assembler, not symbol table
	SymbolID id = SymbolTable::getNextUserSymbolID();
	_symbolLookup.insert(symbolName, id);
    auto val = _symbolDictionary.insert(id, QSharedPointer<SymbolEntry>::create(this, id, symbolName));
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
    if(!exists(symbolName)) insertSymbol(symbolName);
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
    quint32 count = 0;
    for(SymbolTable::SymbolEntryPtr ptr :this->_symbolDictionary) {
        count += ptr->isMultiplyDefined(); // Automatically upcast bool to int
    }
    return count;
}

quint32 SymbolTable::numUndefinedSymbols() const
{
    quint32 count = 0;
    for(SymbolTable::SymbolEntryPtr ptr :this->_symbolDictionary) {
        count += ptr->isUndefined(); // Automatically upcast bool to int
    }
    return count;
}

void SymbolTable::setOffset(quint16 value, quint16 threshhold)
{
    for(SymbolEntryPtr ptr:this->_symbolDictionary) {
        if(ptr->getRawValue()->getSymbolType() == SymbolType::ADDRESS && ptr->getValue() >= threshhold) {
            static_cast<SymbolValueLocation*>(ptr->getRawValue().data())->setOffset(value);
        }
    }
}

void SymbolTable::clearOffset()
{
    // Clearing offsets is the same thing as setting all offsets to 0.
    setOffset(0, 0);
}

QList<SymbolEntryPtr> SymbolTable::getSymbolEntries() const
{
	return _symbolDictionary.values();
}
