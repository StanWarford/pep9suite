#include "pep9microcode.h"

#include "microassembler/specification.h"
#include "pep/pep.h"
#include "symbol/symbolentry.h"


MicroCode::MicroCode(Enu::CPUType cpuType, bool extendedFeatures): AExecutableMicrocode(false,QString(), nullptr),
    cpuType(cpuType), controlSignals(Pep::numControlSignals(), Enu::signalDisabled),
    clockSignals(Pep::numClockSignals(), false), extendedFeatures(extendedFeatures), branchFunc(Enu::Assembler_Assigned),
    trueTargetAddr(nullptr), falseTargetAddr(nullptr)
{
    // Initialize all memory controls, normal controls, and clocklines to disabled.
    for(auto memLines : Pep::memControlToMnemonMap.keys()) {
        controlSignals[memLines] = Enu::signalDisabled;
    }
    for(auto mainCtrlLines : Pep::decControlToMnemonMap.keys()) {
        controlSignals[mainCtrlLines] = Enu::signalDisabled;
    }
    for(auto clockLines : Pep::clockControlToMnemonMap.keys()) {
        clockSignals[clockLines] = 0;
    }
}

QString MicroCode::getObjectCode() const
{
    // QString QString::arg(int a, int fieldWidth = 0, ...)
    // fieldWidth specifies the minimum amount of space that
    //  a is padded to and filled with the character fillChar.
    // A positive value produces right-aligned text; a negative
    //  value produces left-aligned text.

    QString str = "";
    if (cpuType == Enu::OneByteDataBus) {
        str.append(clockSignals[Enu::LoadCk] == 0? "  " : QString("%1").arg(clockSignals[Enu::LoadCk], -2));
        str.append(controlSignals[Enu::C] == Enu::signalDisabled ? "   " : QString("%1").arg(controlSignals[Enu::C], -3));
        str.append(controlSignals[Enu::B] == Enu::signalDisabled ? "   " : QString("%1").arg(controlSignals[Enu::B], -3));
        str.append(controlSignals[Enu::A] == Enu::signalDisabled ? "   " : QString("%1").arg(controlSignals[Enu::A], -3));
        str.append(clockSignals[Enu::MARCk] == 0 ? "  " : QString("%1").arg(clockSignals[Enu::MARCk], -2));
        str.append(clockSignals[Enu::MDRCk] == 0 ? "  " : QString("%1").arg(clockSignals[Enu::MDRCk], -2));
        str.append(controlSignals[Enu::AMux] == Enu::signalDisabled ? "  " : QString("%1").arg(controlSignals[Enu::AMux], -2));
        str.append(controlSignals[Enu::MDRMux] == Enu::signalDisabled ? "  " : QString("%1").arg(controlSignals[Enu::MDRMux], -2));
        str.append(controlSignals[Enu::CMux] == Enu::signalDisabled ? "  " : QString("%1").arg(controlSignals[Enu::CMux], -2));
        str.append(controlSignals[Enu::ALU] == Enu::signalDisabled ? "   " : QString("%1").arg(controlSignals[Enu::ALU], -3));
        str.append(controlSignals[Enu::CSMux] == Enu::signalDisabled ? "  " : QString("%1").arg(controlSignals[Enu::CSMux], -2));
        str.append(clockSignals[Enu::SCk] == 0 ? "  " : QString("%1").arg(clockSignals[Enu::SCk], -2));
        str.append(clockSignals[Enu::CCk] == 0 ? "  " : QString("%1").arg(clockSignals[Enu::CCk], -2));
        str.append(clockSignals[Enu::VCk] == 0 ? "  " : QString("%1").arg(clockSignals[Enu::VCk], -2));
        str.append(controlSignals[Enu::AndZ] == Enu::signalDisabled ? "  " : QString("%1").arg(controlSignals[Enu::AndZ], -2));
        str.append(clockSignals[Enu::ZCk] == 0 ? "  " : QString("%1").arg(clockSignals[Enu::ZCk], -2));
        str.append(clockSignals[Enu::NCk] == 0 ? "  " : QString("%1").arg(clockSignals[Enu::NCk], -2));
        str.append(controlSignals[Enu::MemWrite] != 1 ? "  " : QString("%1").arg(controlSignals[Enu::MemWrite], -2));
        str.append(controlSignals[Enu::MemRead] != 1 ? "  " : QString("%1").arg(controlSignals[Enu::MemRead], -2));
    }
    else if (cpuType == Enu::TwoByteDataBus) {
        str.append(clockSignals[Enu::LoadCk] == 0 ? "  " : QString("%1").arg(clockSignals[Enu::LoadCk], -2));
        str.append(controlSignals[Enu::C] == Enu::signalDisabled ? "   " : QString("%1").arg(controlSignals[Enu::C], -3));
        str.append(controlSignals[Enu::B] == Enu::signalDisabled ? "   " : QString("%1").arg(controlSignals[Enu::B], -3));
        str.append(controlSignals[Enu::A] == Enu::signalDisabled ? "   " : QString("%1").arg(controlSignals[Enu::A], -3));
        str.append(controlSignals[Enu::MARMux] == Enu::signalDisabled ? "  " : QString("%1").arg(controlSignals[Enu::MARMux], -2));
        str.append(clockSignals[Enu::MARCk] == 0 ? "  " : QString("%1").arg(clockSignals[Enu::MARCk], -2));
        str.append(clockSignals[Enu::MDROCk] == 0 ? "  " : QString("%1").arg(clockSignals[Enu::MDROCk], -2));
        str.append(controlSignals[Enu::MDROMux] == Enu::signalDisabled ? "  " : QString("%1").arg(controlSignals[Enu::MDROMux], -2));
        str.append(clockSignals[Enu::MDRECk] == 0 ? "  " : QString("%1").arg(clockSignals[Enu::MDRECk], -2));
        str.append(controlSignals[Enu::MDREMux] == Enu::signalDisabled ? "  " : QString("%1").arg(controlSignals[Enu::MDREMux], -2));
        str.append(controlSignals[Enu::EOMux] == Enu::signalDisabled ? "  " : QString("%1").arg(controlSignals[Enu::EOMux], -2));
        str.append(controlSignals[Enu::AMux] == Enu::signalDisabled ? "  " : QString("%1").arg(controlSignals[Enu::AMux], -2));
        str.append(controlSignals[Enu::CMux] == Enu::signalDisabled ? "  " : QString("%1").arg(controlSignals[Enu::CMux], -2));
        str.append(controlSignals[Enu::ALU] == Enu::signalDisabled ? "   " : QString("%1").arg(controlSignals[Enu::ALU], -3));
        str.append(controlSignals[Enu::CSMux] == Enu::signalDisabled ? "  " : QString("%1").arg(controlSignals[Enu::CSMux], -2));
        str.append(clockSignals[Enu::SCk] == 0 ? "  " : QString("%1").arg(clockSignals[Enu::SCk], -2));
        str.append(clockSignals[Enu::CCk] == 0 ? "  " : QString("%1").arg(clockSignals[Enu::CCk], -2));
        str.append(clockSignals[Enu::VCk] == 0 ? "  " : QString("%1").arg(clockSignals[Enu::VCk], -2));
        str.append(controlSignals[Enu::AndZ] == Enu::signalDisabled ? "  " : QString("%1").arg(controlSignals[Enu::AndZ], -2));
        str.append(clockSignals[Enu::ZCk] == 0 ? "  " : QString("%1").arg(clockSignals[Enu::ZCk], -2));
        str.append(clockSignals[Enu::NCk] == 0 ? "  " : QString("%1").arg(clockSignals[Enu::NCk], -2));
        str.append(controlSignals[Enu::MemWrite] != 1 ? "  " : QString("%1").arg(controlSignals[Enu::MemWrite], -2));
        str.append(controlSignals[Enu::MemRead] != 1 ? "  " : QString("%1").arg(controlSignals[Enu::MemRead], -2));
    }
    str.append("\n");
    return str;
}

