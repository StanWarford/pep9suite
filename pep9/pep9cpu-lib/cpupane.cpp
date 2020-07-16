// File: cpupane.cpp
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

#include "cpupane.h"
#include "ui_cpupane.h"

#include <QCheckBox>
#include <QErrorMessage>
#include <QGraphicsItem>
#include <QLineEdit>
#include <QMessageBox>
#include <QScrollBar>

#include "cpu/interfacemccpu.h"
#include "cpu-diagram/tristatelabel.h"
#include "microassembler/microcode.h"
#include "style/fonts.h"

#include "cpudata.h"
#include "pep9microcode.h"

CpuPane::CpuPane( QWidget *parent) :
        QWidget(parent),
        cpu(nullptr), dataSection(nullptr),
        cpuPaneItems(nullptr), ui(new Ui::CpuPane)
{
    ui->setupUi(this);
    connect(ui->spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &CpuPane::zoomFactorChanged);
    scene = new QGraphicsScene(nullptr);
    ui->graphicsView->setScene(scene);

    ui->graphicsView->setFont(QFont(PepCore::cpuFont, PepCore::cpuFontSize));

    ui->spinBox->hide();
    ui->graphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);


}

void CpuPane::init(QSharedPointer<InterfaceMCCPU> cpu, QSharedPointer<CPUDataSection> dataSection)
{
    this->cpu = cpu;
    this->dataSection = dataSection;
    type = cpu->getCPUType();
    initModel();
    this->setMinimumWidth(static_cast<int>(cpuPaneItems->boundingRect().left())+100);
    // qDebug() << static_cast<int>(cpuPaneItems->boundingRect().right())+45;
    this->setMaximumWidth(static_cast<int>(cpuPaneItems->boundingRect().right())+45);
    // Give this class a larger acceptable size range, so that it will behave better in a splitter

}

CpuPane::~CpuPane()
{
    delete ui;
}

void CpuPane::highlightOnFocus()
{
    if (ui->graphicsView->hasFocus() || ui->spinBox->hasFocus()) {
        ui->label->setAutoFillBackground(true);
    }
    else {
        ui->label->setAutoFillBackground(false);
    }
}

bool CpuPane::hasFocus()
{
    return ui->graphicsView->hasFocus() || ui->spinBox->hasFocus();
}

void CpuPane::giveFocus()
{
    ui->graphicsView->setFocus();
}

