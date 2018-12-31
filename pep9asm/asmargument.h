// File: argument.h
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

#include "isaasm.h"
#include "pep.h"
#include <QSharedPointer>
#include "symbolentry.h"
#include "symboltable.h"
#include "symbolvalue.h"
class SymbolEntry;
// Abstract Argument class
class AsmArgument
{
    friend class IsaAsm;
public:
    virtual ~AsmArgument() { }
    virtual int getArgumentValue() const = 0;
    virtual QString getArgumentString() const = 0;
};

// Concrete argument classes
class CharArgument: public AsmArgument
{
    friend class IsaAsm;
private:
    QString charValue;
public:
    explicit CharArgument(QString cValue);
    virtual ~CharArgument() override = default;
    virtual int getArgumentValue() const override;
    virtual QString getArgumentString() const override;
};

class DecArgument: public AsmArgument
{
private:
    int decValue;
public:
    explicit DecArgument(int dValue);
    virtual ~DecArgument() override = default;
    virtual int getArgumentValue() const override;
    virtual QString getArgumentString() const override;
};

class UnsignedDecArgument: public AsmArgument
{
private:
    int decValue;
public:
    explicit UnsignedDecArgument(int dValue);
    virtual ~UnsignedDecArgument() override = default;
    virtual int getArgumentValue() const override;
    virtual QString getArgumentString() const override;
};

class HexArgument: public AsmArgument
{
private:
    int hexValue;
public:
    explicit HexArgument(int hValue);
    virtual ~HexArgument() override = default;
    virtual int getArgumentValue() const override;
    virtual QString getArgumentString() const override;
};

class StringArgument: public AsmArgument
{
private:
    QString stringValue;
public:
    explicit StringArgument(QString sValue);
    virtual ~StringArgument() override = default;
    virtual int getArgumentValue() const override;
    virtual QString getArgumentString() const override;
};

class SymbolRefArgument: public AsmArgument
{
private:
    QSharedPointer<SymbolEntry> symbolRefValue;
public:
    explicit SymbolRefArgument(QSharedPointer<SymbolEntry> sRefValue);
    virtual ~SymbolRefArgument() override = default;
    virtual int getArgumentValue() const override;
    virtual QString getArgumentString() const override;
    QSharedPointer<SymbolEntry> getSymbolValue();

};

#endif // ARGUMENT_H
