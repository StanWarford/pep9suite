// File: asmcode.cpp
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
//Force annotation of commands to be added to program listing.
//Not recommended for deploying the application.
#define ForceShowAnnotation true
#include <QRegExp>
#include <QSharedPointer>
#include <utility>

#include "assembler/asmcode.h"
#include "assembler/asmargument.h"
#include "assembler/asmparserhelper.h"
#include "pep/pep.h"
#include "symbol/symbolvalue.h"
#include "symbol/symbolentry.h"
#include "symbol/symboltable.h"

AsmCode::AsmCode(): emitObjectCode(true), hasCom(false),
    sourceCodeLine(0), listingCodeLine(0), memAddress(0),
    symbolEntry(QSharedPointer<SymbolEntry>()), comment()
{

}

AsmCode::AsmCode(const AsmCode &other)
{
    this->emitObjectCode = other.emitObjectCode;
    this->hasCom = other.hasCom;
    this->sourceCodeLine = other.sourceCodeLine;
    this->listingCodeLine = other.listingCodeLine;
    this->memAddress = other.memAddress;
    this->symbolEntry = other.symbolEntry;
    this->comment = other.comment;
    //this->trace = other.trace;
}

DotAddrss::DotAddrss(const DotAddrss &other) : AsmCode(other)
{
    this->argument = other.argument;
}

DotAlign::DotAlign(const DotAlign &other) : AsmCode(other)
{
    this->argument = other.argument;
    this->numBytesGenerated = other.numBytesGenerated;
}

DotAscii::DotAscii(const DotAscii &other) : AsmCode(other)
{
    this->argument = other.argument;
}

DotBlock::DotBlock(const DotBlock &other) : AsmCode(other)
{
    this->argument = other.argument;
}

DotBurn::DotBurn(const DotBurn &other) : AsmCode(other)
{
    this->argument = other.argument;
}

DotByte::DotByte(const DotByte &other) : AsmCode(other)
{
    this->argument = other.argument;
}

DotEnd::DotEnd(const DotEnd &other) : AsmCode(other)
{

}

DotEquate::DotEquate(const DotEquate &other) : AsmCode(other)
{
    this->argument = other.argument;
}

DotWord::DotWord(const DotWord &other) : AsmCode(other)
{
    this->argument = other.argument;
}

CommentOnly::CommentOnly(const CommentOnly &other)  : AsmCode(other)
{

}

BlankLine::BlankLine(const BlankLine &other)  : AsmCode(other)
{

}

DotAddrss &DotAddrss::operator=(DotAddrss other)
{
    swap(*this, other);
    return *this;
}

DotAlign &DotAlign::operator=(DotAlign other)
{
    swap(*this, other);
    return *this;
}

DotAscii &DotAscii::operator=(DotAscii other)
{
    swap(*this, other);
    return *this;
}

DotBlock &DotBlock::operator=(DotBlock other)
{
    swap(*this, other);
    return *this;
}

DotBurn &DotBurn::operator=(DotBurn other)
{
    swap(*this, other);
    return *this;
}

DotByte &DotByte::operator=(DotByte other)
{
    swap(*this, other);
    return *this;
}

DotEnd &DotEnd::operator=(DotEnd other)
{
    swap(*this, other);
    return *this;
}

DotEquate &DotEquate::operator=(DotEquate other)
{
    swap(*this, other);
    return *this;
}

DotWord &DotWord::operator=(DotWord other)
{
    swap(*this, other);
    return *this;
}

CommentOnly &CommentOnly::operator=(CommentOnly other)
{
    swap(*this, other);
    return *this;
}

BlankLine &BlankLine::operator=(BlankLine other)
{
    swap(*this, other);
    return *this;
}

AsmCode::~AsmCode() = default;

void AsmCode::setEmitObjectCode(bool emitObject)
{
    emitObjectCode = emitObject;
}

bool AsmCode::getEmitObjectCode() const
{
    return emitObjectCode;
}

bool AsmCode::hasComment() const
{
    return hasCom;
}

QString AsmCode::getComment() const
{
    return comment;
}

void AsmCode::setComment(QString comment)
{
    this->comment = std::move(comment);
    this->hasCom = !this->comment.isEmpty();
}

