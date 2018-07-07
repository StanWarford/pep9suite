#include "iodialog.h"
#include "ui_iodialog.h"
#include "memorysection.h"

IODialog::IODialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::IODialog)
{
    ui->setupUi(this);
}

IODialog::~IODialog()
{
    delete ui;
}

void IODialog::bindToMemorySection(MemorySection *memory)
{
    ui->ioWidget->bindToMemorySection(memory);
}

void IODialog::batchInputToBuffer()
{
    ui->ioWidget->batchInputToBuffer();
}

void IODialog::onClear()
{
    ui->ioWidget->onClear();
}
