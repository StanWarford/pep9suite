#ifndef CPUDATASECTION_H
#define CPUDATASECTION_H

#include <QObject>
#include <QObject>
#include <QVector>
#include <QException>
#include <QString>
#include "enu.h"
class MemorySection;
class MicroCode;
class CPUDataSection: public QObject
{
    Q_OBJECT
    friend class CPUControlSection;
public:
    static CPUDataSection* getInstance();
    virtual ~CPUDataSection();


    Enu::CPUType getCPUFeatures() const;
    //Access CPU registers
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

    MemorySection* getMemorySection();
    const MemorySection* getMemorySection() const;

private:
    CPUDataSection(QObject* parent=0);
    static CPUDataSection* _instance;
    Enu::CPUType cpuFeatures;
    Enu::MainBusState mainBusState;
    bool emitEvents;

    //Data registers
    QVector<quint8> registerBank;
    QVector<quint8> memoryRegisters;
    quint8 NZVCSbits;

    //Control Signals
    QVector<quint8> controlSignals;
    QVector<bool> clockSignals;

    //Error handling
    bool hadDataError=false;
    QString errorMessage="";

    MemorySection* memory;

    // After thorough profiling, the ALU calculation is the second most computationally expensive part of the data section.
    // The ALU calculation is called multiple times per cycle, yet the result can't change within a cycle.
    // So, to cut back on wasteful calculations, the output of the alu is cached within a cycle.
    // At start of a step, isALUCacheValid must be set to false
    mutable bool isALUCacheValid, ALUHasOutputCache;
    mutable quint8 ALUOutputCache, ALUStatusBitCache;

    //Set the values values of the sequential data registers (numbers 22-31)
    void presetStaticRegisters() noexcept;

    //Simulation stepping logic
    void handleMainBusState() noexcept;
    void stepOneByte() noexcept;
    void stepTwoByte() noexcept;

    //Helper functions to clear different aspects of the data section
    void clearControlSignals() noexcept;
    void clearClockSignals() noexcept;
    void clearRegisters() noexcept;
    void clearErrors() noexcept;

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

signals:
    void CPUFeaturesChanged(Enu::CPUType newFeatures); //Thrown whenever onCPUFeaturesChanged(...) is called
    void registerChanged(quint8 reg,quint8 oldVal,quint8 newVal); //Thrown whenever a register in the register bank is changed.
    void memoryRegisterChanged(Enu::EMemoryRegisters,quint8 oldVal,quint8 newVal); //Thrown whenever a memory register is changed.
    void statusBitChanged(Enu::EStatusBit status,bool value);

};

#endif // CPUDATASECTION_H
