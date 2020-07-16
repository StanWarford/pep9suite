#include "pep9.h"
#include "pep9asmhighlighter.h"
#include "pep9microhighlighter.h"

Pep9::Definition::Definition(bool NOP0IsTrap): QObject(), APepVersion()
{
}

Pep9::Definition::~Definition() = default;

PepCore::CPURegisters_number_t Pep9::Definition::get_global_register_number(APepVersion::global_registers reg) const
{
    switch(reg) {
    case APepVersion::global_registers::A:
        return to_uint8_t(Pep9::ISA::CPURegisters::A);
    case APepVersion::global_registers::X:
        return to_uint8_t(Pep9::ISA::CPURegisters::X);
    case APepVersion::global_registers::SP:
        return to_uint8_t(Pep9::ISA::CPURegisters::SP);
    case APepVersion::global_registers::PC:
        return to_uint8_t(Pep9::ISA::CPURegisters::PC);
    case APepVersion::global_registers::IS:
        return to_uint8_t(Pep9::ISA::CPURegisters::IS);
    case APepVersion::global_registers::OS:
        return to_uint8_t(Pep9::ISA::CPURegisters::OS);
    }
}

PepCore::CPUStatusBits_name_t Pep9::Definition::get_global_status_bit_number(APepVersion::global_status_bits bit) const
{
    switch(bit) {
    case APepVersion::global_status_bits::N:
        return static_cast<PepCore::CPUStatusBits_name_t>(Pep9::ISA::EStatusBit::STATUS_N);
    case APepVersion::global_status_bits::Z:
        return static_cast<PepCore::CPUStatusBits_name_t>(Pep9::ISA::EStatusBit::STATUS_Z);
    case APepVersion::global_status_bits::V:
        return static_cast<PepCore::CPUStatusBits_name_t>(Pep9::ISA::EStatusBit::STATUS_V);
    case APepVersion::global_status_bits::C:
        return static_cast<PepCore::CPUStatusBits_name_t>(Pep9::ISA::EStatusBit::STATUS_C);
    }
}

bool Pep9::Definition::isInstructionUnary(quint8 instr) const
{
    return Pep9::ISA::isUnaryMap[Pep9::ISA::decodeMnemonic[instr]];
}

quint8 Pep9::Definition::maxRegisterNumber() const
{
    return 32;
}

quint8 Pep9::Definition::maxStatusBitNumber() const
{
    return 5;
}

ASMHighlighter *Pep9::Definition::getASMHighlighter(PepColors::Colors colors) const
{
    return new Pep9ASMHighlighter(colors, nullptr);
}

MicroHighlighter *Pep9::Definition::getMicroHighlighter(PepCore::CPUType type, bool fullCtrlSection, const PepColors::Colors colors) const
{
    return new Pep9MicroHighlighter(type, fullCtrlSection, colors, nullptr);
}

PepCore::CPURegisters_number_t Pep9::Definition::getStatusBitOffset(Pep9::ISA::EStatusBit bit)
{
    return static_cast<int>(bit);
}
PepCore::CPURegisters_number_t Pep9::Definition::getStatusBitOffset(Pep9::uarch::EStatusBit bit)
{
    return static_cast<int>(bit);
}

InstrIdent Pep9::Definition::getInstructionLookupKey(quint8 instr) const
{
    return {static_cast<uint8_t>(Pep9::ISA::decodeMnemonic[instr])};
}

AddrIdent Pep9::Definition::getInstrAddrMode(quint8 instr) const
{
    return {static_cast<uint8_t>(Pep9::ISA::decodeAddrMode[instr])};
}

QString Pep9::Definition::getAsmMnemonic(InstrIdent instruction_key) const
{
    static QMetaEnum metaenum = Pep9::ISA::staticMetaObject.enumerator(Pep9::ISA::staticMetaObject.indexOfEnumerator("EMnemonic"));
    return QString(metaenum.valueToKey(instruction_key.key)).toLower();
}

QString Pep9::Definition::getAsmMnemonic(quint8 instruction_spec) const
{
    return getAsmMnemonic(getInstructionLookupKey(instruction_spec));
}

quint8 Pep9::Definition::operandDisplayFieldWidth(InstrIdent ident) const
{
    using namespace Pep9::ISA;
    auto mnemon = static_cast<EMnemonic>(ident.key);
    switch(mnemon) {
    // All byte instructions that do not perform stores only need 1 byte
    // wide operands, which is 2 characters. STBr's operand is a memory address,
    // which still needs 2 bytes to be represented.
    case EMnemonic::LDBA:
        [[fallthrough]];
    case EMnemonic::LDBX:
        [[fallthrough]];
    case EMnemonic::CPBA:
        [[fallthrough]];
    case EMnemonic::CPBX:
        return 2;
    // All others use 2 bytes, which is 4 characters.
    default:
        return 4;

    }
}

quint8 Pep9::Definition::operandDisplayFieldWidth(quint8 instruction_spec) const
{
    return operandDisplayFieldWidth(getInstructionLookupKey(instruction_spec));
}

bool Pep9::Definition::isInstructionUnary(InstrIdent ident) const
{
    auto mnemon = static_cast<Pep9::ISA::EMnemonic>(ident.key);
    return Pep9::ISA::isUnaryMap[mnemon];
}

bool Pep9::Definition::isInstructionTrap(quint8 instruction_specifier) const
{
    return isInstructionTrap(getInstructionLookupKey(instruction_specifier));
}

bool Pep9::Definition::isInstructionTrap(InstrIdent ident) const
{
    auto mnemon = static_cast<Pep9::ISA::EMnemonic>(ident.key);
    return Pep9::ISA::isTrapMap[mnemon];
}

QString Pep9::Definition::getAsmAddr(AddrIdent ident) const
{
    using namespace Pep9::ISA;

    auto addr = static_cast<EAddrMode>(ident.key);
    if (addr == EAddrMode::I) return "i";
    if (addr == EAddrMode::D) return "d";
    if (addr == EAddrMode::N) return "n";
    if (addr == EAddrMode::S) return "s";
    if (addr == EAddrMode::SF) return "sf";
    if (addr == EAddrMode::X) return "x";
    if (addr == EAddrMode::SX) return "sx";
    if (addr == EAddrMode::SFX) return "sfx";
    return ""; // Should not occur

}

QString Pep9::Definition::getAsmAddr(quint8 instruction_spec) const
{
    return getAsmAddr(getInstrAddrMode(instruction_spec));


}

uint8_t to_uint8_t(Pep9::ISA::CPURegisters reg)
{
    return static_cast<uint8_t>(reg);
}

uint8_t to_uint8_t(Pep9::uarch::CPURegisters reg)
{
    return static_cast<uint8_t>(reg);
}

uint8_t to_uint8_t(Pep9::uarch::EControlSignals signal)
{
    return static_cast<uint8_t>(signal);
}

uint8_t to_uint8_t(Pep9::uarch::EClockSignals signal)
{
    return static_cast<uint8_t>(signal);
}

