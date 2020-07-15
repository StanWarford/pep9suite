#ifndef CPUDEFS_H
#define CPUDEFS_H

#include <QtCore>

namespace Pep9::uarch {
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
    Q_NAMESPACE
    enum MainBusState {
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
}



#endif // CPUDEFS_H
