#pragma once
#include <qmap.h>
#include <qmutex.h>
#include <memory>
class SymbolEntry;
class AbstractSymbolValue;
class SymbolTable
{
public:
	typedef qint32 SymbolID;
	typedef std::shared_ptr<SymbolEntry> SymbolEntryPtr;
	typedef std::shared_ptr<AbstractSymbolValue> AbstractSymbolValuePtr;
private:
	static SymbolID _nextUserSymbolID;
	static SymbolID getNextUserSymbolID();
	QMap<SymbolID, SymbolEntryPtr> _symbolDictionary;
	QMap<QString, SymbolID> _symbolLookup;
public:
    explicit SymbolTable();
	~SymbolTable();
	SymbolEntryPtr getValue(SymbolID symbolID) const;
	SymbolEntryPtr getValue(const QString& symbolName) const;
	SymbolEntryPtr insertSymbol(const QString& symbolName);
	SymbolEntryPtr setValue(SymbolID symbolID, AbstractSymbolValuePtr value);
	SymbolEntryPtr setValue(const QString & symbolName, AbstractSymbolValuePtr value);
    bool exists(const QString& symbolName) const;
    bool exists(SymbolID symbolID) const;
	quint32 numMultiplyDefinedSymbols() const;
	quint32 numUndefinedSymbols() const;
    QList<SymbolEntryPtr> getSymbolEntries() const;
};