QString MicroCode::getSourceCode() const
{
    QString str = "",symbolString="";
    if(symbol != nullptr && !symbol->getName().startsWith(("_"))) {
        symbolString.append(symbol->getName()+": ");
    }

    if (cpuType == Enu::OneByteDataBus) {
        if (controlSignals[Enu::MemRead] != Enu::signalDisabled) { str.append("MemRead, "); }
        if (controlSignals[Enu::MemWrite] != Enu::signalDisabled) { str.append("MemWrite, "); }
        if (controlSignals[Enu::A] != Enu::signalDisabled) { str.append("A=" + QString("%1").arg(controlSignals[Enu::A]) + ", "); }
        if (controlSignals[Enu::B] != Enu::signalDisabled) { str.append("B=" + QString("%1").arg(controlSignals[Enu::B]) + ", "); }
        if (controlSignals[Enu::AMux] != Enu::signalDisabled) { str.append("AMux=" + QString("%1").arg(controlSignals[Enu::AMux]) + ", "); }
        if (controlSignals[Enu::CSMux]  != Enu::signalDisabled) { str.append("CSMux=" + QString("%1").arg(controlSignals[Enu::CSMux]) + ", "); }
        if (controlSignals[Enu::ALU] != Enu::signalDisabled) { str.append("ALU=" + QString("%1").arg(controlSignals[Enu::ALU]) + ", "); }
        if (controlSignals[Enu::AndZ] != Enu::signalDisabled) { str.append("AndZ=" + QString("%1").arg(controlSignals[Enu::AndZ]) + ", "); }
        if (controlSignals[Enu::CMux] != Enu::signalDisabled) { str.append("CMux=" + QString("%1").arg(controlSignals[Enu::CMux]) + ", "); }
        if (controlSignals[Enu::MDRMux] != Enu::signalDisabled) { str.append("MDRMux=" + QString("%1").arg(controlSignals[Enu::MDRMux]) + ", "); }
        if (controlSignals[Enu::C] != Enu::signalDisabled) { str.append("C=" + QString("%1").arg(controlSignals[Enu::C]) + ", "); }

        if (str != "") { str.chop(2); str.append("; "); }

        if (clockSignals[Enu::NCk] != 0) { str.append("NCk, "); }
        if (clockSignals[Enu::ZCk] != 0) { str.append("ZCk, "); }
        if (clockSignals[Enu::VCk] != 0) { str.append("VCk, "); }
        if (clockSignals[Enu::CCk] != 0) { str.append("CCk, "); }
        if (clockSignals[Enu::SCk] != 0) { str.append("SCk, "); }
        if (clockSignals[Enu::MARCk] != 0) { str.append("MARCk, "); }
        if (clockSignals[Enu::LoadCk] != 0) { str.append("LoadCk, "); }
        if (clockSignals[Enu::MDRCk] != 0) { str.append("MDRCk, "); }

        if (str.endsWith(", ") || str.endsWith("; ")) { str.chop(2); }
    }

    else if (cpuType == Enu::TwoByteDataBus) {
        if (controlSignals[Enu::MemRead] != Enu::signalDisabled) { str.append("MemRead, "); }
        if (controlSignals[Enu::MemWrite] != Enu::signalDisabled) { str.append("MemWrite, "); }
        if (controlSignals[Enu::A] != Enu::signalDisabled) { str.append("A=" + QString("%1").arg(controlSignals[Enu::A]) + ", "); }
        if (controlSignals[Enu::B] != Enu::signalDisabled) { str.append("B=" + QString("%1").arg(controlSignals[Enu::B]) + ", "); }
        if (controlSignals[Enu::MARMux] != Enu::signalDisabled) { str.append("MARMux=" + QString("%1").arg(controlSignals[Enu::MARMux]) + ", "); }
        if (controlSignals[Enu::EOMux] != Enu::signalDisabled) { str.append("EOMux=" + QString("%1").arg(controlSignals[Enu::EOMux]) + ", "); }
        if (controlSignals[Enu::AMux] != Enu::signalDisabled) { str.append("AMux=" + QString("%1").arg(controlSignals[Enu::AMux]) + ", "); }
        if (controlSignals[Enu::CSMux]  != Enu::signalDisabled) { str.append("CSMux=" + QString("%1").arg(controlSignals[Enu::CSMux]) + ", "); }
        if (controlSignals[Enu::ALU] != Enu::signalDisabled) { str.append("ALU=" + QString("%1").arg(controlSignals[Enu::ALU]) + ", "); }
        if (controlSignals[Enu::AndZ] != Enu::signalDisabled) { str.append("AndZ=" + QString("%1").arg(controlSignals[Enu::AndZ]) + ", "); }
        if (controlSignals[Enu::CMux] != Enu::signalDisabled) { str.append("CMux=" + QString("%1").arg(controlSignals[Enu::CMux]) + ", "); }
        if (controlSignals[Enu::MDREMux] != Enu::signalDisabled) { str.append("MDREMux=" + QString("%1").arg(controlSignals[Enu::MDREMux]) + ", "); }
        if (controlSignals[Enu::MDROMux] != Enu::signalDisabled) { str.append("MDROMux=" + QString("%1").arg(controlSignals[Enu::MDROMux]) + ", "); }
        if (controlSignals[Enu::C] != Enu::signalDisabled) { str.append("C=" + QString("%1").arg(controlSignals[Enu::C]) + ", "); }
        if (controlSignals[Enu::PValid] != Enu::signalDisabled) { str.append("PValid=" + QString("%1").arg(controlSignals[Enu::PValid]) + ", "); }

        if (str != "") { str.chop(2); str.append("; "); }

        if (clockSignals[Enu::NCk] != 0) { str.append("NCk, "); }
        if (clockSignals[Enu::ZCk] != 0) { str.append("ZCk, "); }
        if (clockSignals[Enu::VCk] != 0) { str.append("VCk, "); }
        if (clockSignals[Enu::CCk] != 0) { str.append("CCk, "); }
        if (clockSignals[Enu::SCk] != 0) { str.append("SCk, "); }
        if (clockSignals[Enu::MARCk] != 0) { str.append("MARCk, "); }
        if (clockSignals[Enu::LoadCk] != 0) { str.append("LoadCk, "); }
        if (clockSignals[Enu::MDRECk] != 0) { str.append("MDRECk, "); }
        if (clockSignals[Enu::MDROCk] != 0) { str.append("MDROCk, "); }
        if (extendedFeatures && clockSignals[Enu::PValidCk] != 0) { str.append("PValidCk, "); }

        if (str.endsWith(", ") || str.endsWith("; ")) { str.chop(2); }
    }

    if(!extendedFeatures){
        // Do not append a branch function to "basic" microcode lines of Pep9CPU.
    }
    else if (branchFunc == Enu::Unconditional) {
        // If the true target of an unconditional branch is assembler assigned
        // (e.g. _as212), then there is no need to write the implicit goto.
        // The only way to have an assembler assigned symbol is if the branch function
        // at some point was Enu::Assembler_Assigned, which means the symbol's
        // name is meaningless to the user.
        // However, if it is a meaningful symbol (e.g. trap), it should be retained,
        // even if it the next logical line of microcode.
        if(trueTargetAddr->getName().startsWith("_")) {
            // A jump to the next line is assumed by the microassembler, so nothing further must be done
        }
        else {
            if(str.isEmpty()) {
                str.append("goto " + trueTargetAddr->getName());
            }
            else {
               str.append("; goto " + trueTargetAddr->getName());
            }
        }
    }
    else if(branchFunc == Enu::Assembler_Assigned) {
        // Assume that a jump to the next line will be used unless otherwise specified.
    }
    else if (branchFunc == Enu::Stop) {
        if(str.isEmpty()){
            str.append("stopCPU");
        }
        else{
           str.append("; stopCPU");
        }
    }
    else if (branchFunc == Enu::InstructionSpecifierDecoder ||
             branchFunc == Enu::AddressingModeDecoder) {
        if(str.isEmpty()){
            str.append(Pep::branchFuncToMnemonMap[branchFunc]);
        }
        else  {
            str.append("; "+Pep::branchFuncToMnemonMap[branchFunc]);
        }
    }
    else {
        if(str.isEmpty()) {
            str.append("if " + Pep::branchFuncToMnemonMap[branchFunc] + " " +
                       trueTargetAddr->getName() + " else " + falseTargetAddr->getName());
        }
        else {
            str.append("; if " + Pep::branchFuncToMnemonMap[branchFunc] + " "+
                       trueTargetAddr->getName() + " else "+falseTargetAddr->getName());
        }

    }

    if (!cComment.isEmpty()) {
        str.append(" " + cComment);
    }

    symbolString.append(str);
    return symbolString;
}

