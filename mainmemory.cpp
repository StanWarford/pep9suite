// File: mainmemory.cpp
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

#include "mainmemory.h"
#include "ui_mainmemory.h"
#include "pep.h"

#include <QScrollBar>
#include <QResizeEvent>
#include "cpudatasection.h"
#include "colors.h"
#include <QDebug>

MainMemory::MainMemory(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainMemory),modifiedAddresses(),dataSection(CPUDataSection::getInstance()),
    darkMode(false),colors(&PepColors::lightMode)
{
    ui->setupUi(this);

    ui->tableWidget->setFont(QFont(Pep::codeFont, Pep::codeFontSize));
    ui->tableWidget->setColumnCount(1);
    QStringList columns;
    columns << "Hex";
    ui->tableWidget->setHorizontalHeaderLabels(columns);

    ui->tableWidget->setRowCount(1);
    oldRowCount = 1;

    rows << QString("0000");
    ui->tableWidget->setVerticalHeaderLabels(rows);

    int address = 0x0000;
    ui->tableWidget->setItem(0, 0, new QTableWidgetItem("0x" + QString("%1").arg(dataSection->getMemoryByte(address), 2, 16).toUpper().trimmed()));

    refreshMemory();

    ui->tableWidget->resize(ui->tableWidget->size());
    populateMemoryItems();

    connect(ui->verticalScrollBar, SIGNAL(actionTriggered(int)), this, SLOT(sliderMoved(int)));
    connect(ui->tableWidget, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(cellDataChanged(QTableWidgetItem*)));
    connect(ui->lineEdit, SIGNAL(textChanged(QString)), this, SLOT(scrollToChanged(QString)));
    connect(dataSection,&CPUDataSection::memoryChanged,this,&MainMemory::onMemoryValueChanged);
    ui->scrollToLabel->setFont(QFont(ui->scrollToLabel->font().family(), ui->scrollToLabel->font().pointSize()));
    ui->lineEdit->setFont(QFont(ui->lineEdit->font().family(), ui->lineEdit->font().pointSize()));

    ui->tableWidget->setFont(QFont(Pep::labelFont, Pep::labelFontSize));

    ui->tableWidget->viewport()->installEventFilter(this);
}

MainMemory::~MainMemory()
{
    delete ui;
}

void MainMemory::populateMemoryItems()
{
    // disconnect this signal so that modifying the text of the column next to it doesn't fire this signal; reconnect at the end
    disconnect(ui->tableWidget, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(cellDataChanged(QTableWidgetItem*)));

    rows.clear();

    //qDebug() << "scroll value: " << QString("%1").arg(ui->verticalScrollBar->value(), 4, 16, QLatin1Char('0'));
    int scrollBarValue = ui->verticalScrollBar->value();
    for (int i = scrollBarValue; i < scrollBarValue + ui->tableWidget->rowCount(); i++) {
        rows << QString("%1").arg(i, 4, 16, QLatin1Char('0')).toUpper();
    }
    ui->tableWidget->setVerticalHeaderLabels(rows);

    connect(ui->tableWidget, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(cellDataChanged(QTableWidgetItem*)));
    refreshMemory();
}

void MainMemory::refreshMemory()
{
    // disconnect this signal so that modifying the text of the column next to it doesn't fire this signal; reconnect at the end
    disconnect(ui->tableWidget, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(cellDataChanged(QTableWidgetItem*)));

    bool ok;
    int address;

    for (int i = 0; i < ui->tableWidget->rowCount(); i++) {
        address = ui->tableWidget->verticalHeaderItem(i)->text().toInt(&ok, 16);
        if (ok) {
            ui->tableWidget->item(i, 0)->setText("0x" +
                                                 QString("%1").arg(dataSection->getMemoryByte(address), 2, 16, QLatin1Char('0')).toUpper());
        }
    }

    connect(ui->tableWidget, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(cellDataChanged(QTableWidgetItem*)));
}