void CpuPane::initModel()
{
    cpuPaneItems = new CpuGraphicsItems(type, dataSection.get(), ui->graphicsView, nullptr, scene);
    ui->graphicsView->scene()->addItem(cpuPaneItems);

    ui->graphicsView->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    connect(cpuPaneItems->loadCk, SIGNAL(clicked()), scene, SLOT(invalidate()));
    connect(cpuPaneItems->loadCk, &QCheckBox::clicked,this,&CpuPane::onClockChanged);
    connect(cpuPaneItems->cLineEdit, SIGNAL(textChanged(QString)), scene, SLOT(invalidate()));
    connect(cpuPaneItems->bLineEdit, SIGNAL(textChanged(QString)), scene, SLOT(invalidate()));
    connect(cpuPaneItems->aLineEdit, SIGNAL(textChanged(QString)), scene, SLOT(invalidate()));
    connect(cpuPaneItems->cLineEdit, &QLineEdit::textChanged, this, &CpuPane::onBusChanged);
    connect(cpuPaneItems->bLineEdit, &QLineEdit::textChanged, this, &CpuPane::onBusChanged);
    connect(cpuPaneItems->aLineEdit, &QLineEdit::textChanged, this, &CpuPane::onBusChanged);

    connect(cpuPaneItems->MARCk, SIGNAL(clicked()), scene, SLOT(invalidate()));
    connect(cpuPaneItems->MARCk, &QCheckBox::clicked, this, &CpuPane::onClockChanged);

    connect(cpuPaneItems->aMuxTristateLabel, &TristateLabel::clicked, this, &CpuPane::labelClicked);
    connect(cpuPaneItems->aMuxTristateLabel, SIGNAL(clicked()), scene, SLOT(invalidate()));


    connect(cpuPaneItems->cMuxTristateLabel, &TristateLabel::clicked, this, &CpuPane::labelClicked);
    connect(cpuPaneItems->cMuxTristateLabel, SIGNAL(clicked()), scene, SLOT(invalidate()));

    connect(cpuPaneItems->ALULineEdit, SIGNAL(textChanged(QString)), scene, SLOT(invalidate()));

    connect(cpuPaneItems->CSMuxTristateLabel, &TristateLabel::clicked, this, &CpuPane::labelClicked);
    connect(cpuPaneItems->CSMuxTristateLabel, SIGNAL(clicked()), scene, SLOT(invalidate()));

    connect(cpuPaneItems->SCkCheckBox, SIGNAL(clicked()), scene, SLOT(invalidate()));
    connect(cpuPaneItems->CCkCheckBox, SIGNAL(clicked()), scene, SLOT(invalidate()));
    connect(cpuPaneItems->VCkCheckBox, SIGNAL(clicked()), scene, SLOT(invalidate()));
    connect(cpuPaneItems->ZCkCheckBox, SIGNAL(clicked()), scene, SLOT(invalidate()));
    connect(cpuPaneItems->NCkCheckBox, SIGNAL(clicked()), scene, SLOT(invalidate()));
    connect(cpuPaneItems->SCkCheckBox, &QCheckBox::clicked, this, &CpuPane::onClockChanged);
    connect(cpuPaneItems->CCkCheckBox, &QCheckBox::clicked, this, &CpuPane::onClockChanged);
    connect(cpuPaneItems->VCkCheckBox, &QCheckBox::clicked, this, &CpuPane::onClockChanged);
    connect(cpuPaneItems->ZCkCheckBox, &QCheckBox::clicked, this, &CpuPane::onClockChanged);
    connect(cpuPaneItems->NCkCheckBox, &QCheckBox::clicked, this, &CpuPane::onClockChanged);

    connect(cpuPaneItems->AndZTristateLabel, &TristateLabel::clicked, this, &CpuPane::labelClicked);
    connect(cpuPaneItems->AndZTristateLabel, SIGNAL(clicked()), scene, SLOT(invalidate()));

    connect(cpuPaneItems->MemReadTristateLabel, &TristateLabel::clicked, this, &CpuPane::labelClicked);
    connect(cpuPaneItems->MemReadTristateLabel, SIGNAL(clicked()), scene, SLOT(invalidate()));
    connect(cpuPaneItems->MemWriteTristateLabel, &TristateLabel::clicked, this, &CpuPane::labelClicked);
    connect(cpuPaneItems->MemWriteTristateLabel, SIGNAL(clicked()), scene, SLOT(invalidate()));

    connect(cpuPaneItems->nBitLabel, &TristateLabel::clicked, this, &CpuPane::labelClicked);
    connect(cpuPaneItems->zBitLabel, &TristateLabel::clicked, this, &CpuPane::labelClicked);
    connect(cpuPaneItems->vBitLabel, &TristateLabel::clicked, this, &CpuPane::labelClicked);
    connect(cpuPaneItems->cBitLabel, &TristateLabel::clicked, this, &CpuPane::labelClicked);
    connect(cpuPaneItems->sBitLabel, &TristateLabel::clicked, this, &CpuPane::labelClicked);

    // Simulation control connections
    connect(ui->clockPushButton, &QAbstractButton::clicked, this, &CpuPane::clockButtonPushed);

    // Register editing connnections
    connect(cpuPaneItems->aRegLineEdit, &QLineEdit::textEdited, this, &CpuPane::regTextEdited);
    connect(cpuPaneItems->xRegLineEdit, &QLineEdit::textEdited, this, &CpuPane::regTextEdited);
    connect(cpuPaneItems->spRegLineEdit, &QLineEdit::textEdited, this, &CpuPane::regTextEdited);
    connect(cpuPaneItems->pcRegLineEdit, &QLineEdit::textEdited, this, &CpuPane::regTextEdited);
    connect(cpuPaneItems->irRegLineEdit, &QLineEdit::textEdited, this, &CpuPane::regTextEdited);
    connect(cpuPaneItems->t1RegLineEdit, &QLineEdit::textEdited, this, &CpuPane::regTextEdited);
    connect(cpuPaneItems->t2RegLineEdit, &QLineEdit::textEdited, this, &CpuPane::regTextEdited);
    connect(cpuPaneItems->t3RegLineEdit, &QLineEdit::textEdited, this, &CpuPane::regTextEdited);
    connect(cpuPaneItems->t4RegLineEdit, &QLineEdit::textEdited, this, &CpuPane::regTextEdited);
    connect(cpuPaneItems->t5RegLineEdit, &QLineEdit::textEdited, this, &CpuPane::regTextEdited);
    connect(cpuPaneItems->t6RegLineEdit, &QLineEdit::textEdited, this, &CpuPane::regTextEdited);

    connect(cpuPaneItems->aRegLineEdit, &QLineEdit::editingFinished, this, &CpuPane::regTextFinishedEditing);
    connect(cpuPaneItems->xRegLineEdit, &QLineEdit::editingFinished, this, &CpuPane::regTextFinishedEditing);
    connect(cpuPaneItems->spRegLineEdit, &QLineEdit::editingFinished, this, &CpuPane::regTextFinishedEditing);
    connect(cpuPaneItems->pcRegLineEdit, &QLineEdit::editingFinished, this, &CpuPane::regTextFinishedEditing);
    connect(cpuPaneItems->irRegLineEdit, &QLineEdit::editingFinished, this, &CpuPane::regTextFinishedEditing);
    connect(cpuPaneItems->t1RegLineEdit, &QLineEdit::editingFinished, this, &CpuPane::regTextFinishedEditing);
    connect(cpuPaneItems->t2RegLineEdit, &QLineEdit::editingFinished, this, &CpuPane::regTextFinishedEditing);
    connect(cpuPaneItems->t3RegLineEdit, &QLineEdit::editingFinished, this, &CpuPane::regTextFinishedEditing);
    connect(cpuPaneItems->t4RegLineEdit, &QLineEdit::editingFinished, this, &CpuPane::regTextFinishedEditing);
    connect(cpuPaneItems->t5RegLineEdit, &QLineEdit::editingFinished, this, &CpuPane::regTextFinishedEditing);
    connect(cpuPaneItems->t6RegLineEdit, &QLineEdit::editingFinished, this, &CpuPane::regTextFinishedEditing);

    connect(cpuPaneItems->ALULineEdit, &QLineEdit::textChanged, this, &CpuPane::ALUTextEdited);

    // 1 byte signals
    connect(cpuPaneItems->MDRCk, SIGNAL(clicked()), scene, SLOT(invalidate()));
    connect(cpuPaneItems->MDRCk, &QCheckBox::clicked, this, &CpuPane::onClockChanged);
    connect(cpuPaneItems->MDRMuxTristateLabel, &TristateLabel::clicked, this, &CpuPane::labelClicked);
    connect(cpuPaneItems->MDRMuxTristateLabel, SIGNAL(clicked()), scene, SLOT(invalidate()));

    // 2 byte bus signals
    connect(cpuPaneItems->MARMuxTristateLabel, &TristateLabel::clicked, this, &CpuPane::labelClicked);
    connect(cpuPaneItems->MARMuxTristateLabel, SIGNAL(clicked()), scene, SLOT(invalidate()));
    connect(cpuPaneItems->MDROMuxTristateLabel, &TristateLabel::clicked, this, &CpuPane::labelClicked);
    connect(cpuPaneItems->MDROMuxTristateLabel, SIGNAL(clicked()), scene, SLOT(invalidate()));
    connect(cpuPaneItems->MDREMuxTristateLabel, &TristateLabel::clicked, this, &CpuPane::labelClicked);
    connect(cpuPaneItems->MDREMuxTristateLabel, SIGNAL(clicked()), scene, SLOT(invalidate()));
    connect(cpuPaneItems->EOMuxTristateLabel, &TristateLabel::clicked, this, &CpuPane::labelClicked);
    connect(cpuPaneItems->EOMuxTristateLabel, SIGNAL(clicked()), scene, SLOT(invalidate()));
    connect(cpuPaneItems->MDRECk, SIGNAL(clicked()), scene, SLOT(invalidate()));
    connect(cpuPaneItems->MDROCk, SIGNAL(clicked()), scene, SLOT(invalidate()));
    connect(cpuPaneItems->MDRECk, &QCheckBox::clicked,this,&CpuPane::onClockChanged);
    connect(cpuPaneItems->MDROCk, &QCheckBox::clicked,this,&CpuPane::onClockChanged);
}

void CpuPane::startDebugging()
{
    initRegisters();
    ui->clockPushButton->setEnabled(false);
    ui->copyToMicrocodePushButton->setEnabled(false);
    const MicroCode *code = static_cast<const MicroCode *>(cpu->getCurrentMicrocodeLine());
    cpuPaneItems->setCPULabels(code);
    //code->setCpuLabels(cpuPaneItems);
}

void CpuPane::stopDebugging()
{
    ui->clockPushButton->setEnabled(true);
    ui->copyToMicrocodePushButton->setEnabled(true);
}