int AsmCode::getMemoryAddress() const
{
    return memAddress;
}

void AsmCode::setMemoryAddress(quint16 address)
{
    this->memAddress = address;
}

void AsmCode::adjustMemAddress(int addressDelta)
{
    // Memory addresses less than 0 are invalid or don't
    // have a real address (like comments), so they can't
    // be relocated.
    if(memAddress >=0) memAddress += addressDelta;
}

int AsmCode::getSourceLineNumber() const
{
    return sourceCodeLine;
}

void AsmCode::setSourceLineNumber(quint32 lineNumber)
{
    this->sourceCodeLine = lineNumber;
}

int AsmCode::getListingLineNumber() const
{
    return listingCodeLine;
}

void AsmCode::setListingLineNumber(quint32 lineNumber)
{
    this->listingCodeLine = lineNumber;
}

AsmCode *DotAddrss::cloneAsmCode() const
{
    return new DotAddrss(*this);
}

AsmCode *DotAlign::cloneAsmCode() const
{
    return new DotAlign(*this);
}

AsmCode *DotAscii::cloneAsmCode() const
{
    return new DotAscii(*this);
}

AsmCode *DotBlock::cloneAsmCode() const
{
    return new DotBlock(*this);
}

AsmCode *DotBurn::cloneAsmCode() const
{
    return new DotBurn(*this);
}

AsmCode *DotByte::cloneAsmCode() const
{
    return new DotByte(*this);
}

AsmCode *DotEnd::cloneAsmCode() const
{
    return new DotEnd(*this);
}

AsmCode *DotEquate::cloneAsmCode() const
{
    return new DotEquate(*this);
}

AsmCode *DotWord::cloneAsmCode() const
{
    return new DotWord(*this);
}

AsmCode *CommentOnly::cloneAsmCode() const
{
    return new CommentOnly(*this);
}

AsmCode *BlankLine::cloneAsmCode() const
{
    return new BlankLine(*this);
}

void DotAddrss::appendObjectCode(QList<int> &objectCode) const
{
    if(!emitObjectCode) return;
    int symbolValue = this->argument->getArgumentValue();
    objectCode.append(symbolValue / 256);
    objectCode.append(symbolValue % 256);

}

void DotAlign::appendObjectCode(QList<int> &objectCode) const
{
    if(!emitObjectCode) return;
    for (int i = 0; i < numBytesGenerated->getArgumentValue(); i++) {
        objectCode.append(0);
    }
}

void DotAscii::appendObjectCode(QList<int> &objectCode) const
{
    if(!emitObjectCode) return;
        int value;
        QString str = argument->getArgumentString();
        str.remove(0, 1); // Remove the leftmost double quote.
        str.chop(1); // Remove the rightmost double quote.
        while (str.length() > 0) {
            unquotedStringToInt(str, value);
            objectCode.append(value);
        }
}

void DotBlock::appendObjectCode(QList<int> &objectCode) const
{
    if(!emitObjectCode) return;
    for (int i = 0; i < argument->getArgumentValue(); i++) {
        objectCode.append(0);
    }
}

void DotByte::appendObjectCode(QList<int> &objectCode) const
{
    if(!emitObjectCode) return;
    objectCode.append(argument->getArgumentValue());
}

void DotWord::appendObjectCode(QList<int> &objectCode) const
{
    if(!emitObjectCode) return;
    int value = argument->getArgumentValue();
    objectCode.append(value / 256);
    objectCode.append(value % 256);
}

QString DotAddrss::getAssemblerListing() const
{
    QString memStr = QString("%1").arg(memAddress, 4, 16, QLatin1Char('0')).toUpper();
    int symbolValue = this->argument->getArgumentValue();

    // Potentially skip codegen
    QString codeStr;
    if(emitObjectCode) {
        codeStr = QString("%1").arg(symbolValue, 4, 16, QLatin1Char('0')).toUpper();
    }
    else {
        codeStr = "";
    }

    QString lineStr = QString("%1%2%3")
                      .arg(memStr, -6, QLatin1Char(' '))
                      .arg(codeStr, -7, QLatin1Char(' '))
                      .arg(getAssemblerSource());
    return lineStr;
}

