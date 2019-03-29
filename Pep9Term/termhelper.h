// File: termhelper.h
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

#ifndef TERMHELPER_H
#define TERMHELPER_H
#include <QtCore>
#include <QRunnable>

extern const QString errLogOpenErr;
extern const QString hadErr;
extern const QString assemble;

class MainMemory;
class AsmProgramManager;
class IsaCpu;

// Assemble the default operating system for the help documentation,
// and install it into the program manager.
void buildDefaultOperatingSystem(AsmProgramManager& manager);

/*
 * This class is responsible for executing a single assembly language program.
 * Given a string of object code (00 01 .. FF zz), the object code will be loaded into a memory
 * device, programInput will be loaded and buffered as values of charIn,
 * and any program output will be written programOutput.
 *
 * When the simulation finishes running, or is terminated internally for taking too
 * long, finished() will be emitted so that the application may shut down safely.
 */
class RunHelper: public QObject, public QRunnable {
    Q_OBJECT
    const QString objectCodeString;
    QFileInfo programOutput, programInput;
    AsmProgramManager& manager;
    // Runnable will be executed in a separate thread, all objects being pointed to
    // must be constructed in this thread. The object is constructed in the main thread
    // so do not attempt to allocate objects there. Instead, allocate objects,
    // such as the CPU or memory, in run(), since run exectues in the context of the
    // worker thread. This is important for correct parenting of child QObjects.

    // Memory device used by simulation.
    QSharedPointer<MainMemory> memory;
    // The CPU simulator that will perform the computation
    QSharedPointer<IsaCpu> cpu;

    // Potentially multiple output sources, so
    QMap<quint16, QSharedPointer<QTextStream>> outputs;
    // Addresses of the character input / character output ports
    quint16 charIn, charOut;

    // Helper method responsible for buffering input, opening output streams,
    // converting string object code to a byte list, and executing the object
    // code in memory.
    void runProgram();

    // Load the object code of the operating system into memory from manager.
    void loadOperatingSystem();
public:
    // Pass
    explicit RunHelper(const QString objectCodeString, QFileInfo programOutput,
                       QFileInfo programInput, AsmProgramManager& manager,
                       QObject *parent = nullptr);
    ~RunHelper() override;

    // On memory mapped input requested. Assumes there is only one memory mapped input.
    // This might be violated in Pep10, but it will require additional command line
    // parameters to function anyway, so there isn't any major worry here.
    void onInputRequested(quint16 address);

    // On output received. Assumes there could be multiple memory mapped outputs.
    void onOutputReceived(quint16 address, quint8 value);

signals:
    // Signals fired when the computation completes (either successfully or due to an error),
    // or the simulation terminates due to exceeding the maximum number of allowed steps.
    void finished();

    // QRunnable interface
public:
    void run() override;
    // Pre: The operating system has been built and installed.
    // Pre: The Pep9 mnemonic maps have been initizialized correctly.
    // Pre: objectCodeString contains only valid space/newline separated object code bytes
    //      (00, 01, ..., FF, zz).
    // Pre: programOutput is a valid file that can be written to by the program. Will abort otherwise.
    // Post:The program is run to completion, or is terminated for taking too long.
    // Post:All program output is written to programOutput.
};

/*
 * This class is responsible for assembling a single assembly language source file.
 * Takes an assembly language program's text as input, in addition to a program manager
 * which already has the default operating system installed
 *
 * If there are warnings or errors, a error log will be written to a file
 * of the same name as objName with the .extension replaced by -errLog.txt.
 *
 * If program assembly was successful (or the only warnings were trace tag issues),
 * then the object code text will be written to objFile. If objFile doesn't exist,
 * it will bre created, and if it does, it will be truncated.
 *
 * If objFile doesn't exist at the end of the execution of this script, then
 * the file failed to assemble, and as such there must be an error log.
 *
 * When the assembler finishes running, or is terminated, finished() will be emitted
 * so that the application may shut down safely.
 */
class BuildHelper: public QObject, public QRunnable {
    Q_OBJECT
    const QString source;
    QFileInfo objFileInfo;
    AsmProgramManager& manager;
    // Helper method responsible for triggering program assembly.
    bool buildProgram();

public:
    explicit BuildHelper(const QString source, QFileInfo objFileInfo, AsmProgramManager& manager,
                        QObject *parent = nullptr);
    ~BuildHelper() override;


signals:
    // Signals fired when the computation completes (either successfully or due to an error),
    // or the simulation terminates due to exceeding the maximum number of allowed steps.
    void finished();

    // QRunnable interface
public:
    void run() override;
    // Pre: The operating system has been built and installed.
    // Pre: The Pep9 mnemonic maps have been initizialized correctly.
    // Pre: objFile's directory exists.
};

#endif // TERMHELPER_H
