// File: specification.cpp
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

#include "specification.h"
#include "cpucontrolsection.h"
#include <QDebug>
#include "cpudatasection.h"
#include "memorysection.h"
Specification::Specification()
{
}

MemSpecification::MemSpecification(int memoryAddress, int memoryValue, int numberBytes) {
    memAddress = memoryAddress;
    memValue = memoryValue;
    numBytes = numberBytes;
}

void MemSpecification::setUnitPre(CPUDataSection *data)
{
    MemorySection* memory = data->getMemorySection();
    if(numBytes==1) memory->onSetMemoryByte(memAddress,(quint8)memValue);
    else memory->onSetMemoryWord(memAddress,(quint16)memValue);
}

bool MemSpecification::testUnitPost(CPUDataSection *data, QString &errorString)
{
    MemorySection* memory = data->getMemorySection();
    bool retVal;
    if(numBytes==1)
    {
        retVal=memory->getMemoryByte(memAddress)==(quint8)memValue;
        if(!retVal)errorString= "// ERROR: Unit test failed for byte Mem[0x"+
                QString("%1").arg(memAddress, 4, 16, QLatin1Char('0')).toUpper() + "].";
    }
    else
    {
        //Test each individual byte, to avoid memory alignment issues
        retVal=memory->getMemoryByte(memAddress)==memValue/256&&memory->getMemoryByte(memAddress+1)==memValue%256;
        if(!retVal)errorString= "// ERROR: Unit test failed for byte Mem[0x"+
                QString("%1").arg(memAddress, 4, 16, QLatin1Char('0')).toUpper() + "].";
    }
    return retVal;
}

QString MemSpecification::getSourceCode() {
    return "Mem[0x"
            + QString("%1").arg(memAddress, 4, 16, QLatin1Char('0')).toUpper()
            + "]=0x"
            + (numBytes == 1 ?
               QString("%1").arg(memValue, 2, 16, QLatin1Char('0')).toUpper() :
               QString("%1").arg(memValue, 4, 16, QLatin1Char('0')).toUpper()) ;
}

RegSpecification::RegSpecification(Enu::EKeywords registerAddress, int registerValue) {
    regAddress = registerAddress;
    regValue = registerValue;
}

void RegSpecification::setUnitPre(CPUDataSection *data)
{

    switch(regAddress)
    {
    case Enu::Acc:
        data->onSetRegisterWord(0,regValue);
        break;
    case Enu::X:
        data->onSetRegisterWord(2,regValue);
        break;
    case Enu::SP:
        data->onSetRegisterWord(4,regValue);
        break;
    case Enu::PC:
        data->onSetRegisterWord(6,regValue);
        break;
    case Enu::IR:
        data->onSetRegisterWord(8,regValue/256);
        data->onSetRegisterByte(10,regValue%256);
        break;
    case Enu::T1:
        data->onSetRegisterByte(11,regValue);
        break;
    case Enu::T2:
        data->onSetRegisterWord(12,regValue);
        break;
    case Enu::T3:
        data->onSetRegisterWord(14,regValue);
        break;
    case Enu::T4:
        data->onSetRegisterWord(16,regValue);
        break;
    case Enu::T5:
        data->onSetRegisterWord(18,regValue);
        break;
    case Enu::T6:
        data->onSetRegisterWord(20,regValue);
        break;
    case Enu::MARAREG:
        data->onSetMemoryRegister(Enu::MEM_MARA,(quint8)regValue);
        break;
    case Enu::MARBREG:
        data->onSetMemoryRegister(Enu::MEM_MARB,(quint8)regValue);
        break;
    case Enu::MDRREG:
        data->onSetMemoryRegister(Enu::MEM_MDR,(quint8)regValue);
        break;
    case Enu::MDREREG:
        data->onSetMemoryRegister(Enu::MEM_MDRE,(quint8)regValue);
        break;
    case Enu::MDROREG:
        data->onSetMemoryRegister(Enu::MEM_MDRO,(quint8)regValue);
        break;
    default:
        break;
    }
}

bool RegSpecification::testUnitPost(CPUDataSection *data, QString &errorString)
{
    int reg=0;
    switch(regAddress)
    {
    case Enu::Acc:
        reg=0;
        break;
    case Enu::X:
        reg=2;
        break;
    case Enu::SP:
        reg=4;
        break;
    case Enu::PC:
        reg=6;
        break;
    case Enu::IR:
        if(data->getRegisterBankWord(8)==regValue/256&&data->getRegisterBankByte(10)==regValue%256) return true;
        break;
    case Enu::T1:
        if(data->getRegisterBankByte(11)==regValue) return true;
        break;
    case Enu::T2:
        reg=12;
        break;
    case Enu::T3:
        reg=14;
        break;
    case Enu::T4:
        reg=16;
        break;
    case Enu::T5:
        reg=18;
        break;
    case Enu::T6:
        reg=20;
        break;
    default: return true; //By default,
    }

    if(data->getRegisterBankWord(reg)==regValue)return true;
    switch (regAddress) {
    case Enu::Acc: errorString = "// ERROR: Unit test failed for register A."; return false;
    case Enu::X: errorString = "// ERROR: Unit test failed for register X."; return false;
    case Enu::SP: errorString = "// ERROR: Unit test failed for register SP."; return false;
    case Enu::PC: errorString = "// ERROR: Unit test failed for register PC."; return false;
    case Enu::IR: errorString = "// ERROR: Unit test failed for register IR."; return false;
    case Enu::T1: errorString = "// ERROR: Unit test failed for register T1."; return false;
    case Enu::T2: errorString = "// ERROR: Unit test failed for register T2."; return false;
    case Enu::T3: errorString = "// ERROR: Unit test failed for register T3."; return false;
    case Enu::T4: errorString = "// ERROR: Unit test failed for register T4."; return false;
    case Enu::T5: errorString = "// ERROR: Unit test failed for register T5."; return false;
    case Enu::T6: errorString = "// ERROR: Unit test failed for register T6."; return false;
    case Enu::MARAREG: errorString = "// ERROR: Unit test failed for MARA."; return false;
    case Enu::MARBREG: errorString = "// ERROR: Unit test failed for MARB."; return false;
    case Enu::MDRREG: errorString = "// ERROR: Unit test failed for MDR."; return false;
    default: return false;
    }
}

