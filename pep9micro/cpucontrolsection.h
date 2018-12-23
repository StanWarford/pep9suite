#ifndef CPUSTATE_H
#define CPUSTATE_H

#include <QObject>
#include <QVector>
#include <QException>
#include <QString>
#include "enu.h"
#include <QSet>
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
    // Returns true if this class, the data section, or the memory section had an error.
    bool hadErrorOnStep() const;
    // Returns the error message had by this class, the data section, or memory section. If there was no error, returns empty string.
    QString getErrorMessage() const;
    // Returns if the the microcode line at the current µPC has a breakpoint in it
    bool stoppedForBreakpoint() const;
    Enu::DebugLevels getDebugLevel() const;
    void setMicrocodeProgram(MicrocodeProgram* program);
    // Changes the amount of details captured by memoizer. It can not be changed if in a simulation.
    void setDebugLevel(Enu::DebugLevels level);
    // Return the memoizer that has been tracking CPU state.
    const CPUMemoizer* getCPUMemoizer() const;

    void setPCBreakpoints(QSet<quint16> breakpoints);
    const QSet<quint16> getPCBreakpoints() const;
public slots:
    // Prepare CPU for a normal (non-debugging) simulation.
    void onSimulationStarted();
    // Clean up CPU after simulation.
    void onSimulationFinished();
    // Prepare CPU for being stepped through.
    void onDebuggingStarted();
    // Clean up CPU after debugging
    void onDebuggingFinished();

    void onBreakpointsSet(QSet<quint16> addresses);
    void onRemoveAllBreakpoints();
    void onBreakpointRemoved(quint16 address);
    void onBreakpointAdded(quint16 address);

    void onStep() noexcept;  // Execute a single microinstruction
    void onISAStep() noexcept; // Step until µPc == 0
    void onClock() noexcept; // Clock in the values on control lines, as set by CPUPane
    // In debug mode, execute until complete, a breakpoint is hit, or an error occurs.
    // In normal mode, execute until complete, or an error occurs.
    void onRun() noexcept;
    void onClearCPU() noexcept; // Reset the contents of the control section & propogate event to the DataSection.
    void onClearMemory() noexcept; // This event is propogated to the MemorySection so that it clears itself.
signals:

    void simulationFinished();
    // Emitted whenever an ISA level instruction has been completed.
    void simulationInstructionFinished();
    void simulationHitMicroBreakpoint();
    void simulationHitASMBreakpoint();

private:
    CPUControlSection(CPUDataSection* dataSection, MemorySection* memory);
    static CPUControlSection *_instance;
    CPUMemoizer* memoizer;
    CPUDataSection* data;
    MemorySection* memory;
    MicrocodeProgram* program;
    int microprogramCounter, microCycleCounter, instructionCounter, callDepth;
    bool inSimulation, hadControlError, executionFinished, isPrefetchValid;
    // hitBreakpoint indicates that the current line has a breakpoint.
    bool inDebug, microBreakpointHit, asmBreakpointHit;
    QString errorMessage;
    QSet<quint16> breakpointsISA; // Set of memory addresses that should trap if executed

    // Determine if there is a µbreakpoint or asmbreakpoint, and notify appropriate handler
    // Only call if the CPU is in debug mode
    void breakpointHandler();
    void branchHandler(); // Based on the current instruction, set the µPC correctly
    void setSignalsFromMicrocode(const MicroCode *line); // Set signals for the control section based on the microcode program
    void updateAtInstructionEnd(); // Update simulation state at the start of a assembly level instruction
};
#endif // CPUSTATE_H
