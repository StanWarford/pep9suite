// File: asmrunhelper.h
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
#ifndef ASMRUNHELPER_H
#define ASMRUNHELPER_H
#include <QtCore>
#include <QRunnable>

class AsmProgramManager;
class BoundExecIsaCpu;
class MainMemory;

/*
 * This class is responsible for executing a single assembly language program.
 * Given a string of object code (00 01 .. FF zz), the object code will be loaded into a memory
 * device, programInput will be loaded and buffered as values of charIn,
 * and any program output will be written programOutput.
 *
 * When the simulation finishes running, or is terminated internally for taking too
 * long, finished() will be emitted so that the application may shut down safely.
 */
class ASMRunHelper: public QObject, public QRunnable {
    Q_OBJECT
public:
    // Program input may be an empty file. If it is empty or does not
    // exist, then it will be ignored.
    explicit ASMRunHelper(const QString objectCodeString, quint64 maxSimSteps,
                       QFileInfo programOutput, QFileInfo programInput, AsmProgramManager& manager,
                       QObject *parent = nullptr);
    ~ASMRunHelper() override;

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

public:
    // Pre: All computations an outstanding processing events have been finished.
    // Post:The main thread has been signaled to shutdown.
    void onSimulationFinished();

    // Pre: The operating system has been built and installed.
    // Pre: The Pep9 mnemonic maps have been initizialized correctly.
    // Pre: objectCodeString contains only valid space/newline separated object code bytes
    //      (00, 01, ..., FF, zz).
    // Pre: programOutput is a valid file that can be written to by the program. Will abort otherwise.
    // Post:The program is run to completion, or is terminated for taking too long.
    // Post:All program output is written to programOutput.
    void run() override;

    // Echo the values written to CharOut to the console.
    void set_echo_charout(bool echo);
private:
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
    // The CPU simulator that will perform the computation.
    // It is limited to executing a finite numbers of steps,
    // so that applications using Pep9Term will not hang if given a bad program.
    QSharedPointer<BoundExecIsaCpu> cpu;

    // Potentially multiple output sources, but don't take time to simulate now.
    QFile* outputFile;
    // Addresses of the character input / character output ports.
    quint16 charIn, charOut;
    // Maximum number of steps the simulator should execute before force quitting.
    quint64 maxSimSteps;

    // Control if the values written to CharOut get echoed to the console.
    bool echo = false;

    // Helper method responsible for buffering input, opening output streams,
    // converting string object code to a byte list, and executing the object
    // code in memory.
    void runProgram();

    // Load the object code of the operating system into memory from manager.
    void loadOperatingSystem();
};
#endif // ASMRUNHELPER_H
