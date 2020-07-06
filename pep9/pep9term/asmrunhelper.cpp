// File: asmrunhelper.cpp
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
#include "asmrunhelper.h"

#include <iostream>

#include "assembler/asmcode.h"
#include "assembler/asmprogram.h"
#include "assembler/asmprogrammanager.h"
#include "assembler/isaasm.h"
#include "symbol/symbolentry.h"
#include "symbol/symboltable.h"
#include "memory/amemorychip.h"
#include "memory/amemorydevice.h"
#include "memory/mainmemory.h"
#include "memory/memorychips.h"
#include "pep/pep.h"

#include "boundexecisacpu.h"
#include "isacpu.h"
#include "termhelper.h"

ASMRunHelper::ASMRunHelper(const QString objectCodeString,quint64 maxSimSteps,
                     QFileInfo programOutput, QFileInfo programInput, AsmProgramManager &manager,
                     QObject *parent):
    QObject(parent), QRunnable (), objectCodeString(objectCodeString),
    programOutput(programOutput), programInput(programInput) ,manager(manager),
    // Explicitly initialize both simulation objects to nullptr,
    // so that it is clear to that neither object has been allocated
    memory(nullptr), cpu(nullptr), outputFile(nullptr), maxSimSteps(maxSimSteps)

{

}

ASMRunHelper::~ASMRunHelper()
{
    // If we allocated an output file, we need to perform special work to free it.
    if(outputFile != nullptr) {
        outputFile->flush();
        // It might seem like we should close the file here, but it causes read / write violations to do so.
        // Instead, delete it later under the assumption that the operating system will handle that for us.
        // Schedule the output file for deletion via the event loop.
        outputFile->deleteLater();
    }
}

void ASMRunHelper::loadOperatingSystem()
{
    QVector<quint8> values;
    quint16 startAddress;
    values = manager.getOperatingSystem()->getObjectCode();
    startAddress = manager.getOperatingSystem()->getBurnAddress();

    // Get addresses for I/O chips
    auto osSymTable = manager.getOperatingSystem()->getSymbolTable();
    charIn = static_cast<quint16>(osSymTable->getValue("charIn")->getValue());
    charOut = static_cast<quint16>(osSymTable->getValue("charOut")->getValue());

    // Construct main memory according to the current configuration of the operating system.
    QList<MemoryChipSpec> list;
    // Make sure RAM will fill any accidental gaps in the memory map by making it go
    // right up to the start of the operating system.
    list.append({AMemoryChip::ChipTypes::RAM, 0, startAddress});
    // ROM goes from the first byte of memory until the last installed address.
    list.append({AMemoryChip::ChipTypes::ROM, startAddress, static_cast<quint32>(values.length())});
    // Character input / output ports are only 1 byte wide by design.
    list.append({AMemoryChip::ChipTypes::IDEV, charIn, 1});
    list.append({AMemoryChip::ChipTypes::ODEV, charOut, 1});
    memory->constructMemoryDevice(list);


    memory->autoUpdateMemoryMap(true);
    memory->loadValues(manager.getOperatingSystem()->getBurnAddress(), values);
}

void ASMRunHelper::onInputRequested(quint16 address)
{
    // All the input a program will ever receive is loaded into the memory
    // buffer as the program is started. So we can't satisfy the IO request,
    // and thus we need to signal the simulation that the IO request was denied.
    memory->onInputAborted(address);
}

void ASMRunHelper::onOutputReceived(quint16 address, quint8 value)
{
    // We do not currently support memory mapped output
    // other than the charOut.
    if(address != charOut) return;
    if(outputFile != nullptr) {
        // Use a temporary (anonymous) text stream to make writing easy.
        QTextStream (&*outputFile) << QChar(value);
        // Try to block and make sure the IO actually completes.
        outputFile->waitForBytesWritten(300);
        if(echo) {
            std::cout << static_cast<char>(value);
        }
    }
}

void ASMRunHelper::onSimulationFinished()
{
    // There migh be outstanding IO events. Give them a chance to finish
    // before initiating shutdown.
    QCoreApplication::processEvents();
    emit finished();
}

void ASMRunHelper::runProgram()
{

    // Buffer input file into memory mapped input if possible.
    QFile input(programInput.absoluteFilePath());

    // If there is not input, append a newline so that there is a least one character buffered.
    if(!programInput.exists()) {
        memory->onInputReceived(charIn, "\n");
    }
    else if(!input.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug().noquote() << errLogOpenErr.arg(input.fileName());
        throw std::logic_error("Can't open input file.");
    } else {
        QTextStream inputStream(&input);
        memory->onInputReceived(charIn, inputStream.readAll() % "\n");
        input.close();
    }

    // Open up program output file if possible.
    // If output can't be opened up, abort.
    QFile *output = new QFile(programOutput.absoluteFilePath());
    if(!output->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qDebug().noquote() << errLogOpenErr.arg(output->fileName());
        throw std::logic_error("Can't open output file.");
    } else {
        // If it could be opened, map charOut to the file.
        outputFile = output;
    }

    // Make sure to set up any last minute flags needed by CPU to perform simulation.
    cpu->onSimulationStarted();
    if(!cpu->onRun()) {
        qDebug().noquote()
                << "The CPU failed for the following reason: "
                << cpu->getErrorMessage();
        QTextStream (&*outputFile)
                << "[["
                << cpu->getErrorMessage()
                << "]]";
    }

}

void ASMRunHelper::run()
{

    // Construct all needed simulation objects in run, so that the owning
    // thread is the one doing the computation, not the main thread.
    if(memory.isNull()) {
        // Assume memory will always be 64k.
        memory = QSharedPointer<MainMemory>::create(nullptr);
        QSharedPointer<RAMChip> ramChip(new RAMChip(1<<16, 0, memory.get()));
        memory->insertChip(ramChip, 0);

        cpu = QSharedPointer<BoundExecIsaCpu>::create(maxSimSteps, &manager, memory, nullptr);

        // Connect IO events. IO *MUST* complete before execution moves forward.
        // Use a blocking connection to serialize IO. Use asynchronous connection
        // so that memory and helper don't need to reside in the same thread.
        connect(memory.get(), &MainMemory::inputRequested, this, &ASMRunHelper::onInputRequested, Qt::BlockingQueuedConnection);
        connect(memory.get(), &MainMemory::outputWritten, this, &ASMRunHelper::onOutputReceived, Qt::BlockingQueuedConnection);
    }

    // Load operating system & user program into memory.
    loadOperatingSystem();
    auto objCode = convertObjectCodeToIntArray(objectCodeString);
    memory->loadValues(0, objCode);

    // Clear & initialize all values in CPU before starting simulation.
    cpu->reset();
    cpu->initCPU();

    // Instead of directly allowing run() to kill itself, uses events to "schedule"
    // shutting down the application. This should ensure all IO completes. We were
    // having an error where closing IO streams directly after simulation completion would
    // cause a race condition with IO pending for the file. The overhead of the simulation events
    // seems to "serialize" writes / closing.
    connect(cpu.get(), &IsaCpu::simulationFinished, this, &ASMRunHelper::onSimulationFinished);
    runProgram();

    // Make sure any outstanding events are handled.
    QCoreApplication::processEvents();
}

void ASMRunHelper::set_echo_charout(bool echo)
{
    this->echo = echo;
}
