#ifndef TYPETAGS_H
#define TYPETAGS_H
// File: typetags.h
/*
    Pep9 is a virtual machine for writing machine language and assembly
    language programs.

    Copyright (C) 2019  Matthew McRaven, Pepperdine University

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
#include <QtCore>
#include "enu.h"

class SymbolEntry;
/*
 * The Pep9 environment supports a minimal type system.
 * Symbols (specified through a .BLOCK, .WORD, .BYTE, or .EQUATE) may have a primitive type
 * of 2 byte hex, 2 byte dec, 1 byte hex, 1 byte dec, or 1 byte ascii character.
 * Symbols may also be arrays of these primitive types. Symbols may not be arrays of structs
 * Symbols may also be structs composed of any other type.
 */

/*
 * Abstract base class that represents a type
 */
class AType {
public:
    virtual ~AType();
    virtual QList<QPair<Enu::ESymbolFormat, QString>> toPrimitives(QString prefix = "") const = 0;
    virtual QString toString(QString prefix = QString("")) const = 0;
    virtual quint16 size() const = 0;
    virtual operator QString() const = 0;
};

/*
 * Class to represent the fundamental display types in Pep9, which are
 * #2h, #2d, #1h, #1d, #1c.
 */
class PrimitiveType: public AType {
    QSharedPointer<const SymbolEntry> symbol;
    Enu::ESymbolFormat format;
public:
    explicit PrimitiveType(QSharedPointer<const SymbolEntry> symbol, Enu::ESymbolFormat format);
    ~PrimitiveType() override;
    QList<QPair<Enu::ESymbolFormat, QString>> toPrimitives(QString prefix = "") const override;
    QString toString(QString prefix = QString("")) const override;
    quint16 size() const override;
    operator QString() const override;

};
/*
 * Class to represent the fundamental display types in Pep9, which are
 * #2h, #2d, #1h, #1d, #1c.
 */
class LiteralPrimitiveType: public AType {
    QString name;
    Enu::ESymbolFormat format;
public:
    explicit LiteralPrimitiveType(QString name, Enu::ESymbolFormat format);
    ~LiteralPrimitiveType() override;
    QList<QPair<Enu::ESymbolFormat, QString>> toPrimitives(QString prefix = "") const override;
    QString toString(QString prefix = QString("")) const override;
    quint16 size() const override;
    operator QString() const override;

};
/*
 * Class to represent a C-style POD struct.
 */
class StructType: public AType {
    QSharedPointer<const SymbolEntry> symbol;
    QList<QSharedPointer<AType>> members;
public:
    explicit StructType(QSharedPointer<const SymbolEntry> symbol, QList<QSharedPointer<AType>> members);
    ~StructType() override;
    QList<QPair<Enu::ESymbolFormat, QString>> toPrimitives(QString prefix = "") const override;
    QString toString(QString prefix = QString("")) const override;
    quint16 size() const override;
    operator QString() const override;
};
/*
 * Class to represent an array of primitive types
 */
class LiteralArrayType : public AType {
    Enu::ESymbolFormat format;
    quint16 len;
public:
    explicit LiteralArrayType(Enu::ESymbolFormat format, quint16 len);
    ~LiteralArrayType() override;
    QList<QPair<Enu::ESymbolFormat, QString>> toPrimitives(QString prefix = "") const override;
    QString toString(QString prefix = QString("")) const override;
    quint16 size() const override;
    operator QString() const override;
};

/*
 * Class to represent an array of primitive types
 */
class ArrayType : public AType {
    QSharedPointer<const SymbolEntry> symbol;
    Enu::ESymbolFormat format;
    quint16 len;
public:
    explicit ArrayType(QSharedPointer<const SymbolEntry> symbol, Enu::ESymbolFormat format, quint16 len);
    ~ArrayType() override;
    QList<QPair<Enu::ESymbolFormat, QString>> toPrimitives(QString prefix = "") const override;
    QString toString(QString prefix = QString("")) const override;
    quint16 size() const override;
    operator QString() const override;
};
QDebug operator<<(QDebug os, const QSharedPointer<AType>& item);
QDebug operator<<(QDebug os, const QSharedPointer<const AType>& item);

#endif // TYPETAGS_H
