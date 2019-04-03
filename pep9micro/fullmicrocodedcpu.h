#ifndef FULLMICROCODEDCPU_H
#define FULLMICROCODEDCPU_H

#include "interfacemccpu.h"
#include "interfaceisacpu.h"
#include <QElapsedTimer>
#include <array>
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
    // Set the microprogram counter to whatever the value of "start" is.
    // This can be used to skip the initialization steps at the top
    // of a microcode program.
    void setMicroPCToStart() noexcept;

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
    void enableDebugging() override;
    void forceBreakpoint(Enu::BreakpointTypes) override;
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
    void calculateAddrJT();
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

    // For each instruction in the instruction set, map the instruction to
    // the first line of microcode that implements it.This calculation is
    // done each time a microprogram is run to account for mnemonic redefinitons.
    // Any modification to the Pep:: instruction mappings while the simulator is running
    // could cause the microprogram to error in unexpected ways.
    std::array<decoder_entry, 256> instrSpecJT;

    // For each of the 256 instruction specifier values, map the
    // addressing mode associated with that IS to the first line of
    // microcode implementing that addressing mode. Do not modify
    // any of the Pep:: addressing mode maps while the simulator
    // is running, else a microprogram might fail unexpectedly.
    std::array<decoder_entry, 256> addrModeJT;
    quint16 startLine = 0;
};

#endif // FULLMICROCODEDCPU_H
