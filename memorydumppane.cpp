// File: memorydumppane.cpp
/*
    Pep9 is a virtual machine for writing machine language and assembly
    language programs.
    
    Copyright (C) 2018  J. Stanley Warford, Pepperdine University

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
#include <QDebug>
#include <QFontDialog>
#include <QTextCharFormat>
#include <QAbstractTextDocumentLayout>
#include "memorydumppane.h"
#include "ui_memorydumppane.h"
#include "pep.h"
#include "enu.h"
#include <QStyle>
#include "cpudatasection.h"
#include "memorysection.h"
#include "colors.h"
MemoryDumpPane::MemoryDumpPane(QWidget *parent) :
    QWidget(parent), data(new QStandardItemModel(this)), lineSize(320), colors(&PepColors::lightMode),
    ui(new Ui::MemoryDumpPane), inSimulation(false)
{
    ui->setupUi(this);
    if (Pep::getSystem() != "Mac") {
        ui->label->setFont(QFont(Pep::labelFont, Pep::labelFontSize));
        ui->tableView->setFont(QFont(Pep::codeFont, Pep::codeFontSize-1));
    }
    data->insertColumns(0, 1+8+1);
    data->insertRows(0, (1<<16)/8);
    for(int it = 0; it < (1<<16) /8; it++)
    {
        data->setData(data->index(it, 0), QString("0x%1").arg(it*8, 4, 16, QChar('0')));
    }
    ui->tableView->setModel(data);
    ui->tableView->resizeRowsToContents();
    connect(ui->pcPushButton, &QAbstractButton::clicked, this, &MemoryDumpPane::scrollToPC);
    connect(ui->spPushButton, &QAbstractButton::clicked, this, &MemoryDumpPane::scrollToSP);
    connect(ui->scrollToLineEdit, &QLineEdit::textChanged, this, &MemoryDumpPane::scrollToAddress);
}

void MemoryDumpPane::init(MemorySection *memory, CPUDataSection *data)
{
    this->memorySection = memory;
    this->dataSection = data;
    refreshMemory();
}

MemoryDumpPane::~MemoryDumpPane()
{
    delete ui;
    delete data;
}

void MemoryDumpPane::refreshMemory()
{
    bool updates = ui->tableView->updatesEnabled();
    ui->tableView->setUpdatesEnabled(false);
    QString memoryDumpLine;
    QChar ch;
    for(int row = 0; row < (1<<16)/8; row++) {
        memoryDumpLine.clear();
        for(int col = 0; col < 8; col++) {
            data->setData(data->index(row, col + 1), QString("%1").arg(memorySection->getMemoryByte(row*8 + col, false), 2, 16, QChar('0')));
            ch = QChar(memorySection->getMemoryByte(row*8 + col, false));
            if (ch.isPrint()) {
                memoryDumpLine.append(ch);
            } else {
                memoryDumpLine.append(".");
            }
        }
        data->setData(data->index(row, 1+8), memoryDumpLine);
    }
    ui->tableView->setUpdatesEnabled(updates);
    clearHighlight();
    //lineSize = tempSize > 10?tempSize:10;
    ui->tableView->resizeColumnsToContents();
}

void MemoryDumpPane::refreshMemoryLines(quint16 firstByte, quint16 lastByte)
{
    quint16 firstLine = firstByte / 8;
    quint16 lastLine = lastByte / 8;

    bool updates = ui->tableView->updatesEnabled();
    ui->tableView->setUpdatesEnabled(false);
    QString memoryDumpLine;
    QChar ch;
    for(int row = firstLine; row <= lastLine; row++) {
        memoryDumpLine = "";
        for(int col = 0; col < 8; col++) {
            data->setData(data->index(row, col + 1), QString("%1").arg(memorySection->getMemoryByte(row*8 + col, false), 2, 16, QChar('0')));
            ch = QChar(memorySection->getMemoryByte(row*8 + col, false));
            if (ch.isPrint()) {
                memoryDumpLine.append(ch);
            } else {
                memoryDumpLine.append(".");
            }
        }
        data->setData(data->index(row, 1+8), memoryDumpLine);
    }
    ui->tableView->setUpdatesEnabled(updates);
    //quint32 tempSize = ui->tableView->fontMetrics().width(memoryDumpLine);
    //lineSize = tempSize > 10?tempSize:10;
}

void MemoryDumpPane::clearHighlight()
{

    while (!highlightedData.isEmpty()) {
        quint16 address = highlightedData.takeFirst();
        QStandardItem *item = data->item(address/8, address%8 +1);
        item->setData(QVariant(),Qt::BackgroundRole);
        item->setData(QVariant(), Qt::ForegroundRole);
    }
}

void MemoryDumpPane::highlight()
{

    highlightByte(dataSection->getRegisterBankWord(CPURegisters::SP), colors->altTextHighlight, colors->memoryHighlightSP);
    highlightedData.append(dataSection->getRegisterBankWord(CPURegisters::SP));
    quint16 programCounter = dataSection->getRegisterBankWord(CPURegisters::PC);
    if(!Pep::isUnaryMap[Pep::decodeMnemonic[memorySection->getMemoryByte(programCounter,false)]]) {
        for(int it = 0; it < 3; it++) {
            highlightByte(dataSection->getRegisterBankWord(CPURegisters::PC)+it, colors->altTextHighlight, colors->memoryHighlightPC);
            highlightedData.append(dataSection->getRegisterBankWord(CPURegisters::PC)+it);
        }
    }
    else {
        highlightByte(dataSection->getRegisterBankWord(CPURegisters::PC), colors->altTextHighlight, colors->memoryHighlightPC);
        highlightedData.append(dataSection->getRegisterBankWord(CPURegisters::PC));
    }
    for(quint16 byte : lastModifiedBytes) {
        highlightByte(byte, colors->altTextHighlight, colors->memoryHighlightChanged);
        highlightedData.append(byte);
    }

}

void MemoryDumpPane::cacheModifiedBytes()
{
    QSet<quint16> tempConstruct, tempDestruct = memorySection->modifiedBytes();
    modifiedBytes = memorySection->modifiedBytes();
    memorySection->clearModifiedBytes();
    lastModifiedBytes = memorySection->writtenLastCycle();
    for(quint16 val : tempDestruct) {
        if(tempConstruct.contains(val)) continue;
        for(quint16 temp = val - val%8;temp%8<=7;temp++) {
            tempConstruct.insert(temp);
        }
        refreshMemoryLines(val - val%8,val - val%8 + 1);
    }
    /*modifiedBytes.unite(Sim::modifiedBytes);
    if (Sim::tracingTraps) {
        bytesWrittenLastStep.clear();
        bytesWrittenLastStep = Sim::modifiedBytes.toList();
    }
    else if (Sim::trapped) {
        delayLastStepClear = true;
        bytesWrittenLastStep.append(Sim::modifiedBytes.toList());
    }
    else if (delayLastStepClear) {
        delayLastStepClear = false;
    }
    else {
        bytesWrittenLastStep.clear();
        bytesWrittenLastStep = Sim::modifiedBytes.toList();
    }*/
}

