#ifndef ISADEFS_H
#define ISADEFS_H
#include <cstdint>

#include <QtCore>

#include "cpudefs.h"

namespace Pep9::ISA {
    Q_NAMESPACE
    // This is a reduced version of the registers available inside Pep9::uarch.
    // Refer to the register numbers in uarch for consistency
    enum class CPURegisters: uint8_t
    {
        // Two byte registers
        // Present in any version of Pep/9
        A =  uint8_t(Pep9::uarch::CPURegisters::A),
        X =  uint8_t(Pep9::uarch::CPURegisters::X),
        SP = uint8_t(Pep9::uarch::CPURegisters::SP),
        PC = uint8_t(Pep9::uarch::CPURegisters::PC),
        OS = uint8_t(Pep9::uarch::CPURegisters::OS),

        // One byte registers
        // Present in any version of Pep/9
        IS = uint8_t(Pep9::uarch::CPURegisters::IS),
    };

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
    Q_ENUM_NS(EMnemonic)

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
}
#endif // ISADEFS_H
