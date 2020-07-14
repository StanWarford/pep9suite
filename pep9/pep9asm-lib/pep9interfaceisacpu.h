#ifndef PEP9INTERFACEISACPU_H
#define PEP9INTERFACEISACPU_H
#include "cpu/interfaceisacpu.h"

class Pep9InterfaceISACPU : public InterfaceISACPU
{
public:
    explicit Pep9InterfaceISACPU(const AMemoryDevice* dev, const AsmProgramManager* manager) noexcept;
    virtual ~Pep9InterfaceISACPU() = default;

    // InterfaceISACPU interface
protected:
    void calculateStackChangeStart(quint8 instr) override;
    void calculateStackChangeEnd(quint8 instr, quint16 opspec, quint16 sp, quint16 pc, quint16 acc) override;
};
QString mnemonDecode(Enu::EMnemonic instrSpec);
#endif // PEP9INTERFACEISACPU_H
