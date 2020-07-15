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
}

#endif // CPUDEFS_H
