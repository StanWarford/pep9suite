// File: asmcpupane.cpp
/*
    Pep9 is a virtual machine for writing machine language and assembly
    language programs.

    Copyright (C) 2019  J. Stanley Warford & Matthew McRaven, Pepperdine University

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
#include "asmcpupane.h"
#include "ui_asmcpupane.h"

#include <QKeyEvent>
#include <QMessageBox>
#include <QtGlobal>

#include "cpu/acpumodel.h"
#include "cpu/interfaceisacpu.h"
#include "pep/apepversion.h"
#include "pep/pep.h"
#include "pep/enu.h"
#include "style/fonts.h"

AsmCpuPane::AsmCpuPane(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::AsmCpuPane), acpu(nullptr)
{
    ui->setupUi(this);

    clearCpu();
    ui->cpuLabel->setFont(QFont(PepCore::labelFont, PepCore::labelFontSize));

    ui->accLabel->setFont(QFont(PepCore::labelFont));
    ui->xLabel->setFont(QFont(PepCore::labelFont));
    ui->spLabel->setFont(QFont(PepCore::labelFont));
    ui->pcLabel->setFont(QFont(PepCore::labelFont));
    ui->instrSpecLabel->setFont(QFont(PepCore::labelFont));
    ui->oprndSpecLabel->setFont(QFont(PepCore::labelFont));
    ui->oprndLabel->setFont(QFont(PepCore::labelFont));

    ui->pepNLabel->setFont(QFont(PepCore::labelFont));
    ui->pepZLabel->setFont(QFont(PepCore::labelFont));
    ui->pepVLabel->setFont(QFont(PepCore::labelFont));
    ui->pepCLabel->setFont(QFont(PepCore::labelFont));

    ui->nLabel->setFont(QFont(PepCore::labelFont));
    ui->zLabel->setFont(QFont(PepCore::labelFont));
    ui->vLabel->setFont(QFont(PepCore::labelFont));
    ui->cLabel->setFont(QFont(PepCore::labelFont));

    ui->accHexLabel->setFont(QFont(PepCore::labelFont));
    ui->accDecLabel->setFont(QFont(PepCore::labelFont));

    ui->xHexLabel->setFont(QFont(PepCore::labelFont));
    ui->xDecLabel->setFont(QFont(PepCore::labelFont));

    ui->spHexLabel->setFont(QFont(PepCore::labelFont));
    ui->spDecLabel->setFont(QFont(PepCore::labelFont));

    ui->pcHexLabel->setFont(QFont(PepCore::labelFont));
    ui->pcDecLabel->setFont(QFont(PepCore::labelFont));

    ui->instrSpecBinLabel->setFont(QFont(PepCore::labelFont));
    ui->instrSpecMnemonLabel->setFont(QFont(PepCore::labelFont));

    ui->oprndSpecHexLabel->setFont(QFont(PepCore::labelFont));
    ui->oprndSpecDecLabel->setFont(QFont(PepCore::labelFont));
    ui->oprndHexLabel->setFont(QFont(PepCore::labelFont));
    ui->oprndDecLabel->setFont(QFont(PepCore::labelFont));
}

AsmCpuPane::~AsmCpuPane()
{
    delete ui;
}

void AsmCpuPane::init(QSharedPointer<APepVersion> pep_version,
                      QSharedPointer<ACPUModel> cpu, QSharedPointer<InterfaceISACPU> isacpu)
{
    this->pep_version = pep_version;
    this->acpu = cpu;
    this->isacpu = isacpu;
}

void AsmCpuPane::updateCpu() {
    auto is_reg = pep_version->get_global_register_number(APepVersion::global_registers::IS);
    Enu::EAddrMode addrMode = Pep::decodeAddrMode[acpu->getCPURegByteCurrent(is_reg)];

    ui->nLabel->setText(acpu->getStatusBitCurrent(Enu::EStatusBit::STATUS_N) ? "1" : "0");
    ui->zLabel->setText(acpu->getStatusBitCurrent(Enu::EStatusBit::STATUS_Z) ? "1" : "0");
    ui->vLabel->setText(acpu->getStatusBitCurrent(Enu::EStatusBit::STATUS_V) ? "1" : "0");
    ui->cLabel->setText(acpu->getStatusBitCurrent(Enu::EStatusBit::STATUS_C) ? "1" : "0");

    quint16 acc, idx, sp, pc, opsc;
    quint8 is;
    auto a_reg = pep_version->get_global_register_number(APepVersion::global_registers::A);
    acc = acpu->getCPURegWordCurrent(a_reg);
    auto x_reg = pep_version->get_global_register_number(APepVersion::global_registers::X);
    idx = acpu->getCPURegWordCurrent(x_reg);
    auto sp_reg = pep_version->get_global_register_number(APepVersion::global_registers::SP);
    sp = acpu->getCPURegWordCurrent(sp_reg);
    auto pc_reg = pep_version->get_global_register_number(APepVersion::global_registers::PC);
    pc = acpu->getCPURegWordCurrent(pc_reg);
    auto os_reg = pep_version->get_global_register_number(APepVersion::global_registers::OS);
    opsc = acpu->getCPURegWordCurrent(os_reg);
    is = acpu->getCPURegByteCurrent(is_reg);

    ui->accHexLabel->setText(QString("0x") + QString("%1").arg(acc, 4, 16, QLatin1Char('0')).toUpper());
    ui->accDecLabel->setText(QString("%1").arg(static_cast<qint16>(acc)));

    ui->xHexLabel->setText(QString("0x") + QString("%1").arg(idx, 4, 16, QLatin1Char('0')).toUpper());
    ui->xDecLabel->setText(QString("%1").arg(static_cast<qint16>(idx)));

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
        ui->oprndSpecDecLabel->setText(QString("%1").arg(static_cast<quint16>(opsc)));

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


