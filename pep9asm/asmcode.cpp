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

#include <QRegExp>
#include <QSharedPointer>

#include "asmcode.h"
#include "asmargument.h"
#include "isaasm.h"
#include "pep.h"
#include "symbolvalue.h"
#include "symbolentry.h"
#include "symboltable.h"

#pragma message("TODO: The functions requesting object code must manuall check for burn count & memaddress of burn")
// appendObjectCode

AsmCode::AsmCode(): sourceCodeLine(0), memAddress(0), symbolEntry(QSharedPointer<SymbolEntry>()), comment(),
    emitObjectCode(true), hasCom(false)
{

}

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

void AsmCode::adjustMemAddress(int addressDelta)
{
    // Memory addresses less than 0 are invalid or don't
    // have a real address (like comments), so they can't
    // be relocated.
    if(memAddress >=0) memAddress += addressDelta;
}

void UnaryInstruction::appendObjectCode(QList<int> &objectCode) const
{
    if(!emitObjectCode) return;
    objectCode.append(Pep::opCodeMap.value(mnemonic));
}

void NonUnaryInstruction::appendObjectCode(QList<int> &objectCode) const
{
    if(!emitObjectCode) return;
    int instructionSpecifier = Pep::opCodeMap.value(mnemonic);
    if (Pep::addrModeRequiredMap.value(mnemonic)) {
        instructionSpecifier += Pep::aaaAddressField(addressingMode);
    }
    else {
        instructionSpecifier += Pep::aAddressField(addressingMode);
    }
    objectCode.append(instructionSpecifier);
    int operandSpecifier = argument->getArgumentValue();
    objectCode.append(operandSpecifier / 256);
    objectCode.append(operandSpecifier % 256);
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
            IsaParserHelper::unquotedStringToInt(str, value);
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

void UnaryInstruction::appendSourceLine(QStringList &assemblerListingList) const
{
    assemblerListingList.append(getAssemblerListing());
}

void NonUnaryInstruction::appendSourceLine(QStringList &assemblerListingList) const
{
    assemblerListingList.append(getAssemblerListing());
}

void DotAlign::appendSourceLine(QStringList &assemblerListingList) const
{
    assemblerListingList.append(getAssemblerListing().split("\n"));
}

void DotAscii::appendSourceLine(QStringList &assemblerListingList) const
{
    assemblerListingList.append(getAssemblerListing().split("\n"));
}

void DotBlock::appendSourceLine(QStringList &assemblerListingList) const
{
    assemblerListingList.append(getAssemblerListing().split("\n"));
}

bool UnaryInstruction::hasBreakpoint() const
{
    return breakpoint;
}

void UnaryInstruction::setBreakpoint(bool b)
{
    breakpoint = b;
}

bool NonUnaryInstruction::hasBreakpoint() const
{
    return breakpoint;
}

void NonUnaryInstruction::setBreakpoint(bool b)
{
    breakpoint = b;
}

bool NonUnaryInstruction::hasSymbolicOperand() const
{
    return dynamic_cast<SymbolRefArgument*>(argument) != nullptr;
}

QSharedPointer<const SymbolEntry> NonUnaryInstruction::getSymbolicOperand() const
{
    return dynamic_cast<SymbolRefArgument*>(argument)->getSymbolValue();
}

Enu::EMnemonic NonUnaryInstruction::getMnemonic() const
{
    return mnemonic;
}

Enu::EAddrMode NonUnaryInstruction::getAddressingMode() const
{
    return addressingMode;
}

QString UnaryInstruction::getAssemblerListing() const
{
    QString memStr = QString("%1").arg(memAddress, 4, 16, QLatin1Char('0')).toUpper();
    // Potentially skip codegen
    QString codeStr;
    if(emitObjectCode) {
        codeStr = QString("%1").arg(Pep::opCodeMap.value(mnemonic), 2, 16, QLatin1Char('0')).toUpper();
    }
    else {
        codeStr = "  ";
    }
    QString symbolStr;
    if (!symbolEntry.isNull()) {
        symbolStr = symbolEntry->getName()+":";
    }
    QString mnemonStr = Pep::enumToMnemonMap.value(mnemonic);
    QString lineStr = QString("%1%2%3%4%5")
                      .arg(memStr, -6, QLatin1Char(' '))
                      .arg(codeStr, -7, QLatin1Char(' '))
                      .arg(symbolStr, -9, QLatin1Char(' '))
                      .arg(mnemonStr, -8, QLatin1Char(' '))
                      .arg("            " + comment);
    return lineStr;
}

QString NonUnaryInstruction::getAssemblerListing() const
{
    QString memStr = QString("%1").arg(memAddress, 4, 16, QLatin1Char('0')).toUpper();
    int temp = Pep::opCodeMap.value(mnemonic);
    temp += Pep::addrModeRequiredMap.value(mnemonic) ? Pep::aaaAddressField(addressingMode) : Pep::aAddressField(addressingMode);
    // Potentially skip codegen
    QString codeStr;
    QString oprndNumStr;
    if(emitObjectCode) {
        codeStr = QString("%1").arg(temp, 2, 16, QLatin1Char('0')).toUpper();
        oprndNumStr = QString("%1").arg(argument->getArgumentValue(), 4, 16, QLatin1Char('0')).toUpper();
    }
    else {
        codeStr = "  ";
        oprndNumStr = " ";
    }

    QString symbolStr;
    if (!symbolEntry.isNull()) {
        symbolStr = symbolEntry->getName()+":";
    }
    QString mnemonStr = Pep::enumToMnemonMap.value(mnemonic);
    QString oprndStr = argument->getArgumentString();
    if (Pep::addrModeRequiredMap.value(mnemonic)) {
        oprndStr.append("," + Pep::intToAddrMode(addressingMode));
    }
    else if (addressingMode == Enu::EAddrMode::X) {
        oprndStr.append("," + Pep::intToAddrMode(addressingMode));
    }
    QString lineStr = QString("%1%2%3%4%5%6%7")
                      .arg(memStr, -6, QLatin1Char(' '))
                      .arg(codeStr, -2)
                      .arg(oprndNumStr, -5, QLatin1Char(' '))
                      .arg(symbolStr, -9, QLatin1Char(' '))
                      .arg(mnemonStr, -8, QLatin1Char(' '))
                      .arg(oprndStr, -12)
                      .arg(comment);
    return lineStr;
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

    QString symbolStr;
    if (!symbolEntry.isNull()) {
        symbolStr = symbolEntry->getName()+":";
    }
    QString dotStr = ".ADDRSS";
    QString oprndStr = argument->getArgumentString();
    QString lineStr = QString("%1%2%3%4%5%6")
                      .arg(memStr, -6, QLatin1Char(' '))
                      .arg(codeStr, -7, QLatin1Char(' '))
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
        IsaParserHelper::unquotedStringToInt(str, value);
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
            IsaParserHelper::unquotedStringToInt(str, value);
            codeStr.append(QString("%1").arg(value, 2, 16, QLatin1Char('0')).toUpper());
        }
        lineStr.append(QString("      %1").arg(codeStr, -7, QLatin1Char(' '))%"\n");
    }
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
    QString symbolStr;
    if (!symbolEntry.isNull()) {
        symbolStr = symbolEntry->getName()+":";
    }
    QString dotStr = ".BYTE";
    QString oprndStr = argument->getArgumentString();
    if (oprndStr.startsWith("0x")) {
        oprndStr.remove(2, 2); // Display only the last two hex characters
    }
    QString lineStr = QString("%1%2%3%4%5%6")
            .arg(memStr, -6, QLatin1Char(' '))
            .arg(codeStr, -7, QLatin1Char(' '))
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
    QString lineStr = QString("%1       %2%3              %4")
            .arg(memStr, -6, QLatin1Char(' '))
            .arg(symbolStr, -9, QLatin1Char(' '))
            .arg(dotStr, -8, QLatin1Char(' '))
            .arg(comment);
    return lineStr;
}

