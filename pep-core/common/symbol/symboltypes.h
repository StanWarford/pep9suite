#ifndef SYMBOLTYPES_H
#define SYMBOLTYPES_H

#include <QtCore>

// Format for symbols
enum class ESymbolFormat: int
{
    F_NONE, F_1C, F_1D, F_2D, F_1H, F_2H
};

quint16 tagNumBytes(ESymbolFormat symbolFormat);
// Pre: symbolFormat is a valid format trace tag type.
// Post: Returns the corresponding integer number of bytes.

#endif // SYMBOLTYPES_H
