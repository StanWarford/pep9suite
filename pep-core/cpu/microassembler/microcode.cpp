// File: microcode.cpp
/*
    Pep9CPU is a CPU simulator for executing microcode sequences to
    implement instructions in the instruction set of the Pep/9 computer.

    Copyright (C) 2010  J. Stanley Warford, Pepperdine University

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

#include "microcode.h"

#include <QMetaEnum>

#include "microassembler/specification.h"
#include "pep/pep.h"
#include "symbol/symbolentry.h"

AMicroCode::~AMicroCode() = default;
AExecutableMicrocode::~AExecutableMicrocode() = default;

AExecutableMicrocode::AExecutableMicrocode(bool hasBreakpoint, QString comment, const SymbolEntry* symbol):
    breakpoint(hasBreakpoint), cComment(comment), symbol(symbol)
{

}

bool AExecutableMicrocode::isMicrocode() const
{
    return true;
}

void AExecutableMicrocode::setComment(QString comment)
{
    this->cComment = comment;
}

bool AExecutableMicrocode::hasSymbol() const
{
    return symbol != nullptr;
}

const SymbolEntry *AExecutableMicrocode::getSymbol() const
{
    return symbol;
}

void AExecutableMicrocode::setSymbol(const SymbolEntry * symbol)
{
    this->symbol = symbol;
}

bool AExecutableMicrocode::hasBreakpoint() const
{
    return breakpoint;
}

void AExecutableMicrocode::setBreakpoint(bool breakpoint)
{
    this->breakpoint = breakpoint;
}


CommentOnlyCode::CommentOnlyCode(QString comment)
{
    cComment = comment;
}

QString CommentOnlyCode::getSourceCode()const
{
    return cComment;
}