QString DotAddrss::getAssemblerSource() const
{
    QString symbolStr;
    if (!symbolEntry.isNull()) {
        symbolStr = symbolEntry->getName()+":";
    }
    QString dotStr = ".ADDRSS";
    QString oprndStr = argument->getArgumentString();
    QString lineStr = QString("%1%2%3%4")
                      .arg(symbolStr, -9, QLatin1Char(' '))
                      .arg(dotStr, -8, QLatin1Char(' '))
                      .arg(oprndStr, -12)
                      .arg(comment);
    return lineStr;
}

QString DotAlign::getAssemblerListing() const
{
    int numBytes = numBytesGenerated->getArgumentValue();
    QString memStr = numBytes == 0 ? "      " : QString("%1").arg(memAddress, 4, 16, QLatin1Char('0')).toUpper();
    // Potentially skip codegen
    QString codeStr = "";
    while (emitObjectCode && (numBytes > 0) && (codeStr.length() < 6)) {
        codeStr.append("00");
        numBytes--;
    }
    QString symbolStr;
    if (!symbolEntry.isNull()) {
        symbolStr = symbolEntry->getName()+":";
    }
    QString dotStr = ".ALIGN";
    QString oprndStr = argument->getArgumentString();
    QString lineStr = QString("%1%2%3%4%5%6")
            .arg(memStr, -6, QLatin1Char(' '))
            .arg(codeStr, -7, QLatin1Char(' '))
            .arg(symbolStr, -9, QLatin1Char(' '))
            .arg(dotStr, -8, QLatin1Char(' '))
            .arg(oprndStr, -12)
            .arg(comment);
    while (numBytes > 0) {
        codeStr = "";
        while ((numBytes > 0) && (codeStr.length() < 6)) {
            codeStr.append("00");
            numBytes--;
        }
        lineStr.append(QString("\n      %1").arg(codeStr, -7, QLatin1Char(' ')));
    }
    return lineStr;
}

QString DotAlign::getAssemblerSource() const
{
    QString symbolStr;
    if (!symbolEntry.isNull()) {
        symbolStr = symbolEntry->getName()+":";
    }
    QString dotStr = ".ALIGN";
    QString oprndStr = argument->getArgumentString();
    QString lineStr = QString("%1%2%3%4")
            .arg(symbolStr, -9, QLatin1Char(' '))
            .arg(dotStr, -8, QLatin1Char(' '))
            .arg(oprndStr, -12)
            .arg(comment);
    return lineStr;
}

QString DotAscii::getAssemblerListing() const
{
    QString memStr = QString("%1").arg(memAddress, 4, 16, QLatin1Char('0')).toUpper();
    QString str = argument->getArgumentString();
    str.remove(0, 1); // Remove the leftmost double quote.
    str.chop(1); // Remove the rightmost double quote.
    int value;
    // Potentially skip codegen
    QString codeStr = "";
    while (emitObjectCode && (str.length() > 0) && (codeStr.length() < 6)) {
        unquotedStringToInt(str, value);
        codeStr.append(QString("%1").arg(value, 2, 16, QLatin1Char('0')).toUpper());
    }

    QString symbolStr;
    if (!symbolEntry.isNull()) {
        symbolStr = symbolEntry->getName()+":";
    }
    QString dotStr = ".ASCII";
    QString oprndStr = argument->getArgumentString();
    QString lineStr = QString("%1%2%3%4%5%6\n")
            .arg(memStr, -6, QLatin1Char(' '))
            .arg(codeStr, -7, QLatin1Char(' '))
            .arg(symbolStr, -9, QLatin1Char(' '))
            .arg(dotStr, -8, QLatin1Char(' '))
            .arg(oprndStr, -12)
            .arg(comment);
    while (str.length() > 0) {
        codeStr = "";
        while ((str.length() > 0) && (codeStr.length() < 6)) {
            unquotedStringToInt(str, value);
            codeStr.append(QString("%1").arg(value, 2, 16, QLatin1Char('0')).toUpper());
        }
        lineStr.append(QString("      %1").arg(codeStr, -7, QLatin1Char(' '))%"\n");
    }
    return lineStr;
}