bool MicroCode::hasControlSignal(Enu::EControlSignals field) const
{
    return controlSignals[field] != Enu::signalDisabled;
}

bool MicroCode::hasClockSignal(Enu::EClockSignals field) const
{
    return clockSignals[field] == true;
}

void MicroCode::setControlSignal(Enu::EControlSignals field, quint8 value)
{
    controlSignals[field] = value;
}

void MicroCode::setClockSingal(Enu::EClockSignals field, bool value)
{
    clockSignals[field] = value;
}

void MicroCode::setBranchFunction(Enu::EBranchFunctions branch)
{
    branchFunc = branch;
}

void MicroCode::setTrueTarget(const SymbolEntry* target)
{
    trueTargetAddr = target;
}

void MicroCode::setFalseTarget(const SymbolEntry* target)
{
    falseTargetAddr = target;
}

quint8 MicroCode::getControlSignal(Enu::EControlSignals field) const
{
    return controlSignals[field];
}

const QVector<quint8> MicroCode::getControlSignals() const
{
    return controlSignals;
}

bool MicroCode::getClockSignal(Enu::EClockSignals field) const
{
    return clockSignals[field];
}

const QVector<bool> MicroCode::getClockSignals() const
{
    return clockSignals;
}

Enu::EBranchFunctions MicroCode::getBranchFunction() const
{
    return branchFunc;
}

