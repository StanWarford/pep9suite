// File: enu.cpp
/*
    Pep9CPU is a CPU simulator for executing microcode sequences to
    implement instructions in the instruction set of the Pep/9 computer.

    Copyright (C) 2019  J. Stanley Warford, Pepperdine University

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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
    }
    // In case an invalid symbol format is passed,
    // return a default value.
    return 0;
}
