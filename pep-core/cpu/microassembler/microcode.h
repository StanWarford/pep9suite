// File: microcode.h
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
#ifndef MICROCODE_H
#define MICROCODE_H

#include <QMap>
#include <QString>
#include <QVector>

#include "pep/enu.h"

class SymbolEntry;
// Abstract code class
class AMicroCode
{
public:
    virtual ~AMicroCode() = 0;
    virtual bool isMicrocode() const { return false; }
    virtual QString getObjectCode() const { return ""; }
    virtual QString getSourceCode() const { return ""; }
    virtual bool hasUnitPre() const { return false; }
    virtual bool hasUnitPost() const{return false;}

};

class AExecutableMicrocode : public AMicroCode
{
public:
    AExecutableMicrocode(bool hasBreakpoint, QString comment, const SymbolEntry* symbol=nullptr);
    virtual ~AExecutableMicrocode() = 0;

    bool hasBreakpoint() const;
    void setBreakpoint(bool breakpoint);
    bool isMicrocode() const override;

    void setComment(QString comment);

    bool hasSymbol() const;
    const SymbolEntry* getSymbol() const;
    void setSymbol(const SymbolEntry*);
protected:
    bool breakpoint;
    QString cComment;
    const SymbolEntry* symbol;
};

// Concrete code classes
// Code is the union of the elements of the one-byte bus model and two-byte bus model

class CommentOnlyCode: public AMicroCode
{
public:
    CommentOnlyCode(QString comment);
    QString getSourceCode() const override;
private:
    QString cComment;
};

class BlankLineCode: public AMicroCode
{
};

#endif // CODE_H
