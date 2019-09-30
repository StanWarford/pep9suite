#ifndef MICROSTEPHELPER_H
#define MICROSTEPHELPER_H

#include <QObject>
#include <QRunnable>
#include <QSharedPointer>
#include <QFileInfo>
#include "enu.h"
class MainMemory;
class FullMicrocodedCPU;

/*
 * This class is responsible for executing a single assembly language program.
 * Given a string of object code (00 01 .. FF zz), the object code will be loaded into a memory
 * device, programInput will be loaded and buffered as values of charIn,
 * and any program output will be written programOutput.
 *
 * When the simulation finishes running, or is terminated internally for taking too
 * long, finished() will be emitted so that the application may shut down safely.
 */
class MicroStepHelper: public QObject, public QRunnable {
    Q_OBJECT
public:
    // Program input may be an empty file. If it is empty or does not
    // exist, then it will be ignored.
    explicit MicroStepHelper(Enu::CPUType type,
                       const QString microcodeProgram, QFileInfo microcodeProgramFile,
                       const QString preconditionsProgram,
                       QFileInfo programOutput,
                       QObject *parent = nullptr);
    ~MicroStepHelper() override;

signals:
    // Signals fired when the computation completes (either successfully or due to an error),
    // or the simulation terminates due to exceeding the maximum number of allowed steps.
    void finished();

public:
    void onSimulationFinished();
    // Pre: All computations an outstanding processing events have been finished.
    // Post:The main thread has been signaled to shutdown.

    void run() override;
    // Pre: The operating system has been built and installed.
    // Pre: The Pep9 mnemonic maps have been initizialized correctly.
    // Pre: objectCodeString contains only valid space/newline separated object code bytes
    //      (00, 01, ..., FF, zz).
    // Pre: programOutput is a valid file that can be written to by the program. Will abort otherwise.
    // Post:The program is run to completion, or is terminated for taking too long.
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
   QSharedPointer<FullMicrocodedCPU> cpu;

   // Potentially multiple output sources, but don't take time to simulate now.
   QFile* outputFile;

   // Helper method responsible for buffering input, opening output streams,
   // converting string object code to a byte list, and executing the object
   // code in memory.
   void runProgram();

};

#endif // MICROSTEPHELPER_H
