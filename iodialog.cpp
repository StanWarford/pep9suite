#include "iodialog.h"
#include "ui_iodialog.h"

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
