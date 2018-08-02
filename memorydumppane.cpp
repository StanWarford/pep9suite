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
MemoryDumpPane::MemoryDumpPane(QWidget *parent) :
    QWidget(parent), lineSize(0),
    ui(new Ui::MemoryDumpPane)
{
    ui->setupUi(this);
    ui->textEdit->setReadOnly(true);
    if (Pep::getSystem() != "Mac") {
        ui->label->setFont(QFont(Pep::labelFont, Pep::labelFontSize));
        ui->textEdit->setFont(QFont(Pep::codeFont, Pep::codeFontSize));
    }

    connect(ui->pcPushButton, SIGNAL(clicked()), this, SLOT(scrollToPC()));
    connect(ui->spPushButton, SIGNAL(clicked()), this, SLOT(scrollToSP()));
    connect(ui->scrollToLineEdit, SIGNAL(textChanged(QString)), this, SLOT(scrollToAddress(QString)));
}

void MemoryDumpPane::init(MemorySection *memory, CPUDataSection *data)
{
    this->memory = memory;
    this->data=data;
}

MemoryDumpPane::~MemoryDumpPane()
{
    delete ui;
}

void MemoryDumpPane::refreshMemory()
{
    QStringList memoryDump;
    QString memoryDumpLine;
    QChar ch;
    const QVector<quint8> values = memory->getMemory();
    for (int i = 0; i < 65536; i += 8) {
        memoryDumpLine = "";
        memoryDumpLine.append(QString("%1 | ").arg(i, 4, 16, QLatin1Char('0')).toUpper());
        for (int j = 0; j < 8; j++) {
            memoryDumpLine.append(QString("%1 ").arg(values[i + j], 2, 16, QLatin1Char('0')).toUpper());
        }
        memoryDumpLine.append("|");
        for (int j = 0; j < 8; j++) {
            ch = QChar(values[i + j]);
            if (ch.isPrint()) {
                memoryDumpLine.append(ch);
            } else {
                memoryDumpLine.append(".");
            }
        }
        memoryDump.append(memoryDumpLine);
    }
    quint32 tempSize = ui->textEdit->fontMetrics().width(memoryDumpLine);
    lineSize = tempSize > 10?tempSize:10;
    ui->textEdit->setText(memoryDump.join("\n"));
}

void MemoryDumpPane::refreshMemoryLines(quint16 firstByte, quint16 lastByte)
{
    int vertScrollBarPosition = ui->textEdit->verticalScrollBar()->value();
    int horizScrollBarPosition = ui->textEdit->horizontalScrollBar()->value();

    quint16 firstLine = firstByte / 8;
    quint16 lastLine = lastByte / 8;

    QTextCursor cursor(ui->textEdit->document());
    cursor.setPosition(0);
    for (quint16 i = 0; i < firstLine; i++) {
        cursor.movePosition(QTextCursor::NextBlock);
    }

    QString memoryDumpLine;
    QChar ch;
    quint16 byteNum;
    const QVector<quint8> values = memory->getMemory();
    for (quint16 i = firstLine; i <= lastLine; i++) {
        memoryDumpLine = "";
        byteNum = i * 8;
        memoryDumpLine.append(QString("%1 | ").arg(byteNum, 4, 16, QLatin1Char('0')).toUpper());
        for (quint16 j = 0; j < 8; j++) {
            memoryDumpLine.append(QString("%1 ").arg(values[byteNum++], 2, 16, QLatin1Char('0')).toUpper());
        }
        memoryDumpLine.append("|");
        byteNum = i * 8;
        for (quint16 j = 0; j < 8; j++) {
            ch = QChar(values[byteNum++]);
            if (ch.isPrint()) {
                memoryDumpLine.append(ch);
            } else {
                memoryDumpLine.append(".");
            }
        }
        cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
        ui->textEdit->setTextCursor(cursor);
        ui->textEdit->insertPlainText(memoryDumpLine);
        cursor.movePosition(QTextCursor::NextBlock);
    }
    quint32 tempSize = ui->textEdit->fontMetrics().width(memoryDumpLine);
    lineSize = tempSize > 10?tempSize:10;
    ui->textEdit->verticalScrollBar()->setValue(vertScrollBarPosition);
    ui->textEdit->horizontalScrollBar()->setValue(horizScrollBarPosition);
}

void MemoryDumpPane::clearHighlight()
{
    while (!highlightedData.isEmpty()) {
        highlightByte(highlightedData.takeFirst(), Qt::black, Qt::white);
    }
}

