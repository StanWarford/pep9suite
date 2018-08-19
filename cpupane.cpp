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
#include "cpucontrolsection.h"
#include "cpudatasection.h"
#include <QCheckBox>
#include <QLineEdit>
#include <QGraphicsItem>
#include <QErrorMessage>
#include <QMessageBox>

#include <QWebEngineView>
#include <QScrollBar>

#include "tristatelabel.h"
#include "pep.h"
#include "microcode.h"
#include "mainwindow.h"

#include <QDebug>

using namespace Enu;
CpuPane::CpuPane( QWidget *parent) :
        QWidget(parent),
        controlSection(CPUControlSection::getInstance()),dataSection(CPUDataSection::getInstance()),
        ui(new Ui::CpuPane)
{
    ui->setupUi(this);
    connect(ui->spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &CpuPane::zoomFactorChanged);
    cpuPaneItems = nullptr;
    scene = new QGraphicsScene(nullptr);
    ui->graphicsView->setScene(scene);

    ui->graphicsView->setFont(QFont(Pep::cpuFont, Pep::cpuFontSize));

    initModel();

    ui->spinBox->hide();
    ui->graphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    // Give this class a larger acceptable size range, so that it will behave better in a splitter
    this->setMinimumWidth(static_cast<int>(cpuPaneItems->boundingRect().left())+100);
    this->setMaximumWidth(static_cast<int>(cpuPaneItems->boundingRect().right())+45);
}

void CpuPane::init(MainWindow *mainWindow)
{
    this->mainWindow = mainWindow;
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
    cpuPaneItems = new CpuGraphicsItems(Enu::TwoByteDataBus, ui->graphicsView, 0, scene);
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

    // Handle Windows repainting bug
    // This might have a performance penalty, so only enable it on the platform that needs it.
    #ifdef WIN32
        connect(ui->graphicsView->verticalScrollBar(), &QAbstractSlider::actionTriggered, this, &CpuPane::repaintOnScroll);
    #endif
}

void CpuPane::startDebugging()
{
    initRegisters();
    controlSection->onDebuggingStarted();
    ui->clockPushButton->setEnabled(false);
    ui->copyToMicrocodePushButton->setEnabled(false);
    const MicroCode *code = controlSection->getCurrentMicrocodeLine();
    code->setCpuLabels(cpuPaneItems);
    emit updateSimulation();
}

void CpuPane::stopDebugging()
{
    controlSection->onDebuggingFinished();
    ui->clockPushButton->setEnabled(true);
    ui->copyToMicrocodePushButton->setEnabled(true);
}