QString DotAscii::getAssemblerSource() const
{
    QString symbolStr;
    if (!symbolEntry.isNull()) {
        symbolStr = symbolEntry->getName()+":";
    }
    QString dotStr = ".ASCII";
    QString oprndStr = argument->getArgumentString();
    QString lineStr = QString("%1%2%3%4\n")
            .arg(symbolStr, -9, QLatin1Char(' '))
            .arg(dotStr, -8, QLatin1Char(' '))
            .arg(oprndStr, -12)
            .arg(comment);
    return lineStr;
}

QString DotBlock::getAssemblerListing() const
{
    QString memStr = QString("%1").arg(memAddress, 4, 16, QLatin1Char('0')).toUpper();
    int numBytes = argument->getArgumentValue();
    // Potentially skip codegen
    QString codeStr = "";
    while (emitObjectCode && (numBytes > 0) && (codeStr.length() < 6)) {
        codeStr.append("00");
        numBytes--;
    }
    QString symbolStr;
    if (!symbolEntry.isNull()) {
        symbolStr = symbolEntry->getName()+":";
    }
    QString dotStr = ".BLOCK";
    QString oprndStr = argument->getArgumentString();
    QString lineStr = QString("%1%2%3%4%5%6")
            .arg(memStr, -6, QLatin1Char(' '))
            .arg(codeStr, -7, QLatin1Char(' '))
            .arg(symbolStr, -9, QLatin1Char(' '))
            .arg(dotStr, -8, QLatin1Char(' '))
            .arg(oprndStr, -12)
            .arg(comment);
    /*if(ForceShowAnnotation) {
        for(const auto& command : getTraceData()) {
            lineStr.append(QString(" %1").arg(command.toString()));
        }
    }*/
    while (emitObjectCode && numBytes > 0) {
        codeStr = "";
        while ((numBytes > 0) && (codeStr.length() < 6)) {
            codeStr.append("00");
            numBytes--;
        }
        lineStr.append(QString("\n      %1").arg(codeStr, -7, QLatin1Char(' ')));
    }
    return lineStr;
}

QString DotBlock::getAssemblerSource() const
{
    QString symbolStr;
    if (!symbolEntry.isNull()) {
        symbolStr = symbolEntry->getName()+":";
    }
    QString dotStr = ".BLOCK";
    QString oprndStr = argument->getArgumentString();
    QString lineStr = QString("%1%2%3%4")
            .arg(symbolStr, -9, QLatin1Char(' '))
            .arg(dotStr, -8, QLatin1Char(' '))
            .arg(oprndStr, -12)
            .arg(comment);
    return lineStr;
}

QString DotBurn::getAssemblerListing() const
{
    QString memStr = QString("%1").arg(memAddress, 4, 16, QLatin1Char('0')).toUpper();
    QString symbolStr;
    if (!symbolEntry.isNull()) {
        symbolStr = symbolEntry->getName()+":";
    }
    QString dotStr = ".BURN";
    QString oprndStr = argument->getArgumentString();
    QString lineStr = QString("%1       %2%3%4%5")
            .arg(memStr, -6, QLatin1Char(' '))
            .arg(symbolStr, -9, QLatin1Char(' '))
            .arg(dotStr, -8, QLatin1Char(' '))
            .arg(oprndStr, -12)
            .arg(comment);
    return lineStr;
}

QString DotBurn::getAssemblerSource() const
{
    QString symbolStr;
    if (!symbolEntry.isNull()) {
        symbolStr = symbolEntry->getName()+":";
    }
    QString dotStr = ".BURN";
    QString oprndStr = argument->getArgumentString();
    QString lineStr = QString("%1%2%3%4")
            .arg(symbolStr, -9, QLatin1Char(' '))
            .arg(dotStr, -8, QLatin1Char(' '))
            .arg(oprndStr, -12)
            .arg(comment);
    return lineStr;
}

QSharedPointer<AsmArgument>DotBurn::getArgument() const
{
    return this->argument;
}

void DotBurn::setArgument(QSharedPointer<AsmArgument>argument)
{
    this->argument = std::move(argument);
}