void MemoryDumpPane::highlightPC_SP()
{
    highlightByte(data->getRegisterBankWord(CPURegisters::SP), Qt::white, Qt::darkMagenta);
    highlightedData.append(data->getRegisterBankWord(CPURegisters::SP));
    quint16 programCounter = data->getRegisterBankWord(CPURegisters::PC);
    if (!Pep::isUnaryMap[Pep::decodeMnemonic[memory->getMemoryByte(programCounter,false)]]) {
        QTextCursor cursor(ui->textEdit->document());
        QTextCharFormat format;
        format.setBackground(Qt::blue);
        format.setForeground(Qt::white);
        cursor.setPosition(0);
        for (int i = 0; i < programCounter / 8; i++) {
            cursor.movePosition(QTextCursor::NextBlock);
        }
        for (int i = 0; i < 7 + 3 * (programCounter % 8); i++) {
            cursor.movePosition(QTextCursor::NextCharacter);
        }
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 2);
        cursor.mergeCharFormat(format);
        highlightedData.append(programCounter);
        if (programCounter / 8 == (programCounter + 1) / 8) {
            cursor.clearSelection();
            cursor.movePosition(QTextCursor::NextCharacter);
            cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 2);
            cursor.mergeCharFormat(format);
        }
        else {
            cursor.clearSelection();
            cursor.movePosition(QTextCursor::NextBlock);
            cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, 7);
            cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 2);
            cursor.mergeCharFormat(format);
        }
        highlightedData.append(programCounter + 1);
        if ((programCounter + 1) / 8 == (programCounter + 2) / 8) {
            cursor.clearSelection();
            cursor.movePosition(QTextCursor::NextCharacter);
            cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 2);
            cursor.mergeCharFormat(format);
        }
        else {
            cursor.clearSelection();
            cursor.movePosition(QTextCursor::NextBlock);
            cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, 7);
            cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 2);
            cursor.mergeCharFormat(format);
        }
        highlightedData.append(programCounter + 2);
    }
    else { // unary.
        highlightByte(programCounter, Qt::white, Qt::blue);
        highlightedData.append(programCounter);
    }
    bytesWrittenLastStep = bytesWrittenLastStep.toSet().toList();
          qSort(bytesWrittenLastStep);
          #pragma message("Todo: prevent memory highlighting while in trap")
          /*while (!bytesWrittenLastStep.isEmpty()) {
              // This is to prevent bytes modified by the OS from being highlighted when we are not tracing traps:
              if (bytesWrittenLastStep.at(0) < Sim::readWord(Pep::dotBurnArgument - 0x7) || Sim::trapped) {
                  highlightByte(bytesWrittenLastStep.at(0), Qt::white, Qt::red);
                  highlightedData.append(bytesWrittenLastStep.takeFirst());
              }
              else {
                  return;
              }
          */
}

void MemoryDumpPane::highlightLastWritten()
{
    auto vals = memory->writtenLastCycle();
    for(quint16 byte : vals)
    {
        highlightByte(byte, Qt::white, Qt::red);
        highlightedData.append(byte);
    }
}

