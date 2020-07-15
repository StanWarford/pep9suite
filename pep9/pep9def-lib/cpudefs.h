#ifndef CPUDEFS_H
#define CPUDEFS_H

#include <QtCore>

namespace Pep9CPU {
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
