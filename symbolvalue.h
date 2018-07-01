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
#include <QtCore>
/*
 * A symbol's value type can be EMPTY (i.e the symbol is undefined), or an address (i.e. the symbol is singly defined, so it describes a ADDRESS in a program).
 * A symbol's value type is not meaninful in the case of a multiply defined symbol, so no special type is required for it.
 */
enum SymbolType
{
    EMPTY, ADDRESS,
};

/*
 * Abstract base class defining the properties of a symbolic value.
 */
class AbstractSymbolValue
{
public:
	AbstractSymbolValue();
	virtual ~AbstractSymbolValue();
	virtual qint32 getValue() const = 0;
    virtual SymbolType getSymbolType() const = 0;
};

/*
 * A symbol value representing the value contained by an undefined symbol.
 */
class SymbolValueEmpty :
public AbstractSymbolValue
{
public:
    SymbolValueEmpty();
    virtual ~SymbolValueEmpty();
    
    // Inherited via AbstractSymbolValue
    virtual qint32 getValue() const override;
    virtual SymbolType getSymbolType() const override;
};

/*
 * A symbol value representing an address of a line of code.
 */
class SymbolValueLocation :
public AbstractSymbolValue
{
    quint16 _value;
public:
    explicit SymbolValueLocation(quint16 value);
    virtual ~SymbolValueLocation();
    void setValue(quint16);
    // Inherited via AbstractSymbolValue
    virtual qint32 getValue() const override;
    virtual SymbolType getSymbolType() const override;
};

