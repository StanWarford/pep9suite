#ifndef ACPUMODEL_H
#define ACPUMODEL_H

#include <QObject>
#include "enu.h"
class AMemoryDevice;
class ACPUModel: public QObject
{
    Q_OBJECT
public:
    ACPUModel(QSharedPointer<AMemoryDevice> memoryDev, QObject* parent=nullptr);
    virtual ~ACPUModel();

    // Returns non-owning pointer
    AMemoryDevice* getMemoryDevice();
    const AMemoryDevice* getMemoryDevice() const;
    // The control section is not responsible for cleaning up the old memory section.
    void setMemoryDevice(QSharedPointer<AMemoryDevice> newDevice);
    bool getExecutionFinished() const;
    int getCallDepth() const;

    virtual bool getStatusBitCurrent(Enu::EStatusBit) const = 0;
    virtual bool getStatusBitStart(Enu::EStatusBit) const = 0;
    /*
     * Return the current value of the named register. On implementations with non-atomic instructions
     * (e.g. micrcoded CPU's) the value of a register might vary over the course of executing an instruction.
     */
    virtual quint8 getCPURegByteCurrent(Enu::CPURegisters reg) const = 0;
    virtual quint16 getCPURegWordCurrent(Enu::CPURegisters reg) const = 0;
    // Return the value of a register at the start of an instruction
    virtual quint8 getCPURegByteStart(Enu::CPURegisters reg) const = 0;
    virtual quint16 getCPURegWordStart(Enu::CPURegisters reg) const = 0;
    virtual Enu::DebugLevels getDebugLevel() const = 0;
    virtual void setDebugLevel(Enu::DebugLevels level) = 0;
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
    // Wipe all registers & memory, KEEP loaded programs & breakpoints
    virtual void onResetCPU() = 0;
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
    QSharedPointer<AMemoryDevice> memory;
    int callDepth;
    bool inDebug, inSimulation, executionFinished;
    mutable bool controlError;
    mutable QString errorMessage;
};

#endif // ACPUMODEL_H
