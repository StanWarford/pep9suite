// File: boundedexecisacpu.cpp
/*
    Pep9 is a virtual machine for writing machine language and assembly
    language programs.

    Copyright (C) 2019  J. Stanley Warford & Matthew McRaven, Pepperdine University

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
#include "boundexecisacpu.h"
#include "amemorydevice.h"
#include <QTimer>
#include <QCoreApplication>
BoundExecIsaCpu::BoundExecIsaCpu(quint64 stepCount, const AsmProgramManager *manager,
                                   QSharedPointer<AMemoryDevice> memDevice, QObject *parent):
    IsaCpu(manager, memDevice, parent), maxSteps(stepCount)

{

}

BoundExecIsaCpu::~BoundExecIsaCpu()
{

}

quint64 BoundExecIsaCpu::getDefaultMaxSteps()
{
    return defaultMaxSteps;
}

bool BoundExecIsaCpu::onRun()
{
    while(!hadErrorOnStep() && !executionFinished) {
        // Since the sim runs at about 5Mhz, do not process events every single cycle to increase performance.
        if(asmInstructionCounter % 500 == 0) {
            QCoreApplication::processEvents();
        }
        else if(maxSteps <= asmInstructionCounter) {
            controlError = true;
            errorMessage = "Possible endless loop detected.";
        }
        onISAStep();
    }

    //If there was an error on the control flow
    if(hadErrorOnStep()) {
        if(IsaCpu::memory->hadError()) {
            qDebug() << "Memory section reporting an error";
            return false;
        }
        else {
            qDebug() << "Control section reporting an error";
            return false;
        }
    }

    return true;
}
