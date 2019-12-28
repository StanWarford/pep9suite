// File: cpurunhelper.h
/*
    Pep9Term is a  command line tool utility for assembling Pep/9 programs to
    object code and executing object code programs.

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
#include "cpurunhelper.h"

#include "amemorychip.h"
#include "amemorydevice.h"
#include "cpubuildhelper.h"
#include "cpudata.h"
#include "memorychips.h"
#include "mainmemory.h"
#include "microcode.h"
#include "partialmicrocodedcpu.h"
#include "symbolentry.h"
#include "symboltable.h"
#include "termhelper.h"

CPURunHelper::CPURunHelper(Enu::CPUType type, const QString microcodeProgram,
                           QFileInfo microcodeProgramFile,
                           const QString preconditionsProgram,
                           QObject *parent) :
    QObject(parent), QRunnable(), type(type), microcodeProgram(microcodeProgram),
    microcodeProgramFile(microcodeProgramFile),
    preconditionsProgram(preconditionsProgram),
    // Explicitly initialize both simulation objects to nullptr,
    // so that it is clear to that neither object has been allocated
    memory(nullptr), cpu(nullptr)

{
    this->error_log = microcodeProgramFile.absoluteDir().absoluteFilePath(
                microcodeProgramFile.baseName() + "_errLog.txt");
}

CPURunHelper::~CPURunHelper()
{

}

void CPURunHelper::onSimulationFinished()
{
    // There migh be outstanding IO events. Give them a chance to finish
    // before initiating shutdown.
    QCoreApplication::processEvents();
    emit finished();
}

void CPURunHelper::runProgram()
{

    // Construct files that will be needed for assembly
    QFile errorLog(error_log.absoluteFilePath());

    QVector<AMicroCode*> preconditionLines;
    auto programResult = buildMicroprogramHelper(type, false,
                                          microcodeProgram);
    MicrocodeAssemblyResult preconditionResult;

    // If there were errors assembling input program, attempt to write all of
    // them to the error file.
    // If the error file can't be opened, log that failure to standard output.
    if(!programResult.elist.isEmpty()) {
        // Log failure to open file.
        if(!errorLog.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            qDebug().noquote() << errLogOpenErr.arg(errorLog.fileName());
        }
        else {
            QTextStream errAsStream(&errorLog);
            auto textList = microcodeProgram.split("\n");
            for(auto errorPair : programResult.elist) {
                // The first element of the error pair is the line number which
                // caused the error, allowing us to write the offending line
                // and error message to the console.
                errAsStream << textList[errorPair.first] << errorPair.second << endl;
            }
            // Error log should be flushed automatically.
            errorLog.close();
        }
    }
    if(programResult.success && !programResult.program.isNull()) {
        // Program assembly can succeed despite the presence of errors in the
        // case of trace tag warnings. Must gaurd against this.
        if(programResult.elist.isEmpty()) {
            qDebug() << "Program assembled successfully.";
            // Initalize CPU's microcode program.
            cpu->setMicrocodeProgram(programResult.program);
        }
    }
    else {
        qDebug() << "Error(s) generated in microcode input. See error log.";
        emit finished();
        return;
    }

    if(!preconditionsProgram.isEmpty()) {
        preconditionResult = buildMicroprogramHelper(type, false,
                                              preconditionsProgram);
        // If there were errors processing precondition microcode program,
        // attempt to write all of them to the error file.
        // If the error file can't be opened, log that failure to standard output.
        if(!preconditionResult.elist.isEmpty()) {
            if(!errorLog.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
                qDebug().noquote() << errLogOpenErr.arg(errorLog.fileName());
            }
            else {
                QTextStream errAsStream(&errorLog);
                auto textList = microcodeProgram.split("\n");
                for(auto errorPair : preconditionResult.elist) {
                    // The first element of the error pair is the line number which
                    // caused the error, allowing us to write the offending line
                    // and error message to the console.
                    errAsStream << textList[errorPair.first] << errorPair.second << endl;
                }
            }
        }

        if(preconditionResult.success) {
            // Program assembly can succeed despite the presence of errors in the
            // case of trace tag warnings. Must gaurd against this.
            if(preconditionResult.elist.isEmpty()) {
                qDebug() << "Preconditions assembled successfully.";
                // Preconditions program was present and valid, apply preconditions
                // in precondition program.
                preconditionLines = preconditionResult.program->getObjectCode();
            }
        }
        else {
            qDebug() << "Error(s) generated in precondition input. See error log.";
            emit finished();
            return;
        }
    }
    // If no preconditions program was present, apply preconditions
    // in input microcode program.
    else {
        preconditionLines = programResult.program->getObjectCode();
    }

    // Apply active precondition set.
    CPUDataSection* data = cpu->getDataSection().get();
    AMemoryDevice* memory = this->memory.get();
    for(auto line : preconditionLines) {
        if(line->hasUnitPre()) {
            static_cast<UnitPreCode*>(line)->setUnitPre(data, memory);
        }
    }

    // Open up the error log if it is not already open.
    if(errorLog.isOpen() && !errorLog.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qDebug().noquote() << errLogOpenErr.arg(errorLog.fileName());
        return;
    }

    // Make sure to set up any last minute flags needed by CPU to perform simulation.
    cpu->onSimulationStarted();
    bool passed = true;
    QString errorString;

    if(!cpu->onRun()) {
        qDebug().noquote()
                << "The CPU failed for the following reason: "
                << cpu->getErrorMessage();
        QTextStream (&errorLog)
                << "[["
                << cpu->getErrorMessage()
                << "]]";
    }
    else {
        CPUDataSection* data = cpu->getDataSection().get();
        AMemoryDevice* memory = this->memory.get();
        // Iterate over all microcde, and select any that have post conditions.
        for (AMicroCode* x : preconditionLines) {
            if(x->hasUnitPost()) {
                UnitPostCode* code = dynamic_cast<UnitPostCode*>(x);
                // Check if postcondition holds. If not, errorString will be set.
                if(!code->testPostcondition(data, memory, errorString)) {
                    qDebug().noquote() << errorString;
                    // Write the precondition failures to the output file.
                    QTextStream (&errorLog) << errorString;
                    // If any postcondition fails, then the entire execution failed.
                    passed = false;
                }
             }
        }
        // If all unit tests passed, and the CPU had no other issues,
        // we may report a success.
        if(passed) {
            //QTextStream (&errorLog) << "success";
            qDebug() << "Passed unit tests.";
        }
    }
    if(errorLog.isOpen()) errorLog.close();

}

void CPURunHelper::run()
{

    // Construct all needed simulation objects in run, so that the owning
    // thread is the one doing the computation, not the main thread.
    if(memory.isNull()) {
        // Assume memory will always be 64k.
        memory = QSharedPointer<MainMemory>::create(nullptr);
        QSharedPointer<RAMChip> ramChip(new RAMChip(1<<16, 0, memory.get()));
        memory->insertChip(ramChip, 0);

        cpu = QSharedPointer<PartialMicrocodedCPU>::create(type, memory, nullptr);
    }

    // Clear & initialize all values in CPU before starting simulation.
    cpu->onResetCPU();
    cpu->initCPU();

    // Instead of directly allowing run() to kill itself, uses events to "schedule"
    // shutting down the application. This should ensure all IO completes. We were
    // having an error where closing IO streams directly after simulation completion would
    // cause a race condition with IO pending for the file. The overhead of the simulation events
    // seems to "serialize" writes / closing.
    connect(cpu.get(), &PartialMicrocodedCPU::simulationFinished,
            this, &CPURunHelper::onSimulationFinished);
    runProgram();

    // Make sure any outstanding events are handled.
    QCoreApplication::processEvents();
}

void CPURunHelper::set_error_file(QString error_file)
{
    this->error_log = error_file;
}