const SymbolEntry* MicroCode::getTrueTarget() const
{
    return trueTargetAddr;
}

const SymbolEntry* MicroCode::getFalseTarget() const
{
    return falseTargetAddr;
}

bool MicroCode::inRange(Enu::EControlSignals field, int value) const
{
    switch (field) {
    case Enu::C: return 0 <= value && value <= 31;
    case Enu::B: return 0 <= value && value <= 31;
    case Enu::A: return 0 <= value && value <= 31;
    case Enu::AMux: return 0 <= value && value <= 1;
    case Enu::MDRMux: return 0 <= value && value <= 1;
    case Enu::CMux: return 0 <= value && value <= 1;
    case Enu::ALU: return 0 <= value && value <= 15;
    case Enu::CSMux: return 0 <= value && value <= 1;
    case Enu::AndZ: return 0 <= value && value <= 1;
    case Enu::MARMux: return 0 <= value && value <= 1;
    case Enu::MDROMux: return 0 <= value && value <= 1;
    case Enu::MDREMux: return 0 <= value && value <= 1;
    case Enu::EOMux: return 0 <= value && value <= 1;
    default: return true;
    }
}

UnitPreCode::~UnitPreCode()
{
    while (!unitPreList.isEmpty()) {
        delete unitPreList.takeFirst();
    }
}

