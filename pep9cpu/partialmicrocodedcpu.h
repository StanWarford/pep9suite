#ifndef FULLMICROCODEDCPU_H
#define FULLMICROCODEDCPU_H

#include "interfacemccpu.h"
#include <QElapsedTimer>
class NewCPUDataSection;
class PartialMicrocodedMemoizer;
class PartialMicrocodedCPU : public ACPUModel, public InterfaceMCCPU
{
    Q_OBJECT
    friend class CPUMemoizer;
    friend class PartialMicrocodedMemoizer;
public:
    PartialMicrocodedCPU(Enu::CPUType type, QSharedPointer<AMemoryDevice>, QObject* parent = nullptr) noexcept;
    virtual ~PartialMicrocodedCPU() override;
    QSharedPointer<NewCPUDataSection> getDataSection();

    // ACPUModel interface
    bool getStatusBitCurrent(Enu::EStatusBit) const override;
    bool getStatusBitStart(Enu::EStatusBit) const override;
    quint8 getCPURegByteCurrent(Enu::CPURegisters reg) const override;
    quint16 getCPURegWordCurrent(Enu::CPURegisters reg) const override;
    quint8 getCPURegByteStart(Enu::CPURegisters reg) const override;
    quint16 getCPURegWordStart(Enu::CPURegisters reg) const override;
    void initCPU() override;
    bool stoppedForBreakpoint() const noexcept override;
    QString getErrorMessage() const noexcept override;
    bool hadErrorOnStep() const noexcept override;
    Enu::DebugLevels getDebugLevel() const noexcept override;
    void setDebugLevel(Enu::DebugLevels level) override;

    // InterfaceMCCPU interface
    void setCPUType(Enu::CPUType type)  override;
    void onMCStep() override;
    void onClock() override;

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
    void branchHandler() override;
    NewCPUDataSection *data;
    QSharedPointer<NewCPUDataSection> dataShared;
    PartialMicrocodedMemoizer *memoizer;
};

#endif // FULLMICROCODEDCPU_H
