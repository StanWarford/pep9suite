// File: asmprogram.h
/*
    Pep9 is a virtual machine for writing machine language and assembly
    language programs.

    Copyright (C) 2018 Matthew McRaven, Pepperdine University

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
#include <QtCore>
#include <QSharedPointer>

class AsmCode;
class SymbolTable;

class AsmProgram
{
public:
    explicit AsmProgram();
    explicit AsmProgram(QList<QSharedPointer<AsmCode>> programList, QSharedPointer<SymbolTable> symbolTable);
    ~AsmProgram();

    // Getters and setters for program features
    quint16 getProgramLength() const;
    const QVector<quint8> getObjectCode() const;
    const QList<QSharedPointer<AsmCode>> getProgram() const;
    QSharedPointer<SymbolTable> getSymbolTable();
    const QSharedPointer<SymbolTable> getSymbolTable() const;

    AsmCode* getCodeOnLine(quint32 line);
    const AsmCode* getCodeOnLine(quint32 line) const;
    const AsmCode* memAddressToCode(quint16 memAddress) const;
    int numberOfLines() const;
    QPair<quint16, quint16> getProgramBounds() const;

private:

    QPair<quint16, quint16> programBounds;
    QList<QSharedPointer<AsmCode>> program;
    QMap<int, quint16> indexToMemAddress;
    QMap<quint16, int> memAddressToIndex;
    quint16 programByteLength;
    QSharedPointer<SymbolTable> symTable;
    // Memory trace state
    bool traceTagWarning;

    // This map is used to map the program counter to the stringList of tags on the corresponding line
    // For example, line corresponds to 0x12, and has the comment ; Allocate #next #data
    // The stringlist would contain next and data
    static QMap<quint16, QStringList> symbolTraceList;
    // List of symbols for blocks and equates
    QStringList blockSymbols, equateSymbols;
    // This map is for global structs. The key is the symbol defined on the .BLOCK line
    // and QStringList contains the list of symbols from the symbol tags in the .BLOCK comment.
    static QMap<QString, QStringList> globalStructSymbols;

};

#endif // ASMPROGRAM_H
