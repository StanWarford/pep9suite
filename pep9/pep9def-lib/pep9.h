#ifndef PEP9_H
#define PEP9_H
#include <cstdint>

#include "pep/apepversion.h"

#include "isadefs.h"
#include "cpudefs.h"
namespace Pep9
{
class Definition : public QObject, public APepVersion
{
    Q_OBJECT
public:
    Definition(bool NOP0IsTrap=true);
    virtual ~Definition() override;


    // APepVersion interface
public:
    PepCore::CPURegisters_number_t get_global_register_number(global_registers) const override;
    bool isInstructionUnary(quint8) const override;
    quint8 maxRegisterNumber() const override;
    ASMHighlighter *getASMHighlighter(PepColors::Colors) const override;
    MicroHighlighter *getMicroHighlighter(PepCore::CPUType type, bool fullCtrlSection, const PepColors::Colors colors) const override;
private:
    QMap<Pep9::ISA::EMnemonic, int> opCodeMap;
    QMap<Pep9::ISA::EMnemonic, bool> isUnaryMap;
    QMap<Pep9::ISA::EMnemonic, bool> addrModeRequiredMap;
    QMap<Pep9::ISA::EMnemonic, bool> isTrapMap;
    void initMnemonicMaps(bool NOP0IsTrap);

    // Map to specify legal addressing modes for each instruction
    QMap<Pep9::ISA::EMnemonic, int> addrModesMap;
    void initAddrModesMap();

    // Decoder tables
    QVector<Pep9::ISA::EMnemonic> decodeMnemonic;
    QVector<Pep9::ISA::EAddrMode> decodeAddrMode;
    // Does a particular instruction perform a store instead of a load?
    bool isStoreMnemonic(Pep9::ISA::EMnemonic);
    void initDecoderTables();

    // Pep9 micro tables
};
}

uint8_t to_uint8_t(Pep9::ISA::CPURegisters reg);
uint8_t to_uint8_t(Pep9::uarch::CPURegisters reg);
uint8_t to_uint8_t(Pep9::uarch::EControlSignals signal);
uint8_t to_uint8_t(Pep9::uarch::EClockSignals signal);


#endif // PEP9_H