void MainMemory::setMemAddress(int memAddress, int value)
{
    // disconnect this signal so that modifying the text of the column next to it doesn't fire this signal; reconnect at the end
    disconnect(ui->tableWidget, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(cellDataChanged(QTableWidgetItem*)));
    modifiedAddresses.insert(memAddress);
    if (memAddress > 0xffff || memAddress < 0) {
        qDebug() << "invalid address: " << memAddress;
    }
    if (value > 255) {
        value = value & 255;
        qDebug() << "setMemAddr of num larger than 255: val: " << value << ", addr: " << memAddress;
    }

    int firstAddress = ui->tableWidget->verticalHeaderItem(0)->text().toInt();
    int lastAddress = firstAddress + ui->tableWidget->rowCount()-1;

    if (memAddress < firstAddress || lastAddress < memAddress) {
        return;
    }

    ui->tableWidget->item(memAddress-firstAddress, 0)->setText("0x" + QString("%1").arg(value, 2, 16, QLatin1Char('0')).toUpper().trimmed());
    hightlightModifiedBytes();
    connect(ui->tableWidget, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(cellDataChanged(QTableWidgetItem*)));
}

void MainMemory::clearMemory()
{
    for (int i = 0; i < ui->tableWidget->rowCount(); i++) {
        ui->tableWidget->item(i, 0)->setText("0x00");
    }
    modifiedAddresses.clear();
    hightlightModifiedBytes();
    // CPU Data section clears its own memory
}

void MainMemory::showMemEdited(int address)
{
    scrollToAddress(address);
    populateMemoryItems();
    hightlightModifiedBytes();
}

void MainMemory::hightlightModifiedBytes()
{
    // disconnect this signal so that modifying the text of the column next to it doesn't fire this signal; reconnect at the end
    disconnect(ui->tableWidget, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(cellDataChanged(QTableWidgetItem*)));
        // clear all highlighted cells
    for (int i = 0; i < ui->tableWidget->rowCount(); i++) {
        ui->tableWidget->item(i,0)->setBackgroundColor(PepColors::transparent);
    }
    // for each item in the table:
    for(int i=0;i<ui->tableWidget->rowCount();i++)
    {
        bool ok;
        int j = ui->tableWidget->verticalHeaderItem(i)->text().right(4).toInt(&ok, 16);
        if(ok&&modifiedAddresses.contains(j))
        {
            ui->tableWidget->item(i,0)->setBackgroundColor(colors->memoryHighlight);
        }
    }


    connect(ui->tableWidget, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(cellDataChanged(QTableWidgetItem*)));
}

void MainMemory::scrollToAddress(int address)
{
    // disconnect this signal so that modifying the text of the column next to it doesn't fire this signal; reconnect at the end
    //disconnect(ui->tableWidget, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(cellDataChanged(QTableWidgetItem*)));

    if (address >= 0 && address <= 0xffff) { // defensive programming!
        if (address > ui->verticalScrollBar->maximum()) { // ensure we only scroll to the bottom, and don't show values larger than 0xffff
            ui->verticalScrollBar->setValue(ui->verticalScrollBar->maximum());
        }
        else { // simple case:
            ui->verticalScrollBar->setValue(address);
        }
    }
    // else, ignore, we're getting told to do something out of the correct range.

    //connect(ui->tableWidget, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(cellDataChanged(QTableWidgetItem*)));
    hightlightModifiedBytes();
}

void MainMemory::highlightOnFocus()
{
    if (ui->tableWidget->hasFocus()) {
        ui->label->setAutoFillBackground(true);
    }
    else {
        ui->label->setAutoFillBackground(false);
    }
}

void MainMemory::onMemoryValueChanged(quint16 address, quint8, quint8 newVal)
{
    setMemAddress(address,newVal);
    populateMemoryItems();
    scrollToAddress(address);
}

bool MainMemory::hasFocus()
{
    return ui->tableWidget->hasFocus();
}

void MainMemory::onDarkModeChange(bool darkMode)
{
    if(darkMode)
    {
        colors = &PepColors::darkMode;
    }
    else
    {
        colors = &PepColors::lightMode;
    }
    populateMemoryItems();
    hightlightModifiedBytes();
}

void MainMemory::sliderMoved(int pos)
{
    qDebug() << "slider moved: " << pos;
    populateMemoryItems();
    hightlightModifiedBytes();
}

