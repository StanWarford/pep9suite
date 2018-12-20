#include "addrmodedialog.h"
#include "ui_addrmodedialog.h"
#include "addrmodewidget.h"
AddrModeDialog::AddrModeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddrModeDialog)
{
    ui->setupUi(this);
}

AddrModeDialog::~AddrModeDialog()
{
    delete ui;
}

void AddrModeDialog::addAddrMode(QString addrMode, QString addrSymbol)
{
    AddrModeWidget* newWidget = new AddrModeWidget(addrMode,addrSymbol,this);
    ui->gridLayout->addWidget(newWidget,ui->gridLayout->rowCount(),0);
}

void AddrModeDialog::clearAddrModes()
{
    //Todo
}

QList<QPair<QString, QString> > AddrModeDialog::allAddrModes()
{
    return QList<QPair<QString,QString>>();
}
