// File: code.h
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

#ifndef ASMCODE_H
#define ASMCODE_H

#include "pep.h"
#include "enu.h"

class AsmArgument; // Forward declaration for attributes of code classes.

// Abstract Code class
class AsmCode
{
    friend class IsaAsm;
public:
    virtual ~AsmCode() { }
    virtual void appendObjectCode(QList<int> &objectCode) = 0;
    virtual void appendSourceLine(QStringList &assemblerListingList, QStringList &listingTraceList, QList<bool> &hasCheckBox) = 0;
    void adjustMemAddress(int addressDelta) { memAddress += addressDelta; }
    virtual bool processFormatTraceTags(int &, QString &) { return true; }
    virtual bool processSymbolTraceTags(int &, QString &) { return true; }

protected:
    int memAddress;
    int sourceCodeLine;
    QString symbolDef;
    QString comment;
};

// Concrete code classes
class UnaryInstruction: public AsmCode
{
    friend class IsaAsm;
private:
    Enu::EMnemonic mnemonic;
public:
    void appendObjectCode(QList<int> &objectCode);
    void appendSourceLine(QStringList &assemblerListingList, QStringList &listingTraceList, QList<bool> &hasCheckBox);
//    NO LONGER WITH PEP/9. FOR RET1, RET2, ..., RET7
//    bool processSymbolTraceTags(int &sourceLine, QString &errorString);
};

class NonUnaryInstruction: public AsmCode
{
    friend class IsaAsm;
private:
    Enu::EMnemonic mnemonic;
    Enu::EAddrMode addressingMode;
    AsmArgument *argument;
public:
    // ~NonUnaryInstruction() { delete argument; }
    void appendObjectCode(QList<int> &objectCode);
    void appendSourceLine(QStringList &assemblerListingList, QStringList &listingTraceList, QList<bool> &hasCheckBox);
    bool processFormatTraceTags(int &sourceLine, QString &errorString);
    bool processSymbolTraceTags(int &sourceLine, QString &errorString);
};

class DotAddrss: public AsmCode
{
    friend class IsaAsm;
private:
    AsmArgument *argument;
public:
    void appendObjectCode(QList<int> &objectCode);
    void appendSourceLine(QStringList &assemblerListingList, QStringList &listingTraceList, QList<bool> &hasCheckBox);
};

class DotAlign: public AsmCode
{
    friend class IsaAsm;
private:
    AsmArgument *argument;
    AsmArgument *numBytesGenerated;
public:
    void appendObjectCode(QList<int> &objectCode);
    void appendSourceLine(QStringList &assemblerListingList, QStringList &listingTraceList, QList<bool> &hasCheckBox);
};

class DotAscii: public AsmCode
{
    friend class IsaAsm;
private:
    AsmArgument *argument;
public:
    void appendObjectCode(QList<int> &objectCode);
    void appendSourceLine(QStringList &assemblerListingList, QStringList &listingTraceList, QList<bool> &hasCheckBox);
};

class DotBlock: public AsmCode
{
    friend class IsaAsm;
private:
    AsmArgument *argument;
public:
    void appendObjectCode(QList<int> &objectCode);
    void appendSourceLine(QStringList &assemblerListingList, QStringList &listingTraceList, QList<bool> &hasCheckBox);
    bool processFormatTraceTags(int &sourceLine, QString &errorString);
    bool processSymbolTraceTags(int &sourceLine, QString &errorString);
};

class DotBurn: public AsmCode
{
    friend class IsaAsm;
private:
    AsmArgument *argument;
public:
    void appendObjectCode(QList<int> &objectCode);
    void appendSourceLine(QStringList &assemblerListingList, QStringList &listingTraceList, QList<bool> &hasCheckBox);
};

class DotByte: public AsmCode
{
    friend class IsaAsm;
private:
    AsmArgument *argument;
public:
    void appendObjectCode(QList<int> &objectCode);
    void appendSourceLine(QStringList &assemblerListingList, QStringList &listingTraceList, QList<bool> &hasCheckBox);
};

class DotEnd: public AsmCode
{
    friend class IsaAsm;
public:
    void appendObjectCode(QList<int> &objectCode);
    void appendSourceLine(QStringList &assemblerListingList, QStringList &listingTraceList, QList<bool> &hasCheckBox);
};

class DotEquate: public AsmCode
{
    friend class IsaAsm;
private:
    AsmArgument *argument;
public:
    void appendObjectCode(QList<int> &objectCode);
    void appendSourceLine(QStringList &assemblerListingList, QStringList &listingTraceList, QList<bool> &hasCheckBox);
    bool processFormatTraceTags(int &sourceLine, QString &errorString);
};

class DotWord: public AsmCode
{
    friend class IsaAsm;
private:
    AsmArgument *argument;
public:
    void appendObjectCode(QList<int> &objectCode);
    void appendSourceLine(QStringList &assemblerListingList, QStringList &listingTraceList, QList<bool> &hasCheckBox);
};

class CommentOnly: public AsmCode
{
    friend class IsaAsm;
public:
    void appendObjectCode(QList<int> &objectCode);
    void appendSourceLine(QStringList &assemblerListingList, QStringList &listingTraceList, QList<bool> &hasCheckBox);
};

class BlankLine: public AsmCode
{
    friend class IsaAsm;
public:
    void appendObjectCode(QList<int> &objectCode);
    void appendSourceLine(QStringList &assemblerListingList, QStringList &listingTraceList, QList<bool> &hasCheckBox);
};

#endif // CODE_H
