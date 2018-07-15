#ifndef CPUSTATE_H
#define CPUSTATE_H

#include <QObject>
#include <QVector>
#include <QException>
#include <QString>
#include "enu.h"

/*

THEN
    Fix format-from microcode

 */

/*
 * Class that feeds microcode lines to the CPUDataSection
 * To trigger execution, a onSumulationStarted/onDebuggingStarted is called.
 * This will typically be done by some button on the main window.
 */
class MicroCode;
class MicrocodeProgram;
class CPUDataSection;
class MemorySection;
class CPUMemoizer;
class CPUControlSection: public QObject
{
    Q_OBJECT
    friend class CPUMemoizer;
public:
    static CPUControlSection* getInstance();
    virtual ~CPUControlSection();
    void initCPU();
    int getLineNumber() const;
    const MicrocodeProgram* getProgram() const;
    const MicroCode* getCurrentMicrocodeLine() const;
    bool getExecutionFinished() const;
    bool hadErrorOnStep() const;
    QString getErrorMessage() const;

    void setMicrocodeProgram(MicrocodeProgram* program);
public slots:
    void onSimulationStarted();
    void onSimulationFinished();
    void onDebuggingStarted();
    void onDebuggingFinished();
    void onStep() noexcept;
    void onClock() noexcept;
    void onRun() noexcept;
    void onClearCPU() noexcept; //This event is propogated to the DataSection
    void onClearMemory() noexcept; //This event is propogated to the MemorySection
signals:
    void simulationStarted();
    void simulationStepped();
    void simulationFinished();
    void simulationInstructionFinished();
private:
    CPUControlSection(CPUDataSection* dataSection, MemorySection* memory);
    static CPUControlSection *_instance;
    CPUMemoizer* memoizer;
    CPUDataSection* data;
    MemorySection* memory;
    MicrocodeProgram* program;
    int microprogramCounter,microCycleCounter,macroCycleCounter;
    bool inSimulation,hadControlError,executionFinished,isPrefetchValid;
    QString errorMessage;

    void branchHandler(); //Based on the current instruction, set the MPC correctly
    void setSignalsFromMicrocode(const MicroCode *line); //Set signals for the control section based on the microcode program
    //Update simulation state at the start of a assembly level instruction
    void updateAtInstructionEnd();
};

class CPUTester: public QObject
{
    Q_OBJECT
public:
    static CPUTester* getInstance();
    virtual ~CPUTester();
private:
    static CPUTester* _instance;
    CPUTester(CPUControlSection *control, CPUDataSection *data);
    CPUControlSection* control;
    CPUDataSection* data;
};

#endif // CPUSTATE_H
