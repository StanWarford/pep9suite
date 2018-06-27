#include "addrmodewidget.h"
#include "ui_addrmodewidget.h"

AddrModeWidget::AddrModeWidget(QString addrMode, QString addrSymbol, QWidget *parent): QWidget(parent),
ui(new Ui::AddrModeWidget)
{
    ui->setupUi(this);
    setAddrMode(addrMode);
    setAddrSymbol(addrSymbol);
}


AddrModeWidget::AddrModeWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AddrModeWidget)
{
    ui->setupUi(this);
}

AddrModeWidget::~AddrModeWidget()
{
    delete ui;
}

void AddrModeWidget::setAddrMode(QString addrMode)
{
    ui->addrModeLabel->setText(addrMode);
}

void AddrModeWidget::setAddrSymbol(QString addrSymbol)
{
    ui->addrModeSymbolEdit->setText(addrSymbol);
}

QString AddrModeWidget::addrMode() const
{
    return ui->addrModeLabel->text();
}

QString AddrModeWidget::addrSymbol() const
{
    return ui->addrModeSymbolEdit->text();
}

