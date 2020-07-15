#ifndef PEP9MICROCODE_H
#define PEP9MICROCODE_H

#include <QMap>
#include <QString>
#include <QVector>

#include "microassembler/microcode.h"
#include "pep/enu.h"

class SymbolEntry;
class Specification;
class CPUDataSection;
class AMemoryDevice;

class MicroCode: public AExecutableMicrocode
{
public:
    MicroCode(Enu::CPUType cpuType, bool useExtendedFatures);

    QString getObjectCode() const override;
    QString getSourceCode() const override;

    bool hasControlSignal(Enu::EControlSignals field) const;
    bool hasClockSignal(Enu::EClockSignals field) const;

    quint8 getControlSignal(Enu::EControlSignals field) const;
    const QVector<quint8> getControlSignals() const;
    bool getClockSignal(Enu::EClockSignals field) const;
    const QVector<bool> getClockSignals() const;


    Enu::EBranchFunctions getBranchFunction() const;

    const SymbolEntry* getTrueTarget() const;
    const SymbolEntry* getFalseTarget() const;

    bool inRange(Enu::EControlSignals field, int value) const;
    void setControlSignal(Enu::EControlSignals field, quint8 value);
    void setClockSingal(Enu::EClockSignals field,bool value);

    void setBranchFunction(Enu::EBranchFunctions branch);
    void setTrueTarget(const SymbolEntry* target);
    void setFalseTarget(const SymbolEntry* target);

private:
    Enu::CPUType cpuType;
    QVector<quint8> controlSignals;
    QVector<bool> clockSignals;
    bool extendedFeatures;
    Enu::EBranchFunctions branchFunc = Enu::Unconditional;
    const SymbolEntry* trueTargetAddr;
    const SymbolEntry* falseTargetAddr;
};

class UnitPreCode: public AMicroCode
{
public:
    ~UnitPreCode() override;
    QString getSourceCode() const override;
    bool hasUnitPre() const override;
    void setUnitPre(CPUDataSection* data, AMemoryDevice* memoryDevice);
    void appendSpecification(Specification *specification);
    void setComment(QString comment);
private:
    QList<Specification *> unitPreList;
    QString cComment;
};

class UnitPostCode: public AMicroCode
{
public:
    ~UnitPostCode() override;
    QString getSourceCode() const override;
    bool testPostcondition(CPUDataSection *data, AMemoryDevice* memoryDevice, QString &err);
    void appendSpecification(Specification *specification);
    void setComment(QString comment);
    bool hasUnitPost() const override;
private:
    QList<Specification *> unitPostList;
    QString cComment;
};

#endif // PEP9MICROCODE_H
