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
    enum class EStatusBit
    {
        STATUS_N = int(Pep9::uarch::EStatusBit::STATUS_N),
        STATUS_Z = int(Pep9::uarch::EStatusBit::STATUS_Z),
        STATUS_V = int(Pep9::uarch::EStatusBit::STATUS_V),
        STATUS_C = int(Pep9::uarch::EStatusBit::STATUS_C),
    };
    Q_ENUM_NS(EStatusBit);
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
    Q_ENUM_NS(EAddrMode)

    extern const QString defaultUnaryMnemonic0;
    extern const QString defaultUnaryMnemonic1;
    extern const QString defaultNonUnaryMnemonic0;
    extern const int defaultMnemon0AddrModes;
    extern const QString defaultNonUnaryMnemonic1;
    extern const int defaultMnemon1AddrModes;
    extern const QString defaultNonUnaryMnemonic2;
    extern const int defaultMnemon2AddrModes;
    extern const QString defaultNonUnaryMnemonic3;
    extern const int defaultMnemon3AddrModes;
    extern const QString defaultNonUnaryMnemonic4;
    extern const int defaultMnemon4AddrModes;


    // Functions for computing instruction specifiers
    int aaaAddressField(EAddrMode addressMode);
    int aAddressField(EAddrMode addressMode);
    QString intToAddrMode(EAddrMode addressMode);
    QString addrModeToCommaSpace(EAddrMode addressMode);

    // Function to compute the number of display character in an operand.
    // (e.g. LDBX only uses a 1 byte operand, while LDWX uses 2,
    // so LDBX needs 2 chars and LDWX 4).
    int operandDisplayFieldWidth(EMnemonic mnemon);

    // Maps between mnemonic enums and strings
    extern QMap<EMnemonic, QString> enumToMnemonMap;
    extern QMap<QString, EMnemonic> mnemonToEnumMap;
    void initEnumMnemonMaps();

    // Maps to characterize each instruction
    extern QMap<EMnemonic, int> opCodeMap;
    extern QMap<EMnemonic, bool> isUnaryMap;
    extern QMap<EMnemonic, bool> addrModeRequiredMap;
    extern QMap<EMnemonic, bool> isTrapMap;
    void initMnemonicMaps(bool NOP0IsTrap);


    // Map to specify legal addressing modes for each instruction
    extern QMap<EMnemonic, int> addrModesMap;
    void initAddrModesMap();

    // Decoder tables
    extern QVector<Pep9::ISA::EMnemonic> decodeMnemonic;
    extern QVector<Pep9::ISA::EAddrMode> decodeAddrMode;
    // Does a particular instruction perform a store instead of a load?
    bool isStoreMnemonic(Pep9::ISA::EMnemonic);
    void initDecoderTables();

    // Map mnemonic to the symbol in microcode which implements that instruction.
    extern QMap<Pep9::ISA::EMnemonic, QString> defaultEnumToMicrocodeInstrSymbol;
    // Map mnemonic to the symbopl in microcode which implements that iunstruction.
    extern QMap<Pep9::ISA::EAddrMode, QString> defaultEnumToMicrocodeAddrSymbol;
    extern QVector<QString> instSpecToMicrocodeInstrSymbol;
    extern QVector<QString> instSpecToMicrocodeAddrSymbol;
    // The default symbol to denote the start of the von-Neumann cycle
    extern QString defaultStartSymbol;
    void initMicroDecoderTables();


}
#endif // ISADEFS_H
