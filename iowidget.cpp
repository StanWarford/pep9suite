#include "iowidget.h"
#include "ui_iowidget.h"
#include "memorysection.h"
#include "mainwindow.h"
#include <QString>
#include <QDebug>
IOWidget::IOWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::IOWidget)
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

void IOWidget::bindToMemorySection(MemorySection *memory)
{
    connect(ui->terminalIO, &TerminalPane::inputReady, memory,& MemorySection::onAppendInBuffer);
    connect(memory, &MemorySection::charRequestedFromInput, this, &IOWidget::onDataRequested);
    connect(memory, &MemorySection::charWrittenToOutput, this, &IOWidget::onDataReceived);
    this->memory = memory;
}

void IOWidget::batchInputToBuffer()
{
    if(ui->tabWidget->currentIndex() == 0)
    {
        memory->onAppendInBuffer(ui->batchInput->toPlainText()+'\n');
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

const int IOWidget::editActions() const
{
    int ret = EditButtons::COPY;
    if(ui->batchInput->hasFocus()) {
        ret |= EditButtons::CUT | EditButtons::PASTE;
        ret |= EditButtons::REDO * ui->batchInput->isRedoable() | EditButtons::UNDO * ui->batchInput->isUndoable();
    }
    else if(ui->terminalIO->hasFocus()) {
        ret |= EditButtons::CUT | EditButtons::PASTE;
        ret |= EditButtons::REDO * ui->terminalIO->isRedoable() | EditButtons::UNDO * ui->terminalIO->isUndoable();
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

void IOWidget::onDataReceived(QChar data)
{
    QString oData = QString::number((quint8)data.toLatin1(),16).leftJustified(2,'0')+" ";
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

void IOWidget::onDataRequested()
{
    switch(ui->tabWidget->currentIndex())
    {
    case 0:
        //If there's no input for the memory, there never will be.
        //So, let the simulation begin to error and unwind.
        memory->onCancelWaiting();
        break;
    case 1:
        ui->terminalIO->waitingForInput();
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
        memory->onAppendInBuffer(ui->batchInput->toPlainText().append('\n'));
        break;
    default:
        break;
    }
}

