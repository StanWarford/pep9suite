#ifndef PEP9MICROCODE_H
#define PEP9MICROCODE_H

#include <QMap>
#include <QString>
#include <QVector>

#include "microassembler/microcode.h"
#include "pep/constants.h"
#include "pep/enu.h"

#include "cpudefs.h"

class SymbolEntry;
class Specification;
class CPUDataSection;
class AMemoryDevice;

class MicroCode: public AExecutableMicrocode
{
public:
    MicroCode(PepCore::CPUType cpuType, bool useExtendedFatures);

    QString getObjectCode() const override;
    QString getSourceCode() const override;

    bool hasControlSignal(Pep9::uarch::EControlSignals field) const;
    bool hasClockSignal(Pep9::uarch::EClockSignals field) const;

    quint8 getControlSignal(Pep9::uarch::EControlSignals field) const;
    const QVector<quint8> getControlSignals() const;
    bool getClockSignal(Pep9::uarch::EClockSignals field) const;
    const QVector<bool> getClockSignals() const;


    Pep9::uarch::EBranchFunctions getBranchFunction() const;

    const SymbolEntry* getTrueTarget() const;
    const SymbolEntry* getFalseTarget() const;

    bool inRange(Pep9::uarch::EControlSignals field, int value) const;
    void setControlSignal(Pep9::uarch::EControlSignals field, quint8 value);
    void setClockSingal(Pep9::uarch::EClockSignals field,bool value);

    void setBranchFunction(Pep9::uarch::EBranchFunctions branch);
    void setTrueTarget(const SymbolEntry* target);
    void setFalseTarget(const SymbolEntry* target);

private:
    PepCore::CPUType cpuType;
    QVector<quint8> controlSignals;
    QVector<bool> clockSignals;
    bool extendedFeatures;
    Pep9::uarch::EBranchFunctions branchFunc = Pep9::uarch::EBranchFunctions::Unconditional;
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
