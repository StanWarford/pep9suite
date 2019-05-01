// File: symboltable.cpp
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

SymbolTable::SymbolTable():symbolDictionary(), symbolLookup()
{
}

SymbolTable::~SymbolTable()
{
}

SymbolEntryPtr SymbolTable::getValue(SymbolID symbolID) const
{

    auto index = symbolDictionary.find(symbolID);
    if(index == symbolDictionary.end()) return QSharedPointer<SymbolEntry>();
    return index.value();
}

SymbolEntryPtr SymbolTable::getValue(const QString & symbolName) const
{
    auto index = symbolLookup.find(symbolName);
    if(index == symbolLookup.end())  return nullptr;
    return getValue(index.value());
}

SymbolEntryPtr SymbolTable::insertSymbol(const QString & symbolName)
{
    // We don't want multiple symbols to exists in the same table with the same name.
    if(exists(symbolName)) return getValue(symbolName);
	SymbolID id = SymbolTable::getNextUserSymbolID();
    symbolLookup.insert(symbolName, id);
    auto val = symbolDictionary.insert(id, QSharedPointer<SymbolEntry>::create(this, id, symbolName));
	return val.value();
}

SymbolEntryPtr SymbolTable::setValue(SymbolID symbolID, AbstractSymbolValuePtr value)
{
    SymbolEntryPtr rval = symbolDictionary[symbolID];
    // If the symbol has already been defined, this function vall constitutes a redefinition.
    if(rval->isDefined()) {
        rval->setMultiplyDefined();
    }
    rval->setValue(value);
    return rval;
}

SymbolEntryPtr SymbolTable::setValue(const QString & symbolName, AbstractSymbolValuePtr value)
{
    // If the table doesn't contain a symbol, create it first.
    if(!exists(symbolName)) insertSymbol(symbolName);
    return setValue(symbolLookup.find(symbolName).value(), value);
}

bool SymbolTable::exists(const QString& symbolName) const
{
    return symbolLookup.find(symbolName) != symbolLookup.end();
}

bool SymbolTable::exists(SymbolID symbolID) const
{
    return symbolDictionary.find(symbolID) != symbolDictionary.end();
}

quint32 SymbolTable::numMultiplyDefinedSymbols() const
{
    quint32 count = 0;
    for(SymbolTable::SymbolEntryPtr ptr : this->symbolDictionary) {
        count += ptr->isMultiplyDefined() ? 1 : 0;
    }
    return count;
}

quint32 SymbolTable::numUndefinedSymbols() const
{
    quint32 count = 0;
    for(SymbolTable::SymbolEntryPtr ptr : this->symbolDictionary) {
        count += ptr->isUndefined() ? 1 : 0;
    }
    return count;
}

void SymbolTable::setOffset(quint16 value, quint16 threshhold)
{
    for(SymbolEntryPtr ptr:this->symbolDictionary) {
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
    return symbolDictionary.values();
}

const QMap<SymbolTable::SymbolID, SymbolTable::SymbolEntryPtr> SymbolTable::getSymbolMap() const
{
    return symbolDictionary;
}


QString SymbolTable::getSymbolTableListing() const
{

    static const QString line = "--------------------------------------\n";
    static const QString symTableStr = "Symbol table\n";
    static const QString headerStr = "Symbol    Value        Symbol    Value\n";
    QString build;
    QList<QSharedPointer<SymbolEntry>> list = getSymbolEntries();
    std::sort(list.begin(),list.end(), SymbolAlphabeticComparator);

    for(auto it = list.begin(); it != list.end(); ++it) {
        if(it + 1 ==list.end()) {
            QString hexString = QString("%1").arg((*it)->getValue(), 4, 16, QLatin1Char('0')).toUpper();
            build.append(QString("%1%2\n").arg((*it)->getName(), -10).arg(hexString, -13));
        }
        else {
            QString hexString = QString("%1").arg((*it)->getValue(), 4, 16, QLatin1Char('0')).toUpper();
            build.append(QString("%1%2").arg((*it)->getName(), -10).arg(hexString, -13));
            ++it;
            hexString = QString("%1").arg((*it)->getValue(), 4, 16, QLatin1Char('0')).toUpper();
            build.append(QString("%1%2\n").arg((*it)->getName(), -10).arg(hexString, -13));
        }
    }
    return symTableStr % line % headerStr %line % build % line;
}