void MemoryDumpPane::cacheModifiedBytes()
{
    QSet<quint16> tempConstruct, tempDestruct = memory->modifiedBytes();
    modifiedBytes = memory->modifiedBytes();
    memory->clearModifiedBytes();
    for(quint16 val : tempDestruct)
    {
        if(tempConstruct.contains(val)) continue;
        for(quint16 temp = val - val%8;temp%8<=7;temp++)
        {
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
    int vertScrollBarPosition = ui->textEdit->verticalScrollBar()->value();
    int horizScrollBarPosition = ui->textEdit->horizontalScrollBar()->value();

    QList<quint16> list;
    QSet<quint16> linesToBeUpdated;
    QString memoryDumpLine;
    QChar ch;
    quint16 byteNum;
    quint16 lineNum;
    modifiedBytes.unite(memory->modifiedBytes());
    memory->clearModifiedBytes();
    list = modifiedBytes.toList();
    while(!list.isEmpty()) {
        linesToBeUpdated.insert(list.takeFirst() / 8);
    }
    list = linesToBeUpdated.toList();
    qSort(list.begin(), list.end());
    QTextCursor cursor(ui->textEdit->document());
    cursor.setPosition(0);
    lineNum = 0;
    for(auto x: list)
    {
        refreshMemoryLines(x*8, x*8+1);
    }
    /*while (!list.isEmpty()) {
        while (lineNum < list.first()) {
            cursor.movePosition(QTextCursor::NextBlock);
            lineNum++;
        }

        memoryDumpLine = "";
        byteNum = lineNum * 8;
        memoryDumpLine.append(QString("%1 | ").arg(byteNum, 4, 16, QLatin1Char('0')).toUpper());
        for (int j = 0; j < 8; j++) {
            memoryDumpLine.append(QString("%1 ").arg(memory->getMemoryByte(byteNum++,false), 2, 16, QLatin1Char('0')).toUpper());
        }
        memoryDumpLine.append("|");
        byteNum = lineNum * 8;
        for (int j = 0; j < 8; j++) {
            ch = QChar(memory->getMemoryByte(byteNum++,false));
            if (ch.isPrint()) {
                memoryDumpLine.append(ch);
            } else {
                memoryDumpLine.append(".");
            }
        }
        cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
        ui->textEdit->setTextCursor(cursor);
        ui->textEdit->insertPlainText(memoryDumpLine);
        cursor.movePosition(QTextCursor::NextBlock);
        lineNum++;
        list.removeFirst();
    }*/
    modifiedBytes.clear();

    ui->textEdit->verticalScrollBar()->setValue(vertScrollBarPosition);
    ui->textEdit->horizontalScrollBar()->setValue(horizScrollBarPosition);
}

void MemoryDumpPane::scrollToTop()
{
    ui->textEdit->verticalScrollBar()->setValue(0);
    ui->textEdit->horizontalScrollBar()->setValue(0);
}

void MemoryDumpPane::highlightOnFocus()
{
    if (ui->textEdit->hasFocus() || ui->scrollToLineEdit->hasFocus()) {
        ui->label->setAutoFillBackground(true);
    }
    else {
        ui->label->setAutoFillBackground(false);
    }
}

bool MemoryDumpPane::hasFocus()
{
    return ui->textEdit->hasFocus() || ui->scrollToLineEdit->hasFocus();
}

void MemoryDumpPane::copy()
{
    ui->textEdit->copy();
}

int MemoryDumpPane::memoryDumpWidth()
{
    quint32 a = this->lineSize;
    quint32 b = ui->textEdit->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
    quint32 c = 15;
    return  a + b + c;
}

void MemoryDumpPane::onFontChanged(QFont font)
{
    ui->textEdit->setFont(font);
}

void MemoryDumpPane::onDarkModeChanged(bool darkMode)
{
#pragma message ("TODO: handle dark mode changes gracefully")
}

void MemoryDumpPane::onMemoryChanged(quint16 address, quint8, quint8)
{
    this->refreshMemoryLines(address,address+1);
}

void MemoryDumpPane::highlightByte(quint16 memAddr, QColor foreground, QColor background)
{
    QTextCursor cursor(ui->textEdit->document());
    cursor.setPosition(0);
    for (int i = 0; i < memAddr / 8; i++) {
        cursor.movePosition(QTextCursor::NextBlock);
    }
    for (int i = 0; i < 7 + 3 * (memAddr % 8); i++) {
        cursor.movePosition(QTextCursor::NextCharacter);
    }
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 2);
    QTextCharFormat format;
    format.setBackground(background);
    format.setForeground(foreground);
    cursor.mergeCharFormat(format);
}

void MemoryDumpPane::mouseReleaseEvent(QMouseEvent *)
{
    ui->textEdit->setFocus();
}

void MemoryDumpPane::scrollToByte(quint16 byte)
{
    quint32 min = ui->textEdit->verticalScrollBar()->minimum();
    quint32 max = ui->textEdit->verticalScrollBar()->maximum();
    ui->textEdit->verticalScrollBar()->setValue(min + static_cast<quint32>(8 * (byte / 4096 - 8) + ((byte - byte % 8) / 65536.0) * (max - min)));
}

void MemoryDumpPane::scrollToPC()
{
    ui->scrollToLineEdit->setText(QString("0x") + QString("%1").arg(data->getRegisterBankWord(CPURegisters::PC), 4, 16, QLatin1Char('0')).toUpper());
}

void MemoryDumpPane::scrollToSP()
{
    ui->scrollToLineEdit->setText(QString("0x") + QString("%1").arg(data->getRegisterBankWord(CPURegisters::SP), 4, 16, QLatin1Char('0')).toUpper());
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
