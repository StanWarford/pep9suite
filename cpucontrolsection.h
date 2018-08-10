#ifndef CPUSTATE_H
#define CPUSTATE_H

#include <QObject>
#include <QVector>
#include <QException>
#include <QString>
#include "enu.h"

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
    // Initialize the CPU's non-static registers (such as SP) to proper values for starting execution.
    void initCPU();
    int getLineNumber() const; // Return the current line number of the microcode program
    // Returns the current call stack depth of the CPU which is equal to the (#CALL + #Traps) - (#RET + #RETTR).
    // This is helpful information for a debugger.
    int getCallDepth() const;
    const MicrocodeProgram* getProgram() const;
    const MicroCode* getCurrentMicrocodeLine() const;
    bool getExecutionFinished() const;
    bool hadErrorOnStep() const;
    QString getErrorMessage() const;
    Enu::DebugLevels getDebugLevel() const;
    void setMicrocodeProgram(MicrocodeProgram* program);
    // Changes the amount of details captured by memoizer. It can not be changed if in a simulation.
    void setDebugLevel(Enu::DebugLevels level);

public slots:
    // Prepare CPU for simulation.
    void onSimulationStarted();
    // Clean up CPU after simulation.
    void onSimulationFinished();
    // Prepare CPU for being stepped through.
    void onDebuggingStarted();
    // Clean up CPU after debugging
    void onDebuggingFinished();

    void onStep() noexcept;  // Execute a single microinstruction
    void onISAStep() noexcept; // Step until µPc == 0
    void onClock() noexcept; // Clock in the values on control lines, as set by CPUPane
    // Step continuously until a STOP or error is hit. Continue to process screen events so application remains responsive.
    void onRun() noexcept;
    void onClearCPU() noexcept; // This event is propogated to the DataSection
    void onClearMemory() noexcept; // This event is propogated to the MemorySection
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
    int microprogramCounter, microCycleCounter, instructionCounter, callDepth;
    bool inSimulation, hadControlError, executionFinished, isPrefetchValid;
    QString errorMessage;

    void branchHandler(); // Based on the current instruction, set the µPC correctly
    void setSignalsFromMicrocode(const MicroCode *line); // Set signals for the control section based on the microcode program
    void updateAtInstructionEnd(); // Update simulation state at the start of a assembly level instruction
};
#endif // CPUSTATE_H
