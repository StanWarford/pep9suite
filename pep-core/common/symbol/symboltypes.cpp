#include "symboltypes.h"

quint16 tagNumBytes(ESymbolFormat symbolFormat)
{
    switch (symbolFormat) {
    case ESymbolFormat::F_1C: return 1;
    case ESymbolFormat::F_1D: return 1;
    case ESymbolFormat::F_2D: return 2;
    case ESymbolFormat::F_1H: return 1;
    case ESymbolFormat::F_2H: return 2;
    case ESymbolFormat::F_NONE: return 0;
    }
    // In case an invalid symbol format is passed,
    // return a default value.
    return 0;
}
