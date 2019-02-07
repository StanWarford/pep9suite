#ifndef CPUDATASECTION_H
#define CPUDATASECTION_H

#include <QObject>
#include <QObject>
#include <QVector>
#include <QException>
#include <QString>
#include "enu.h"
class AMemoryDevice;
class InterfaceMCCPU;
class MemorySection;
class MicroCode;
class RegisterFile;
class NewCPUDataSection: public QObject
{
    Q_OBJECT
    friend class CPUControlSection;
    friend class InterfaceMCCPU;
public:
    NewCPUDataSection(Enu::CPUType type, QSharedPointer<AMemoryDevice> memDevice, QObject *parent = nullptr );
    virtual ~NewCPUDataSection();


    Enu::CPUType getCPUType() const;
    //Access CPU registers
    RegisterFile& getRegisterBank();
    const RegisterFile& getRegisterBank() const;
    quint8 getRegisterBankByte(quint8 registerNumber) const;
    quint16 getRegisterBankWord(quint8 registerNumber) const; //Follows even/odd conventions of pep/9
    quint8 getRegisterBankByte(Enu::CPURegisters registerNumber) const;
    quint16 getRegisterBankWord(Enu::CPURegisters registerNumber) const; //Follows even/odd conventions of pep/9
    quint8 getMemoryRegister(Enu::EMemoryRegisters registerNumber)const;

    //Access register & Memory Buses
    bool valueOnABus(quint8& result) const;
    bool valueOnBBus(quint8& result) const;
    bool valueOnCBus(quint8& result) const;
    Enu::MainBusState getMainBusState() const;

    //Test for Signals and Registers
    quint8 getControlSignals(Enu::EControlSignals controlSignal) const;
    bool getClockSignals(Enu::EClockSignals) const;
    bool getStatusBit(Enu::EStatusBit) const;

    bool setSignalsFromMicrocode(const MicroCode* line);
    void setEmitEvents(bool b);
    //Return information about errors on the last step
    bool hadErrorOnStep() const;
    QString getErrorMessage() const;

    /*
     * Information about CPU internals
     */
    //Is the CPU function Unary?
    bool aluFnIsUnary() const;
    //Return true if AMux has output, and set result equal to the value of the output.
    //Works for one and two byte buses
    bool getAMuxOutput(quint8 &result) const;
    //Return  true if CSMux has an ouput, and set result equal to the output if present
    bool calculateCSMuxOutput(bool& result) const;
    //Return if the ALU has an ouput, and set result & NZVC bits according to the ALU function
    bool calculateALUOutput(quint8& result,quint8 &NZVC) const;

    //Helper functions to clear different aspects of the data section
    void clearControlSignals() noexcept;
    void clearClockSignals() noexcept;

private:
    void setMemoryDevice(QSharedPointer<AMemoryDevice> newDevice);
    QSharedPointer<AMemoryDevice> memDevice;

    Enu::CPUType cpuFeatures;
    Enu::MainBusState mainBusState;
    bool emitEvents;

    //Data registers
    QSharedPointer<RegisterFile> registerBank;
    QVector<quint8> memoryRegisters;

    //Control Signals
    QVector<quint8> controlSignals;
    QVector<bool> clockSignals;

    //Error handling
    bool hadDataError;
    QString errorMessage;

    // After thorough profiling, the ALU calculation is the second most computationally expensive part of the data section.
    // The ALU calculation is called multiple times per cycle, yet the result can't change within a cycle.
    // So, to cut back on wasteful calculations, the output of the alu is cached within a cycle.
    // At start of a step, isALUCacheValid must be set to false
    mutable bool isALUCacheValid, ALUHasOutputCache;
    mutable quint8 ALUOutputCache, ALUStatusBitCache;

    //Set the values values of the sequential data registers (numbers 22-31)
    void presetStaticRegisters() noexcept;

    void clearRegisters() noexcept;
    void clearErrors() noexcept;

    //Simulation stepping logic
    void handleMainBusState() noexcept;
    void stepOneByte() noexcept;
    void stepTwoByte() noexcept;

public slots:
    void onSetStatusBit(Enu::EStatusBit,bool val);
    void onSetRegisterByte(quint8 reg,quint8 val);
    void onSetRegisterWord(quint8 reg,quint16 val);
    void onSetMemoryRegister(Enu::EMemoryRegisters,quint8 val);
    void onSetClock(Enu::EClockSignals, bool value);
    void onSetControlSignal(Enu::EControlSignals, quint8 value);
    void onStep() noexcept;
    void onClock() noexcept;
    void onClearCPU() noexcept;
    void onSetCPUType(Enu::CPUType type);

signals:
    void registerChanged(quint8 reg, quint8 oldVal, quint8 newVal); //Thrown whenever a register in the register bank is changed.
    void memoryRegisterChanged(Enu::EMemoryRegisters, quint8 oldVal, quint8 newVal); //Thrown whenever a memory register is changed.
    void statusBitChanged(Enu::EStatusBit status, bool value);
    void CPUTypeChanged(Enu::CPUType type);

};

#endif // CPUDATASECTION_H
