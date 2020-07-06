// File: iowidget.cpp
/*
    The Pep/9 suite of applications (Pep9, Pep9CPU, Pep9Micro) are
    simulators for the Pep/9 virtual machine, and allow users to
    create, simulate, and debug across various levels of abstraction.

    Copyright (C) 2018  J. Stanley Warford & Matthew McRaven, Pepperdine University

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
#include "iowidget.h"
#include "ui_iowidget.h"

#include <QDebug>
#include <QString>

#include "io/inputpane.h"
#include "io/outputpane.h"
#include "io/terminalpane.h"
#include "memory/mainmemory.h"
#include "pep/enu.h"

#include <QSplitter>
IOWidget::IOWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::IOWidget), charInAddr(0), charOutAddr(0), activePane(1)
{
    ui->setupUi(this);
    activePane =  ui->tabWidget->indexOf(ui->batchIOTab);
    ui->batchInput->setFocusProxy(this);
    ui->batchOutput->setFocusProxy(this);
    ui->terminalIO->setFocusProxy(this);
    connect(ui->batchInput, &InputPane::undoAvailable, this, &IOWidget::onSetUndoability);
    connect(ui->batchInput, &InputPane::redoAvailable, this, &IOWidget::onSetRedoability);
    connect(ui->terminalIO, &TerminalPane::undoAvailable, this, &IOWidget::onSetUndoability);
    connect(ui->terminalIO, &TerminalPane::redoAvailable, this, &IOWidget::onSetRedoability);
    QSplitter *split = new QSplitter(nullptr);
    split->setOrientation(Qt::Vertical);
    split->addWidget(ui->batchInput);
    split->addWidget(ui->batchOutput);
    ui->gridLayout_1->addWidget(split);
}

IOWidget::~IOWidget()
{
    delete ui;
}

void IOWidget::setBatchInput(QString text)
{
    ui->batchInput->setText(text);
}

void IOWidget::setActivePane(Enu::EPane pane)
{
    if(pane == Enu::EPane::EBatchIO) {
        ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->batchIOTab));
    }
    else if(pane == Enu::EPane::ETerminal) {
        ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->terminalIOTab));
    }
}

bool IOWidget::inBatchMode() const
{
    return activePane == batch_index;
}

bool IOWidget::inInteractiveMode() const
{
    return activePane == terminal_index;
}

void IOWidget::cancelWaiting()
{
    switch(activePane)
    {
    case terminal_index:
        ui->terminalIO->cancelWaiting();
        break;
    default:
        break;
    }
}

void IOWidget::setInputChipAddress(quint16 address)
{
    charInAddr = address;
}

void IOWidget::setOutputChipAddress(quint16 address)
{
    charOutAddr = address;
}

void IOWidget::bindToMemorySection(MainMemory *memory)
{
    connect(ui->terminalIO, &TerminalPane::inputReady, this, &IOWidget::onInputReady);
    // Multiple versions of this slot exist, so the following function cast must be used to select the correct one
    connect(this, &IOWidget::inputReady, memory,  static_cast<void (MainMemory::*)(quint16, quint8)>(&MainMemory::onInputReceived));
    this->memory = memory;
}

bool IOWidget::isUndoable() const
{
    if(ui->batchInput->hasFocus()) {
        return ui->batchInput->isUndoable();
    }
    else if(ui->terminalIO->hasFocus()) {
        return ui->terminalIO->isUndoable();
    }
    return false;
}

bool IOWidget::isRedoable() const
{
    if(ui->batchInput->hasFocus()) {
        return ui->batchInput->isUndoable();
    }
    else if(ui->terminalIO->hasFocus()) {
        return ui->terminalIO->isRedoable();
    }
    return false;
}

int IOWidget::editActions() const
{
    int ret = Enu::EditButton::COPY;
    if(ui->batchInput->hasFocus()) {
        ret |= Enu::EditButton::CUT | Enu::EditButton::PASTE;
        ret |= Enu::EditButton::REDO * ui->batchInput->isRedoable() | Enu::EditButton::UNDO * ui->batchInput->isUndoable();
    }
    else if(ui->terminalIO->hasFocus()) {
        ret |= Enu::EditButton::CUT | Enu::EditButton::PASTE;
        ret |= Enu::EditButton::REDO * ui->terminalIO->isRedoable() | Enu::EditButton::UNDO * ui->terminalIO->isUndoable();
    }
    return ret;
}

void IOWidget::highlightOnFocus()
{
    // We don't know which widget is gaining or losing focus,
    // so just re-highlight all of them.
    ui->batchInput->highlightOnFocus();
    ui->batchOutput->highlightOnFocus();
    ui->terminalIO->highlightOnFocus();
}

void IOWidget::onClear()
{
    ui->batchOutput->clearText();
    ui->terminalIO->clearTerminal();
}

void IOWidget::onFontChanged(QFont font)
{
    ui->batchInput->onFontChanged(font);
    ui->batchOutput->onFontChanged(font);
    ui->terminalIO->onFontChanged(font);
}

void IOWidget::copy() const
{
    if(ui->batchInput->hasFocus()) {
        ui->batchInput->copy();
    }
    else if(ui->batchOutput->hasFocus()) {
        ui->batchOutput->copy();
    }
    else if(ui->terminalIO->hasFocus()) {
        ui->terminalIO->copy();
    }
}

void IOWidget::cut()
{
    if(ui->batchInput->hasFocus()) {
        ui->batchInput->cut();
    }
    else if(ui->terminalIO->hasFocus()) {
        ui->terminalIO->cut();
    }
}

void IOWidget::paste()
{
    if(ui->batchInput->hasFocus()) {
        ui->batchInput->paste();
    }
    else if(ui->terminalIO->hasFocus()) {
        ui->terminalIO->paste();
    }
}

void IOWidget::undo()
{
    if(ui->batchInput->hasFocus()) {
        ui->batchInput->undo();
    }
    else if(ui->terminalIO->hasFocus()) {
        ui->terminalIO->undo();
    }
}

void IOWidget::redo()
{
    if(ui->batchInput->hasFocus()) {
        ui->batchInput->redo();
    }
    else if(ui->terminalIO->hasFocus()) {
        ui->terminalIO->redo();
    }
}

void IOWidget::onSetRedoability(bool b)
{
    if(ui->batchInput->hasFocus()) {
        emit redoAvailable(b);
    }
    else if(ui->batchOutput->hasFocus()) {
        emit redoAvailable(false);
    }
    else if(ui->terminalIO->hasFocus()) {
        emit redoAvailable(b);
    }
    else emit redoAvailable(false);
}

void IOWidget::onSetUndoability(bool b)
{
    if(ui->batchInput->hasFocus()) {
        emit undoAvailable(b);
    }
    else if(ui->batchOutput->hasFocus()) {
        emit undoAvailable(false);
    }
    else if(ui->terminalIO->hasFocus()) {
        emit undoAvailable(b);
    }
    else emit undoAvailable(false);
}

void IOWidget::onInputReady(QString value)
{
    for(QChar ch : value) {
        emit inputReady(charInAddr, static_cast<quint8>(ch.toLatin1()));
    }
}

void IOWidget::onOutputReceived(quint16 address, QChar data)
{
    // IOWidget only keeps track of output concerning charOut. If the address is
    // something other than charOut, then it is not to be displayed.
    if(address != charOutAddr) {
        return;
    }

    switch(activePane)
    {
    case batch_index:
        ui->batchOutput->appendOutput(QString(data));
        break;
    case terminal_index:
        ui->terminalIO->appendOutput(QString(data));
        break;
    default:
        break;
    }
}

void IOWidget::onDataRequested(quint16 address)
{
    // If the memory mapped output is not coming from terminal output chip, ignore the event
    // since this widget is only concerned with I/O.
    if(address != charInAddr) {
        return;
    }
    switch(activePane)
    {
    case batch_index:
        //If there's no input for the memory, there never will be.
        //So, let the simulation begin to error and unwind.
        memory->onInputAborted(charInAddr);
        break;
    case terminal_index:
        ui->terminalIO->waitingForInput();
        break;
    default:
        break;
    }
}

void IOWidget::onSimulationStart()
{
    // Cache whether batch or terminal IO was selected when the simulation started,
    // to avoid bugs where the user switches tabs mid simulation.
    activePane = ui->tabWidget->currentIndex();
    switch(activePane)
    {
    case batch_index:
        // When the simulation starts, pass all needed input to memory's input buffer.
        // Append \n as an input terminator
        memory->onInputReceived(charInAddr, ui->batchInput->toPlainText().append('\n'));
        break;
    default:
        break;
    }
}
