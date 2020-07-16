#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QtCore>

namespace PepCore {
    Q_NAMESPACE
    enum class BreakpointTypes: int
    {
        MICROCODE = 1<<0, ASSEMBLER = 1<<1,
    };

    // Bit masks that signal which editing actions should be available through context menus
    enum EditButton: int
    {
        COPY = 1<<0, CUT = 1<<1, PASTE = 1<<2, UNDO = 1<<3, REDO = 1<<4
    };

    enum class EPane
    {
        ESource,
        EObject,
        EListing,
        EListingTrace,
        EMemoryTrace,
        EBatchIO,
        ETerminal,
        EMicrocode,
        EDataSection,
    };
    Q_ENUM_NS(EPane)

    enum class CPUType {
        OneByteDataBus,
        TwoByteDataBus,
    };

    static const quint8 signalDisabled = 255;

}

#endif // CONSTANTS_H

