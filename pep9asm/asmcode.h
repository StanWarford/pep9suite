// File: asmcode.h
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
#include <QSharedPointer>
#include <QSet>
class AsmArgument; // Forward declaration for attributes of code classes.
class SymbolEntry;

/*
 * Abstract Code class that represents a single line of assembly code.
 * It contains methods for generating object code, & pretty-printing source code.
 * It assists the memory trace pane's stack frames with trace tag processing.
 * It also provides default implementations for most functions to reduce code reuse in subclasses.
 */
class AsmCode
{
    friend class IsaAsm;
public:
    AsmCode();
    virtual ~AsmCode() { }
    bool hasSymbolEntry() const {return !symbolEntry.isNull();}
    // Before attempting to use the value return by this function, check if the symbol is null.
    // Dereferencing an empty shared pointer causes memory access violatations that are hard to debug.
    QSharedPointer<const SymbolEntry> getSymbolEntry() const {return symbolEntry;}
    void setEmitObjectCode(bool emitObject);
    bool getEmitObjectCode() const;
    virtual bool hasComment() const;
    QString getComment() const;

    virtual void appendObjectCode(QList<int> &) const { return; }
    virtual void appendSourceLine(QStringList &assemblerListingList) const{ assemblerListingList.append(getAssemblerListing()); }
    void adjustMemAddress(int addressDelta);
    //virtual bool processFormatTraceTags(int &, QString &, SymbolListings &) { return true; }
    //virtual bool processSymbolTraceTags(int &, QString &, SymbolListings &) { return true; }
    virtual int getMemoryAddress() const {return memAddress; }
    virtual QString getAssemblerListing() const = 0;
    virtual QString getAssemblerTrace() const { return getAssemblerListing(); }
    virtual quint16 objectCodeLength() const {return 0;}
    virtual bool hasBreakpoint() const { return false;}
    virtual void setBreakpoint(bool) {}
    virtual bool hasSymbolicOperand() const {return false;}
    virtual QSharedPointer<const SymbolEntry> getSymbolicOperand() const { return nullptr;}
protected:
    int sourceCodeLine, memAddress =-1;
    QSharedPointer<SymbolEntry> symbolEntry;
    QString comment;
    bool emitObjectCode, hasCom;
};

// Concrete code classes
class UnaryInstruction: public AsmCode
{
    friend class IsaAsm;
private:
    Enu::EMnemonic mnemonic;
    bool breakpoint = false;
public:
    virtual void appendObjectCode(QList<int> &objectCode) const override;
    virtual void appendSourceLine(QStringList &assemblerListingList) const override;
//    NO LONGER WITH PEP/9. FOR RET1, RET2, ..., RET7
//    bool processSymbolTraceTags(int &sourceLine, QString &errorString);

    // AsmCode interface
    virtual QString getAssemblerListing() const override;
    virtual quint16 objectCodeLength() const override;
    virtual bool hasBreakpoint() const override;
    virtual void setBreakpoint(bool b) override;
};

class NonUnaryInstruction: public AsmCode
{
    friend class IsaAsm;
private:
    Enu::EMnemonic mnemonic;
    Enu::EAddrMode addressingMode;
    AsmArgument *argument = nullptr;
    bool breakpoint = false;
public:
    // ~NonUnaryInstruction() { delete argument; }
    virtual void appendObjectCode(QList<int> &objectCode) const override;
    virtual void appendSourceLine(QStringList &assemblerListingList) const override;
    //virtual bool processFormatTraceTags(int &sourceLine, QString &errorString, SymbolListings & symbolListing) override;
    //virtual bool processSymbolTraceTags(int &sourceLine, QString &errorString, SymbolListings & symbolListing) override;