QString UnitPreCode::getSourceCode() const
{
    QString str = "UnitPre: ";
    for (int i = 0; i < unitPreList.size(); i++) {
        str.append(unitPreList.at(i)->getSourceCode() + ", ");
    }
    if (str.endsWith(", ")) {
        str.chop(2);
    }
    if (!cComment.isEmpty()) {
        str.append(" " + cComment);
    }
    return str;
}

bool UnitPreCode::hasUnitPre() const
{
    return !unitPreList.isEmpty();
}

void UnitPreCode::setUnitPre(CPUDataSection *data, AMemoryDevice* memoryDevice)
{
    for(auto unitTest : unitPreList) {
        unitTest->setUnitPre(data, memoryDevice);
    }
}

void UnitPreCode::appendSpecification(Specification *specification)
{
    unitPreList.append(specification);
}

void UnitPreCode::setComment(QString comment)
{
    cComment = comment;
}

UnitPostCode::~UnitPostCode()
{
    while (!unitPostList.isEmpty()) {
        delete unitPostList.takeFirst();
    }
}

QString UnitPostCode::getSourceCode() const
{
    QString str = "UnitPost: ";
    for (int i = 0; i < unitPostList.size(); i++) {
        str.append(unitPostList.at(i)->getSourceCode() + ", ");
    }
    if (str.endsWith(", ")) {
        str.chop(2);
    }
    if (!cComment.isEmpty()) {
        str.append(" " + cComment);
    }
    return str;
}

bool UnitPostCode::testPostcondition(CPUDataSection *data,
                                     AMemoryDevice* memoryDevice,
                                     QString &err)
{
    bool val = true;;
    for(auto unitTest : unitPostList){
        val &= unitTest->testUnitPost(data, memoryDevice, err);
        if(!val) break;
    }
    return val;
}

void UnitPostCode::appendSpecification(Specification *specification)
{
    unitPostList.append(specification);
}

void UnitPostCode::setComment(QString comment)
{
    cComment = comment;
}

bool UnitPostCode::hasUnitPost() const
{
    return true;
}