void CpuPane::setRegister(Enu::EKeywords reg, int value)
{
    switch (reg) {
    case Enu::Acc:
        cpuPaneItems->aRegLineEdit->setText("0x" + QString("%1").arg(value, 4, 16, QLatin1Char('0')).toUpper());
        break;
    case Enu::X:
        cpuPaneItems->xRegLineEdit->setText("0x" + QString("%1").arg(value, 4, 16, QLatin1Char('0')).toUpper());
        break;
    case Enu::SP:
        cpuPaneItems->spRegLineEdit->setText("0x" + QString("%1").arg(value, 4, 16, QLatin1Char('0')).toUpper());
        break;
    case Enu::PC:
        cpuPaneItems->pcRegLineEdit->setText("0x" + QString("%1").arg(value, 4, 16, QLatin1Char('0')).toUpper());
        break;
    case Enu::IR:
        cpuPaneItems->irRegLineEdit->setText("0x" + QString("%1").arg(value, 6, 16, QLatin1Char('0')).toUpper());
        break;
    case Enu::T1:
        cpuPaneItems->t1RegLineEdit->setText("0x" + QString("%1").arg(value, 2, 16, QLatin1Char('0')).toUpper());
        break;
    case Enu::T2:
        cpuPaneItems->t2RegLineEdit->setText("0x" + QString("%1").arg(value, 4, 16, QLatin1Char('0')).toUpper());
        break;
    case Enu::T3:
        cpuPaneItems->t3RegLineEdit->setText("0x" + QString("%1").arg(value, 4, 16, QLatin1Char('0')).toUpper());
        break;
    case Enu::T4:
        cpuPaneItems->t4RegLineEdit->setText("0x" + QString("%1").arg(value, 4, 16, QLatin1Char('0')).toUpper());
        break;
    case Enu::T5:
        cpuPaneItems->t5RegLineEdit->setText("0x" + QString("%1").arg(value, 4, 16, QLatin1Char('0')).toUpper());
        break;
    case Enu::T6:
        cpuPaneItems->t6RegLineEdit->setText("0x" + QString("%1").arg(value, 4, 16, QLatin1Char('0')).toUpper());
        break;
    case Enu::MARAREG:
        cpuPaneItems->MARALabel->setText("0x" + QString("%1").arg(value, 2, 16, QLatin1Char('0')).toUpper());
        break;
    case Enu::MARBREG:
        cpuPaneItems->MARBLabel->setText("0x" + QString("%1").arg(value, 2, 16, QLatin1Char('0')).toUpper());
        break;
    case Enu::MDROREG:
        cpuPaneItems->MDROLabel->setText("0x" + QString("%1").arg(value, 2, 16, QLatin1Char('0')).toUpper());
        break;
    case Enu::MDREREG:
        cpuPaneItems->MDRELabel->setText("0x" + QString("%1").arg(value, 2, 16, QLatin1Char('0')).toUpper());
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
    //Set register bank
    for(int it=0;it<22;it++){
        setRegisterByte(it,dataSection->getRegisterBankByte(it));
    }

    //Set status bits
    setStatusBit(Enu::N,dataSection->getStatusBit(Enu::STATUS_N));
    setStatusBit(Enu::Z,dataSection->getStatusBit(Enu::STATUS_Z));
    setStatusBit(Enu::V,dataSection->getStatusBit(Enu::STATUS_V));
    setStatusBit(Enu::Cbit,dataSection->getStatusBit(Enu::STATUS_C));
    setStatusBit(Enu::S,dataSection->getStatusBit(Enu::STATUS_S));
}

void CpuPane::setStatusBit(Enu::EKeywords bit, bool value)
{
    switch (bit) {
    case Enu::N:
        cpuPaneItems->nBitLabel->setText(QString("%1").arg(value ? "1" : "0"));
        break;
    case Enu::Z:
        cpuPaneItems->zBitLabel->setText(QString("%1").arg(value ? "1" : "0"));
        break;
    case Enu::V:
        cpuPaneItems->vBitLabel->setText(QString("%1").arg(value ? "1" : "0"));
        break;
    case Enu::Cbit:
        cpuPaneItems->cBitLabel->setText(QString("%1").arg(value ? "1" : "0"));
        break;
    case Enu::S:
        cpuPaneItems->sBitLabel->setText(QString("%1").arg(value ? "1" : "0"));
        break;
    default:
        break;
    }
}

void CpuPane::setRegPrecondition(Enu::EKeywords reg, int value)
{
    setRegister(reg, value);
}

void CpuPane::setStatusPrecondition(Enu::EKeywords bit, bool value)
{
    setStatusBit(bit, value);
}

void CpuPane::clearCpu()
{
    clearCpuControlSignals();
    setRegister(Enu::Acc, 0);
    setRegister(Enu::X, 0);
    setRegister(Enu::SP, 0);
    setRegister(Enu::PC, 0);
    setRegister(Enu::IR, 0);
    setRegister(Enu::T1, 0);
    setRegister(Enu::T2, 0);
    setRegister(Enu::T3, 0);
    setRegister(Enu::T4, 0);
    setRegister(Enu::T5, 0);
    setRegister(Enu::T6, 0);

    setRegister(Enu::MARAREG, 0);
    setRegister(Enu::MARBREG, 0);
    setRegister(Enu::MDRREG, 0);

    setRegister(Enu::MDREREG, 0);
    setRegister(Enu::MDROREG, 0);


    setStatusBit(Enu::S, false);
    setStatusBit(Enu::Cbit, false);
    setStatusBit(Enu::V, false);
    setStatusBit(Enu::Z, false);
    setStatusBit(Enu::N, false);
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

    // new signals:
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
        emit registerChanged(0,(quint8)((regValue)/256));
        emit registerChanged(1,(quint8)(regValue)%256);
    }
    else if (lineEdit == cpuPaneItems->xRegLineEdit) {
        emit registerChanged(2,(quint8)((regValue)/256));
        emit registerChanged(3,(quint8)(regValue)%256);
    }
    else if (lineEdit == cpuPaneItems->spRegLineEdit) {
        emit registerChanged(4,(quint8)((regValue)/256));
        emit registerChanged(5,(quint8)(regValue)%256);
    }
    else if (lineEdit == cpuPaneItems->pcRegLineEdit) {
        emit registerChanged(6,(quint8)((regValue)/256));
        emit registerChanged(7,(quint8)(regValue)%256);
    }
    else if (lineEdit == cpuPaneItems->irRegLineEdit) {
        emit registerChanged(8,(quint8)((regValue)/65536));
        emit registerChanged(9,(quint8)((regValue)/256));
        emit registerChanged(10,(quint8)(regValue)%256);

    }
    else if (lineEdit == cpuPaneItems->t1RegLineEdit) {
        emit registerChanged(11,(quint8)(regValue)%256);
    }
    else if (lineEdit == cpuPaneItems->t2RegLineEdit) {
        emit registerChanged(12,(quint8)((regValue)/256));
        emit registerChanged(3,(quint8)(regValue)%256);
    }
    else if (lineEdit == cpuPaneItems->t3RegLineEdit) {
        emit registerChanged(14,(quint8)((regValue)/256));
        emit registerChanged(15,(quint8)(regValue)%256);
    }
    else if (lineEdit == cpuPaneItems->t4RegLineEdit) {
        emit registerChanged(16,(quint8)((regValue)/256));
        emit registerChanged(17,(quint8)(regValue)%256);
    }
    else if (lineEdit == cpuPaneItems->t5RegLineEdit) {
        emit registerChanged(18,(quint8)((regValue)/256));
        emit registerChanged(19,(quint8)(regValue)%256);
    }
    else if (lineEdit == cpuPaneItems->t6RegLineEdit) {
        emit registerChanged(20,(quint8)((regValue)/256));
        emit registerChanged(21,(quint8)(regValue)%256);
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
    QMatrix matrix;
    matrix.scale(factor * .01, factor * .01);
    ui->graphicsView->setMatrix(matrix);
    ui->graphicsView->update();
}

void CpuPane::labelClicked()
{
    TristateLabel *label = qobject_cast<TristateLabel *>(sender());
    label->toggle();
    QString temp="";
    quint8 tempVal=0;
    Enu::EControlSignals control=(Enu::EControlSignals)128;
    Enu::EStatusBit status = (Enu::EStatusBit)128;
    if(label == cpuPaneItems->aMuxTristateLabel){
        temp = cpuPaneItems->aMuxTristateLabel->text();
        control= Enu::AMux;
    }

    else if(label == cpuPaneItems->cMuxTristateLabel){
        temp = cpuPaneItems->cMuxTristateLabel->text();
        control= Enu::CMux;
    }
    else if(label == cpuPaneItems->CSMuxTristateLabel){
        temp = cpuPaneItems->CSMuxTristateLabel->text();
        control= Enu::CSMux;
    }
    else if(label == cpuPaneItems->AndZTristateLabel){
        temp = cpuPaneItems->AndZTristateLabel->text();
        control= Enu::AndZ;
    }
    else if(label == cpuPaneItems->MemWriteTristateLabel){
        temp = cpuPaneItems->MemWriteTristateLabel->text();
        control= Enu::MemWrite;
    }
    else if(label == cpuPaneItems->MemReadTristateLabel){
        temp = cpuPaneItems->MemReadTristateLabel->text();
        control= Enu::MemRead;
    }
    else if(label == cpuPaneItems->MDREMuxTristateLabel){
        temp = cpuPaneItems->MDREMuxTristateLabel->text();
        control= Enu::MDREMux;
    }
    else if(label == cpuPaneItems->MDROMuxTristateLabel){
        temp = cpuPaneItems->MDROMuxTristateLabel->text();
        control= Enu::MDROMux;
    }
    else if(label == cpuPaneItems->EOMuxTristateLabel){
        temp = cpuPaneItems->EOMuxTristateLabel->text();
        control= Enu::EOMux;
    }
    else if(label == cpuPaneItems->nBitLabel){
        status = Enu::STATUS_N;
        temp = cpuPaneItems->nBitLabel->text();
    }
    else if(label == cpuPaneItems->zBitLabel){
        status = Enu::STATUS_Z;
        temp = cpuPaneItems->zBitLabel->text();
    }
    else if(label == cpuPaneItems->vBitLabel){
        status = Enu::STATUS_V;
        temp = cpuPaneItems->vBitLabel->text();
    }
    else if(label == cpuPaneItems->cBitLabel){
        status = Enu::STATUS_C;
        temp = cpuPaneItems->cBitLabel->text();
    }
    else if(label == cpuPaneItems->sBitLabel){
        status = Enu::STATUS_S;
        temp = cpuPaneItems->sBitLabel->text();
    }
    if(temp == "0") tempVal = 0;
    else if(temp == "1") tempVal = 1;
    else tempVal = Enu::signalDisabled;
    if(control!=(Enu::EControlSignals)128)dataSection->onSetControlSignal(control,tempVal);
    else if(status!=(Enu::EStatusBit)128)dataSection->onSetStatusBit(status,tempVal);
}

void CpuPane::clockButtonPushed()
{
    QString errorString;
    controlSection->onClock();
    if (controlSection->hadErrorOnStep()) {
        // simulation had issues.
        QMessageBox::warning(0, "Pep/9", controlSection->getErrorMessage());
        emit stopSimulation();
    }
    scene->invalidate();
    clearCpuControlSignals();
}

void CpuPane::on_copyToMicrocodePushButton_clicked() // union of all models
{
    MicroCode code;
    if (cpuPaneItems->loadCk->isChecked()) {
        code.setClockSingal(Enu::LoadCk, 1);
    }
    if (cpuPaneItems->cLineEdit->text() != "") {
        code.setControlSignal(Enu::C, cpuPaneItems->cLineEdit->text().toInt());
    }
    if (cpuPaneItems->bLineEdit->text() != "") {
        code.setControlSignal(Enu::B, cpuPaneItems->bLineEdit->text().toInt());
    }
    if (cpuPaneItems->aLineEdit->text() != "") {
        code.setControlSignal(Enu::A, cpuPaneItems->aLineEdit->text().toInt());
    }
    if (cpuPaneItems->MARCk->isChecked()) {
        code.setClockSingal(Enu::MARCk, 1);
    }
    if (cpuPaneItems->MARMuxTristateLabel->text() != "") { // 2 byte bus
        code.setControlSignal(Enu::MARMux, cpuPaneItems->MARMuxTristateLabel->text().toInt());
    }
    if (cpuPaneItems->aMuxTristateLabel->text() != "") {
        code.setControlSignal(Enu::AMux, cpuPaneItems->aMuxTristateLabel->text().toInt());
    }
    if (cpuPaneItems->MDROCk->isChecked()) { // 2 byte bus
        code.setClockSingal(Enu::MDROCk, 1);
    }
    if (cpuPaneItems->MDROMuxTristateLabel->text() != "") { // 2 byte bus
        code.setControlSignal(Enu::MDROMux, cpuPaneItems->MDROMuxTristateLabel->text().toInt());
    }
    if (cpuPaneItems->MDRECk->isChecked()) { // 2 byte bus
        code.setClockSingal(Enu::MDRECk, 1);
    }
    if (cpuPaneItems->MDREMuxTristateLabel->text() != "") { // 2 byte bus
        code.setControlSignal(Enu::MDREMux, cpuPaneItems->MDREMuxTristateLabel->text().toInt());
    }
    if (cpuPaneItems->EOMuxTristateLabel->text() != "") { // 2 byte bus
        code.setControlSignal(Enu::EOMux, cpuPaneItems->EOMuxTristateLabel->text().toInt());
    }
    if (cpuPaneItems->cMuxTristateLabel->text() != "") {
        code.setControlSignal(Enu::CMux, cpuPaneItems->cMuxTristateLabel->text().toInt());
    }
    if (cpuPaneItems->ALULineEdit->text() != "") {
        code.setControlSignal(Enu::ALU, cpuPaneItems->ALULineEdit->text().toInt());
    }
    if (cpuPaneItems->CSMuxTristateLabel->text() != "") {
        code.setControlSignal(Enu::CSMux, cpuPaneItems->CSMuxTristateLabel->text().toInt());
    }
    if (cpuPaneItems->SCkCheckBox->isChecked()) {
        code.setClockSingal(Enu::SCk, 1);
    }
    if (cpuPaneItems->CCkCheckBox->isChecked()) {
        code.setClockSingal(Enu::CCk, 1);
    }
    if (cpuPaneItems->VCkCheckBox->isChecked()) {
        code.setClockSingal(Enu::VCk, 1);
    }
    if (cpuPaneItems->AndZTristateLabel->text() != "") {
        code.setControlSignal(Enu::AndZ, cpuPaneItems->AndZTristateLabel->text().toInt());
    }
    if (cpuPaneItems->ZCkCheckBox->isChecked()) {
        code.setClockSingal(Enu::ZCk, 1);
    }
    if (cpuPaneItems->NCkCheckBox->isChecked()) {
        code.setClockSingal(Enu::NCk, 1);
    }
    if (cpuPaneItems->MemReadTristateLabel->text() != "") {
        code.setControlSignal(Enu::MemRead, cpuPaneItems->MemReadTristateLabel->text().toInt());
    }
    if (cpuPaneItems->MemWriteTristateLabel->text() != "") {
        code.setControlSignal(Enu::MemWrite, cpuPaneItems->MemWriteTristateLabel->text().toInt());
    }
    code.setBranchFunction(Enu::Assembler_Assigned);
    emit appendMicrocodeLine(code.getSourceCode());
}

void CpuPane::ALUTextEdited(QString str)
{
    if (str == "") {
        cpuPaneItems->ALUFunctionLabel->setText("");
    }
    else {
        int num = str.toInt();
        dataSection->onSetControlSignal(Enu::ALU,(Enu::EALUFunc)num);
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
    QCheckBox* send = qobject_cast<QCheckBox*>(sender());
    if(send==cpuPaneItems->NCkCheckBox)
    {
        dataSection->onSetClock(Enu::NCk,cpuPaneItems->NCkCheckBox->checkState());
    }
    else if(send==cpuPaneItems->ZCkCheckBox)
    {
        dataSection->onSetClock(Enu::ZCk,cpuPaneItems->ZCkCheckBox->checkState());
    }
    else if(send==cpuPaneItems->VCkCheckBox)
    {
        dataSection->onSetClock(Enu::VCk,cpuPaneItems->VCkCheckBox->checkState());
    }
    else if(send==cpuPaneItems->CCkCheckBox)
    {
        dataSection->onSetClock(Enu::CCk,cpuPaneItems->CCkCheckBox->checkState());
    }
    else if(send==cpuPaneItems->SCkCheckBox)
    {
        dataSection->onSetClock(Enu::SCk,cpuPaneItems->SCkCheckBox->checkState());
    }
    else if(send==cpuPaneItems->MARCk)
    {
        dataSection->onSetClock(Enu::MARCk,cpuPaneItems->MARCk->checkState());
    }
    else if(send==cpuPaneItems->MDRECk)
    {
        dataSection->onSetClock(Enu::MDRECk,cpuPaneItems->MDRECk->checkState());
    }
    else if(send==cpuPaneItems->MDROCk)
    {
        dataSection->onSetClock(Enu::MDROCk,cpuPaneItems->MDROCk->checkState());
    }
    else if(send==cpuPaneItems->loadCk)
    {
        dataSection->onSetClock(Enu::LoadCk,cpuPaneItems->loadCk->checkState());
    }
}

void CpuPane::onBusChanged()
{
    QLineEdit* bus = qobject_cast<QLineEdit*>(sender());
    quint8 val;
    if(bus==cpuPaneItems->aLineEdit)
    {
        val = cpuPaneItems->aLineEdit->text().toInt();
        dataSection->onSetControlSignal(Enu::A,val);
    }
    else if(bus==cpuPaneItems->bLineEdit)
    {
        val = cpuPaneItems->bLineEdit->text().toInt();
        dataSection->onSetControlSignal(Enu::B,val);
    }
    else if(bus==cpuPaneItems->cLineEdit)
    {
        val = cpuPaneItems->cLineEdit->text().toInt();
        dataSection->onSetControlSignal(Enu::C,val);
    }
}

void CpuPane::onRegisterChanged(quint8 which, quint8 , quint8 newVal)
{
    setRegisterByte(which,newVal);
}

void CpuPane::onMemoryRegisterChanged(EMemoryRegisters reg, quint8, quint8 newVal)
{
    QLatin1Char x = QLatin1Char('0');
    switch(reg){
    case Enu::MEM_MARA:
        cpuPaneItems->MARALabel->setText("0x" + QString("%1").arg(newVal, 2, 16, x).toUpper());
        break;
    case Enu::MEM_MARB:
        cpuPaneItems->MARBLabel->setText("0x" + QString("%1").arg(newVal, 2, 16, x).toUpper());
        break;
    case Enu::MEM_MDRE:
        cpuPaneItems->MDRELabel->setText("0x" + QString("%1").arg(newVal, 2, 16, x).toUpper());
        break;
    case Enu::MEM_MDRO:
        cpuPaneItems->MDROLabel->setText("0x" + QString("%1").arg(newVal, 2, 16, x).toUpper());
        break;
    }
}

void CpuPane::onStatusBitChanged(EStatusBit bit, bool value)
{
    switch(bit){
    case Enu::STATUS_N:
        setStatusBit(Enu::N,value);
        break;
    case Enu::STATUS_Z:
        setStatusBit(Enu::Z,value);
        break;
    case Enu::STATUS_V:
        setStatusBit(Enu::V,value);
        break;
    case Enu::STATUS_C:
        setStatusBit(Enu::Cbit,value);
        break;
    case Enu::STATUS_S:
        setStatusBit(Enu::S,value);
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
    setRegister(Enu::Acc, dataSection->getRegisterBankWord(CPURegisters::A));
    setRegister(Enu::X, dataSection->getRegisterBankWord(CPURegisters::X));
    setRegister(Enu::SP, dataSection->getRegisterBankWord(CPURegisters::SP));
    setRegister(Enu::PC, dataSection->getRegisterBankWord(CPURegisters::PC));
    setRegister(Enu::IR, ((quint32)dataSection->getRegisterBankByte(CPURegisters::IS)<<16) + dataSection->getRegisterBankWord(CPURegisters::OS));
    setRegister(Enu::T1, dataSection->getRegisterBankByte(CPURegisters::T1));
    setRegister(Enu::T2, dataSection->getRegisterBankWord(CPURegisters::T2));
    setRegister(Enu::T3, dataSection->getRegisterBankWord(CPURegisters::T3));
    setRegister(Enu::T4, dataSection->getRegisterBankWord(CPURegisters::T4));
    setRegister(Enu::T5, dataSection->getRegisterBankWord(CPURegisters::T5));
    setRegister(Enu::T6, dataSection->getRegisterBankWord(CPURegisters::T6));
    setRegister(Enu::MARAREG, dataSection->getMemoryRegister(Enu::MEM_MARA));
    setRegister(Enu::MARBREG, dataSection->getMemoryRegister(Enu::MEM_MARB));
    setRegister(Enu::MDROREG, dataSection->getMemoryRegister(Enu::MEM_MDRO));
    setRegister(Enu::MDREREG, dataSection->getMemoryRegister(Enu::MEM_MDRE));
    setStatusBit(Enu::N, dataSection->getStatusBit(Enu::STATUS_N));
    setStatusBit(Enu::Z, dataSection->getStatusBit(Enu::STATUS_Z));
    setStatusBit(Enu::V, dataSection->getStatusBit(Enu::STATUS_V));
    setStatusBit(Enu::Cbit, dataSection->getStatusBit(Enu::STATUS_C));
    setStatusBit(Enu::S, dataSection->getStatusBit(Enu::STATUS_S));
    const MicroCodeBase *code = controlSection->getCurrentMicrocodeLine();
    code->setCpuLabels(cpuPaneItems);
    update();
}

void CpuPane::onDarkModeChanged(bool darkMode)
{
    cpuPaneItems->onDarkModeChanged(darkMode);
    ui->graphicsView->invalidateScene();
}




