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
#ifndef CPURUNHELPER_H
#define CPURUNHELPER_H

#include <QFileInfo>
#include <QObject>
#include <QRunnable>
#include <QSharedPointer>

#include "enu.h"

class MainMemory;
class PartialMicrocodedCPU;

/*
 * This class is responsible for executing a single microcode program using
 * the Pep/9 CPU model. The data bus size of the CPU may be specified through
 * the type parameter.
 *
 * The microcode program must contain a valid Pep9CPU microcode program,
 * with line numbers already stripped. The microcodeProgramFile should
 * contain the file from which the program to execute was loaded.
 *
 * Optionally a preconditionsProgram may be loaded. If this value is non-empty,
 * ONLY unitpres and posts from preconditionsProgram will affect the CPU.
 * Any unit tests in microcodeProgram will be ignored,
 * If preconditionsProgram is empty, then unit tests from microcodeProgram
 * will work as expected.
 *
 * If unit tests are succesful, "success" will be written to programOutput.
 *
 * It is only capable of executing programs from Pep9CPU-it does not support
 * the features of Pep9Micro. For Pep9Micro emulation, see MicroStepHelper.
 *
 * When the simulation finishes running, or is terminated internally for taking too
 * long, finished() will be emitted so that the application may shut down safely.
 */
class CPURunHelper: public QObject, public QRunnable {
    Q_OBJECT
public:
    // Program input may be an empty file. If it is empty or does not
    // exist, then it will be ignored.
    explicit CPURunHelper(Enu::CPUType type,
                       const QString microcodeProgram, QFileInfo microcodeProgramFile,
                       const QString preconditionsProgram,
                       QFileInfo programOutput,
                       QObject *parent = nullptr);
    ~CPURunHelper() override;

signals:
    // Signals fired when the computation completes (either successfully or due to an error),
    // or the simulation terminates due to exceeding the maximum number of allowed steps.
    void finished();

public:
    void onSimulationFinished();
    // Pre: All computations an outstanding processing events have been finished.
    // Post:The main thread has been signaled to shutdown.

    void run() override;
    // Pre: CPU type is either one or two byte.
    // Pre: The Pep9 mnemonic maps have been initizialized correctly.
    // Pre: The MicrocodeProgram does not contain line numbers.
    // Pre: If present, preconditionsProgram does not contain line numbers.
    // Pre: microcodeProgramFile points to a real file.
    // Pre: programOutput is a valid file that can be written to by the program. Will abort otherwise.
    // Post:The program is run to completion and evaluated by any present unit tests.
    // Post:All program output is written to programOutput.
private:
   Enu::CPUType type;
   const QString microcodeProgram;
   QFileInfo microcodeProgramFile;
   const QString preconditionsProgram;
   QFileInfo programOutput;

   // Runnable will be executed in a separate thread, all objects being pointed to
   // must be constructed in this thread. The object is constructed in the main thread
   // so do not attempt to allocate objects there. Instead, allocate objects,
   // such as the CPU or memory, in run(), since run exectues in the context of the
   // worker thread. This is important for correct parenting of child QObjects.

   // Memory device used by simulation.
   QSharedPointer<MainMemory> memory;
   // The CPU simulator that will perform the computation
   QSharedPointer<PartialMicrocodedCPU> cpu;

   // Potentially multiple output sources, but don't take time to simulate now.
   QFile* outputFile;

   // Helper method responsible for buffering input, opening output streams,
   // converting string object code to a byte list, and executing the object
   // code in memory.
   void runProgram();

};


#endif // CPURUNHELPER_H
