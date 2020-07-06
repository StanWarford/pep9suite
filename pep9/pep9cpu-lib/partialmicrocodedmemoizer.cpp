// File: partialmicrocodedmemoizer.cpp
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
#include "partialmicrocodedmemoizer.h"

#include <cassert>

#include <QDebug>
#include <QString>
#include <QtCore>

#include "cpu/cpudata.h"
#include "cpu/registerfile.h"
#include "memory/amemorydevice.h"
#include "pep/pep.h"

#include "partialmicrocodedcpu.h"

const QString stackFrameEnter("%1\n===CALL===\n");
const QString stackFrameLeave("%1\n===RET====\n");
const QString trapEnter("%1\n===TRAP===\n");
const QString trapLeave("%1\n===RETR===\n");


PartialMicrocodedMemoizer::PartialMicrocodedMemoizer(PartialMicrocodedCPU& item): cpu(item)
{
}

void PartialMicrocodedMemoizer::clear()
{
}

void PartialMicrocodedMemoizer::storeStateInstrEnd()
{

}

void PartialMicrocodedMemoizer::storeStateInstrStart()
{
    cpu.getMemoryDevice()->onInstructionFinished(cpu.data->getRegisterBank().getIRCache());
}

QString PartialMicrocodedMemoizer::memoize()
{
    QString build, AX, NZVC;
    const RegisterFile& file = cpu.data->getRegisterBank();
    AX = QString(" A=%1, X=%2, SP=%3, ")

            .arg(formatNum(file.readRegisterWordCurrent(Enu::CPURegisters::A)),
                 formatNum(file.readRegisterWordCurrent(Enu::CPURegisters::X)),
                 formatNum(file.readRegisterWordCurrent(Enu::CPURegisters::SP)));
    NZVC = QString(" SNZVC=") % QString("%1").arg(QString::number(file.readStatusBitsCurrent(), 2), 5, '0');
    build = AX;
    build += NZVC;
    return build;
}

QString PartialMicrocodedMemoizer::finalStatistics()
{
    QString output = "";
    return output;
}

//Properly formats a number as a 4 char hex
QString PartialMicrocodedMemoizer::formatNum(quint16 number)
{
    return QString("%1").arg(QString::number(number,16),4,'0').toUpper();
}

//Properly format a number as 2 char hex
QString PartialMicrocodedMemoizer::formatNum(quint8 number)
{
    return QString("%1").arg(QString::number(number,16),2,'0').toUpper();
}
