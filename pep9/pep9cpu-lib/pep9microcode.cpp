#include "pep9microcode.h"

#include "microassembler/specification.h"
#include "pep/pep.h"
#include "symbol/symbolentry.h"

#include "pep9.h"


MicroCode::MicroCode(PepCore::CPUType cpuType, bool extendedFeatures): AExecutableMicrocode(false,QString(), nullptr),
    cpuType(cpuType), controlSignals(Pep9::uarch::numControlSignals(), Enu::signalDisabled),
    clockSignals(Pep9::uarch::numClockSignals(), false), extendedFeatures(extendedFeatures), branchFunc(Pep9::uarch::EBranchFunctions::Assembler_Assigned),
    trueTargetAddr(nullptr), falseTargetAddr(nullptr)
{
    // Initialize all memory controls, normal controls, and clocklines to disabled.
    for(auto memLines : Pep9::uarch::memControlToMnemonMap.keys()) {
        controlSignals[to_uint8_t(memLines)] = Enu::signalDisabled;
    }
    for(auto mainCtrlLines : Pep9::uarch::decControlToMnemonMap.keys()) {
        controlSignals[to_uint8_t(mainCtrlLines)] = Enu::signalDisabled;
    }
    for(auto clockLines : Pep9::uarch::clockControlToMnemonMap.keys()) {
        clockSignals[to_uint8_t(clockLines)] = 0;
    }
}
const auto LoadCk_t = to_uint8_t(Pep9::uarch::EClockSignals::LoadCk);
const auto C_t = to_uint8_t(Pep9::uarch::EControlSignals::C);
const auto B_t = to_uint8_t(Pep9::uarch::EControlSignals::B);
const auto A_t = to_uint8_t(Pep9::uarch::EControlSignals::A);
const auto MARCk_t = to_uint8_t(Pep9::uarch::EClockSignals::MARCk);
const auto MDRCk_t = to_uint8_t(Pep9::uarch::EClockSignals::MDRCk);
const auto AMux_t = to_uint8_t(Pep9::uarch::EControlSignals::AMux);
const auto MDRMux_t = to_uint8_t(Pep9::uarch::EControlSignals::MDRMux);
const auto CMux_t = to_uint8_t(Pep9::uarch::EControlSignals::CMux);
const auto ALU_t = to_uint8_t(Pep9::uarch::EControlSignals::ALU);
const auto CSMux_t = to_uint8_t(Pep9::uarch::EControlSignals::CSMux);
const auto SCk_t = to_uint8_t(Pep9::uarch::EClockSignals::SCk);
const auto CCk_t = to_uint8_t(Pep9::uarch::EClockSignals::CCk);
const auto VCk_t = to_uint8_t(Pep9::uarch::EClockSignals::VCk);
const auto AndZ_t = to_uint8_t(Pep9::uarch::EControlSignals::AndZ);
const auto ZCk_t = to_uint8_t(Pep9::uarch::EClockSignals::ZCk);
const auto NCk_t = to_uint8_t(Pep9::uarch::EClockSignals::NCk);
const auto MemWrite_t = to_uint8_t(Pep9::uarch::EControlSignals::MemWrite);
const auto MemRead_t = to_uint8_t(Pep9::uarch::EControlSignals::MemRead);

const auto MARMux_t = to_uint8_t(Pep9::uarch::EControlSignals::MARMux);
const auto MDROCk_t = to_uint8_t(Pep9::uarch::EClockSignals::MDROCk);
const auto MDROMux_t = to_uint8_t(Pep9::uarch::EControlSignals::MDROMux);
const auto MDRECk_t = to_uint8_t(Pep9::uarch::EClockSignals::MDRECk);
const auto MDREMux_t = to_uint8_t(Pep9::uarch::EControlSignals::MDREMux);
const auto EOMux_t = to_uint8_t(Pep9::uarch::EControlSignals::EOMux);

