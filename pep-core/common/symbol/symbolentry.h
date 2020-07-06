// File: symbolentry.h
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
#ifndef SYMBOLENTRY_H
#define SYMBOLENTRY_H

#include <qstring.h>

#include "pep/enu.h"
#include "symbol/symboltable.h"

/*
 * A symbol can either be:
 *  1) Undefined: A symbol is not defined, and referenced 1+ times.
 *  2) Singlely Defined: A symbol is defined once, and referenced 0+ times.
 *  3) Multiply Defined: A symbol is defined 2+ times, and referenced 0+ times.
 */
enum class DefStates
{
    SINGLE, MULTIPLE, UNDEFINED
};

struct SymbolFormat
{
    Enu::ESymbolFormat spec = Enu::ESymbolFormat::F_NONE;
    quint8 size = 0;
};

/*
 * A symbol entry represents one named symbol from a microprogram.
 * Symbols have multiple definition states that allow the microassembler to error if symbols are defined incorrectly.
 * Symbols' value can either be empty (i.e. undefined) or store the address of a line of microcode.
 * The symbol will automatically adjust between SINGLE and UNDEFINED based on the value pased to setValue(...),
 * but the class will not automatically set its definition state to MULTIPLE.
 * The function setMultiplyDefined() is the only way to notify a Symbol that it has been defined more than once.
 *
 * This function should be invoked from whatever assembler of compiler created the SymbolEntry - the SymbolTable will not do this for you.
 * This is because symbols could constantly be updated and changed (i.e. code relocation), so it would be very difficult for this class to decide if it has
 * been defined more than once, instead of simply being updated in place.
 */
class SymbolEntry
{
private:
    // Unique identifier describing this symbol
    SymbolTable::SymbolID symbolID;
    // Unique string name of symbol as appearing in the sources
    QString name;
    SymbolTable::AbstractSymbolValuePtr symbolValue;
    DefStates definedState;
    // Non-owning pointer to parent. DO NOT DELETE.
    SymbolTable* parent;
public:
    //Default constructor, assumes value is SymbolEmpty
    SymbolEntry(SymbolTable* parent, SymbolTable::SymbolID symbolID, QString name);
    SymbolEntry(SymbolTable* parent, SymbolTable::SymbolID symbolID, QString name, SymbolTable::AbstractSymbolValuePtr value);
    //
    ~SymbolEntry();
    const SymbolTable* getParentTable() const;
    void setValue(SymbolTable::AbstractSymbolValuePtr value);
    //Get the string name of the symbol
    QString getName() const;
    bool isDefined() const;
    bool isUndefined() const;
    bool isMultiplyDefined() const;
    void setMultiplyDefined();
    SymbolTable::SymbolID getSymbolID() const;
    //Uses the internal data pointer, and returns the value of its getValue() method
    qint32 getValue() const;
    //Returns the internal data pointer, in case one wishes to access its other methods
    SymbolTable::AbstractSymbolValuePtr getRawValue();
};

QDebug operator<<(QDebug os, SymbolEntry&);
QDebug operator<<(QDebug os, const SymbolEntry&);
QDebug operator<<(QDebug os, const QSharedPointer<SymbolEntry>&);
QDebug operator<<(QDebug os, const QSharedPointer<const SymbolEntry>&);

bool SymbolAlphabeticComparator(QSharedPointer<SymbolEntry> &lhs, QSharedPointer<SymbolEntry> &rhs);


#endif // SYMBOLENTRY_H
