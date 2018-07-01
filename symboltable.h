/*
    Pep9CPU is a CPU simulator for executing microcode sequences to
    implement instructions in the instruction set of the Pep/9 computer.

    Copyright (C) 2018  Matthew McRaven, Pepperdine University

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
#pragma once
#include <qmap.h>
#include <qmutex.h>
#include <memory>
class SymbolEntry;
class AbstractSymbolValue;
/*
 * The SymbolTable class provides lookups base on the names and unique identifiers of a group of SymbolEntries.
 * A SymbolEntry is created by calling insertSymbol(...), and can then be looked up by name or by its unique identifier.
 *
 */
class SymbolTable
{
public:
    //This type uniquely identifies a SymbolEntry within a symbol table.
    //It is not gaurenteed to be unique across runs or between multiple SymbolTable instances at runtime.
    typedef QAtomicInt SymbolID;
    //Convenience typdefs of commonly used templated types to reduce code verbosity
	typedef std::shared_ptr<SymbolEntry> SymbolEntryPtr;
	typedef std::shared_ptr<AbstractSymbolValue> AbstractSymbolValuePtr;
private:
    static SymbolID nextUserSymbolID;
	static SymbolID getNextUserSymbolID();
	QMap<SymbolID, SymbolEntryPtr> _symbolDictionary;
    QMap<QString, SymbolID> _symbolLookup;
public:
    explicit SymbolTable();
	~SymbolTable();
    //Return a symbol entry by name or ID. Returns an empty shared_ptr if the symbol being looked for doesn't exist.
	SymbolEntryPtr getValue(SymbolID symbolID) const;
	SymbolEntryPtr getValue(const QString& symbolName) const;
    //Create a new SymbolEntry with the passed name
	SymbolEntryPtr insertSymbol(const QString& symbolName);
    //Change the value of a SymbolEntry
	SymbolEntryPtr setValue(SymbolID symbolID, AbstractSymbolValuePtr value);
	SymbolEntryPtr setValue(const QString & symbolName, AbstractSymbolValuePtr value);
    //Check if a symbol exists
    bool exists(const QString& symbolName) const;
    bool exists(SymbolID symbolID) const;
    //Get the count of symbols that have definition problems
	quint32 numMultiplyDefinedSymbols() const;
	quint32 numUndefinedSymbols() const;
    QList<SymbolEntryPtr> getSymbolEntries() const;
};
