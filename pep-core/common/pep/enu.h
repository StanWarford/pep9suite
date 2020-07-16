// File: enu.h
/*
    The Pep/9 suite of applications (Pep9, Pep9CPU, Pep9Micro) are
    simulators for the Pep/9 virtual machine, and allow users to
    create, simulate, and debug across various levels of abstraction.

    Copyright (C) 2010  J. Stanley Warford, Pepperdine University

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
#ifndef ENU_H
#define ENU_H

#include <QtCore>

namespace Enu {
    Q_NAMESPACE

    /*
     * Enumerations for all applications
     */
    static const quint8 maxRegisterNumber = 31;
    static const quint8 signalDisabled = 255;


    /*
     * Enumerations for Pep9
     */
    enum class EMnemonic: int
    {
        ADDA, ADDX, ADDSP, ANDA, ANDX, ASLA, ASLX, ASRA, ASRX,
        BR, BRC, BREQ, BRGE, BRGT, BRLE, BRLT, BRNE, BRV,
        CALL, CPBA, CPBX, CPWA, CPWX,
        DECI, DECO,
        HEXO,
        LDBA, LDBX, LDWA, LDWX,
        MOVAFLG, MOVFLGA, MOVSPA,
        NEGA, NEGX, NOP, NOP0, NOP1, NOTA, NOTX,
        ORA, ORX,
        RET, RETTR, ROLA, ROLX, RORA, RORX,
        STBA, STBX, STWA, STWX, STOP, STRO, SUBA, SUBX, SUBSP

    };
    Q_ENUM_NS(EMnemonic);

    // Addressing modes for instructions
    enum class EAddrMode: int
    {
        NONE = 0,
        I = 1,
        D = 2,
        N = 4,
        S = 8,
        SF = 16,
        X = 32,
        SX = 64,
        SFX = 128,
        ALL = 255
    };
    Q_ENUM_NS(EAddrMode);

    enum class EExecState
    {
        EStart,
        ERun, ERunAwaitIO,
        EDebugAwaitIO, EDebugAwaitClick, EDebugRunToBP, EDebugSingleStep
    };

    /*
     * Enumerations for Pep9CPU
     */

    enum EStatusBit
    {
        STATUS_N,STATUS_Z,STATUS_V,STATUS_C,STATUS_S
    };
    Q_ENUM_NS(EStatusBit);


}
#endif
