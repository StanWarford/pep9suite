// File: symbolvalue.h
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
#ifndef SYMBOLVALUE_H
#define SYMBOLVALUE_H

#include <QtCore>
#include <QSharedPointer>

class SymbolEntry;

/*
 * A symbol's value type can be EMPTY (i.e the symbol is undefined), or an address (i.e. the symbol is singly defined, so it describes a ADDRESS in a program).
 * A symbol's value type is not meaninful in the case of a multiply defined symbol, so no special type is required for it.
 */
enum class SymbolType
{
    EMPTY, ADDRESS, NUMERIC_CONSTANT
};

/*
 * Abstract base class defining the properties of a symbolic value.
 */
class AbstractSymbolValue
{
private:
public:
    AbstractSymbolValue();
	virtual ~AbstractSymbolValue();
	virtual qint32 getValue() const = 0;
    virtual SymbolType getSymbolType() const = 0;
    virtual bool canRelocate() const {return false;}
};

/*
 * A symbol value representing the value contained by an undefined symbol.
 */
class SymbolValueEmpty :
public AbstractSymbolValue
{
public:
    SymbolValueEmpty();
    virtual ~SymbolValueEmpty() override;
    
    // Inherited via AbstractSymbolValue
    virtual qint32 getValue() const override;
    virtual SymbolType getSymbolType() const override;
};

/*
 * A symbol value representing an constant numeric value.
 */
class SymbolValueNumeric :
public AbstractSymbolValue
{
    quint16 value;
public:
    explicit SymbolValueNumeric(quint16 value);
    virtual ~SymbolValueNumeric() override;
    void setValue(quint16 value);
    // Inherited via AbstractSymbolValue
    virtual qint32 getValue() const override;
    virtual SymbolType getSymbolType() const override;
};

/*
 * A symbol value representing an address of a line of code.
 * The effective address (and thus the value) is base + offset.
 *
 * Having a seperate offset parameter allows for easy relocation of programs,
 * which is necessary when compiling the Pep9OS.
 */
class SymbolValueLocation :
public AbstractSymbolValue
{
    quint16 base, offset;
public:
    explicit SymbolValueLocation(quint16 value);
    virtual ~SymbolValueLocation() override; 
    void setBase(quint16 value);
    void setOffset(quint16 value);
    // Inherited via AbstractSymbolValue
    virtual qint32 getValue() const override;
    virtual SymbolType getSymbolType() const override;
    virtual bool canRelocate() const override;
    quint16 getOffset() const;
    quint16 getBase() const;
};

#endif // SYMBOLVALUE_H
