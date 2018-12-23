#ifndef ACPUMODEL_H
#define ACPUMODEL_H

#include <QObject>
#include "enu.h"
class AMemoryDevice;
class ACPUModel: public QObject
{
    Q_OBJECT
public:
    ACPUModel(AMemoryDevice* memoryDev, QObject* parent=nullptr);
    virtual ~ACPUModel();

    AMemoryDevice* getMemoryDevice();
    const AMemoryDevice* getMemoryDevice() const;
    // The control section is not responsible for cleaning up the old memory section.
    void setMemoryDevice(AMemoryDevice* newDevice);
    void setDebugLevel(Enu::DebugLevels level);
    bool getExecutionFinished() const;
    int getCallDepth() const;

    /*
     * Return the current value of the named register. On implementations with non-atomic instructions
     * (e.g. micrcoded CPU's) the value of a register might vary over the course of executing an instruction.
     */
    virtual quint8 getByteCPURegCurrent(Enu::CPURegisters reg) const = 0;
    virtual quint16 getWordCPURegCurrent(Enu::CPURegisters reg) const = 0;
    // Return the value of a register at the start of an instruction
    virtual quint8 getByteCPURegStart(Enu::CPURegisters reg) const = 0;
    virtual quint16 getWordCPURegStart(Enu::CPURegisters reg) const = 0;
    virtual Enu::DebugLevels setDebugLevel(Enu::DebugLevels level) const = 0;
    virtual QString getErrorMessage() const = 0;
    virtual bool hadErrorOnStep() const = 0;
    virtual void initCPU() = 0;
    virtual bool stoppedForBreakpoint() const = 0;

public slots:
    // Prepare CPU for a normal (non-debugging) simulation.
    virtual void onSimulationStarted() = 0;
    // Clean up CPU after simulation.
    virtual void onSimulationFinished() = 0;
    // Prepare CPU for being stepped through.
    virtual void onDebuggingStarted() = 0;
    // Clean up CPU after debugging.
    virtual void onDebuggingFinished() = 0;
    // Cancel execution (and clean up) without raising any warnings.
    virtual void onCancelExecution() = 0;

    virtual bool onRun() = 0;
    virtual void onClearCPU() = 0;
    virtual void onClearMemory();

signals:
    void simulationFinished();
    /*
     * If a simulator doesn't support microcode or ASM (e.g. Pep9
     * doesn't support microcode) then the associated event will not be triggered
     */
    void hitBreakpoint(Enu::BreakpointTypes type);
    void asmInstructionFinished();

protected:
    AMemoryDevice* memory;
    int callDepth;
    bool inDebug, inSimulation, executionFinished;
    mutable bool controlError;
    mutable QString errorMessage;
};

#endif // ACPUMODEL_H
