#include "iowidget.h"
#include "ui_iowidget.h"
#include "memorysection.h"
#include <QString>
#include <QDebug>
IOWidget::IOWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::IOWidget)
{
    ui->setupUi(this);
}

IOWidget::~IOWidget()
{
    delete ui;
}

void IOWidget::bindToMemorySection(MemorySection *memory)
{
    connect(ui->terminalIO,&TerminalPane::inputReady,memory,&MemorySection::onAppendInBuffer);
    connect(memory,&MemorySection::charRequestedFromInput,this,&IOWidget::onDataRequested);
    connect(memory,&MemorySection::charWrittenToOutput,this,&IOWidget::onDataReceived);
}

void IOWidget::onDataReceived(QChar data)
{
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
        memory->onAppendInBuffer(ui->batchInput->toPlainText());
        break;
    default:
        break;
    }
}