void MemoryDumpPane::updateMemory()
{
    QList<quint16> list;
    QSet<quint16> linesToBeUpdated;
    modifiedBytes.unite(memorySection->modifiedBytes());
    memorySection->clearModifiedBytes();
    lastModifiedBytes = memorySection->writtenLastCycle();
    list = modifiedBytes.toList();
    while(!list.isEmpty()) {
        linesToBeUpdated.insert(list.takeFirst() / 8);
    }
    list = linesToBeUpdated.toList();
    qSort(list.begin(), list.end());
    for(auto x: list)
    {
        refreshMemoryLines(x*8, x*8);
    }
    modifiedBytes.clear();
    ui->tableView->resizeColumnsToContents();
}

void MemoryDumpPane::scrollToTop()
{
    ui->tableView->scrollToTop();
}

void MemoryDumpPane::highlightOnFocus()
{
    if (ui->tableView->hasFocus() || ui->scrollToLineEdit->hasFocus()) {
        ui->label->setAutoFillBackground(true);
    }
    else {
        ui->label->setAutoFillBackground(false);
    }
}

bool MemoryDumpPane::hasFocus()
{
    return ui->tableView->hasFocus() || ui->scrollToLineEdit->hasFocus();
}

void MemoryDumpPane::copy()
{
#pragma message("TODO: Figure out copy mechanics on memory pane")
    //ui->textEdit->copy();
}

int MemoryDumpPane::memoryDumpWidth()
{
    quint32 tableSize = lineSize;
    quint32 extraPad = 15;
    return  tableSize + extraPad;
}

void MemoryDumpPane::onFontChanged(QFont font)
{
    ui->tableView->setFont(font);
}

void MemoryDumpPane::onDarkModeChanged(bool darkMode)
{
    if(darkMode) colors = &PepColors::darkMode;
    else colors = &PepColors::lightMode;
    if(inSimulation) {
        clearHighlight();
        highlight();
    }
}

void MemoryDumpPane::onMemoryChanged(quint16 address, quint8, quint8)
{
    this->refreshMemoryLines(address,address);
}

void MemoryDumpPane::onSimulationStarted()
{
    inSimulation = true;
}

void MemoryDumpPane::onSimulationFinished()
{
    inSimulation = false;
}

void MemoryDumpPane::highlightByte(quint16 memAddr, QColor foreground, QColor background)
{
    QModelIndex index = data->index(memAddr/8, memAddr%8 +1);
    data->setData(index,foreground,Qt::ForegroundRole);
    data->setData(index,background,Qt::BackgroundRole);
}

void MemoryDumpPane::mouseReleaseEvent(QMouseEvent *)
{
    ui->tableView->setFocus();
}

void MemoryDumpPane::scrollToByte(quint16 byte)
{
    quint32 min = ui->tableView->verticalScrollBar()->minimum();
    quint32 max = ui->tableView->verticalScrollBar()->maximum();
    ui->tableView->scrollTo(data->index(byte/8, byte%8 +1));
}

void MemoryDumpPane::scrollToPC()
{
    ui->scrollToLineEdit->setText(QString("0x") + QString("%1").arg(dataSection->getRegisterBankWord(CPURegisters::PC), 4, 16, QLatin1Char('0')).toUpper());
}

void MemoryDumpPane::scrollToSP()
{
    ui->scrollToLineEdit->setText(QString("0x") + QString("%1").arg(dataSection->getRegisterBankWord(CPURegisters::SP), 4, 16, QLatin1Char('0')).toUpper());
}

void MemoryDumpPane::scrollToAddress(QString string)
{
    bool ok;
    int byte;
    if (string.startsWith("0x", Qt::CaseInsensitive)) {
        byte = string.toInt(&ok, 16);
        if (ok) {
            if (byte > 65535) {
                ui->scrollToLineEdit->setText("0xFFFF");
            } else {
                scrollToByte(byte);
            }
        }
        else {
            ui->scrollToLineEdit->setText("0x");
        }
    }
    else {
        ui->scrollToLineEdit->setText("0x");
    }
}
