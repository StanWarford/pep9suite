#ifndef PEP9_H
#define PEP9_H
#include <cstdint>

#include "pep/apepversion.h"
class Pep9 : public QObject, public APepVersion
{
    Q_OBJECT
public:
    Pep9(bool NOP0IsTrap=true);
    virtual ~Pep9() override;
    enum class CPURegisters: uint8_t
    {
        // Two byte registers
        // Present in any version of Pep/9
        A = 0, X = 2, SP = 4, PC = 6, OS = 9,
        // Present in any derivative of Pep9CPU
        T2 = 12, T3 = 14,
        T4 = 16, T5 = 18, T6 = 20, M1 = 22, M2 = 24, M3 = 26,
        M4 = 28, M5 = 30,
        // "Fictitious" registers for Pep9micro
        MicroProgramCounter = 128,

        // One byte registers
        // Present in any version of Pep/9
        IS=8,
        // Present in any derivative of Pep9CPU
        T1=11
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
    Q_ENUM(EMnemonic)

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

    // APepVersion interface
public:
    PepCore::CPURegisters_number_t get_global_register_number(global_registers) const override;
    bool isInstructionUnary(quint8) const override;
    quint8 maxRegisterNumber() const override;
private:
    QMap<EMnemonic, int> opCodeMap;
    QMap<EMnemonic, bool> isUnaryMap;
    QMap<EMnemonic, bool> addrModeRequiredMap;
    QMap<EMnemonic, bool> isTrapMap;
    void initMnemonicMaps(bool NOP0IsTrap);

    // Map to specify legal addressing modes for each instruction
    QMap<EMnemonic, int> addrModesMap;
    void initAddrModesMap();

    // Decoder tables
    QVector<EMnemonic> decodeMnemonic;
    QVector<EAddrMode> decodeAddrMode;
    // Does a particular instruction perform a store instead of a load?
    bool isStoreMnemonic(EMnemonic);
    void initDecoderTables();
};

uint8_t to_uint8_t(Pep9::CPURegisters reg);


#endif // PEP9_H


