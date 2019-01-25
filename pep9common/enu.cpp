#include "enu.h"

quint16 Enu::tagNumBytes(Enu::ESymbolFormat symbolFormat)
{
    switch (symbolFormat) {
    case Enu::ESymbolFormat::F_1C: return 1;
    case Enu::ESymbolFormat::F_1D: return 1;
    case Enu::ESymbolFormat::F_2D: return 2;
    case Enu::ESymbolFormat::F_1H: return 1;
    case Enu::ESymbolFormat::F_2H: return 2;
    case Enu::ESymbolFormat::F_NONE: return 0;
    default: return 0; // Should not occur.
    }
}
