#ifndef MICROCODEPROGRAM_H
#define MICROCODEPROGRAM_H
#include "enu.h"
#include <QVector>
class AMicroCode;
class MicroCode;
class SymbolTable;
class MicrocodeProgram
{
private:
    QSharedPointer<SymbolTable> symTable;
    QVector<AMicroCode*> programVec;
    QVector<int> preconditionsVec,postconditionsVec,microcodeVec;
public:
    MicrocodeProgram();
    ~MicrocodeProgram();
    MicrocodeProgram(QVector<AMicroCode*>objectCode, QSharedPointer<SymbolTable> symbolTable);
    QSharedPointer<const SymbolTable> getSymTable() const;
    const QVector<AMicroCode*> getObjectCode() const;
    const QString format() const;
    int codeLineToProgramLine(int codeLine) const;
    bool hasMicrocode() const;
    bool hasUnitPre() const;
    const MicroCode* getCodeLine(quint16 codeLine) const;
    MicroCode* getCodeLine(quint16 codeLine);
    int codeLength() const;
};

#endif // MICROCODEPROGRAM_H