void CpuPane::setRegister(Pep9::uarch::ECPUKeywords reg, int value)
{
    using namespace Pep9::uarch;
    switch (reg) {
    case ECPUKeywords::Acc:
        cpuPaneItems->aRegLineEdit->setText("0x" + QString("%1").arg(value, 4, 16, QLatin1Char('0')).toUpper());
        break;
    case ECPUKeywords::X:
        cpuPaneItems->xRegLineEdit->setText("0x" + QString("%1").arg(value, 4, 16, QLatin1Char('0')).toUpper());
        break;
    case ECPUKeywords::SP:
        cpuPaneItems->spRegLineEdit->setText("0x" + QString("%1").arg(value, 4, 16, QLatin1Char('0')).toUpper());
        break;
    case ECPUKeywords::PC:
        cpuPaneItems->pcRegLineEdit->setText("0x" + QString("%1").arg(value, 4, 16, QLatin1Char('0')).toUpper());
        break;
    case ECPUKeywords::IR:
        cpuPaneItems->irRegLineEdit->setText("0x" + QString("%1").arg(value, 6, 16, QLatin1Char('0')).toUpper());
        break;
    case ECPUKeywords::T1:
        cpuPaneItems->t1RegLineEdit->setText("0x" + QString("%1").arg(value, 2, 16, QLatin1Char('0')).toUpper());
        break;
    case ECPUKeywords::T2:
        cpuPaneItems->t2RegLineEdit->setText("0x" + QString("%1").arg(value, 4, 16, QLatin1Char('0')).toUpper());
        break;
    case ECPUKeywords::T3:
        cpuPaneItems->t3RegLineEdit->setText("0x" + QString("%1").arg(value, 4, 16, QLatin1Char('0')).toUpper());
        break;
    case ECPUKeywords::T4:
        cpuPaneItems->t4RegLineEdit->setText("0x" + QString("%1").arg(value, 4, 16, QLatin1Char('0')).toUpper());
        break;
    case ECPUKeywords::T5:
        cpuPaneItems->t5RegLineEdit->setText("0x" + QString("%1").arg(value, 4, 16, QLatin1Char('0')).toUpper());
        break;
    case ECPUKeywords::T6:
        cpuPaneItems->t6RegLineEdit->setText("0x" + QString("%1").arg(value, 4, 16, QLatin1Char('0')).toUpper());
        break;
    case ECPUKeywords::MARAREG:
        cpuPaneItems->MARALabel->setText("0x" + QString("%1").arg(value, 2, 16, QLatin1Char('0')).toUpper());
        break;
    case ECPUKeywords::MARBREG:
        cpuPaneItems->MARBLabel->setText("0x" + QString("%1").arg(value, 2, 16, QLatin1Char('0')).toUpper());
        break;
    case ECPUKeywords::MDROREG:
        cpuPaneItems->MDROLabel->setText("0x" + QString("%1").arg(value, 2, 16, QLatin1Char('0')).toUpper());
        break;
    case ECPUKeywords::MDREREG:
        cpuPaneItems->MDRELabel->setText("0x" + QString("%1").arg(value, 2, 16, QLatin1Char('0')).toUpper());
        break;
    case ECPUKeywords::MDRREG:
        cpuPaneItems->MDRLabel->setText("0x" + QString("%1").arg(value, 2, 16, QLatin1Char('0')).toUpper());
        break;
    default:
        // the remainder of the array is 'read only' in our simulated CPU
        break;
    }
}

void CpuPane::setRegisterByte(quint8 reg, quint8 value)
{
    QLatin1Char ch = QLatin1Char('0');
    switch (reg) {
    case 0:
        cpuPaneItems->aRegLineEdit->setText("0x" + QString("%1").arg(value * 256 + dataSection->getRegisterBankByte(1), 4, 16, ch).toUpper());
        break;
    case 1:
        cpuPaneItems->aRegLineEdit->setText("0x" + QString("%1").arg(dataSection->getRegisterBankByte(0) * 256 + value, 4, 16, ch).toUpper());
        break;
    case 2:
        cpuPaneItems->xRegLineEdit->setText("0x" + QString("%1").arg(value * 256 + dataSection->getRegisterBankByte(3), 4, 16, ch).toUpper());
        break;
    case 3:
        cpuPaneItems->xRegLineEdit->setText("0x" + QString("%1").arg(dataSection->getRegisterBankByte(2) * 256 + value, 4, 16, ch).toUpper());
        break;
    case 4:
        cpuPaneItems->spRegLineEdit->setText("0x" + QString("%1").arg(value * 256 + dataSection->getRegisterBankByte(5), 4, 16, ch).toUpper());
        break;
    case 5:
        cpuPaneItems->spRegLineEdit->setText("0x" + QString("%1").arg(dataSection->getRegisterBankByte(4) * 256 + value, 4, 16, ch).toUpper());
        break;
    case 6:
        cpuPaneItems->pcRegLineEdit->setText("0x" + QString("%1").arg(value * 256 + dataSection->getRegisterBankByte(7), 4, 16, ch).toUpper());
        break;
    case 7:
        cpuPaneItems->pcRegLineEdit->setText("0x" + QString("%1").arg(dataSection->getRegisterBankByte(6) * 256 + value, 4, 16, ch).toUpper());
        break;
    case 8:
        cpuPaneItems->irRegLineEdit->setText("0x" + QString("%1").arg(value * 65536 + dataSection->getRegisterBankByte(9) * 256 + dataSection->getRegisterBankByte(10), 6, 16, ch).toUpper());
        break;
    case 9:
        cpuPaneItems->irRegLineEdit->setText("0x" + QString("%1").arg(dataSection->getRegisterBankByte(8) * 65536 + value * 256 + dataSection->getRegisterBankByte(10), 6, 16, ch).toUpper());
        break;
    case 10:
        cpuPaneItems->irRegLineEdit->setText("0x" + QString("%1").arg(dataSection->getRegisterBankByte(8) * 65536 + dataSection->getRegisterBankByte(9) * 256 + value, 6, 16, ch).toUpper());
        break;
    case 11:
        cpuPaneItems->t1RegLineEdit->setText("0x" + QString("%1").arg(value, 2, 16, ch).toUpper());
        break;
    case 12:
        cpuPaneItems->t2RegLineEdit->setText("0x" + QString("%1").arg(value * 256 + dataSection->getRegisterBankByte(13), 4, 16, ch).toUpper());
        break;
    case 13:
        cpuPaneItems->t2RegLineEdit->setText("0x" + QString("%1").arg(dataSection->getRegisterBankByte(12) * 256 + value, 4, 16, ch).toUpper());
        break;
    case 14:
        cpuPaneItems->t3RegLineEdit->setText("0x" + QString("%1").arg(value * 256 + dataSection->getRegisterBankByte(15), 4, 16, ch).toUpper());
        break;
    case 15:
        cpuPaneItems->t3RegLineEdit->setText("0x" + QString("%1").arg(dataSection->getRegisterBankByte(14) * 256 + value, 4, 16, ch).toUpper());
        break;
    case 16:
        cpuPaneItems->t4RegLineEdit->setText("0x" + QString("%1").arg(value * 256 + dataSection->getRegisterBankByte(17), 4, 16, ch).toUpper());
        break;
    case 17:
        cpuPaneItems->t4RegLineEdit->setText("0x" + QString("%1").arg(dataSection->getRegisterBankByte(16) * 256 + value, 4, 16, ch).toUpper());
        break;
    case 18:
        cpuPaneItems->t5RegLineEdit->setText("0x" + QString("%1").arg(value * 256 + dataSection->getRegisterBankByte(19), 4, 16, ch).toUpper());
        break;
    case 19:
        cpuPaneItems->t5RegLineEdit->setText("0x" + QString("%1").arg(dataSection->getRegisterBankByte(18) * 256 + value, 4, 16, ch).toUpper());
        break;
    case 20:
        cpuPaneItems->t6RegLineEdit->setText("0x" + QString("%1").arg(value * 256 + dataSection->getRegisterBankByte(21), 4, 16, ch).toUpper());
        break;
    case 21:
        cpuPaneItems->t6RegLineEdit->setText("0x" + QString("%1").arg(dataSection->getRegisterBankByte(20) * 256 + value, 4, 16, ch).toUpper());
        break;
    default:
        // the remainder of the array is 'read only' in our simulated CPU, or outside the bounds
        break;
    }
}

