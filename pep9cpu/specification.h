// File: specification.h
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
#ifndef SPECIFICATION_H
#define SPECIFICATION_H

#include <QString>
#include "enu.h"
#include "cpupane.h"


class NewCPUDataSection; //Forward declare CPUDataSection to avoid inclusion loops
class AMemoryDevice;
class Specification
{
public:
    virtual ~Specification() { }
    Specification();
    virtual void setUnitPre(NewCPUDataSection*) { }
    virtual bool testUnitPost(NewCPUDataSection*, QString&){return true;}
    virtual QString getSourceCode() = 0;
};

class MemSpecification: public Specification {    
public:
    MemSpecification(AMemoryDevice* mem, int memoryAddress, int memoryValue, int numberBytes);
    void setUnitPre(NewCPUDataSection*) override;
    bool testUnitPost(NewCPUDataSection *data, QString &errString) override;
    QString getSourceCode() override;
private:
    AMemoryDevice* memDevice;
    int memAddress;
    int memValue;
    int numBytes;
};

class RegSpecification: public Specification {
public:
    RegSpecification(Enu::ECPUKeywords registerAddress, int registerValue);
    void setUnitPre(NewCPUDataSection*) override;
    bool testUnitPost(NewCPUDataSection *data, QString &errString) override;
    QString getSourceCode() override;
private:
    Enu::ECPUKeywords regAddress;
    int regValue;
};

class StatusBitSpecification: public Specification {
public:
    StatusBitSpecification(Enu::ECPUKeywords statusBitAddress, bool statusBitValue);
    void setUnitPre(NewCPUDataSection*) override;
    bool testUnitPost(NewCPUDataSection *data, QString &errString) override;
    QString getSourceCode() override;
private:
    Enu::ECPUKeywords nzvcsAddress;
    bool nzvcsValue;
};

#endif // SPECIFICATION_H
