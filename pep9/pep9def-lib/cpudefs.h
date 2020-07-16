#ifndef CPUDEFS_H
#define CPUDEFS_H

#include <QtCore>

#include "pep/constants.h"

namespace Pep9::uarch {
    Q_NAMESPACE
    enum class CPURegisters: uint8_t
    {
        // Two byte registers
        // Present in any version of Pep/9
        A = 0, X = 2, SP = 4, PC = 6, OS = 9,
        // Present in any derivative of Pep9CPU
        T2 = 12, T3 = 14,
        T4 = 16, T5 = 18, T6 = 20, M1 = 22, M2 = 24, M3 = 26,
        M4 = 28, M5 = 30,
        // "Fictitious" registers for Pep9micro
        MicroProgramCounter = 128,

        // One byte registers
        // Present in any version of Pep/9
        IS=8,
        // Present in any derivative of Pep9CPU
        T1=11
    };

    // For ALU function 15, and used to mask status bits |'ed
    // together in a single integer.
    enum EMask
    {
        SMask = 0x10,
        NMask = 0x08,
        ZMask = 0x04,
        VMask = 0x02,
        CMask = 0x01,
    };

    enum class EStatusBit
    {
        STATUS_N,STATUS_Z,STATUS_V,STATUS_C,STATUS_S
    };
    Q_ENUM_NS(EStatusBit);


    enum class MainBusState {
        None,
        MemReadFirstWait,
        MemReadSecondWait,
        MemReadReady,
        MemWriteFirstWait,
        MemWriteSecondWait,
        MemWriteReady,
    };
    Q_ENUM_NS(MainBusState);

    enum class EMemoryRegisters
    {
        MEM_MARA,MEM_MARB,MEM_MDR,MEM_MDRO,MEM_MDRE
    };
    Q_ENUM_NS(EMemoryRegisters);

    enum class EALUFunc
    {
        A_func=0,ApB_func=1,ApBpCin_func=2,ApnBp1_func=3,
        ApnBpCin_func=4,AandB_func=5,nAandB_func=6,AorB_func=7,
        nAorB_func=8,AxorB_func=9,nA_func=10,ASLA_func=11,
        ROLA_func=12,ASRA_func=13,RORA_func=14,NZVCA_func=15,
        UNDEFINED_func=255,
    };

    // Instruction mnemonics
    enum class EControlSignals
    {
        MemRead=0, MemWrite,
        A,B,EOMux, MARMux,MARA, MARB, AMux, ALU,CSMux, AndZ,
        CMux, C,
        MDRMux, MDROMux, MDREMux,MDR, MDRE, MDRO,
        PValid,
    };
    Q_ENUM_NS(EControlSignals);

    enum class EClockSignals{
        NCk=0,
        ZCk,VCk,CCk,SCk,MARCk,LoadCk,MDRCk, MDROCk, MDRECk,
        PValidCk,
    };
    Q_ENUM_NS(EClockSignals);

    enum class ECPUKeywords {
        Pre, Post,
        Mem, Acc, X, SP, PC, IR,
        T1, T2, T3, T4, T5, T6,
        N, Z, V,Cbit, S,
        MARAREG, MARBREG,
        MDRREG, MDREREG, MDROREG
    };

    /*
     * Enumerations for Pep9Micro
     */
    enum class EBranchFunctions{
        Unconditional = 0,
        uBRGT = 1, uBRGE = 2, uBREQ = 3, uBRLE = 4, uBRLT = 5,
        uBRNE = 6, uBRV = 7, uBRC = 8, uBRS = 9,
        IsPrefetchValid = 10,
        IsUnary = 11,
        PCisOdd = 12,
        AddressingModeDecoder = 13, // Adressing modes jump table
        InstructionSpecifierDecoder = 14, // Instruction jump table
        Stop = 15,
        Assembler_Assigned = 16
    };
    Q_ENUM_NS(EBranchFunctions);


    // Must declare as extern, or values will not be preserved between translation units.
    extern QMap<EControlSignals, QString> decControlToMnemonMap;
    extern QMap<EControlSignals, QString> memControlToMnemonMap;
    extern QMap<EClockSignals, QString> clockControlToMnemonMap;
    extern QMap<ECPUKeywords, QString> specificationToMnemonMap;
    extern QMap<ECPUKeywords, QString> memSpecToMnemonMap;
    extern QMap<ECPUKeywords, QString> regSpecToMnemonMap;
    extern QMap<ECPUKeywords, QString> statusSpecToMnemonMap;
    extern QMap<EBranchFunctions,QString> branchFuncToMnemonMap;
    extern QMap<QString, EBranchFunctions> mnemonToBranchFuncMap;
    extern QMap<QString, EControlSignals> mnemonToDecControlMap;
    extern QMap<QString, EControlSignals> mnemonToMemControlMap;
    extern QMap<QString, EClockSignals> mnemonToClockControlMap;
    extern QMap<QString, ECPUKeywords> mnemonToSpecificationMap;
    extern QMap<QString, ECPUKeywords> mnemonToMemSpecMap;
    extern QMap<QString, ECPUKeywords> mnemonToRegSpecMap;
    extern QMap<QString, ECPUKeywords> mnemonToStatusSpecMap;
    void initMicroEnumMnemonMaps(PepCore::CPUType cpuType, bool fullCtrlSection);
    quint8 numControlSignals();
    quint8 numClockSignals();
}



#endif // CPUDEFS_H