void CpuPane::initRegisters()
{
    using namespace Pep9::uarch;
    //Set register bank
    for(quint8 it = 0 ;it < 22; it++) {
        setRegisterByte(it, dataSection->getRegisterBankByte(it));
    }

    //Set status bits
    setStatusBit(ECPUKeywords::N,dataSection->getStatusBit(EStatusBit::STATUS_N));
    setStatusBit(ECPUKeywords::Z,dataSection->getStatusBit(EStatusBit::STATUS_Z));
    setStatusBit(ECPUKeywords::V,dataSection->getStatusBit(EStatusBit::STATUS_V));
    setStatusBit(ECPUKeywords::Cbit,dataSection->getStatusBit(EStatusBit::STATUS_C));
    setStatusBit(ECPUKeywords::S,dataSection->getStatusBit(EStatusBit::STATUS_S));
}

void CpuPane::setStatusBit(Pep9::uarch::ECPUKeywords bit, bool value)
{
    using namespace Pep9::uarch;

    switch (bit) {
    case ECPUKeywords::N:
        cpuPaneItems->nBitLabel->setText(QString("%1").arg(value ? "1" : "0"));
        break;
    case ECPUKeywords::Z:
        cpuPaneItems->zBitLabel->setText(QString("%1").arg(value ? "1" : "0"));
        break;
    case ECPUKeywords::V:
        cpuPaneItems->vBitLabel->setText(QString("%1").arg(value ? "1" : "0"));
        break;
    case ECPUKeywords::Cbit:
        cpuPaneItems->cBitLabel->setText(QString("%1").arg(value ? "1" : "0"));
        break;
    case ECPUKeywords::S:
        cpuPaneItems->sBitLabel->setText(QString("%1").arg(value ? "1" : "0"));
        break;
    default:
        break;
    }
}

void CpuPane::setRegPrecondition(Pep9::uarch::ECPUKeywords reg, int value)
{
    setRegister(reg, value);
}

void CpuPane::setStatusPrecondition(Pep9::uarch::ECPUKeywords bit, bool value)
{
    setStatusBit(bit, value);
}

void CpuPane::clearCpu()
{
    using namespace Pep9::uarch;
    clearCpuControlSignals();
    setRegister(ECPUKeywords::Acc, 0);
    setRegister(ECPUKeywords::X, 0);
    setRegister(ECPUKeywords::SP, 0);
    setRegister(ECPUKeywords::PC, 0);
    setRegister(ECPUKeywords::IR, 0);
    setRegister(ECPUKeywords::T1, 0);
    setRegister(ECPUKeywords::T2, 0);
    setRegister(ECPUKeywords::T3, 0);
    setRegister(ECPUKeywords::T4, 0);
    setRegister(ECPUKeywords::T5, 0);
    setRegister(ECPUKeywords::T6, 0);

    setRegister(ECPUKeywords::MARAREG, 0);
    setRegister(ECPUKeywords::MARBREG, 0);

    // Clear 1 & 2 byte data registers
    setRegister(ECPUKeywords::MDRREG, 0);

    setRegister(ECPUKeywords::MDREREG, 0);
    setRegister(ECPUKeywords::MDROREG, 0);


    setStatusBit(ECPUKeywords::S, false);
    setStatusBit(ECPUKeywords::Cbit, false);
    setStatusBit(ECPUKeywords::V, false);
    setStatusBit(ECPUKeywords::Z, false);
    setStatusBit(ECPUKeywords::N, false);
}

void CpuPane::clearCpuControlSignals()
{
    cpuPaneItems->loadCk->setChecked(false);
    cpuPaneItems->cLineEdit->setText("");
    cpuPaneItems->bLineEdit->setText("");
    cpuPaneItems->aLineEdit->setText("");
    cpuPaneItems->MARCk->setChecked(false);
    cpuPaneItems->aMuxTristateLabel->setText("");
    cpuPaneItems->cMuxTristateLabel->setText("");
    cpuPaneItems->ALULineEdit->setText("");
    cpuPaneItems->CSMuxTristateLabel->setText("");
    cpuPaneItems->SCkCheckBox->setChecked(false);
    cpuPaneItems->CCkCheckBox->setChecked(false);
    cpuPaneItems->VCkCheckBox->setChecked(false);
    cpuPaneItems->AndZTristateLabel->setText("");
    cpuPaneItems->ZCkCheckBox->setChecked(false);
    cpuPaneItems->NCkCheckBox->setChecked(false);
    cpuPaneItems->MemReadTristateLabel->setText("");
    cpuPaneItems->MemWriteTristateLabel->setText("");

    // One byte signals:
    cpuPaneItems->MDRCk->setChecked(false);
    cpuPaneItems->MDRMuxTristateLabel->setText("");

    // Two byte signals:
    cpuPaneItems->MDRECk->setChecked(false);
    cpuPaneItems->MDROCk->setChecked(false);
    cpuPaneItems->MDROMuxTristateLabel->setText("");
    cpuPaneItems->MDREMuxTristateLabel->setText("");
    cpuPaneItems->EOMuxTristateLabel->setText("");

}

void CpuPane::clock()
{
    clockButtonPushed();
}

QSize CpuPane::sizeHint() const
{
    qint32 width;
    if(cpuPaneItems == nullptr) {
        width = QWidget::sizeHint().width();
    }else {
        width = 10 +  static_cast<qint32>(cpuPaneItems->boundingRect().width());
    }
    qint32 height = QWidget::sizeHint().height();
    return QSize(width,height);
}

