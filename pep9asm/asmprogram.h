// File: asmprogram.h
/*
    Pep9 is a virtual machine for writing machine language and assembly
    language programs.

    Copyright (C) 2018  J. Stanley Warford & Matthew McRaven, Pepperdine University

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

#ifndef ASMPROGRAM_H
#define ASMPROGRAM_H

#include <ostream>
#include <QtCore>
#include <QSharedPointer>

#include "enu.h"
class AType;
class AsmCode;
class SymbolEntry;
class SymbolTable;

// Contains meta-info about the formats and types of symbols in a program
struct StaticTraceInfo
{
    StaticTraceInfo();
    // Did the static analysis find a trace error?
    bool staticTraceError, hadTraceTags;
    // Associate a symbol with its rendering type
    QMap<QSharedPointer<const SymbolEntry>, QSharedPointer<AType>> dynamicAllocSymbolTypes,
    staticAllocSymbolTypes;

    // For the instruction located at an address, what symbols are being pushed, popped, or allocated?
    QMap<quint16, QList<QSharedPointer<AType> > > instrToSymlist;
    // Does the program have both malloc and a heap?
    bool hasHeapMalloc;
    // If they exist, store the pointer to their values
    QSharedPointer<const SymbolEntry> heapPtr, mallocPtr;
};


class AsmProgram
{
public:
    explicit AsmProgram();
    explicit AsmProgram(QList<QSharedPointer<AsmCode>> programList, QSharedPointer<SymbolTable> symbolTable, QSharedPointer<const StaticTraceInfo> traceInfo);
    explicit AsmProgram(QList<QSharedPointer<AsmCode>> programList, QSharedPointer<SymbolTable> symbolTable, QSharedPointer<const StaticTraceInfo> traceInfo, quint16 burnAddress, quint16 burnValue);
    ~AsmProgram();

    // Getters and setters for program features
    quint16 getProgramLength() const;
    const QVector<quint8> getObjectCode() const;
    const QList<QSharedPointer<AsmCode>> getProgram() const;
    QSharedPointer<SymbolTable> getSymbolTable();
    const QSharedPointer<SymbolTable> getSymbolTable() const;

    bool hasBurn() const;
    // Get the address of the line containing the .BURN statement
    quint16 getBurnAddress() const;
    // Get the value of the .BURN statement
    quint16 getBurnValue() const;

    AsmCode* getCodeAtIndex(quint32 line);
    const AsmCode* getCodeAtIndex(quint32 line) const;
    const AsmCode* memAddressToCode(quint16 memAddress) const;
    int numberOfLines() const;
    QPair<quint16, quint16> getProgramBounds() const;
    QSharedPointer<const StaticTraceInfo> getTraceInfo() const;


    // Get code properly formatted for the source code pane
    QString getFormattedSourceCode() const;
    // Get formatted code of the style as used in the listing.
    QString getProgramListingCode() const;
    // Return the program listing including header + footer.
    QString getProgramListing() const;


private:
    QPair<quint16, quint16> programBounds;
    QList<QSharedPointer<AsmCode>> program;
    QMap<int, quint16> indexToMemAddress;
    QMap<quint16, int> memAddressToIndex;
    quint16 programByteLength;
    QSharedPointer<SymbolTable> symTable;
    QSharedPointer<const StaticTraceInfo> traceInfo;

    bool burn;
    quint16 burnAddress, burnValue;
};

#endif // ASMPROGRAM_H