    // AsmCode interface
    virtual QString getAssemblerListing() const override;
    virtual quint16 objectCodeLength() const override;
    virtual bool hasBreakpoint() const override;
    virtual void setBreakpoint(bool b) override;
    bool hasSymbolicOperand() const override;
    QSharedPointer<const SymbolEntry> getSymbolicOperand() const override;
    Enu::EMnemonic getMnemonic() const;
    Enu::EAddrMode getAddressingMode() const;
};

class DotAddrss: public AsmCode
{
    friend class IsaAsm;
private:
    AsmArgument *argument = nullptr;
public:
     virtual void appendObjectCode(QList<int> &objectCode) const override;

    // AsmCode interface
    virtual QString getAssemblerListing() const override;
    virtual quint16 objectCodeLength() const override;
    bool hasSymbolicOperand() const override;
    QSharedPointer<const SymbolEntry> getSymbolicOperand() const override;
};

class DotAlign: public AsmCode
{
    friend class IsaAsm;
private:
    AsmArgument *argument = nullptr;
    AsmArgument *numBytesGenerated = nullptr;
public:
    virtual void appendObjectCode(QList<int> &objectCode) const override;
    virtual void appendSourceLine(QStringList &assemblerListingList) const override;

    // AsmCode interface
    virtual QString getAssemblerListing() const override;
    virtual quint16 objectCodeLength() const override;

};

class DotAscii: public AsmCode
{
    friend class IsaAsm;
private:
    AsmArgument *argument = nullptr;
public:
    virtual void appendObjectCode(QList<int> &objectCode) const override;
    virtual void appendSourceLine(QStringList &assemblerListingList) const override;

    // AsmCode interface
    virtual QString getAssemblerListing() const override;
    virtual quint16 objectCodeLength() const override;
};

class DotBlock: public AsmCode
{
    friend class IsaAsm;
private:
    AsmArgument *argument;
public:
    virtual void appendObjectCode(QList<int> &objectCode) const override;
    virtual void appendSourceLine(QStringList &assemblerListingList) const override;
    //virtual bool processFormatTraceTags(int &sourceLine, QString &errorString, SymbolListings & symbolListing) override;
    //virtual bool processSymbolTraceTags(int &sourceLine, QString &errorString, SymbolListings & symbolListing) override;

    // AsmCode interface
    virtual QString getAssemblerListing() const override;
    virtual quint16 objectCodeLength() const override;
};

class DotBurn: public AsmCode
{
    friend class IsaAsm;
private:
    AsmArgument *argument = nullptr;

public:
    // AsmCode interface
    virtual QString getAssemblerListing() const override;
};

class DotByte: public AsmCode
{
    friend class IsaAsm;
private:
    AsmArgument *argument = nullptr;

public:
    virtual void appendObjectCode(QList<int> &objectCode) const override;
    // AsmCode interface
    virtual QString getAssemblerListing() const override;
    virtual quint16 objectCodeLength() const override;
};

class DotEnd: public AsmCode
{
    friend class IsaAsm;

public:
    // AsmCode interface
    virtual QString getAssemblerListing() const override;
};

class DotEquate: public AsmCode
{
    friend class IsaAsm;
private:
    AsmArgument *argument = nullptr;
public:
    //virtual bool processFormatTraceTags(int &sourceLine, QString &errorString, SymbolListings & symbolListing) override;
    // AsmCode interface
    virtual QString getAssemblerListing() const override;
};

class DotWord: public AsmCode
{
    friend class IsaAsm;
private:
    AsmArgument *argument = nullptr;
public:
    virtual void appendObjectCode(QList<int> &objectCode) const override;
    // AsmCode interface
    virtual QString getAssemblerListing() const override;
    virtual quint16 objectCodeLength() const override;
};

class CommentOnly: public AsmCode
{
    friend class IsaAsm;

public:
    // AsmCode interface
    virtual QString getAssemblerListing() const override;
};

class BlankLine: public AsmCode
{
    friend class IsaAsm;

public:
    // AsmCode interface
    virtual QString getAssemblerListing() const override;
};

#endif // CODE_H
