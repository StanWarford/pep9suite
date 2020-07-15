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
}

#endif // CPUDEFS_H
