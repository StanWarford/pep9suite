// File: microcodeprogram.h
/*
    Pep9CPU is a CPU simulator for executing microcode sequences to
    implement instructions in the instruction set of the Pep/9 computer.

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
#ifndef MICROCODEPROGRAM_H
#define MICROCODEPROGRAM_H

#include <QSharedPointer>
#include <QVector>


class AMicroCode;
class AExecutableMicrocode;
class SymbolTable;
class MicrocodeProgram
{
private:
    QSharedPointer<SymbolTable> symTable;
    QVector<AMicroCode*> programVec;
    QVector<int> preconditionsVec,postconditionsVec,microcodeVec;
public:
    MicrocodeProgram();
    ~MicrocodeProgram();
    MicrocodeProgram(QVector<AMicroCode*>objectCode, QSharedPointer<SymbolTable> symbolTable);
    QSharedPointer<const SymbolTable> getSymTable() const;
    const QVector<AMicroCode*> getObjectCode() const;
    const QString format() const;
    int codeLineToProgramLine(int codeLine) const;
    bool hasMicrocode() const;
    bool hasUnitPre() const;
    const AExecutableMicrocode* getCodeLine(quint16 codeLine) const;
    AExecutableMicrocode* getCodeLine(quint16 codeLine);
    int codeLength() const;
};

#endif // MICROCODEPROGRAM_H