void MainMemory::cellDataChanged(QTableWidgetItem *item)
{
    // disconnect this signal so that modifying the text of the column next to it doesn't fire this signal; reconnect at the end
    disconnect(ui->tableWidget, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(cellDataChanged(QTableWidgetItem*)));
    disconnect(dataSection,&CPUDataSection::memoryChanged,this,&MainMemory::onMemoryValueChanged);

    int row = item->row();
    QString contents = item->text();
    contents = contents.trimmed();
    QRegExp rx = item->column() == 0 ? QRegExp("(0x)?[0-9a-fA-F]+") : QRegExp("[0-9a-fA-F]+");

    bool addrConvOk;
    bool dataOk;
    int base = contents.startsWith("0x", Qt::CaseInsensitive) ? 16 : 10;
    int address = ui->tableWidget->verticalHeaderItem(row)->text().toInt(&addrConvOk, 16);
    int data = item->text().toInt(&dataOk, base);
    data = data % 256;

    if (contents.contains(rx) && dataOk && addrConvOk) {
        dataSection->onSetMemoryByte(address,(quint8)data);
        qDebug() << "Sim::Mem[" << address << "]: " << data;
        ui->tableWidget->item(row, 0)->setText("0x" + QString("%1").arg(data, 2, 16, QLatin1Char('0')).toUpper().trimmed());
    }
    else if (addrConvOk && !dataOk) {
        qDebug() << "Conversion from text to int failed. data = " << item->text();
        data = dataSection->getMemoryByte(address);
        ui->tableWidget->item(row, 0)->setText("0x" + QString("%1").arg(data, 2, 16, QLatin1Char('0')).toUpper().trimmed());
    }
    else if (addrConvOk) { // we have problems, the labels are incorrectly formatted
        populateMemoryItems(); //ui->verticalScrollBar->value());
    }

    connect(ui->tableWidget, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(cellDataChanged(QTableWidgetItem*)));
    connect(dataSection,&CPUDataSection::memoryChanged,this,&MainMemory::onMemoryValueChanged);
}

void MainMemory::scrollToChanged(QString string)
{
    bool ok;
    int byte;
    if (string.startsWith("0x", Qt::CaseInsensitive)) {
        byte = string.toInt(&ok, 16);
        if (ok) {
            if (byte > 65535) {
                ui->lineEdit->setText("0xFFFF");
            } else {
                ui->verticalScrollBar->setValue(byte);
                sliderMoved(0);
            }
        }
        else {
            ui->lineEdit->setText("0x");
        }
    }
    else {
        ui->lineEdit->setText("0x");
    }

    // make sure the cells are correctly highlighted
    hightlightModifiedBytes();
}

void MainMemory::changeEvent(QEvent *e)
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

void MainMemory::resizeEvent(QResizeEvent *)
{
    int newRowCount = ui->tableWidget->height()/ui->tableWidget->rowHeight(0) - 1;

    // Set the maximum row count to be 64k - (num visible rows)
    ui->verticalScrollBar->setMaximum(ui->verticalScrollBar->maximum() - newRowCount + 1);

    // make sure the scroll bar stays at the bottom when resizing
    if (ui->verticalScrollBar->value() > ui->verticalScrollBar->maximum() - newRowCount) {
        ui->verticalScrollBar->setValue(ui->verticalScrollBar->maximum()); //0xffff - newRowCount);
    }

    if (newRowCount > oldRowCount) {
        ui->tableWidget->setRowCount(newRowCount);

        bool addrConvOk;
        int address;

        for (int i = oldRowCount; i < newRowCount; i++) {
            rows << QString("%1").arg(i, 4, 16, QLatin1Char('0')).toUpper();
        }
        ui->tableWidget->setVerticalHeaderLabels(rows);

        disconnect(ui->tableWidget, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(cellDataChanged(QTableWidgetItem*)));

        for (int row = oldRowCount; row < newRowCount; row++) {
            address = ui->tableWidget->verticalHeaderItem(row)->text().toInt(&addrConvOk, 16);
            if (addrConvOk) {
                ui->tableWidget->setItem(row, 0, new QTableWidgetItem("0x" + QString("%1").arg(dataSection->getMemoryByte(address), 2, 16).toUpper().trimmed()));
                //ui->tableWidget->itemAt(row, 0)->setFont(QFont(Pep::codeFont, Pep::codeFontSize));
            }
            else { // malformed address labels
            }
        }

        connect(ui->tableWidget, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(cellDataChanged(QTableWidgetItem*)));

        refreshMemory();
    }
    else if (oldRowCount > newRowCount) {
        ui->tableWidget->setRowCount(newRowCount);
        for (int i = oldRowCount; i > newRowCount; i--) {
            delete ui->tableWidget->item(i, 0);
            rows.removeLast();
        }
        refreshMemory();
    }
}

bool MainMemory::eventFilter(QObject *, QEvent *e)
{
    if (e->type() == QEvent::Wheel) {

        qApp->sendEvent(ui->verticalScrollBar, e);
        return true;
    }
    return false;
}


