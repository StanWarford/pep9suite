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
    PepCore::CPUStatusBits_name_t get_global_status_bit_number(global_status_bits) const override;
    bool isInstructionUnary(quint8) const override;
    quint8 maxRegisterNumber() const override;
    quint8 maxStatusBitNumber() const override;
    ASMHighlighter *getASMHighlighter(PepColors::Colors) const override;
    MicroHighlighter *getMicroHighlighter(PepCore::CPUType type, bool fullCtrlSection, const PepColors::Colors colors) const override;
public:
    static PepCore::CPURegisters_number_t getStatusBitOffset(Pep9::ISA::EStatusBit);
    static PepCore::CPURegisters_number_t getStatusBitOffset(Pep9::uarch::EStatusBit);

    // APepVersion interface
public:
    InstrIdent getInstructionLookupKey(quint8 instruction_spec) const override;
    AddrIdent getInstrAddrMode(quint8 instruction_spec) const override;
    QString getAsmMnemonic(InstrIdent instruction_key) const override;
    QString getAsmMnemonic(quint8 instruction_spec) const override;
    quint8 operandDisplayFieldWidth(InstrIdent) const override;
    quint8 operandDisplayFieldWidth(quint8 instruction_spec) const override;
    bool isInstructionUnary(InstrIdent) const override;
    bool isInstructionTrap(quint8 instruction_specifier) const override;
    bool isInstructionTrap(InstrIdent) const override;
    QString getAsmAddr(AddrIdent) const override;
    QString getAsmAddr(quint8 instruction_spec) const override;
};
}

uint8_t to_uint8_t(Pep9::ISA::CPURegisters reg);
uint8_t to_uint8_t(Pep9::uarch::CPURegisters reg);
uint8_t to_uint8_t(Pep9::uarch::EControlSignals signal);
uint8_t to_uint8_t(Pep9::uarch::EClockSignals signal);


#endif // PEP9_H


