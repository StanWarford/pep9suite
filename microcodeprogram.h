#ifndef MICROCODEPROGRAM_H
#define MICROCODEPROGRAM_H
#include "enu.h"
class Code;
class MicroCode;
class MicrocodeProgram
{
private:
    QVector<Code*> programVec;
    QVector<int> preconditionsVec,postconditionsVec,microcodeVec;
public:
    MicrocodeProgram();
    ~MicrocodeProgram();
    MicrocodeProgram(QVector<Code*>objectCode);
    const QVector<Code*> getObjectCode() const;
    int codeLineToProgramLine(int codeLine) const;
    bool hasMicrocode() const;
    bool hasUnitPre() const;
    const MicroCode* getCodeLine(quint16 codeLine) const;
    int codeLength() const;
};

#endif // MICROCODEPROGRAM_H