void CpuPane::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void CpuPane::regTextEdited(QString str)
{
    QLineEdit *lineEdit = qobject_cast<QLineEdit *>(sender());

    // Make sure the string isn't mangled
    if (str == "0") {
        str = "0x";
        lineEdit->setText(str);
    }
    else if (!str.startsWith("0x")) {
        str = str.toUpper();
        str.prepend("0x");
        lineEdit->setText(str);
    }
    else {
        str.remove(0, 2);
        str = str.toUpper();
        str.prepend("0x");
        int pos = lineEdit->cursorPosition();
        lineEdit->setText(str);
        lineEdit->setCursorPosition(pos);
    }

    // Get the hex value of the string
    int regValue = 0;
    str.remove(0, 2);
    if (str.length() > 0) {
        bool ok;
        regValue = str.toInt(&ok, 16);
    }
    else {
        // Exactly "0x" remains, so do nothing
        return;
    }


}

void CpuPane::regTextFinishedEditing()
{
    QLineEdit *lineEdit = qobject_cast<QLineEdit *>(sender());

    QString str = lineEdit->text();
    //qDebug() << "str: " << str;

    // Get the hex value of the string
    int regValue = 0;
    bool ok;
    regValue = str.toInt(&ok, 16);

    //qDebug() << "reg val: " << regValue;
    if (lineEdit == cpuPaneItems->aRegLineEdit) {
        emit registerChanged(0, static_cast<quint8>(regValue / 256));
        emit registerChanged(1, static_cast<quint8>(regValue % 256));
    }
    else if (lineEdit == cpuPaneItems->xRegLineEdit) {
        emit registerChanged(2, static_cast<quint8>(regValue / 256));
        emit registerChanged(3, static_cast<quint8>(regValue % 256));
    }
    else if (lineEdit == cpuPaneItems->spRegLineEdit) {
        emit registerChanged(4, static_cast<quint8>(regValue / 256));
        emit registerChanged(5, static_cast<quint8>(regValue % 256));
    }
    else if (lineEdit == cpuPaneItems->pcRegLineEdit) {
        emit registerChanged(6, static_cast<quint8>(regValue / 256));
        emit registerChanged(7, static_cast<quint8>(regValue % 256));
    }
    else if (lineEdit == cpuPaneItems->irRegLineEdit) {
        emit registerChanged(8,static_cast<quint8>(regValue / 65536));
        emit registerChanged(9, static_cast<quint8>(regValue / 256));
        emit registerChanged(10,static_cast<quint8>(regValue % 256));

    }
    else if (lineEdit == cpuPaneItems->t1RegLineEdit) {
        emit registerChanged(11, static_cast<quint8>(regValue % 256));
    }
    else if (lineEdit == cpuPaneItems->t2RegLineEdit) {
        emit registerChanged(12, static_cast<quint8>(regValue / 256));
        emit registerChanged(13, static_cast<quint8>(regValue % 256));
    }
    else if (lineEdit == cpuPaneItems->t3RegLineEdit) {
        emit registerChanged(14, static_cast<quint8>(regValue / 256));
        emit registerChanged(15, static_cast<quint8>(regValue % 256));
    }
    else if (lineEdit == cpuPaneItems->t4RegLineEdit) {
        emit registerChanged(16, static_cast<quint8>(regValue / 256));
        emit registerChanged(17, static_cast<quint8>(regValue % 256));
    }
    else if (lineEdit == cpuPaneItems->t5RegLineEdit) {
        emit registerChanged(18, static_cast<quint8>(regValue / 256));
        emit registerChanged(19, static_cast<quint8>(regValue % 256));
    }
    else if (lineEdit == cpuPaneItems->t6RegLineEdit) {
        emit registerChanged(20, static_cast<quint8>(regValue / 256));
        emit registerChanged(21, static_cast<quint8>(regValue % 256));
    }
    if (lineEdit == cpuPaneItems->irRegLineEdit) {
        lineEdit->setText(QString("0x%1").arg(regValue, 6, 16, QLatin1Char('0')).toUpper());
    }
    else if (lineEdit == cpuPaneItems->t1RegLineEdit) {
        lineEdit->setText(QString("0x") + QString("%1").arg(regValue, 2, 16, QLatin1Char('0')).toUpper());
    }
    else {
        lineEdit->setText(QString("0x") + QString("%1").arg(regValue, 4, 16, QLatin1Char('0')).toUpper());
    }

}

void CpuPane::zoomFactorChanged(int factor)
{
    ui->graphicsView->scale(factor * .01, factor * .01);
    ui->graphicsView->update();
}

void CpuPane::labelClicked()
{
    using namespace Pep9::uarch;
    TristateLabel *label = qobject_cast<TristateLabel *>(sender());
    label->toggle();
    QString temp="";
    quint8 tempVal=0;
    bool hadCtrl = false, hadStatus = false;
    // Must initialize the below values to 0, otherwise static analyzer
    // becomes upset. This values will only be used if their associated
    // boolean flag is true, but I can't pass this hint to the compiler.
    EControlSignals control = EControlSignals::A;
    auto status = EStatusBit::STATUS_C;
    if(label == cpuPaneItems->aMuxTristateLabel){
        temp = cpuPaneItems->aMuxTristateLabel->text();
        control = EControlSignals::AMux;
        hadCtrl = true;
    }
    else if(label == cpuPaneItems->cMuxTristateLabel) {
        temp = cpuPaneItems->cMuxTristateLabel->text();
        control= EControlSignals::CMux;
        hadCtrl = true;
    }
    else if(label == cpuPaneItems->CSMuxTristateLabel) {
        temp = cpuPaneItems->CSMuxTristateLabel->text();
        control = EControlSignals::CSMux;
        hadCtrl = true;
    }
    else if(label == cpuPaneItems->AndZTristateLabel) {
        temp = cpuPaneItems->AndZTristateLabel->text();
        control = EControlSignals::AndZ;
        hadCtrl = true;
    }
    else if(label == cpuPaneItems->MemWriteTristateLabel) {
        if(cpuPaneItems->MemReadTristateLabel->text().isEmpty()){
            temp = cpuPaneItems->MemWriteTristateLabel->text();
            control= EControlSignals::MemWrite;
            hadCtrl = true;
        }
        else {
            cpuPaneItems->MemWriteTristateLabel->setState(-1);
            return;
        }

    }
    else if(label == cpuPaneItems->MemReadTristateLabel) {
        if(cpuPaneItems->MemWriteTristateLabel->text().isEmpty()){
            temp = cpuPaneItems->MemReadTristateLabel->text();
            control = EControlSignals::MemRead;
            hadCtrl = true;
        }
        else {
            cpuPaneItems->MemWriteTristateLabel->setState(-1);
            return;
        }
    }
    else if(label == cpuPaneItems->MDRMuxTristateLabel) {
        temp = cpuPaneItems->MDRMuxTristateLabel->text();
        control = EControlSignals::MDRMux;
        hadCtrl = true;
    }
    else if(label == cpuPaneItems->MDREMuxTristateLabel) {
        temp = cpuPaneItems->MDREMuxTristateLabel->text();
        control = EControlSignals::MDREMux;
        hadCtrl = true;
    }
    else if(label == cpuPaneItems->MDROMuxTristateLabel) {
        temp = cpuPaneItems->MDROMuxTristateLabel->text();
        control = EControlSignals::MDROMux;
        hadCtrl = true;
    }
    else if(label == cpuPaneItems->EOMuxTristateLabel) {
        temp = cpuPaneItems->EOMuxTristateLabel->text();
        control = EControlSignals::EOMux;
        hadCtrl = true;
    }
    else if(label == cpuPaneItems->nBitLabel) {
        status = EStatusBit::STATUS_N;
        temp = cpuPaneItems->nBitLabel->text();
        hadStatus = true;
    }
    else if(label == cpuPaneItems->zBitLabel) {
        status = EStatusBit::STATUS_Z;
        temp = cpuPaneItems->zBitLabel->text();
        hadStatus = true;
    }
    else if(label == cpuPaneItems->vBitLabel) {
        status = EStatusBit::STATUS_V;
        temp = cpuPaneItems->vBitLabel->text();
        hadStatus = true;
    }
    else if(label == cpuPaneItems->cBitLabel){
        status = EStatusBit::STATUS_C;
        temp = cpuPaneItems->cBitLabel->text();
        hadStatus = true;
    }
    else if(label == cpuPaneItems->sBitLabel) {
        status = EStatusBit::STATUS_S;
        temp = cpuPaneItems->sBitLabel->text();
        hadStatus = true;
    }
    if(temp == "0") tempVal = 0;
    else if(temp == "1") tempVal = 1;
    else tempVal = PepCore::signalDisabled;
    if(hadCtrl)dataSection->onSetControlSignal(control,tempVal);
    else if(hadStatus)dataSection->onSetStatusBit(status,tempVal);
}