QString RegSpecification::getSourceCode() {
    switch (regAddress) {
    case Enu::Acc: return "A=0x" + QString("%1").arg(regValue, 4, 16, QLatin1Char('0')).toUpper();
    case Enu::X: return "X=0x" + QString("%1").arg(regValue, 4, 16, QLatin1Char('0')).toUpper();
    case Enu::SP: return "SP=0x" + QString("%1").arg(regValue, 4, 16, QLatin1Char('0')).toUpper();
    case Enu::PC: return "PC=0x" + QString("%1").arg(regValue, 4, 16, QLatin1Char('0')).toUpper();
    case Enu::IR: return "IR=0x" + QString("%1").arg(regValue, 6, 16, QLatin1Char('0')).toUpper();
    case Enu::T1: return "T1=0x" + QString("%1").arg(regValue, 2, 16, QLatin1Char('0')).toUpper();
    case Enu::T2: return "T2=0x" + QString("%1").arg(regValue, 4, 16, QLatin1Char('0')).toUpper();
    case Enu::T3: return "T3=0x" + QString("%1").arg(regValue, 4, 16, QLatin1Char('0')).toUpper();
    case Enu::T4: return "T4=0x" + QString("%1").arg(regValue, 4, 16, QLatin1Char('0')).toUpper();
    case Enu::T5: return "T5=0x" + QString("%1").arg(regValue, 4, 16, QLatin1Char('0')).toUpper();
    case Enu::T6: return "T6=0x" + QString("%1").arg(regValue, 4, 16, QLatin1Char('0')).toUpper();
    case Enu::MARAREG: return "MARA=0x" + QString("%1").arg(regValue, 4, 16, QLatin1Char('0')).toUpper();
    case Enu::MARBREG: return "MARB=0x" + QString("%1").arg(regValue, 4, 16, QLatin1Char('0')).toUpper();
    case Enu::MDRREG: return "MDR=0x" + QString("%1").arg(regValue, 4, 16, QLatin1Char('0')).toUpper();
    default: return "";
    }
}

StatusBitSpecification::StatusBitSpecification(Enu::EKeywords statusBitAddress, bool statusBitValue) {
    nzvcsAddress = statusBitAddress;
    nzvcsValue = statusBitValue;
}

void StatusBitSpecification::setUnitPre(CPUDataSection *data)
{
    Enu::EStatusBit status;
    switch(nzvcsAddress)
    {
    case Enu::N:
        status = Enu::STATUS_N;
        break;
    case Enu::Z:
        status = Enu::STATUS_Z;
        break;
    case Enu::V:
        status = Enu::STATUS_V;
        break;
    case Enu::Cbit:
        status = Enu::STATUS_C;
        break;
    case Enu::S:
        status = Enu::STATUS_S;
        break;
    default: //If this case is ever reached, then nzvcsAddress wasn't actually a NZVCS flag
        return;
    }
    data->onSetStatusBit(status,nzvcsValue);
}

bool StatusBitSpecification::testUnitPost(CPUDataSection *data, QString &errorString)
{
    Enu::EStatusBit status;
    switch(nzvcsAddress)
    {
    case Enu::N:
        status = Enu::STATUS_N;
        break;
    case Enu::Z:
        status = Enu::STATUS_Z;
        break;
    case Enu::V:
        status = Enu::STATUS_V;
        break;
    case Enu::Cbit:
        status = Enu::STATUS_C;
        break;
    case Enu::S:
        status = Enu::STATUS_S;
        break;
    default: return true;
    }
    if(data->getStatusBit(status)==nzvcsValue) return true;
    switch (nzvcsAddress) {
    case Enu::N: errorString = "// ERROR: Unit test failed for status bit N."; return false;
    case Enu::Z: errorString = "// ERROR: Unit test failed for status bit Z."; return false;
    case Enu::V: errorString = "// ERROR: Unit test failed for status bit V."; return false;
    case Enu::Cbit: errorString = "// ERROR: Unit test failed for status bit C."; return false;
    case Enu::S: errorString = "// ERROR: Unit test failed for status bit S."; return false;
    default: return false;
    }
}

QString StatusBitSpecification::getSourceCode() {
    switch (nzvcsAddress) {
    case Enu::N: return "N=" + QString("%1").arg(nzvcsValue);
    case Enu::Z: return "Z=" + QString("%1").arg(nzvcsValue);
    case Enu::V: return "V=" + QString("%1").arg(nzvcsValue);
    case Enu::Cbit: return "C=" + QString("%1").arg(nzvcsValue);
    case Enu::S: return "S=" + QString("%1").arg(nzvcsValue);
    default: return "";
    }
}
