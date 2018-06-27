// File: code.cpp
/*
    Pep9CPU is a CPU simulator for executing microcode sequences to
    implement instructions in the instruction set of the Pep/9 computer.

    Copyright (C) 2010  J. Stanley Warford, Pepperdine University

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "code.h"
#include "cpugraphicsitems.h"
#include "pep.h"
#include "cpudatasection.h"
#include "SymbolEntry.h"
#include <QMetaEnum>
MicroCode::MicroCode():controlSignals(22),clockSignals(12),branchFunc(Enu::Assembler_Assigned),
    symbol(nullptr),trueTargetAddr(nullptr),falseTargetAddr(nullptr)
{
    for(auto memLines : Pep::memControlToMnemonMap.keys())
    {
        controlSignals[memLines]=Enu::signalDisabled;
    }
    for(auto mainCtrlLines : Pep::decControlToMnemonMap.keys())
    {
        controlSignals[mainCtrlLines]=Enu::signalDisabled;
    }
    for(auto clockLines : Pep::clockControlToMnemonMap.keys())
    {
        clockSignals[clockLines]=0;
    }
}

bool MicroCode::isMicrocode() const { return true; }

void MicroCode::setCpuLabels(CpuGraphicsItems *cpuPaneItems) const
{
    cpuPaneItems->loadCk->setChecked(clockSignals[Enu::LoadCk] == 1);
    cpuPaneItems->cLineEdit->setText(controlSignals[Enu::C] == Enu::signalDisabled ? "" : QString("%1").arg(controlSignals[Enu::C]));
    cpuPaneItems->bLineEdit->setText(controlSignals[Enu::B] == Enu::signalDisabled ? "" : QString("%1").arg(controlSignals[Enu::B]));
    cpuPaneItems->aLineEdit->setText(controlSignals[Enu::A] == Enu::signalDisabled ? "" : QString("%1").arg(controlSignals[Enu::A]));
    cpuPaneItems->MARCk->setChecked(clockSignals[Enu::MARCk] == 1);
    cpuPaneItems->MDRCk->setChecked(clockSignals[Enu::MDRCk] == 1);
    cpuPaneItems->MDRECk->setChecked(clockSignals[Enu::MDRECk] == 1);
    cpuPaneItems->MDROCk->setChecked(clockSignals[Enu::MDROCk] == 1);
    cpuPaneItems->aMuxTristateLabel->setState(controlSignals[Enu::AMux] == Enu::signalDisabled ? -1 : controlSignals[Enu::AMux] );
    cpuPaneItems->MDRMuxTristateLabel->setState(controlSignals[Enu::MDRMux] == Enu::signalDisabled ? -1 : controlSignals[Enu::MDRMux] );
    cpuPaneItems->MDREMuxTristateLabel->setState(controlSignals[Enu::MDREMux] == Enu::signalDisabled ? -1 : controlSignals[Enu::MDREMux] );
    cpuPaneItems->MDROMuxTristateLabel->setState(controlSignals[Enu::MDROMux] == Enu::signalDisabled ? -1 : controlSignals[Enu::MDROMux] );
    cpuPaneItems->EOMuxTristateLabel->setState(controlSignals[Enu::EOMux] == Enu::signalDisabled ? -1 : controlSignals[Enu::EOMux] );
    cpuPaneItems->MARMuxTristateLabel->setState(controlSignals[Enu::MARMux] == Enu::signalDisabled ? -1 : controlSignals[Enu::MARMux] );
    cpuPaneItems->cMuxTristateLabel->setState(controlSignals[Enu::CMux] == Enu::signalDisabled ? -1 : controlSignals[Enu::CMux] );
    cpuPaneItems->ALULineEdit->setText(controlSignals[Enu::ALU] == Enu::signalDisabled ? "" : QString("%1").arg(controlSignals[Enu::ALU]));
    cpuPaneItems->CSMuxTristateLabel->setState(controlSignals[Enu::CSMux] == Enu::signalDisabled ? -1 : controlSignals[Enu::CSMux] );
    cpuPaneItems->SCkCheckBox->setChecked(clockSignals[Enu::SCk] == 1);
    cpuPaneItems->CCkCheckBox->setChecked(clockSignals[Enu::CCk] == 1);
    cpuPaneItems->VCkCheckBox->setChecked(clockSignals[Enu::VCk] == 1);
    cpuPaneItems->AndZTristateLabel->setState(controlSignals[Enu::AndZ] == Enu::signalDisabled ? -1 : controlSignals[Enu::AndZ] );
    cpuPaneItems->ZCkCheckBox->setChecked(clockSignals[Enu::ZCk] == 1);
    cpuPaneItems->NCkCheckBox->setChecked(clockSignals[Enu::NCk] == 1);
    cpuPaneItems->MemReadTristateLabel->setState(controlSignals[Enu::MemRead] == Enu::signalDisabled ? -1 : controlSignals[Enu::MemRead] );
    cpuPaneItems->MemWriteTristateLabel->setState(controlSignals[Enu::MemWrite] == Enu::signalDisabled ? -1 : controlSignals[Enu::MemWrite] );
}

QString MicroCode::getObjectCode() const
{
    // QString QString::arg(int a, int fieldWidth = 0, ...)
    // fieldWidth specifies the minimum amount of space that
    //  a is padded to and filled with the character fillChar.
    // A positive value produces right-aligned text; a negative
    //  value produces left-aligned text.

    QString str = "";
    if (CPUDataSection::getInstance()->getCPUFeatures() == Enu::OneByteDataBus) {
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
        str.append(controlSignals[Enu::MemWrite] == Enu::signalDisabled ? "  " : QString("%1").arg(controlSignals[Enu::MemWrite], -2));
        str.append(controlSignals[Enu::MemRead] == Enu::signalDisabled ? "  " : QString("%1").arg(controlSignals[Enu::MemRead], -2));
    }
    else if (CPUDataSection::getInstance()->getCPUFeatures() == Enu::TwoByteDataBus) {
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
        str.append(controlSignals[Enu::MemWrite] == Enu::signalDisabled ? "  " : QString("%1").arg(controlSignals[Enu::MemWrite], -2));
        str.append(controlSignals[Enu::MemRead] == Enu::signalDisabled ? "  " : QString("%1").arg(controlSignals[Enu::MemRead], -2));
    }
    str.append("\n");
    return str;
}

QString MicroCode::getSourceCode() const
{
    QString str = "",symbolString="";
    if(symbol!=nullptr&&!symbol->getName().startsWith(("_")))
    {
        symbolString.append(symbol->getName()+": ");
    }
    if (CPUDataSection::getInstance()->getCPUFeatures() == Enu::OneByteDataBus) {
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
    else if (CPUDataSection::getInstance()->getCPUFeatures() == Enu::TwoByteDataBus) {
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
        if (clockSignals[Enu::PValidCk] != 0) { str.append("PValidCk, "); }

        if (str.endsWith(", ") || str.endsWith("; ")) { str.chop(2); }
    }

    if (branchFunc == Enu::Unconditional){
        if(symbol->getValue()+1==trueTargetAddr->getValue()){

        }
        else{
            if(str.isEmpty()){
                str.append("goto "+trueTargetAddr->getName());
            }
            else{
               str.append("; goto "+trueTargetAddr->getName());
            }
        }
    }
    else if (branchFunc == Enu::Stop){
        if(str.isEmpty()){
            str.append("stop");
        }
        else{
           str.append("; stop");
        }
    }
    else if (branchFunc == Enu::InstructionSpecifierDecoder){
        str.append("; "+Pep::branchFuncToMnemonMap[branchFunc]);
    }
    else if (branchFunc == Enu::AddressingModeDecoder){
        str.append("; "+Pep::branchFuncToMnemonMap[branchFunc]);
    }
    else{
        if(str.isEmpty()){
            str.append("if "+Pep::branchFuncToMnemonMap[branchFunc]+" "+
                       trueTargetAddr->getName()+" else "+falseTargetAddr->getName());
        }
        else{
            str.append("; if "+Pep::branchFuncToMnemonMap[branchFunc]+" "+
                       trueTargetAddr->getName()+" else "+falseTargetAddr->getName());
        }

    }

    if (!cComment.isEmpty()) {
        str.append(" " + cComment);
    }
    symbolString.append(str);
    return symbolString;
}

bool MicroCode::hasSymbol() const
{
    return symbol != nullptr;
}

bool MicroCode::hasControlSignal(Enu::EControlSignals field) const
{
    return controlSignals[field]!=Enu::signalDisabled;
}

bool MicroCode::hasClockSignal(Enu::EClockSignals field) const
{
    return clockSignals[field]==true;
}

const SymbolEntry* MicroCode::getSymbol() const
{
    return symbol;
}

SymbolEntry* MicroCode::getSymbol()
{
    return symbol;
}

void MicroCode::setControlSignal(Enu::EControlSignals field, quint8 value)
{
    controlSignals[field]=value;
}

void MicroCode::setClockSingal(Enu::EClockSignals field, bool value)
{
    clockSignals[field]=value;
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

int MicroCode::getControlSignal(Enu::EControlSignals field) const
{
    return controlSignals[field];
}

bool MicroCode::getClockSignal(Enu::EClockSignals field) const
{
    return clockSignals[field];
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

void MicroCode::setSymbol(SymbolEntry* symbol)
{
    this->symbol=symbol;
}

CommentOnlyCode::CommentOnlyCode(QString comment)
{
    cComment = comment;
}

QString CommentOnlyCode::getSourceCode()const
{
    return cComment;
}

UnitPreCode::~UnitPreCode()
{
    while (!unitPreList.isEmpty())
    {
        delete unitPreList.takeFirst();
    }
}

QString UnitPreCode::getSourceCode() const
{
    QString str = "UnitPre: ";
    for (int i = 0; i < unitPreList.size(); i++)
    {
        str.append(unitPreList.at(i)->getSourceCode() + ", ");
    }
    if (str.endsWith(", "))
    {
        str.chop(2);
    }
    if (!cComment.isEmpty())
    {
        str.append(" " + cComment);
    }
    return str;
}

bool UnitPreCode::hasUnitPre() const
{
    return !unitPreList.isEmpty();
}


void UnitPreCode::setUnitPre(CPUDataSection *data)
{
    for(auto x : unitPreList)
    {
        x->setUnitPre(data);
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
    while (!unitPostList.isEmpty())
    {
        delete unitPostList.takeFirst();
    }
}

QString UnitPostCode::getSourceCode() const
{
    QString str = "UnitPost: ";
    for (int i = 0; i < unitPostList.size(); i++)
    {
        str.append(unitPostList.at(i)->getSourceCode() + ", ");
    }
    if (str.endsWith(", "))
    {
        str.chop(2);
    }
    if (!cComment.isEmpty())
    {
        str.append(" " + cComment);
    }
    return str;
}

bool UnitPostCode::testPostcondition(CPUDataSection *data, QString &err)
{
    bool val=true;;
    for(auto x : unitPostList)
    {
        val&=x->testUnitPost(data,err);
        if(!val) return false;
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