void CpuPane::clockButtonPushed()
{
    QString errorString;
    cpu->onClock();
    if (dataSection->hadErrorOnStep()) {
        // simulation had issues.
        QMessageBox::warning(nullptr, "Pep/9", dataSection->getErrorMessage());
    }
    scene->invalidate();
    clearCpuControlSignals();
}

void CpuPane::on_copyToMicrocodePushButton_clicked() // union of all models
{
    using namespace Pep9::uarch;

    MicroCode code(dataSection->getCPUType(), false);
    if (cpuPaneItems->loadCk->isChecked()) {
        code.setClockSingal(EClockSignals::LoadCk, 1);
    }
    if (cpuPaneItems->cLineEdit->text() != "") {
        code.setControlSignal(EControlSignals::C, static_cast<quint8>(cpuPaneItems->cLineEdit->text().toInt()));
    }
    if (cpuPaneItems->bLineEdit->text() != "") {
        code.setControlSignal(EControlSignals::B, static_cast<quint8>(cpuPaneItems->bLineEdit->text().toInt()));
    }
    if (cpuPaneItems->aLineEdit->text() != "") {
        code.setControlSignal(EControlSignals::A, static_cast<quint8>(cpuPaneItems->aLineEdit->text().toInt()));
    }
    if (cpuPaneItems->MARCk->isChecked()) {
        code.setClockSingal(EClockSignals::MARCk, 1);
    }
    if (cpuPaneItems->aMuxTristateLabel->text() != "") {
        code.setControlSignal(EControlSignals::AMux, static_cast<quint8>(cpuPaneItems->aMuxTristateLabel->text().toInt()));
    }
    if (cpuPaneItems->cMuxTristateLabel->text() != "") {
        code.setControlSignal(EControlSignals::CMux, static_cast<quint8>(cpuPaneItems->cMuxTristateLabel->text().toInt()));
    }
    if (cpuPaneItems->ALULineEdit->text() != "") {
        code.setControlSignal(EControlSignals::ALU, static_cast<quint8>(cpuPaneItems->ALULineEdit->text().toInt()));
    }
    if (cpuPaneItems->CSMuxTristateLabel->text() != "") {
        code.setControlSignal(EControlSignals::CSMux, static_cast<quint8>(cpuPaneItems->CSMuxTristateLabel->text().toInt()));
    }
    if (cpuPaneItems->SCkCheckBox->isChecked()) {
        code.setClockSingal(EClockSignals::SCk, 1);
    }
    if (cpuPaneItems->CCkCheckBox->isChecked()) {
        code.setClockSingal(EClockSignals::CCk, 1);
    }
    if (cpuPaneItems->VCkCheckBox->isChecked()) {
        code.setClockSingal(EClockSignals::VCk, 1);
    }
    if (cpuPaneItems->AndZTristateLabel->text() != "") {
        code.setControlSignal(EControlSignals::AndZ, static_cast<quint8>(cpuPaneItems->AndZTristateLabel->text().toInt()));
    }
    if (cpuPaneItems->ZCkCheckBox->isChecked()) {
        code.setClockSingal(EClockSignals::ZCk, 1);
    }
    if (cpuPaneItems->NCkCheckBox->isChecked()) {
        code.setClockSingal(EClockSignals::NCk, 1);
    }
    if (cpuPaneItems->MemReadTristateLabel->text() != "") {
        code.setControlSignal(EControlSignals::MemRead, static_cast<quint8>(cpuPaneItems->MemReadTristateLabel->text().toInt()));
    }
    if (cpuPaneItems->MemWriteTristateLabel->text() != "") {
        code.setControlSignal(EControlSignals::MemWrite, static_cast<quint8>(cpuPaneItems->MemWriteTristateLabel->text().toInt()));
    }
    if (cpu->getCPUType() == PepCore::CPUType::TwoByteDataBus &&
            cpuPaneItems->MARMuxTristateLabel->text() != "") { // 2 byte bus
        code.setControlSignal(EControlSignals::MARMux, static_cast<quint8>(cpuPaneItems->MARMuxTristateLabel->text().toInt()));
    }

    // 1 byte exclusive controls.
    if (cpu->getCPUType() == PepCore::CPUType::OneByteDataBus &&
            cpuPaneItems->MDRCk->isChecked()) { // 1 byte bus
        code.setClockSingal(EClockSignals::MDRCk, 1);
    }
    if (cpu->getCPUType() == PepCore::CPUType::OneByteDataBus &&
            cpuPaneItems->MDRMuxTristateLabel->text() != "") { // 1 byte bus
        code.setControlSignal(EControlSignals::MDRMux, static_cast<quint8>(cpuPaneItems->MDRMuxTristateLabel->text().toInt()));
    }

    // 2 byte exclusive controls.
    if (cpu->getCPUType() == PepCore::CPUType::TwoByteDataBus &&
            cpuPaneItems->MARMuxTristateLabel->text() != "") { // 2 byte bus
        code.setControlSignal(EControlSignals::MARMux, static_cast<quint8>(cpuPaneItems->MARMuxTristateLabel->text().toInt()));
    }
    if (cpu->getCPUType() == PepCore::CPUType::TwoByteDataBus &&
            cpuPaneItems->MDROCk->isChecked()) { // 2 byte bus
        code.setClockSingal(EClockSignals::MDROCk, 1);
    }
    if (cpu->getCPUType() == PepCore::CPUType::TwoByteDataBus &&
            cpuPaneItems->MDROMuxTristateLabel->text() != "") { // 2 byte bus
        code.setControlSignal(EControlSignals::MDROMux, static_cast<quint8>(cpuPaneItems->MDROMuxTristateLabel->text().toInt()));
    }
    if (cpu->getCPUType() == PepCore::CPUType::TwoByteDataBus &&
            cpuPaneItems->MDRECk->isChecked()) { // 2 byte bus
        code.setClockSingal(EClockSignals::MDRECk, 1);
    }
    if (cpu->getCPUType() == PepCore::CPUType::TwoByteDataBus &&
            cpuPaneItems->MDREMuxTristateLabel->text() != "") { // 2 byte bus
        code.setControlSignal(EControlSignals::MDREMux, static_cast<quint8>(cpuPaneItems->MDREMuxTristateLabel->text().toInt()));
    }
    if (cpu->getCPUType() == PepCore::CPUType::TwoByteDataBus &&
            cpuPaneItems->EOMuxTristateLabel->text() != "") { // 2 byte bus
        code.setControlSignal(EControlSignals::EOMux, static_cast<quint8>(cpuPaneItems->EOMuxTristateLabel->text().toInt()));
    }
    code.setBranchFunction(EBranchFunctions::Assembler_Assigned);
    emit appendMicrocodeLine(code.getSourceCode());
}

