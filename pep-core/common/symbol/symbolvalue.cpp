// File: symbolvalue.cpp
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

#include "symbolvalue.h"

AbstractSymbolValue::AbstractSymbolValue()
{
}

AbstractSymbolValue::~AbstractSymbolValue()
{

}

SymbolValueEmpty::SymbolValueEmpty(): AbstractSymbolValue()
{
}

SymbolValueEmpty::~SymbolValueEmpty()
{
}

qint32 SymbolValueEmpty::getValue() const
{
    return qint32();
}

SymbolType SymbolValueEmpty::getSymbolType() const
{
    return SymbolType::EMPTY;
}

SymbolValueNumeric::SymbolValueNumeric(quint16 value): value(value)
{

}

SymbolValueNumeric::~SymbolValueNumeric()
{

}

void SymbolValueNumeric::setValue(quint16 value)
{
    this->value = value;
}

qint32 SymbolValueNumeric::getValue() const
{
    return value;
}

SymbolType SymbolValueNumeric::getSymbolType() const
{
    return SymbolType::NUMERIC_CONSTANT;
}

SymbolValueLocation::SymbolValueLocation(quint16 value):AbstractSymbolValue(), base(value), offset(0)
{
}

SymbolValueLocation::~SymbolValueLocation()
{
}

void SymbolValueLocation::setBase(quint16 value)
{
    this->base = value;
}

void SymbolValueLocation::setOffset(quint16 value)
{
    this->offset = value;
}

qint32 SymbolValueLocation::getValue() const
{
    return base + offset;

}

SymbolType SymbolValueLocation::getSymbolType() const
{
    return SymbolType::ADDRESS;
}

bool SymbolValueLocation::canRelocate() const
{
    return true;
}

quint16 SymbolValueLocation::getOffset() const
{
    return offset;
}

quint16 SymbolValueLocation::getBase() const
{
    return base;
}
