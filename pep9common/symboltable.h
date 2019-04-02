// File: symboltable.h
/*
    The Pep/9 suite of applications (Pep9, Pep9CPU, Pep9Micro) are
    simulators for the Pep/9 virtual machine, and allow users to
    create, simulate, and debug across various levels of abstraction.

    Copyright (C) 2018 J. Stanley Warford & Matthew McRaven, Pepperdine University

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
#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <memory>
#include <QMap>
#include <QMutex>
#include <QSharedPointer>

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
    // This type uniquely identifies a SymbolEntry within a symbol table.
    // It is not gaurenteed to be unique across runs or between multiple SymbolTable instances at runtime.
    typedef QAtomicInt SymbolID;
    // Convenience typdefs of commonly used templated types to reduce code verbosity.
    typedef QSharedPointer<SymbolEntry> SymbolEntryPtr;
    typedef QSharedPointer<AbstractSymbolValue> AbstractSymbolValuePtr;

private:
    static SymbolID nextUserSymbolID;
	static SymbolID getNextUserSymbolID();
    QMap<SymbolID, SymbolEntryPtr> symbolDictionary;
    QMap<QString, SymbolID> symbolLookup;

public:
    explicit SymbolTable();
	~SymbolTable();
    // Return a symbol entry by name or ID. Returns an empty shared_ptr if the symbol being looked for doesn't exist.
	SymbolEntryPtr getValue(SymbolID symbolID) const;
	SymbolEntryPtr getValue(const QString& symbolName) const;
    // Create a new SymbolEntry with the passed name.
	SymbolEntryPtr insertSymbol(const QString& symbolName);
    // Change the value of a SymbolEntry.
    // If the symbol already exists, it will be be set to multiple defined.
    // If one wants to update a symbol value in place, getValue(...) on the symbol,
    // and setValue(...) on that symbol.
	SymbolEntryPtr setValue(SymbolID symbolID, AbstractSymbolValuePtr value);
	SymbolEntryPtr setValue(const QString & symbolName, AbstractSymbolValuePtr value);
    // Check if a symbol exists.
    bool exists(const QString& symbolName) const;
    bool exists(SymbolID symbolID) const;
    // Get the count of symbols that have definition problems.
	quint32 numMultiplyDefinedSymbols() const;
	quint32 numUndefinedSymbols() const;
    // Set the offset of all relocatable symbols with an address
    // above threshhold to value.
    void setOffset(quint16 value, quint16 threshhold = 0);
    // Set the offset of all relocatable symbols to 0.
    void clearOffset();
    QList<SymbolEntryPtr> getSymbolEntries() const;
    // Return the map of all symbolic definitions. Used to provide access to iterators
    // to perform custom operations and comparisions over all symbols.
    const QMap<SymbolID, SymbolEntryPtr> getSymbolMap() const;

    QString getSymbolTableListing() const;
};

#endif // SYMBOLTABLE_H
