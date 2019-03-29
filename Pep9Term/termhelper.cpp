// File: termhelper.cpp
/*
    Pep9 is a virtual machine for writing machine language and assembly
    language programs.

    Copyright (C) 2019  J. Stanley Warford & Matthew McRaevn, Pepperdine University

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
#include "termhelper.h"
#include "isaasm.h"
#include "pep.h"
#include "asmprogrammanager.h"
#include "asmprogram.h"
#include "asmcode.h"
#include "isacpu.h"
#include "amemorydevice.h"
#include "symboltable.h"
#include "symbolentry.h"
#include "memorychips.h"
#include "amemorychip.h"
#include "mainmemory.h"
// Error messages potentially used in multiple places;
const QString errLogOpenErr = "Could not open file: %1";
const QString hadErr        = "Errors/warnings encountered while generating output for file: %1";
const QString assemble      = "About to assemble %1 into object file %2";

// Helper function that turns hexadecimal object code into a vector of
// unsigned characters, which is easier to copy into memory.
QVector<quint8> convertObjectCodeToIntArray(QString program)
{
    bool ok = false;
    quint8 temp;
    QVector<quint8> output;
    program.replace(QRegExp("\n")," ");
    for(QString byte : program.split(" ")) {
        // toShort(...) should never throw any errors, so there should be no concerns if byte is not a hex constant.
        temp = static_cast<quint8>(byte.toShort(&ok, 16));
        // There could be a loss in precision if given text outside the range of an uchar but in range of a ushort.
        if(ok && byte.length()>0) output.append(temp);
    }
    return output;
}

void buildDefaultOperatingSystem(AsmProgramManager &manager)
{
    // Need to assemble operating system.
    QString defaultOSText = Pep::resToString(":/help-asm/figures/pep9os.pep");
    // If there is text, attempt to assemble it
    if(!defaultOSText.isEmpty()) {
        QSharedPointer<AsmProgram> prog;
        auto elist = QList<QPair<int, QString>>();
        IsaAsm assembler(manager);
        if(assembler.assembleOperatingSystem(defaultOSText, true, prog, elist)) {
            manager.setOperatingSystem(prog);
        }
        // If the operating system failed to assembly, we can't progress any further.
        // All application functionality depends on the operating system being defined.
        else {
            qDebug() << "OS failed to assemble.";
            auto textList = defaultOSText.split("\n");
            for(auto errorPair : elist) {
                qDebug() << textList[errorPair.first] << errorPair.second << endl;
            }
            throw std::logic_error("The default operating system failed to assemble.");
        }
    }
    // If the operating system couldn't be found, we can't progress any further.
    // All application functionality depends on the operating system being defined.
    else {
        throw std::logic_error("Could not find default operating system.");
    }

}

RunHelper::RunHelper(const QString objectCodeString, QFileInfo programOutput,
                     QFileInfo programInput, AsmProgramManager &manager, QObject *parent):
    QObject(parent), QRunnable (),objectCodeString(objectCodeString),
    programOutput(programOutput), programInput(programInput),manager(manager),
    // Explicitly initialize both simulation objects to nullptr,
    // so that it is clear to that neither object has been allocated
    memory(nullptr), cpu(nullptr)

{

}

RunHelper::~RunHelper()
{
    // All of our memory is owned by sharedpointers, so we
    // should not attempt to delete anything ourselves.
}

void RunHelper::loadOperatingSystem()
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
    list.append({AMemoryChip::ChipTypes::ROM, startAddress, static_cast<quint32>(values.length())});
    // Character input / output ports are only 1 byte wide by design.
    list.append({AMemoryChip::ChipTypes::IDEV, charIn, 1});
    list.append({AMemoryChip::ChipTypes::ODEV, charOut, 1});
    memory->constructMemoryDevice(list);

    memory->autoUpdateMemoryMap(true);
    memory->loadValues(manager.getOperatingSystem()->getBurnAddress(), values);
}

void RunHelper::onInputRequested(quint16 address)
{
    // All the input a program will ever receive is loaded into the memory
    // buffer as the program is started. So we can't satisfy the IO request,
    // and thus we need to signal the simulation that the IO request was denied.
    memory->onInputAborted(address);
}

void RunHelper::onOutputReceived(quint16 address, quint8 value)
{
    // We only care about the output if it happens to be an output port that we
    // are tracking in out outputs map.
    if(outputs.contains(address)) {
        *(outputs[address].get()) << QChar(value);
    }
}

void RunHelper::runProgram()
{

    // Buffer input file into memory mapped input if possible.
    QFile input(programInput.absoluteFilePath());

    // If there is not input, append a newline so that there is a least one character buffered.
    if(!programInput.exists()) {
        memory->onInputReceived(charIn, "\n");
    }
    if(programInput.exists() && !input.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug().noquote() << errLogOpenErr.arg(input.fileName());
    } else {
        QTextStream inputStream(&input);
        memory->onInputReceived(charIn, inputStream.readAll() % "\n");
        input.close();
    }

    // Open up program output file if possible.
    // If output can't be opened up, abort.
    QFile output(programOutput.absoluteFilePath());
    if(!output.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qDebug().noquote() << errLogOpenErr.arg(output.fileName());
        throw std::logic_error("Can't open output file.");
    } else {
        // If it could be opened, map charOut to the file.
        QSharedPointer<QTextStream> outputStream( new QTextStream(&output));
        outputs[charOut] = outputStream;
    }
    cpu->onSimulationStarted();
    if(!cpu->onRun()) {
        qDebug().noquote() << "The CPU failed for the following reason: "<<cpu->getErrorMessage();
        *(outputs[charOut].get()) << "[[" << cpu->getErrorMessage() << "]]";
    }
    outputs[charOut].clear();
    output.close();

}

void RunHelper::run()
{

    // Construct all needed simulation objects in run, so that the owning
    // thread is the one doing the computation, not the main thread.
    if(memory.isNull()) {
        // Assume memory will always be 64k.
        memory = QSharedPointer<MainMemory>::create(nullptr);
        QSharedPointer<RAMChip> ramChip(new RAMChip(1<<16, 0, memory.get()));
        memory->insertChip(ramChip, 0);

        cpu = QSharedPointer<IsaCpu>::create(&manager, memory, nullptr);

        // Connect IO events
        connect(memory.get(), &MainMemory::inputRequested, this, &RunHelper::onInputRequested, Qt::QueuedConnection);
        connect(memory.get(), &MainMemory::outputWritten, this, &RunHelper::onOutputReceived, Qt::QueuedConnection);
    }

    // Load operating system & user program into memory.
    loadOperatingSystem();
    auto objCode = convertObjectCodeToIntArray(objectCodeString);
    memory->loadValues(0, objCode);

    // Clear & initialize all values in CPU before starting simulation.
    cpu->reset();
    cpu->initCPU();
    runProgram();
    // Application will live forever if we don't signal it to die.
    emit finished();
}

BuildHelper::BuildHelper(const QString source, QFileInfo objFileInfo,
                         AsmProgramManager &manager, QObject *parent): QObject(parent),
    QRunnable(), source(source), objFileInfo(objFileInfo), manager(manager)
{

}

BuildHelper::~BuildHelper()
{
    // All of our memory is owned by sharedpointers, so we
    // should not attempt to delete anything ourselves.
}

void BuildHelper::run()
{
    // All set up work is done in build program, so all run needs to do is attempt
    if(buildProgram()) {
       // Placeholder for potential work needing to be done after successful assembly.
    }

    // Application will live forever if we don't signal it to die.
    emit finished();
}

bool BuildHelper::buildProgram()
{
    // Construct files that will be needed for assembly
    QFile objectFile(objFileInfo.absoluteFilePath());
    QFile errorLog(QFileInfo(objectFile).absoluteDir().absoluteFilePath(
                       QFileInfo(objectFile).baseName() + "_errLog.txt"));
    QSharedPointer<AsmProgram> program;
    auto elist = QList<QPair<int, QString> >();
    IsaAsm assmembler(manager);
    // Returns true if object code is successfully generated (i.e. program is non-null).
    bool success = assmembler.assembleUserProgram(source, program, elist);

    // If there were errors, attempt to write all of them to the error file.
    // If the error file can't be opened, log that failure to standard output.
    if(!elist.isEmpty()) {
        if(!errorLog.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            qDebug().noquote() << errLogOpenErr.arg(errorLog.fileName());
        }
        else {
            QTextStream errAsStream(&errorLog);
            auto textList = source.split("\n");
            for(auto errorPair : elist) {
                errAsStream << textList[errorPair.first] << errorPair.second << endl;
            }
            // Error log should be flushed automatically.
            errorLog.close();
        }
    }

    // Only open & write object code file if assembly was successful.
    if(success) {
        // Program assembly can succeed despite the presence of errors in the
        // case of trace tag warnings. Must gaurd against this.
        if(elist.isEmpty()) {
            qDebug() << "Program assembled successfully.";
        }
        else {
            qDebug() << "Warning(s) generated. See error log.";
        }
        // Attempt to open object code file. Write error to standard out if it fails.
        if(!objectFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            qDebug().noquote() << errLogOpenErr.arg(objectFile.fileName());
        }
        else {
            // Below code copeid from object code pane's formatting
            QString objectCodeString = "";
            auto objectCode = program->getObjectCode();
            for (int i = 0; i < objectCode.length(); i++) {
                objectCodeString.append(QString("%1").arg(objectCode[i], 2, 16, QLatin1Char('0')).toUpper());
                objectCodeString.append((i % 16) == 15 ? '\n' : ' ');
            }
            objectCodeString.append("zz");
            QTextStream objStream(&objectFile);
            objStream << objectCodeString << "\n";
            objectFile.close();
        }

        // Also attempt to generate listing file from assembled program as well.
        QFile listingFile(QFileInfo(objectFile).absoluteDir().absoluteFilePath(
                              QFileInfo(objectFile).baseName() + ".pepl"));
        if(!listingFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            qDebug().noquote() << errLogOpenErr.arg(listingFile.fileName());
        }
        else {
            QTextStream listingStream(&listingFile);
            for (auto line : program->getProgram()) {
                listingStream << line->getAssemblerListing() << endl;
            }
            listingFile.close();
        }
    }
    else {
        qDebug() << "Error(s) generated. See error log.";
    }
    return success;
}
