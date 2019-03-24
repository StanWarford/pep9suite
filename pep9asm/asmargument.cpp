// File: asmargument.cpp
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

#include <QString>
#include <QSharedPointer>
#include <QWidget>

#include "asmargument.h"
#include "symboltable.h"
#include "symbolentry.h"
#include "symbolvalue.h"

AsmArgument::~AsmArgument()
{

}

SymbolRefArgument::SymbolRefArgument(QSharedPointer<SymbolEntry> sRefValue): symbolRefValue(sRefValue)
{

}

int SymbolRefArgument::getArgumentValue() const
{
    return symbolRefValue->getValue();
}

QString SymbolRefArgument::getArgumentString() const
{
    return symbolRefValue->getName();
}

QSharedPointer<SymbolEntry> SymbolRefArgument::getSymbolValue()
{
    return symbolRefValue;
}

HexArgument::HexArgument(int hValue) :hexValue(hValue)
{

}

int HexArgument::getArgumentValue() const
{
    return hexValue;
}

QString HexArgument::getArgumentString() const
{
    return "0x" + QString("%1").arg(hexValue, 4, 16, QLatin1Char('0')).toUpper();
}

StringArgument::StringArgument(QString sValue): stringValue(sValue)
{

}

int StringArgument::getArgumentValue() const
{
    return IsaParserHelper::string2ArgumentToInt(stringValue);
}

QString StringArgument::getArgumentString() const
{
    return stringValue;
}

CharArgument::CharArgument(QString cValue) : charValue(cValue)
{

}

int CharArgument::getArgumentValue() const
{
    return IsaParserHelper::charStringToInt(charValue);
}

QString CharArgument::getArgumentString() const
{
    return charValue;
}

DecArgument::DecArgument(int dValue): decValue(dValue)
{

}

int DecArgument::getArgumentValue() const
{
    return decValue;
}

QString DecArgument::getArgumentString() const
{
    int temp = decValue >= 32768 ? decValue - 65536 : decValue;
    return QString("%1").arg(temp);
}

UnsignedDecArgument::UnsignedDecArgument(int dValue): decValue(dValue)
{

}

int UnsignedDecArgument::getArgumentValue() const
{
    return decValue;
}

QString UnsignedDecArgument::getArgumentString() const
{
    return QString("%1").arg(decValue);
}
