#ifndef FULLMICROCODEMEMOIZER_H
#define FULLMICROCODEMEMOIZER_H

#include <QString>
#include "enu.h"
#include "memoizerhelper.h"

class FullMicrocodedCPU;
class FullMicrocodedMemoizer
{
public:
    explicit FullMicrocodedMemoizer(FullMicrocodedCPU& item);

    void clear();
    void storeStateInstrEnd();
    // Must initialize InterfaceISACPU:opValCache here for FullMicrocoded CPU
    // to fulfill its contract with InterfaceISACPU.
    void storeStateInstrStart();
    QString memoize();
    QString finalStatistics();

private:
    FullMicrocodedCPU& cpu;
    CPUState state;
    // The InterfaceISACPU requires that the CPU capture the value of the decoded
    // operand specifier. The below set of functions handle the different possible
    // ways an operand might need to be decoded.
    // calculateOpVal() is the entry point, and will dispatch to the correct helper method.
    void calculateOpVal() const;
    void calculateOpValStoreHelper(Enu::EAddrMode addrMode, quint16 opSpec) const;
    void calculateOpValByteHelper(Enu::EAddrMode addrMode, quint16 opSpec) const;
    void calculateopValWordHelper(Enu::EAddrMode addrMode, quint16 opSpec) const;
};

#endif // FULLMICROCODEMEMOIZER_H
