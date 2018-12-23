#ifndef FULLMICROCODEDCPU_H
#define FULLMICROCODEDCPU_H

#include "amccpumodel.h"
#include "aisacpumodel.h"
#include <QElapsedTimer>
class CPUDataSection;
class FullMicrocodedCPU : public ACPUModel, public InterfaceMCCPU, public InterfaceISACPU
{
    Q_OBJECT
public:
    FullMicrocodedCPU(AMemoryDevice* memoryDev, QObject* parent = nullptr);
    virtual ~FullMicrocodedCPU() override;

    // ACPUModel interface
    quint8 getByteCPURegCurrent(Enu::CPURegisters reg) const override;
    quint16 getWordCPURegCurrent(Enu::CPURegisters reg) const override;
    quint8 getByteCPURegStart(Enu::CPURegisters reg) const override;
    quint16 getWordCPURegStart(Enu::CPURegisters reg) const override;
    void initCPU() override;
    bool stoppedForBreakpoint() const override;
    QString getErrorMessage() const override;
    bool hadErrorOnStep() const override;
    Enu::DebugLevels setDebugLevel(Enu::DebugLevels level) const override;

    // InterfaceMCCPU interface
    void onMCStep() override;
    void onClock() override;

    // InterfaceISACPU interface
    void setCPUType() override;
    void onISAStep() override;

    // ACPUModel interface
public slots:
    void onSimulationStarted() override;
    void onSimulationFinished() override;
    void onDebuggingStarted() override;
    void onDebuggingFinished() override;
    void onCancelExecution() override;
    bool onRun() override;
    void onClearCPU() override;

private:
    void breakpointHandler();
    void setSignalsFromMicrocode(const MicroCode *line);
    void branchHandler() override;
    void updateAtInstructionEnd() override;
    bool isPrefetchValid;
    QElapsedTimer timer;
    CPUDataSection* data;
};

#endif // FULLMICROCODEDCPU_H
