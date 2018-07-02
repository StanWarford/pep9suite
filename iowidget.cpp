#include "iowidget.h"
#include "ui_iowidget.h"

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
