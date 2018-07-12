#ifndef MICROCODEPROGRAM_H
#define MICROCODEPROGRAM_H
#include "enu.h"
class AsmCode;
class MicroCode;
class SymbolTable;
class MicrocodeProgram
{
private:
    SymbolTable* symTable;
    QVector<AsmCode*> programVec;
    QVector<int> preconditionsVec,postconditionsVec,microcodeVec;
public:
    MicrocodeProgram();
    ~MicrocodeProgram();
    MicrocodeProgram(QVector<AsmCode*>objectCode,SymbolTable* symbolTable);
    const SymbolTable* getSymTable() const;
    const QVector<AsmCode*> getObjectCode() const;
    const QString format() const;
    int codeLineToProgramLine(int codeLine) const;
    bool hasMicrocode() const;
    bool hasUnitPre() const;
    const MicroCode* getCodeLine(quint16 codeLine) const;
    int codeLength() const;
};

#endif // MICROCODEPROGRAM_H
