#ifndef ISACPU_H
#define ISACPU_H
#include "interfaceisacpu.h"
#include <QElapsedTimer>
#include "registerfile.h"

/* Though not part of the specification, the trap mechanism  must
 * set the index register to 0 to prevent a bug in OS where
 * non-unary instructions fail due to junk in the high order byte of the index
 * register. This flag enables or disables this behavior.
 */
#define performTrapFix true

/* Though not part of the specification, the trap mechanism  must
 * increment the program counter by 2 in Pep/9 for the operating system
 * to function correctly. This flag enables or disables this behvaior
 */
#define hardwarePCIncr true
class NewCPUDataSection;
class IsaCpuMemoizer;
class IsaCpu: public ACPUModel, public InterfaceISACPU
{
    friend class IsaCpuMemoizer;
public:
    explicit IsaCpu(const AsmProgramManager* manager, QSharedPointer<AMemoryDevice>, QObject* parent = nullptr);
    virtual ~IsaCpu() override;
    // InterfaceISACPU interface
public:
    void stepOver() override;
    bool canStepInto() const override;
    void stepInto() override;
    void stepOut() override;
    bool getOperandWordValue(quint16 operand, Enu::EAddrMode addrMode, quint16& opVal);
    bool getOperandByteValue(quint16 operand, Enu::EAddrMode addrMode, quint8& opVal);
    RegisterFile& getRegisterBank();
    const RegisterFile& getRegisterBank() const;

protected:
    void onISAStep() override;
    void updateAtInstructionEnd() override;
    bool readOperandWordValue(quint16 operand, Enu::EAddrMode addrMode, quint16& opVal);
    bool readOperandByteValue(quint16 operand, Enu::EAddrMode addrMode, quint8& opVal);


    // ACPUModel interface
public:
    void initCPU() override;
    bool getStatusBitCurrent(Enu::EStatusBit) const override;
    bool getStatusBitStart(Enu::EStatusBit) const override;
    quint8 getCPURegByteCurrent(Enu::CPURegisters reg) const override;
    quint16 getCPURegWordCurrent(Enu::CPURegisters reg) const override;
    quint8 getCPURegByteStart(Enu::CPURegisters reg) const override;
    quint16 getCPURegWordStart(Enu::CPURegisters reg) const override;
    QString getErrorMessage() const noexcept override;
    bool hadErrorOnStep() const noexcept override;
    bool stoppedForBreakpoint() const noexcept override;

public slots:
    void onSimulationStarted() override;
    void onSimulationFinished() override;
    void onDebuggingStarted() override;
    void onDebuggingFinished() override;
    void onCancelExecution() override;
    bool onRun() override;
    void onResetCPU() override;


private:
    RegisterFile registerBank;
    QElapsedTimer timer;
    IsaCpuMemoizer* memoizer;
    bool operandWordValueHelper(quint16 operand, Enu::EAddrMode addrMode,
                           bool (AMemoryDevice::*readFunc)(quint16, quint16&) const, quint16& opVal);
    bool operandByteValueHelper(quint16 operand, Enu::EAddrMode addrMode,
                         bool (AMemoryDevice::*readFunc)(quint16, quint8&) const, quint8& opVal);
    bool writeOperandWord(quint16 operand, quint16 value, Enu::EAddrMode addrMode);
    bool writeOperandByte(quint16 operand, quint8 value, Enu::EAddrMode addrMode);
    void executeUnary(Enu::EMnemonic mnemon);
    void executeNonunary(Enu::EMnemonic mnemon, quint16 opSpec, Enu::EAddrMode addrMode);
    void executeTrap(Enu::EMnemonic mnemon);
    void breakpointHandler();


};

#endif // ISACPU_H
