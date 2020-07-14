// File: asmargument.h
/*
    Pep9 is a virtual machine for writing machine language and assembly
    language programs.
    
    Copyright (C) 2009  J. Stanley Warford, Pepperdine University

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

#ifndef ARGUMENT_H
#define ARGUMENT_H

#include <QSharedPointer>

#include "symbol/symbolentry.h"
#include "symbol/symboltable.h"
#include "symbol/symbolvalue.h"

class SymbolEntry;
// Abstract Argument class
class AsmArgument
{
public:
    virtual ~AsmArgument();
    virtual int getArgumentValue() const = 0;
    virtual QString getArgumentString() const = 0;
};

// Concrete argument classes
class CharArgument: public AsmArgument
{
public:
    explicit CharArgument(QString cValue);
    virtual ~CharArgument() override = default;
    virtual int getArgumentValue() const override;
    virtual QString getArgumentString() const override;
private:
    QString charValue;
};

class DecArgument: public AsmArgument
{
public:
    explicit DecArgument(int dValue);
    virtual ~DecArgument() override = default;
    virtual int getArgumentValue() const override;
    virtual QString getArgumentString() const override;
private:
    int decValue;
};

class UnsignedDecArgument: public AsmArgument
{

public:
    explicit UnsignedDecArgument(int dValue);
    virtual ~UnsignedDecArgument() override = default;
    virtual int getArgumentValue() const override;
    virtual QString getArgumentString() const override;
private:
    int decValue;
};

class HexArgument: public AsmArgument
{
public:
    explicit HexArgument(int hValue);
    virtual ~HexArgument() override = default;
    virtual int getArgumentValue() const override;
    virtual QString getArgumentString() const override;
private:
    int hexValue;
};

class StringArgument: public AsmArgument
{
public:
    explicit StringArgument(QString sValue);
    virtual ~StringArgument() override = default;
    virtual int getArgumentValue() const override;
    virtual QString getArgumentString() const override;
private:
    QString stringValue;
};

class SymbolRefArgument: public AsmArgument
{
public:
    explicit SymbolRefArgument(QSharedPointer<SymbolEntry> sRefValue);
    virtual ~SymbolRefArgument() override = default;
    virtual int getArgumentValue() const override;
    virtual QString getArgumentString() const override;
    QSharedPointer<SymbolEntry> getSymbolValue();
private:
    QSharedPointer<SymbolEntry> symbolRefValue;

};

#endif // ARGUMENT_H
