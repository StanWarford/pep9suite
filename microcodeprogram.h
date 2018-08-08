#ifndef MICROCODEPROGRAM_H
#define MICROCODEPROGRAM_H
#include "enu.h"
class MicroCodeBase;
class MicroCode;
class SymbolTable;
class MicrocodeProgram
{
private:
    SymbolTable* symTable;
    QVector<MicroCodeBase*> programVec;
    QVector<int> preconditionsVec,postconditionsVec,microcodeVec;
public:
    MicrocodeProgram();
    ~MicrocodeProgram();
    MicrocodeProgram(QVector<MicroCodeBase*>objectCode,SymbolTable* symbolTable);
    const SymbolTable* getSymTable() const;
    const QVector<MicroCodeBase*> getObjectCode() const;
    const QString format() const;
    int codeLineToProgramLine(int codeLine) const;
    bool hasMicrocode() const;
    bool hasUnitPre() const;
    const MicroCode* getCodeLine(quint16 codeLine) const;
    int codeLength() const;
};

#endif // MICROCODEPROGRAM_H