const auto PValidCk_t = to_uint8_t(Pep9::uarch::EClockSignals::PValidCk);
const auto PValid_t = to_uint8_t(Pep9::uarch::EControlSignals::PValid);
QString MicroCode::getObjectCode() const
{
    using namespace Pep9::uarch;
    // QString QString::arg(int a, int fieldWidth = 0, ...)
    // fieldWidth specifies the minimum amount of space that
    //  a is padded to and filled with the character fillChar.
    // A positive value produces right-aligned text; a negative
    //  value produces left-aligned text.

    QString str = "";
    if (cpuType == PepCore::CPUType::OneByteDataBus) {
        str.append(clockSignals[LoadCk_t] == 0? "  " : QString("%1").arg(clockSignals[LoadCk_t], -2));
        str.append(controlSignals[C_t] == Enu::signalDisabled ? "   " : QString("%1").arg(controlSignals[C_t], -3));
        str.append(controlSignals[B_t] == Enu::signalDisabled ? "   " : QString("%1").arg(controlSignals[B_t], -3));
        str.append(controlSignals[A_t] == Enu::signalDisabled ? "   " : QString("%1").arg(controlSignals[A_t], -3));
        str.append(clockSignals[MARCk_t] == 0 ? "  " : QString("%1").arg(clockSignals[MARCk_t], -2));
        str.append(clockSignals[MDRCk_t] == 0 ? "  " : QString("%1").arg(clockSignals[MDRCk_t], -2));
        str.append(controlSignals[AMux_t] == Enu::signalDisabled ? "  " : QString("%1").arg(controlSignals[AMux_t], -2));
        str.append(controlSignals[MDRMux_t] == Enu::signalDisabled ? "  " : QString("%1").arg(controlSignals[MDRMux_t], -2));
        str.append(controlSignals[CMux_t] == Enu::signalDisabled ? "  " : QString("%1").arg(controlSignals[CMux_t], -2));
        str.append(controlSignals[ALU_t] == Enu::signalDisabled ? "   " : QString("%1").arg(controlSignals[ALU_t], -3));
        str.append(controlSignals[CSMux_t] == Enu::signalDisabled ? "  " : QString("%1").arg(controlSignals[CSMux_t], -2));
        str.append(clockSignals[SCk_t] == 0 ? "  " : QString("%1").arg(clockSignals[SCk_t], -2));
        str.append(clockSignals[CCk_t] == 0 ? "  " : QString("%1").arg(clockSignals[CCk_t], -2));
        str.append(clockSignals[VCk_t] == 0 ? "  " : QString("%1").arg(clockSignals[VCk_t], -2));
        str.append(controlSignals[AndZ_t] == Enu::signalDisabled ? "  " : QString("%1").arg(controlSignals[AndZ_t], -2));
        str.append(clockSignals[ZCk_t] == 0 ? "  " : QString("%1").arg(clockSignals[ZCk_t], -2));
        str.append(clockSignals[NCk_t] == 0 ? "  " : QString("%1").arg(clockSignals[NCk_t], -2));
        str.append(controlSignals[MemWrite_t] != 1 ? "  " : QString("%1").arg(controlSignals[MemWrite_t], -2));
        str.append(controlSignals[MemRead_t] != 1 ? "  " : QString("%1").arg(controlSignals[MemRead_t], -2));
    }
    else if (cpuType == PepCore::CPUType::TwoByteDataBus) {
        str.append(clockSignals[LoadCk_t] == 0? "  " : QString("%1").arg(clockSignals[LoadCk_t], -2));
        str.append(controlSignals[C_t] == Enu::signalDisabled ? "   " : QString("%1").arg(controlSignals[C_t], -3));
        str.append(controlSignals[B_t] == Enu::signalDisabled ? "   " : QString("%1").arg(controlSignals[B_t], -3));
        str.append(controlSignals[A_t] == Enu::signalDisabled ? "   " : QString("%1").arg(controlSignals[A_t], -3));
        str.append(controlSignals[MARMux_t] == Enu::signalDisabled ? "  " : QString("%1").arg(controlSignals[MARMux_t], -2));
        str.append(clockSignals[MARCk_t] == 0 ? "  " : QString("%1").arg(clockSignals[MARCk_t], -2));
        str.append(clockSignals[MDROCk_t] == 0 ? "  " : QString("%1").arg(clockSignals[MDROCk_t], -2));
        str.append(controlSignals[MDROMux_t] == Enu::signalDisabled ? "  " : QString("%1").arg(controlSignals[MDROMux_t], -2));
        str.append(clockSignals[MDRECk_t] == 0 ? "  " : QString("%1").arg(clockSignals[MDRECk_t], -2));
        str.append(controlSignals[MDREMux_t] == Enu::signalDisabled ? "  " : QString("%1").arg(controlSignals[MDREMux_t], -2));
        str.append(controlSignals[EOMux_t] == Enu::signalDisabled ? "  " : QString("%1").arg(controlSignals[EOMux_t], -2));
        str.append(controlSignals[AMux_t] == Enu::signalDisabled ? "  " : QString("%1").arg(controlSignals[AMux_t], -2));
        str.append(controlSignals[CMux_t] == Enu::signalDisabled ? "  " : QString("%1").arg(controlSignals[CMux_t], -2));
        str.append(controlSignals[ALU_t] == Enu::signalDisabled ? "   " : QString("%1").arg(controlSignals[ALU_t], -3));
        str.append(controlSignals[CSMux_t] == Enu::signalDisabled ? "  " : QString("%1").arg(controlSignals[CSMux_t], -2));
        str.append(clockSignals[SCk_t] == 0 ? "  " : QString("%1").arg(clockSignals[SCk_t], -2));
        str.append(clockSignals[CCk_t] == 0 ? "  " : QString("%1").arg(clockSignals[CCk_t], -2));
        str.append(clockSignals[VCk_t] == 0 ? "  " : QString("%1").arg(clockSignals[VCk_t], -2));
        str.append(controlSignals[AndZ_t] == Enu::signalDisabled ? "  " : QString("%1").arg(controlSignals[AndZ_t], -2));
        str.append(clockSignals[ZCk_t] == 0 ? "  " : QString("%1").arg(clockSignals[ZCk_t], -2));
        str.append(clockSignals[NCk_t] == 0 ? "  " : QString("%1").arg(clockSignals[NCk_t], -2));
        str.append(controlSignals[MemWrite_t] != 1 ? "  " : QString("%1").arg(controlSignals[MemWrite_t], -2));
        str.append(controlSignals[MemRead_t] != 1 ? "  " : QString("%1").arg(controlSignals[MemRead_t], -2));
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

    if (cpuType == PepCore::CPUType::OneByteDataBus) {
        if (controlSignals[MemRead_t] != Enu::signalDisabled) { str.append("MemRead, "); }
        if (controlSignals[MemWrite_t] != Enu::signalDisabled) { str.append("MemWrite, "); }
        if (controlSignals[A_t] != Enu::signalDisabled) { str.append("A=" + QString("%1").arg(controlSignals[A_t]) + ", "); }
        if (controlSignals[B_t] != Enu::signalDisabled) { str.append("B=" + QString("%1").arg(controlSignals[B_t]) + ", "); }
        if (controlSignals[AMux_t] != Enu::signalDisabled) { str.append("AMux=" + QString("%1").arg(controlSignals[AMux_t]) + ", "); }
        if (controlSignals[CSMux_t]  != Enu::signalDisabled) { str.append("CSMux=" + QString("%1").arg(controlSignals[CSMux_t]) + ", "); }
        if (controlSignals[ALU_t] != Enu::signalDisabled) { str.append("ALU=" + QString("%1").arg(controlSignals[ALU_t]) + ", "); }
        if (controlSignals[AndZ_t] != Enu::signalDisabled) { str.append("AndZ=" + QString("%1").arg(controlSignals[AndZ_t]) + ", "); }
        if (controlSignals[CMux_t] != Enu::signalDisabled) { str.append("CMux=" + QString("%1").arg(controlSignals[CMux_t]) + ", "); }
        if (controlSignals[MDRMux_t] != Enu::signalDisabled) { str.append("MDRMux=" + QString("%1").arg(controlSignals[MDRMux_t]) + ", "); }
        if (controlSignals[C_t] != Enu::signalDisabled) { str.append("C=" + QString("%1").arg(controlSignals[C_t]) + ", "); }

        if (str != "") { str.chop(2); str.append("; "); }

        if (clockSignals[NCk_t] != 0) { str.append("NCk, "); }
        if (clockSignals[ZCk_t] != 0) { str.append("ZCk, "); }
        if (clockSignals[VCk_t] != 0) { str.append("VCk, "); }
        if (clockSignals[CCk_t] != 0) { str.append("CCk, "); }
        if (clockSignals[SCk_t] != 0) { str.append("SCk, "); }
        if (clockSignals[MARCk_t] != 0) { str.append("MARCk, "); }
        if (clockSignals[LoadCk_t] != 0) { str.append("LoadCk, "); }
        if (clockSignals[MDRCk_t] != 0) { str.append("MDRCk, "); }

        if (str.endsWith(", ") || str.endsWith("; ")) { str.chop(2); }
    }

    else if (cpuType == PepCore::CPUType::TwoByteDataBus) {
        if (controlSignals[MemRead_t] != Enu::signalDisabled) { str.append("MemRead, "); }
        if (controlSignals[MemWrite_t] != Enu::signalDisabled) { str.append("MemWrite, "); }
        if (controlSignals[A_t] != Enu::signalDisabled) { str.append("A=" + QString("%1").arg(controlSignals[A_t]) + ", "); }
        if (controlSignals[B_t] != Enu::signalDisabled) { str.append("B=" + QString("%1").arg(controlSignals[B_t]) + ", "); }
        if (controlSignals[MARMux_t] != Enu::signalDisabled) { str.append("MARMux=" + QString("%1").arg(controlSignals[MARMux_t]) + ", "); }
        if (controlSignals[EOMux_t] != Enu::signalDisabled) { str.append("EOMux=" + QString("%1").arg(controlSignals[EOMux_t]) + ", "); }
        if (controlSignals[AMux_t] != Enu::signalDisabled) { str.append("AMux=" + QString("%1").arg(controlSignals[AMux_t]) + ", "); }
        if (controlSignals[CSMux_t]  != Enu::signalDisabled) { str.append("CSMux=" + QString("%1").arg(controlSignals[CSMux_t]) + ", "); }
        if (controlSignals[ALU_t] != Enu::signalDisabled) { str.append("ALU=" + QString("%1").arg(controlSignals[ALU_t]) + ", "); }
        if (controlSignals[AndZ_t] != Enu::signalDisabled) { str.append("AndZ=" + QString("%1").arg(controlSignals[AndZ_t]) + ", "); }
        if (controlSignals[CMux_t] != Enu::signalDisabled) { str.append("CMux=" + QString("%1").arg(controlSignals[CMux_t]) + ", "); }
        if (controlSignals[MDREMux_t] != Enu::signalDisabled) { str.append("MDREMux=" + QString("%1").arg(controlSignals[MDREMux_t]) + ", "); }
        if (controlSignals[MDROMux_t] != Enu::signalDisabled) { str.append("MDROMux=" + QString("%1").arg(controlSignals[MDROMux_t]) + ", "); }
        if (controlSignals[C_t] != Enu::signalDisabled) { str.append("C=" + QString("%1").arg(controlSignals[C_t]) + ", "); }
        if (extendedFeatures && controlSignals[PValid_t] != Enu::signalDisabled) { str.append("PValid=" + QString("%1").arg(controlSignals[PValid_t]) + ", "); }

        if (str != "") { str.chop(2); str.append("; "); }

        if (clockSignals[NCk_t] != 0) { str.append("NCk, "); }
        if (clockSignals[ZCk_t] != 0) { str.append("ZCk, "); }
        if (clockSignals[VCk_t] != 0) { str.append("VCk, "); }
        if (clockSignals[CCk_t] != 0) { str.append("CCk, "); }
        if (clockSignals[SCk_t] != 0) { str.append("SCk, "); }
        if (clockSignals[MARCk_t] != 0) { str.append("MARCk, "); }
        if (clockSignals[LoadCk_t] != 0) { str.append("LoadCk, "); }
        if (clockSignals[MDRECk_t] != 0) { str.append("MDRECk, "); }
        if (clockSignals[MDROCk_t] != 0) { str.append("MDROCk, "); }
        if (extendedFeatures && clockSignals[PValidCk_t] != 0) { str.append("PValidCk, "); }

        if (str.endsWith(", ") || str.endsWith("; ")) { str.chop(2); }
    }

    if(!extendedFeatures){
        // Do not append a branch function to "basic" microcode lines of Pep9CPU.
    }
    else if (branchFunc == Pep9::uarch::EBranchFunctions::Unconditional) {
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
    else if(branchFunc == Pep9::uarch::EBranchFunctions::Assembler_Assigned) {
        // Assume that a jump to the next line will be used unless otherwise specified.
    }
    else if (branchFunc == Pep9::uarch::EBranchFunctions::Stop) {
        if(str.isEmpty()){
            str.append("stopCPU");
        }
        else{
           str.append("; stopCPU");
        }
    }
    else if (branchFunc == Pep9::uarch::EBranchFunctions::InstructionSpecifierDecoder ||
             branchFunc == Pep9::uarch::EBranchFunctions::AddressingModeDecoder) {
        if(str.isEmpty()){
            str.append(Pep9::uarch::branchFuncToMnemonMap[branchFunc]);
        }
        else  {
            str.append("; "+Pep9::uarch::branchFuncToMnemonMap[branchFunc]);
        }
    }
    else {
        if(str.isEmpty()) {
            str.append("if " + Pep9::uarch::branchFuncToMnemonMap[branchFunc] + " " +
                       trueTargetAddr->getName() + " else " + falseTargetAddr->getName());
        }
        else {
            str.append("; if " + Pep9::uarch::branchFuncToMnemonMap[branchFunc] + " "+
                       trueTargetAddr->getName() + " else "+falseTargetAddr->getName());
        }

    }

    if (!cComment.isEmpty()) {
        str.append(" " + cComment);
    }

    symbolString.append(str);
    return symbolString;
}

bool MicroCode::hasControlSignal(Pep9::uarch::EControlSignals field) const
{
    return controlSignals[to_uint8_t(field)] != Enu::signalDisabled;
}

bool MicroCode::hasClockSignal(Pep9::uarch::EClockSignals field) const
{
    return clockSignals[to_uint8_t(field)] == true;
}

void MicroCode::setControlSignal(Pep9::uarch::EControlSignals field, quint8 value)
{
    controlSignals[to_uint8_t(field)] = value;
}

void MicroCode::setClockSingal(Pep9::uarch::EClockSignals field, bool value)
{
    clockSignals[to_uint8_t(field)] = value;
}

void MicroCode::setBranchFunction(Pep9::uarch::EBranchFunctions branch)
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

quint8 MicroCode::getControlSignal(Pep9::uarch::EControlSignals field) const
{
    return controlSignals[to_uint8_t(field)];
}

const QVector<quint8> MicroCode::getControlSignals() const
{
    return controlSignals;
}

bool MicroCode::getClockSignal(Pep9::uarch::EClockSignals field) const
{
    return clockSignals[to_uint8_t(field)];
}

const QVector<bool> MicroCode::getClockSignals() const
{
    return clockSignals;
}

Pep9::uarch::EBranchFunctions MicroCode::getBranchFunction() const
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

bool MicroCode::inRange(Pep9::uarch::EControlSignals field, int value) const
{
    using namespace Pep9::uarch;
    switch (field) {
    case EControlSignals::C: return 0 <= value && value <= 31;
    case EControlSignals::B: return 0 <= value && value <= 31;
    case EControlSignals::A: return 0 <= value && value <= 31;
    case EControlSignals::AMux: return 0 <= value && value <= 1;
    case EControlSignals::MDRMux: return 0 <= value && value <= 1;
    case EControlSignals::CMux: return 0 <= value && value <= 1;
    case EControlSignals::ALU: return 0 <= value && value <= 15;
    case EControlSignals::CSMux: return 0 <= value && value <= 1;
    case EControlSignals::AndZ: return 0 <= value && value <= 1;
    case EControlSignals::MARMux: return 0 <= value && value <= 1;
    case EControlSignals::MDROMux: return 0 <= value && value <= 1;
    case EControlSignals::MDREMux: return 0 <= value && value <= 1;
    case EControlSignals::EOMux: return 0 <= value && value <= 1;
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