QString DotEquate::getAssemblerListing() const
{
    QString symbolStr;
    if (!symbolEntry.isNull()) {
        symbolStr = symbolEntry->getName()+":";
    }
    QString dotStr = ".EQUATE";
    QString oprndStr = argument->getArgumentString();
    QString lineStr = QString("             %1%2%3%4")
            .arg(symbolStr, -9, QLatin1Char(' '))
            .arg(dotStr, -8, QLatin1Char(' '))
            .arg(oprndStr, -12)
            .arg(comment);
    return lineStr;
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
    QString symbolStr;
    if (!symbolEntry.isNull()) {
        symbolStr = symbolEntry->getName()+":";
    }
    QString dotStr = ".WORD";
    QString oprndStr = argument->getArgumentString();
    QString lineStr = QString("%1%2%3%4%5%6")
            .arg(memStr, -6, QLatin1Char(' '))
            .arg(codeStr, -7, QLatin1Char(' '))
            .arg(symbolStr, -9, QLatin1Char(' '))
            .arg(dotStr, -8, QLatin1Char(' '))
            .arg(oprndStr, -12)
            .arg(comment);
    return lineStr;
}

QString CommentOnly::getAssemblerListing() const
{
    return "             " + comment;
}

QString BlankLine::getAssemblerListing() const
{
    return "";
}

quint16 UnaryInstruction::objectCodeLength() const
{
    return 1;
}

quint16 NonUnaryInstruction::objectCodeLength() const
{
    return 3;
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
    return static_cast<SymbolRefArgument*>(argument)->getSymbolValue();
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

quint16 DotAscii::objectCodeLength() const
{
    QList<int> num;
    appendObjectCode(num);
    return static_cast<quint16>(num.length());
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

quint16 DotByte::objectCodeLength() const
{
    if(emitObjectCode) {
        return 1;
    }
    else {
        return 0;
    }
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