QString DotByte::getAssemblerListing() const
{
    QString memStr = QString("%1").arg(memAddress, 4, 16, QLatin1Char('0')).toUpper();
    // Potentially skip codegen
    QString codeStr;
    if(emitObjectCode) {
        codeStr = QString("%1").arg(argument->getArgumentValue(), 2, 16, QLatin1Char('0')).toUpper();
    }
    else {
        codeStr = "";
    }
    QString lineStr = QString("%1%2%3")
            .arg(memStr, -6, QLatin1Char(' '))
            .arg(codeStr, -7, QLatin1Char(' '))
            .arg(getAssemblerSource());
    /*if(ForceShowAnnotation) {
        for(const auto& command : getTraceData()) {
            lineStr.append(QString(" %1").arg(command.toString()));
        }
    }*/
    return lineStr;
}

QString DotByte::getAssemblerSource() const
{
    QString symbolStr;
    if (!symbolEntry.isNull()) {
        symbolStr = symbolEntry->getName()+":";
    }
    QString dotStr = ".BYTE";
    QString oprndStr = argument->getArgumentString();
    if (oprndStr.startsWith("0x")) {
        oprndStr.remove(2, 2); // Display only the last two hex characters
    }
    QString lineStr = QString("%1%2%3%4")
            .arg(symbolStr, -9, QLatin1Char(' '))
            .arg(dotStr, -8, QLatin1Char(' '))
            .arg(oprndStr, -12)
            .arg(comment);
    return lineStr;
}

QString DotEnd::getAssemblerListing() const
{
    QString memStr = QString("%1").arg(memAddress, 4, 16, QLatin1Char('0')).toUpper();
    QString symbolStr;
    if (!symbolEntry.isNull()) {
        symbolStr = symbolEntry->getName()+":";
    }
    QString dotStr = ".END";
    QString lineStr = QString("%1       %2")
            .arg(memStr, -6, QLatin1Char(' '))
            .arg(getAssemblerSource());
    return lineStr;
}

QString DotEnd::getAssemblerSource() const
{
    QString symbolStr;
    if (!symbolEntry.isNull()) {
        symbolStr = symbolEntry->getName()+":";
    }
    QString dotStr = ".END";
    QString lineStr = QString("%1%2              %3")
            .arg(symbolStr, -9, QLatin1Char(' '))
            .arg(dotStr, -8, QLatin1Char(' '))
            .arg(comment);
    return lineStr;
}

QString DotEquate::getAssemblerListing() const
{
    return "             " + getAssemblerSource();
}

QString DotEquate::getAssemblerSource() const
{
    QString symbolStr;
    if (!symbolEntry.isNull()) {
        symbolStr = symbolEntry->getName()+":";
    }
    QString dotStr = ".EQUATE";
    QString oprndStr = argument->getArgumentString();
    QString lineStr = QString("%1%2%3%4")
            .arg(symbolStr, -9, QLatin1Char(' '))
            .arg(dotStr, -8, QLatin1Char(' '))
            .arg(oprndStr, -12)
            .arg(comment);
    return lineStr;
}

QSharedPointer<AsmArgument>DotEquate::getArgument() const
{
    return this->argument;
}

void DotEquate::setArgument(QSharedPointer<AsmArgument>argument)
{
    this->argument = std::move(argument);
}

bool DotEquate::tracksTraceTags() const
{
    return true;
}

QString DotWord::getAssemblerListing() const
{
    QString memStr = QString("%1").arg(memAddress, 4, 16, QLatin1Char('0')).toUpper();
    // Potentially skip codegen
    QString codeStr;
    if(emitObjectCode) {
        codeStr = QString("%1").arg(argument->getArgumentValue(), 4, 16, QLatin1Char('0')).toUpper();
    }
    else {
        codeStr = "";
    }

    QString dotStr = ".WORD";
    QString oprndStr = argument->getArgumentString();
    QString lineStr = QString("%1%2%3")
            .arg(memStr, -6, QLatin1Char(' '))
            .arg(codeStr, -7, QLatin1Char(' '))
            .arg(getAssemblerSource());
    /*if(ForceShowAnnotation) {
        for(const auto& command : getTraceData()) {
            lineStr.append(QString(" %1").arg(command.toString()));
        }
    }*/
    return lineStr;
}

