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

class CpuGraphicsItems;
class SymbolEntry;
class Specification;
class CPUDataSection;
class AMemoryDevice;
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
class MicroCode: public AExecutableMicrocode
{
public:
    MicroCode(Enu::CPUType cpuType, bool useExtendedFatures);

    QString getObjectCode() const override;
    QString getSourceCode() const override;

    bool hasControlSignal(Enu::EControlSignals field) const;
    bool hasClockSignal(Enu::EClockSignals field) const;

    quint8 getControlSignal(Enu::EControlSignals field) const;
    const QVector<quint8> getControlSignals() const;
    bool getClockSignal(Enu::EClockSignals field) const;
    const QVector<bool> getClockSignals() const;


    Enu::EBranchFunctions getBranchFunction() const;

    const SymbolEntry* getTrueTarget() const;
    const SymbolEntry* getFalseTarget() const;

    bool inRange(Enu::EControlSignals field, int value) const;
    void setControlSignal(Enu::EControlSignals field, quint8 value);
    void setClockSingal(Enu::EClockSignals field,bool value);

    void setBranchFunction(Enu::EBranchFunctions branch);
    void setTrueTarget(const SymbolEntry* target);
    void setFalseTarget(const SymbolEntry* target);

private:
    Enu::CPUType cpuType;
    QVector<quint8> controlSignals;
    QVector<bool> clockSignals;
    bool extendedFeatures;
    Enu::EBranchFunctions branchFunc = Enu::Unconditional;
    const SymbolEntry* trueTargetAddr;
    const SymbolEntry* falseTargetAddr;
};

class CommentOnlyCode: public AMicroCode
{
public:
    CommentOnlyCode(QString comment);
    QString getSourceCode() const override;
private:
    QString cComment;
};

class UnitPreCode: public AMicroCode
{
public:
    ~UnitPreCode() override;
    QString getSourceCode() const override;
    bool hasUnitPre() const override;
    void setUnitPre(CPUDataSection* data, AMemoryDevice* memoryDevice);
    void appendSpecification(Specification *specification);
    void setComment(QString comment);
private:
    QList<Specification *> unitPreList;
    QString cComment;
};

class UnitPostCode: public AMicroCode
{
public:
    ~UnitPostCode() override;
    QString getSourceCode() const override;
    bool testPostcondition(CPUDataSection *data, AMemoryDevice* memoryDevice, QString &err);
    void appendSpecification(Specification *specification);
    void setComment(QString comment);
    bool hasUnitPost() const override;
private:
    QList<Specification *> unitPostList;
    QString cComment;
};

class BlankLineCode: public AMicroCode
{
};

#endif // CODE_H