void CpuPane::ALUTextEdited(QString str)
{
    using namespace Pep9::uarch;

    if (str == "") {
        cpuPaneItems->ALUFunctionLabel->setText("");
    }
    else {
        int num = str.toInt();
        dataSection->onSetControlSignal(EControlSignals::ALU, num);
        //
        switch (num) {
        case 0:
            cpuPaneItems->ALUFunctionLabel->setText("A");
            break;
        case 1:
            cpuPaneItems->ALUFunctionLabel->setText("A plus B");
            break;
        case 2:
            cpuPaneItems->ALUFunctionLabel->setText("A plus B plus Cin");
            break;
        case 3:
            cpuPaneItems->ALUFunctionLabel->setText("A plus \u00AC B plus 1");
            break;
        case 4:
            cpuPaneItems->ALUFunctionLabel->setText("A plus \u00AC B plus Cin");
            break;
        case 5:
            cpuPaneItems->ALUFunctionLabel->setText("A \u00b7 B");
            break;
        case 6:
            cpuPaneItems->ALUFunctionLabel->setText("\u00AC (A \u00b7 B)");
            break;
        case 7:
            cpuPaneItems->ALUFunctionLabel->setText("A + B");
            break;
        case 8:
            cpuPaneItems->ALUFunctionLabel->setText("\u00AC (A + B)");
            break;
        case 9:
            cpuPaneItems->ALUFunctionLabel->setText("A XOR B");
            break;
        case 10:
            cpuPaneItems->ALUFunctionLabel->setText("\u00AC A");
            break;
        case 11:
            cpuPaneItems->ALUFunctionLabel->setText("ASL A");
            break;
        case 12:
            cpuPaneItems->ALUFunctionLabel->setText("ROL A");
            break;
        case 13:
            cpuPaneItems->ALUFunctionLabel->setText("ASR A");
            break;
        case 14:
            cpuPaneItems->ALUFunctionLabel->setText("ROR A");
            break;
        case 15:
            cpuPaneItems->ALUFunctionLabel->setText("NZVC A");
            break;
        default:
            break;
        }
    }
}

void CpuPane::onClockChanged()
{
    using namespace Pep9::uarch;
    QCheckBox* send = qobject_cast<QCheckBox*>(sender());
    if(send==cpuPaneItems->NCkCheckBox) {
        dataSection->onSetClock(EClockSignals::NCk,cpuPaneItems->NCkCheckBox->checkState());
    }
    else if(send==cpuPaneItems->ZCkCheckBox) {
        dataSection->onSetClock(EClockSignals::ZCk,cpuPaneItems->ZCkCheckBox->checkState());
    }
    else if(send==cpuPaneItems->VCkCheckBox) {
        dataSection->onSetClock(EClockSignals::VCk,cpuPaneItems->VCkCheckBox->checkState());
    }
    else if(send==cpuPaneItems->CCkCheckBox) {
        dataSection->onSetClock(EClockSignals::CCk,cpuPaneItems->CCkCheckBox->checkState());
    }
    else if(send==cpuPaneItems->SCkCheckBox) {
        dataSection->onSetClock(EClockSignals::SCk,cpuPaneItems->SCkCheckBox->checkState());
    }
    else if(send==cpuPaneItems->MARCk) {
        dataSection->onSetClock(EClockSignals::MARCk,cpuPaneItems->MARCk->checkState());
    }
    else if(send==cpuPaneItems->MDRCk) {
        dataSection->onSetClock(EClockSignals::MDRCk,cpuPaneItems->MDRCk->checkState());
    }
    else if(send==cpuPaneItems->MDRECk) {
        dataSection->onSetClock(EClockSignals::MDRECk,cpuPaneItems->MDRECk->checkState());
    }
    else if(send==cpuPaneItems->MDROCk) {
        dataSection->onSetClock(EClockSignals::MDROCk,cpuPaneItems->MDROCk->checkState());
    }
    else if(send==cpuPaneItems->loadCk) {
        dataSection->onSetClock(EClockSignals::LoadCk,cpuPaneItems->loadCk->checkState());
    }
}

void CpuPane::onBusChanged()
{
    using namespace Pep9::uarch;

    QLineEdit* bus = qobject_cast<QLineEdit*>(sender());
    quint8 val;
    if(bus==cpuPaneItems->aLineEdit)
    {
        val = static_cast<quint8>(cpuPaneItems->aLineEdit->text().toInt());
        dataSection->onSetControlSignal(EControlSignals::A,val);
    }
    else if(bus==cpuPaneItems->bLineEdit)
    {
        val = static_cast<quint8>(cpuPaneItems->bLineEdit->text().toInt());
        dataSection->onSetControlSignal(EControlSignals::B,val);
    }
    else if(bus==cpuPaneItems->cLineEdit)
    {
        val = static_cast<quint8>(cpuPaneItems->cLineEdit->text().toInt());
        dataSection->onSetControlSignal(EControlSignals::C,val);
    }
}

