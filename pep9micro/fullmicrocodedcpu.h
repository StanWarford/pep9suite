#ifndef FULLMICROCODEDCPU_H
#define FULLMICROCODEDCPU_H

#include "amccpumodel.h"
#include "aisacpumodel.h"
#include <QElapsedTimer>
class NewCPUDataSection;
class FullMicrocodedMemoizer;
class FullMicrocodedCPU : public ACPUModel, public InterfaceMCCPU, public InterfaceISACPU
{
    Q_OBJECT
    friend class CPUMemoizer;
public:
    FullMicrocodedCPU(QSharedPointer<AMemoryDevice>, QObject* parent = nullptr);
    virtual ~FullMicrocodedCPU() override;
    QSharedPointer<NewCPUDataSection> getDataSection();

    // ACPUModel interface
    bool getStatusBitCurrent(Enu::EStatusBit) const override;
    bool getStatusBitStart(Enu::EStatusBit) const override;
    quint8 getCPURegByteCurrent(Enu::CPURegisters reg) const override;
    quint16 getCPURegWordCurrent(Enu::CPURegisters reg) const override;
    quint8 getCPURegByteStart(Enu::CPURegisters reg) const override;
    quint16 getCPURegWordStart(Enu::CPURegisters reg) const override;
    void initCPU() override;
    bool stoppedForBreakpoint() const override;
    QString getErrorMessage() const override;
    bool hadErrorOnStep() const override;
    Enu::DebugLevels getDebugLevel() const override;
    void setDebugLevel(Enu::DebugLevels level) override;

    // InterfaceMCCPU interface
    // Will
    void setCPUType(Enu::CPUType type)  override;
    void onMCStep() override;
    void onClock() override;

    // InterfaceISACPU interface
    void onISAStep() override;

    // ACPUModel interface
public slots:
    void onSimulationStarted() override;
    void onSimulationFinished() override;
    void onDebuggingStarted() override;
    void onDebuggingFinished() override;
    void onCancelExecution() override;
    bool onRun() override;
    void onResetCPU() override;

private:
    void breakpointHandler();
    void setSignalsFromMicrocode(const MicroCode *line);
    void branchHandler() override;
    void updateAtInstructionEnd() override;
    bool isPrefetchValid;
    QElapsedTimer timer;
    NewCPUDataSection *data;
    QSharedPointer<NewCPUDataSection> dataShared;
    FullMicrocodedMemoizer *memoizer;
};

#endif // FULLMICROCODEDCPU_H
