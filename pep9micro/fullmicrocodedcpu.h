#ifndef FULLMICROCODEDCPU_H
#define FULLMICROCODEDCPU_H

#include "interfacemccpu.h"
#include "interfaceisacpu.h"
#include <QElapsedTimer>
class NewCPUDataSection;
class FullMicrocodedMemoizer;
class FullMicrocodedCPU : public ACPUModel, public InterfaceMCCPU, public InterfaceISACPU
{
    Q_OBJECT
    friend class CPUMemoizer;
    friend class FullMicrocodedMemoizer;
public:
    FullMicrocodedCPU(const AsmProgramManager* manager, QSharedPointer<AMemoryDevice>, QObject* parent = nullptr) noexcept;
    virtual ~FullMicrocodedCPU() override;
    QSharedPointer<NewCPUDataSection> getDataSection();
    // Returns true if the microprogram counter is at the
    // start of the von neumann cycle.
    bool atMicroprogramStart() const noexcept;

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

    // InterfaceISACPU interface
    void stepOver() override;
    bool canStepInto() const override;
    void stepInto() override;
    void stepOut() override;

public slots:
    void onSimulationStarted() override;
    void onSimulationFinished() override;
    void onDebuggingStarted() override;
    void onDebuggingFinished() override;
    void onCancelExecution() override;
    bool onRun() override;
    void onResetCPU() override;

protected:
    // InterfaceISACPU interface
    void onISAStep() override;

private:
    void breakpointHandler();
    void setSignalsFromMicrocode(const MicroCode *line);
    void branchHandler() override;
    void updateAtInstructionEnd() override;
    // For all 256 instructions in the Pep/9 insturction set,
    // map the instruction to the first line of microcode that implements it.
    void calculateInstrJT();
    bool isPrefetchValid;
    QElapsedTimer timer;
    NewCPUDataSection *data;
    QSharedPointer<NewCPUDataSection> dataShared;
    FullMicrocodedMemoizer *memoizer;
    // A class to represent a single item in the instruction specifier
    // or addressing mode decoder.
    struct decoder_entry {
        quint16 addr;
        bool isValid;
    };
    /* For each instruction, specify the first line of microcode that implements
     * that instruction. This jump table is constructed at the beginning of each
     * microprogram-execution to reduce the number of lookups performed by the CPU.
     */
    std::array<decoder_entry, 256> instrSpecJT;
};

#endif // FULLMICROCODEDCPU_H