void CpuPane::onRegisterChanged(quint8 which, quint8 , quint8 newVal)
{
    setRegisterByte(which, newVal);
}

void CpuPane::onMemoryRegisterChanged(Pep9::uarch::EMemoryRegisters reg, quint8, quint8 newVal)
{
    QLatin1Char x = QLatin1Char('0');
    switch(reg){
    case Pep9::uarch::EMemoryRegisters::MEM_MARA:
        cpuPaneItems->MARALabel->setText("0x" + QString("%1").arg(newVal, 2, 16, x).toUpper());
        break;
    case Pep9::uarch::EMemoryRegisters::MEM_MARB:
        cpuPaneItems->MARBLabel->setText("0x" + QString("%1").arg(newVal, 2, 16, x).toUpper());
        break;
    case Pep9::uarch::EMemoryRegisters::MEM_MDR:
        cpuPaneItems->MDRLabel->setText("0x" + QString("%1").arg(newVal, 2, 16, x).toUpper());
        break;
    case Pep9::uarch::EMemoryRegisters::MEM_MDRE:
        cpuPaneItems->MDRELabel->setText("0x" + QString("%1").arg(newVal, 2, 16, x).toUpper());
        break;
    case Pep9::uarch::EMemoryRegisters::MEM_MDRO:
        cpuPaneItems->MDROLabel->setText("0x" + QString("%1").arg(newVal, 2, 16, x).toUpper());
        break;
    }
}

void CpuPane::onStatusBitChanged(Pep9::uarch::EStatusBit bit, bool value)
{
    using namespace Pep9::uarch;

    switch(bit){
    case EStatusBit::STATUS_N:
        setStatusBit(ECPUKeywords::N,value);
        break;
    case EStatusBit::STATUS_Z:
        setStatusBit(ECPUKeywords::Z,value);
        break;
    case EStatusBit::STATUS_V:
        setStatusBit(ECPUKeywords::V,value);
        break;
    case EStatusBit::STATUS_C:
        setStatusBit(ECPUKeywords::Cbit,value);
        break;
    case EStatusBit::STATUS_S:
        setStatusBit(ECPUKeywords::S,value);
        break;
    default:
        break;
    }
}

void CpuPane::repaintOnScroll(int distance)
{
    (void)distance; //Ugly fix to get compiler to silence unused variable warning
    cpuPaneItems->update();
}

void CpuPane::onSimulationUpdate()
{
    using namespace Pep9::uarch;

    setRegister(ECPUKeywords::Acc, dataSection->getRegisterBankWord(to_uint8_t(CPURegisters::A)));
    setRegister(ECPUKeywords::X, dataSection->getRegisterBankWord(to_uint8_t(CPURegisters::X)));
    setRegister(ECPUKeywords::SP, dataSection->getRegisterBankWord(to_uint8_t(CPURegisters::SP)));
    setRegister(ECPUKeywords::PC, dataSection->getRegisterBankWord(to_uint8_t(CPURegisters::PC)));
    setRegister(ECPUKeywords::IR, static_cast<int>(dataSection->getRegisterBankByte(to_uint8_t(CPURegisters::IS))<<16) +
                dataSection->getRegisterBankWord(to_uint8_t(CPURegisters::OS)));
    setRegister(ECPUKeywords::T1, dataSection->getRegisterBankByte(to_uint8_t(CPURegisters::T1)));
    setRegister(ECPUKeywords::T2, dataSection->getRegisterBankWord(to_uint8_t(CPURegisters::T2)));
    setRegister(ECPUKeywords::T3, dataSection->getRegisterBankWord(to_uint8_t(CPURegisters::T3)));
    setRegister(ECPUKeywords::T4, dataSection->getRegisterBankWord(to_uint8_t(CPURegisters::T4)));
    setRegister(ECPUKeywords::T5, dataSection->getRegisterBankWord(to_uint8_t(CPURegisters::T5)));
    setRegister(ECPUKeywords::T6, dataSection->getRegisterBankWord(to_uint8_t(CPURegisters::T6)));
    setRegister(ECPUKeywords::MARAREG, dataSection->getMemoryRegister(Pep9::uarch::EMemoryRegisters::MEM_MARA));
    setRegister(ECPUKeywords::MARBREG, dataSection->getMemoryRegister(Pep9::uarch::EMemoryRegisters::MEM_MARB));
    setRegister(ECPUKeywords::MDRREG, dataSection->getMemoryRegister(Pep9::uarch::EMemoryRegisters::MEM_MDR));
    setRegister(ECPUKeywords::MDROREG, dataSection->getMemoryRegister(Pep9::uarch::EMemoryRegisters::MEM_MDRO));
    setRegister(ECPUKeywords::MDREREG, dataSection->getMemoryRegister(Pep9::uarch::EMemoryRegisters::MEM_MDRE));
    setStatusBit(ECPUKeywords::N, dataSection->getStatusBit(EStatusBit::STATUS_N));
    setStatusBit(ECPUKeywords::Z, dataSection->getStatusBit(EStatusBit::STATUS_Z));
    setStatusBit(ECPUKeywords::V, dataSection->getStatusBit(EStatusBit::STATUS_V));
    setStatusBit(ECPUKeywords::Cbit, dataSection->getStatusBit(EStatusBit::STATUS_C));
    setStatusBit(ECPUKeywords::S, dataSection->getStatusBit(EStatusBit::STATUS_S));
    const AMicroCode *code = cpu->getCurrentMicrocodeLine();
    if(auto* as_microcode = dynamic_cast<const MicroCode*>(code);
             as_microcode != nullptr) {
        cpuPaneItems->setCPULabels(as_microcode);
    }

    ui->graphicsView->invalidateScene();
}

void CpuPane::onSimulationFinished()
{
    // Update any registers changed since start.
    onSimulationUpdate();
    clearCpuControlSignals();
    // Create a dummy microcode line that will reset CPU pane.
    // The CPU pane will never render control section signals,
    // but set the boolean flag to true just in case.
    const MicroCode code(cpu->getCPUType(), true);
    cpuPaneItems->setCPULabels(&code);
    cpuPaneItems->update();

}

void CpuPane::onDarkModeChanged(bool darkMode, QString styleSheet)
{
    cpuPaneItems->darkModeChanged(darkMode, styleSheet);
    if(darkMode) {
        ui->graphicsView->setBackgroundBrush(QBrush(PepColors::darkMode.backgroundFill));
    } else {
        ui->graphicsView->setBackgroundBrush(QBrush(PepColors::lightMode.backgroundFill));
    }

    ui->graphicsView->invalidateScene();
}

void CpuPane::onCPUTypeChanged()
{
    type = cpu->getCPUType();
    cpuPaneItems->CPUTypeChanged(type);
    repaint();

}




