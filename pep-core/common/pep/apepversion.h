#ifndef APEPVERSION_H
#define APEPVERSION_H

#include <QtCore>

#include "pep/constants.h"
#include "pep/types.h"
#include "style/colors.h"

class ASMHighlighter;
class MicroHighlighter;

enum class PepVersion
{
    Pep8,
    Pep9,
    Pep10,
};
class APepVersion
{
public:
    enum class global_registers
    {
        A, X, SP, PC, IS, OS
    };
    enum class global_status_bits
    {
        N, Z, V, C
    };

    APepVersion();
    virtual ~APepVersion() = 0;

    virtual PepCore::CPURegisters_number_t get_global_register_number(global_registers) const = 0;
    virtual PepCore::CPUStatusBits_name_t get_global_status_bit_number(global_status_bits) const = 0;

    virtual bool isInstructionUnary(quint8) const = 0;
    virtual quint8 maxRegisterNumber() const = 0;
    virtual quint8 maxStatusBitNumber() const = 0;

    virtual ASMHighlighter* getASMHighlighter(PepColors::Colors) const = 0;
    virtual MicroHighlighter* getMicroHighlighter(PepCore::CPUType type, bool fullCtrlSection, const PepColors::Colors colors) const = 0;
};

#endif // APEPVERSION_H
