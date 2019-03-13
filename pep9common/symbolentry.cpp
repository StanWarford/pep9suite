// File: symbolentry.cpp
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

#include "symbolentry.h"
#include "symbolvalue.h"

SymbolEntry::SymbolEntry(SymbolTable* parent, SymbolTable::SymbolID symbolID, QString name):symbolID(symbolID), name(name),
symbolValue(QSharedPointer<SymbolValueEmpty>::create()), definedState(DefStates::UNDEFINED), parent(parent)
{
}

SymbolEntry::SymbolEntry(SymbolTable* parent, SymbolTable::SymbolID symbolID, QString name,
                         SymbolTable::AbstractSymbolValuePtr value): symbolID(symbolID), name(name),
    symbolValue(nullptr), parent(parent)
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
    // A SymbolEntry will not transfer from SINGLE to MULTIPLE on its own.
    // This is because a symbol table / assembler might need to update the value
    // of the symbol in place to achieve code relocation, and so the responsibility
    // to make that decision is delegated to owning objects.
	symbolValue = value;
    // If given an empty value, then the symbol is undefined
    if (dynamic_cast<SymbolValueEmpty*>(value.data())) {
        definedState = DefStates::UNDEFINED;
	}
    //If the symbol is multiply defined, it remains multiply defined
    else if(definedState == DefStates::MULTIPLE) {
        definedState = DefStates::MULTIPLE;
    }
    else {
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

QDebug operator<<(QDebug os, SymbolEntry& ent)
{
    return os.noquote() << QString("symbol: %1")
          .arg(ent.getName());
}

QDebug operator<<(QDebug os, const SymbolEntry& ent)
{
    return os.noquote() << QString("symbol: %1")
          .arg(ent.getName());
}

QDebug operator<<(QDebug os, const QSharedPointer<SymbolEntry>& ent)
{
    return os.noquote() << QString("symbol: %1")
          .arg(ent->getName());
}

QDebug operator<<(QDebug os, const QSharedPointer<const SymbolEntry>& ent)
{
    return os.noquote() << QString("symbol: %1")
          .arg(ent->getName());
}
