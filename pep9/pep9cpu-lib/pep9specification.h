#ifndef PEP9SPECIFICATION_H
#define PEP9SPECIFICATION_H
#include <QString>

#include "memory/mainmemory.h"
#include "microassembler/specification.h"

#include "cpudata.h"
class MemSpecification: public Specification {
public:
    MemSpecification(int memoryAddress, int memoryValue, int numberBytes) noexcept;
    void setUnitPre(CPUDataSection*, AMemoryDevice*) noexcept override;
    bool testUnitPost(const CPUDataSection *data, const AMemoryDevice*,
                      QString &errString) const noexcept override;
    QString getSourceCode() const noexcept override;
private:
    int memAddress;
    int memValue;
    int numBytes;
};

class RegSpecification: public Specification {
public:
    RegSpecification(Pep9::uarch::ECPUKeywords registerAddress, int registerValue) noexcept;
    void setUnitPre(CPUDataSection*, AMemoryDevice*) noexcept override;
    bool testUnitPost(const CPUDataSection *data, const AMemoryDevice*,
                      QString &errString) const noexcept override;
    QString getSourceCode() const noexcept override;
private:
    Pep9::uarch::ECPUKeywords regAddress;
    int regValue;
};

class StatusBitSpecification: public Specification {
public:
    StatusBitSpecification(Pep9::uarch::ECPUKeywords statusBitAddress, bool statusBitValue) noexcept;
    void setUnitPre(CPUDataSection*, AMemoryDevice*) noexcept override;
    bool testUnitPost(const CPUDataSection *data, const AMemoryDevice*,
                      QString &errString) const noexcept override;
    QString getSourceCode() const noexcept override;
private:
    Pep9::uarch::ECPUKeywords nzvcsAddress;
    bool nzvcsValue;
};
#endif // PEP9SPECIFICATION_H