QString DotWord::getAssemblerSource() const
{
    QString symbolStr;
    if (!symbolEntry.isNull()) {
        symbolStr = symbolEntry->getName()+":";
    }
    QString dotStr = ".WORD";
    QString oprndStr = argument->getArgumentString();
    QString lineStr = QString("%1%2%3%4")
            .arg(symbolStr, -9, QLatin1Char(' '))
            .arg(dotStr, -8, QLatin1Char(' '))
            .arg(oprndStr, -12)
            .arg(comment);
    return lineStr;
}

QString CommentOnly::getAssemblerListing() const
{
    return "             " + getAssemblerSource();
}

QString CommentOnly::getAssemblerSource() const
{
    return comment;
}

QString BlankLine::getAssemblerListing() const
{
    return "";
}

QString BlankLine::getAssemblerSource() const
{
    return "";
}

quint16 DotAddrss::objectCodeLength() const
{
    return 2;
}

bool DotAddrss::hasSymbolicOperand() const
{
    return true;
}

QSharedPointer<const SymbolEntry> DotAddrss::getSymbolicOperand() const
{
    // The value of a .addrss instruction is always the value of another symbol.
    return static_cast<SymbolRefArgument*>(argument.get())->getSymbolValue();
}

QSharedPointer<AsmArgument>DotAddrss::getArgument() const
{
    return this->argument;
}

void DotAddrss::setArgument(QSharedPointer<AsmArgument>argument)
{
    this->argument = std::move(argument);
}

quint16 DotAlign::objectCodeLength() const
{
    if(emitObjectCode) {
        return static_cast<quint16>(numBytesGenerated->getArgumentValue());
    }
    else {
        return 0;
    }
}

QSharedPointer<AsmArgument>DotAlign::getArgument() const
{
    return this->argument;
}

void DotAlign::setArgument(QSharedPointer<AsmArgument>argument)
{
    this->argument = std::move(argument);
}

quint16 DotAlign::getNumBytesGenerated() const
{
    return numBytesGenerated->getArgumentValue();
}

void DotAlign::setNumBytesGenerated(quint16 numBytes)
{
    numBytesGenerated = QSharedPointer<DecArgument>::create(numBytes);
}

quint16 DotAscii::objectCodeLength() const
{
    QList<int> num;
    appendObjectCode(num);
    return static_cast<quint16>(num.length());
}

QSharedPointer<AsmArgument>DotAscii::getArgument() const
{
    return this->argument;
}

void DotAscii::setArgument(QSharedPointer<AsmArgument> argument)
{
    this->argument = std::move(argument);
}

quint16 DotBlock::objectCodeLength() const
{
    if(emitObjectCode) {
        return static_cast<quint16>(argument->getArgumentValue());
    }
    else {
        return 0;
    }
}

QSharedPointer<AsmArgument>DotBlock::getArgument() const
{
    return this->argument;
}

void DotBlock::setArgument(QSharedPointer<AsmArgument> argument)
{
    this->argument = std::move(argument);
}

bool DotBlock::tracksTraceTags() const
{
    return true;
}

quint16 DotByte::objectCodeLength() const
{
    if(emitObjectCode) {
        return 1;
    }
    else {
        return 0;
    }
}

QSharedPointer<AsmArgument>DotByte::getArgument() const
{
    return this->argument;
}

void DotByte::setArgument(QSharedPointer<AsmArgument>argument)
{
    this->argument = std::move(argument);
}

bool DotByte::tracksTraceTags() const
{
    return true;
}

quint16 DotWord::objectCodeLength() const
{
    if(emitObjectCode) {
        return 2;
    }
    else {
        return 0;
    }
}

QSharedPointer<AsmArgument> DotWord::getArgument() const
{
    return this->argument;
}

void DotWord::setArgument(QSharedPointer<AsmArgument> argument)
{
    this->argument = std::move(argument);
}

bool DotWord::tracksTraceTags() const
{
    return true;
}










