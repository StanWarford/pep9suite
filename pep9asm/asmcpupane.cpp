// File: AsmCpuPane.cpp
/*
    Pep9 is a virtual machine for writing machine language and assembly
    language programs.

    Copyright (C) 2009  J. Stanley Warford, Pepperdine University

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
#include <QMessageBox>
#include <QKeyEvent>
#include "asmcpupane.h"
#include "ui_asmcpupane.h"
#include "pep.h"
#include "enu.h"
#include <QtGlobal>
#include "acpumodel.h"
#include "interfaceisacpu.h"

AsmCpuPane::AsmCpuPane(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::AsmCpuPane), acpu(nullptr)
{
    ui->setupUi(this);

    clearCpu();

    if (Pep::getSystem() != "Mac") {
        ui->cpuLabel->setFont(QFont(Pep::labelFont, Pep::labelFontSize));

        ui->accLabel->setFont(QFont(Pep::labelFont));
        ui->xLabel->setFont(QFont(Pep::labelFont));
        ui->spLabel->setFont(QFont(Pep::labelFont));
        ui->pcLabel->setFont(QFont(Pep::labelFont));
        ui->instrSpecLabel->setFont(QFont(Pep::labelFont));
        ui->oprndSpecLabel->setFont(QFont(Pep::labelFont));
        ui->oprndLabel->setFont(QFont(Pep::labelFont));

        ui->pepNLabel->setFont(QFont(Pep::labelFont));
        ui->pepZLabel->setFont(QFont(Pep::labelFont));
        ui->pepVLabel->setFont(QFont(Pep::labelFont));
        ui->pepCLabel->setFont(QFont(Pep::labelFont));

        ui->nLabel->setFont(QFont(Pep::labelFont));
        ui->zLabel->setFont(QFont(Pep::labelFont));
        ui->vLabel->setFont(QFont(Pep::labelFont));
        ui->cLabel->setFont(QFont(Pep::labelFont));

        ui->accHexLabel->setFont(QFont(Pep::labelFont));
        ui->accDecLabel->setFont(QFont(Pep::labelFont));

        ui->xHexLabel->setFont(QFont(Pep::labelFont));
        ui->xDecLabel->setFont(QFont(Pep::labelFont));

        ui->spHexLabel->setFont(QFont(Pep::labelFont));
        ui->spDecLabel->setFont(QFont(Pep::labelFont));

        ui->pcHexLabel->setFont(QFont(Pep::labelFont));
        ui->pcDecLabel->setFont(QFont(Pep::labelFont));

        ui->instrSpecBinLabel->setFont(QFont(Pep::labelFont));
        ui->instrSpecMnemonLabel->setFont(QFont(Pep::labelFont));

        ui->oprndSpecHexLabel->setFont(QFont(Pep::labelFont));
        ui->oprndSpecDecLabel->setFont(QFont(Pep::labelFont));
        ui->oprndHexLabel->setFont(QFont(Pep::labelFont));
        ui->oprndDecLabel->setFont(QFont(Pep::labelFont));
    }
}

AsmCpuPane::~AsmCpuPane()
{
    delete ui;
}

void AsmCpuPane::init(QSharedPointer<ACPUModel> cpu, QSharedPointer<InterfaceISACPU> isacpu)
{
    this->acpu = cpu;
    this->isacpu = isacpu;
}

void AsmCpuPane::updateCpu() {
    Enu::EAddrMode addrMode = Pep::decodeAddrMode[acpu->getCPURegByteCurrent(Enu::CPURegisters::IS)];

    ui->nLabel->setText(acpu->getStatusBitCurrent(Enu::EStatusBit::STATUS_N) ? "1" : "0");
    ui->zLabel->setText(acpu->getStatusBitCurrent(Enu::EStatusBit::STATUS_Z) ? "1" : "0");
    ui->vLabel->setText(acpu->getStatusBitCurrent(Enu::EStatusBit::STATUS_V) ? "1" : "0");
    ui->cLabel->setText(acpu->getStatusBitCurrent(Enu::EStatusBit::STATUS_C) ? "1" : "0");

    quint16 acc, idx, sp, pc, opsc;
    quint8 is;
    acc = acpu->getCPURegWordCurrent(Enu::CPURegisters::A);
    idx = acpu->getCPURegWordCurrent(Enu::CPURegisters::X);
    sp = acpu->getCPURegWordCurrent(Enu::CPURegisters::SP);
    pc = acpu->getCPURegWordCurrent(Enu::CPURegisters::PC);
    opsc = acpu->getCPURegWordCurrent(Enu::CPURegisters::OS);
    is = acpu->getCPURegByteCurrent(Enu::CPURegisters::IS);
    ui->accHexLabel->setText(QString("0x") + QString("%1").arg(acc, 4, 16, QLatin1Char('0')).toUpper());
    ui->accDecLabel->setText(QString("%1").arg(static_cast<qint16>(acc)));

    ui->xHexLabel->setText(QString("0x") + QString("%1").arg(idx, 4, 16, QLatin1Char('0')).toUpper());
    ui->xDecLabel->setText(QString("%1").arg(static_cast<qint16>(acc)));

    ui->spHexLabel->setText(QString("0x") + QString("%1").arg(sp, 4, 16, QLatin1Char('0')).toUpper());
    ui->spDecLabel->setText(QString("%1").arg(sp));

    ui->pcHexLabel->setText(QString("0x") + QString("%1").arg(pc, 4, 16, QLatin1Char('0')).toUpper());
    ui->pcDecLabel->setText(QString("%1").arg(pc));

    ui->instrSpecBinLabel->setText(QString("%1").arg(is, 8, 2, QLatin1Char('0')).toUpper());
    ui->instrSpecMnemonLabel->setText(" " + Pep::enumToMnemonMap.value(Pep::decodeMnemonic[is])
                                           + Pep::addrModeToCommaSpace(addrMode));

    /* We no longer have Enu::None as a valid instruction, but keep code for reference
     * if (Pep::decodeAddrMode.value(Sim::instructionSpecifier) == Enu::NONE) {
        ui->oprndSpecHexLabel->setText("");
        ui->oprndSpecDecLabel->setText("");
        ui->oprndHexLabel->setText("");
        ui->oprndDecLabel->setText("");
    }*/
    //else {
    if(Pep::isUnaryMap[Pep::decodeMnemonic[is]]) {
        ui->oprndSpecHexLabel->setText("");
        ui->oprndSpecDecLabel->setText("");
        ui->oprndHexLabel->setText("");
        ui->oprndDecLabel->setText("");
    }
    else {
        ui->oprndSpecHexLabel->setText(QString("0x") + QString("%1").arg(opsc, 4,
                                                                         16, QLatin1Char('0')).toUpper());
        ui->oprndSpecDecLabel->setText(QString("%1").arg(static_cast<qint16>(opsc)));

        quint16 opVal = isacpu->getOperandValue();

        if(Pep::operandDisplayFieldWidth(Pep::decodeMnemonic[is]) == 2) {
            opVal &= 0xff;
        }

        ui->oprndHexLabel->setText(QString("0x") + QString("%1").arg(opVal,
                                                     Pep::operandDisplayFieldWidth(Pep::decodeMnemonic[is]),
                                                     16, QLatin1Char('0')).toUpper());
        ui->oprndDecLabel->setText(QString("%1").arg(static_cast<qint16>(opVal)));
    }
}

void AsmCpuPane::clearCpu()
{
    ui->nLabel->setText("");
    ui->zLabel->setText("");
    ui->vLabel->setText("");
    ui->cLabel->setText("");

    ui->accHexLabel->setText("");
    ui->accDecLabel->setText("");

    ui->xHexLabel->setText("");
    ui->xDecLabel->setText("");

    ui->spHexLabel->setText("");
    ui->spDecLabel->setText("");

    ui->pcHexLabel->setText("");
    ui->pcDecLabel->setText("");

    ui->instrSpecBinLabel->setText("");
    ui->instrSpecMnemonLabel->setText("");

    ui->oprndSpecHexLabel->setText("");
    ui->oprndSpecDecLabel->setText("");
    ui->oprndHexLabel->setText("");
    ui->oprndDecLabel->setText("");
}

void AsmCpuPane::highlightOnFocus()
{
    if (this->isAncestorOf(focusWidget())) {
        ui->cpuLabel->setAutoFillBackground(true);
    }
    else {
        ui->cpuLabel->setAutoFillBackground(false);
    }
}

bool AsmCpuPane::hasFocus()
{
    return this->isAncestorOf(focusWidget());
}

void AsmCpuPane::onSimulationUpdate()
{
    updateCpu();
}


