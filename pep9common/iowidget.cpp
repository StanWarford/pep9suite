#include "iowidget.h"
#include "ui_iowidget.h"
#include <QString>
#include <QDebug>
#include "enu.h"
#include "mainmemory.h"
IOWidget::IOWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::IOWidget), iChipAddr(0), oChipAddr(0)
{
    ui->setupUi(this);
    ui->batchInput->setFocusProxy(this);
    ui->batchOutput->setFocusProxy(this);
    ui->terminalIO->setFocusProxy(this);
    connect(ui->batchInput, &InputPane::undoAvailable, this, &IOWidget::onSetUndoability);
    connect(ui->batchInput, &InputPane::redoAvailable, this, &IOWidget::onSetRedoability);
    connect(ui->terminalIO, &TerminalPane::undoAvailable, this, &IOWidget::onSetUndoability);
    connect(ui->terminalIO, &TerminalPane::redoAvailable, this, &IOWidget::onSetRedoability);
}

IOWidget::~IOWidget()
{
    delete ui;
}

void IOWidget::cancelWaiting()
{
    switch(ui->tabWidget->currentIndex())
    {
    case 1:
        ui->terminalIO->cancelWaiting();
        break;
    default:
        break;
    }
}

void IOWidget::setInputChipAddress(quint16 address)
{
    iChipAddr = address;
}

void IOWidget::setOutputChipAddress(quint16 address)
{
    oChipAddr = address;
}

void IOWidget::bindToMemorySection(MainMemory *memory)
{
    connect(ui->terminalIO, &TerminalPane::inputReady, this, &IOWidget::onInputReady);
    // Multiple versions of this slot exist, so the following function cast must be used to select the correct one
    connect(this, &IOWidget::inputReady, memory,  static_cast<void (MainMemory::*)(quint16, quint8)>(&MainMemory::onInputReceived));
    //connect(memory, &MainMemory::inputRequested, this, &IOWidget::onDataRequested);
    //connect(memory, &MainMemory::outputWritten, this, &IOWidget::onDataReceived);
    this->memory = memory;
}

void IOWidget::batchInputToBuffer()
{
    if(ui->tabWidget->currentIndex() == 0)
    {
        // Make sure no additional IO is waiting before queueing more
        memory->clearIO();
        memory->onInputReceived(iChipAddr, ui->batchInput->toPlainText().append('\n'));
    }
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
        emit inputReady(iChipAddr, static_cast<quint8>(ch.toLatin1()));
    }
}

void IOWidget::onDataReceived(quint16 address, QChar data)
{
    // If the memory mapped output is not coming from terminal output chip, ignore the event
    if(address != oChipAddr) {
        return;
    }
    QString oData = QString::number(static_cast<quint8>(data.toLatin1()), 16).leftJustified(2,'0')+" ";
    //qDebug()<<called++;
    switch(ui->tabWidget->currentIndex())
    {
    case 0:
        ui->batchOutput->appendOutput(QString(data));
        break;
    case 1:
        ui->terminalIO->appendOutput(QString(data));
        break;
    default:
        break;
    }
}

void IOWidget::onDataRequested(quint16 /*address*/)
{
    switch(ui->tabWidget->currentIndex())
    {
    case 0:
        //If there's no input for the memory, there never will be.
        //So, let the simulation begin to error and unwind.
        memory->onInputCanceled(iChipAddr);
        break;
    case 1:
        ui->terminalIO->waitingForInput();
        break;
    default:
        break;
    }
}


void IOWidget::onSimulationStart()
{
    switch(ui->tabWidget->currentIndex())
    {
    case 0:
        //When the simulation starts, pass all needed input to memory's input buffer
        memory->onInputReceived(iChipAddr, ui->batchInput->toPlainText().append('\n'));
        break;
    default:
        break;
    }
}

