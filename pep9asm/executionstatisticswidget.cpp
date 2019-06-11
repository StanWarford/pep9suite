#include "executionstatisticswidget.h"
#include "ui_executionstatisticswidget.h"
#include "pep.h"
ExecutionStatisticsWidget::ExecutionStatisticsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ExecutionStatisticsWidget), model(new QStandardItemModel(this))
{
    ui->setupUi(this);
    ui->treeView->setModel(model);
    model->setHorizontalHeaderLabels({"Instruction", "Frequency"});
}

void ExecutionStatisticsWidget::init(QSharedPointer<InterfaceISACPU> cpu, bool showCycles)
{
    this->cpu = cpu;
    if(!showCycles) {
        ui->label->hide();
        ui->lineEdit_Cycles->hide();
    }
}

ExecutionStatisticsWidget::~ExecutionStatisticsWidget()
{
    delete ui;
}

void ExecutionStatisticsWidget::onClear()
{
    ui->lineEdit_Cycles->clear();
    ui->lineEdit_Instructions->clear();
    model->removeRows(0, model->rowCount());
    // Sort by a non-existent column to prevent the "sorting arrow"
    // from appearing over unsorted data.
    ui->treeView->sortByColumn(-1);
}

void ExecutionStatisticsWidget::onSimulationStarted()
{
    onClear();
}

void ExecutionStatisticsWidget::onSimulationFinished()
{
    ui->lineEdit_Cycles->setText(QString("%1").arg(cpu->getCycleCount()));
    ui->lineEdit_Instructions->setText(QString("%1").arg(cpu->getInstructionCount()));
    fillModel(cpu->getInstructionHistogram());
    //ui->textBrowser_Histogrram->setText(cpu->getInstructionHistogram());
}

void ExecutionStatisticsWidget::fillModel(const QVector<quint32> histogram)
{
    model->removeRows(0, model->rowCount());

    int tallyIt = 0;
    Enu::EMnemonic mnemon = Enu::EMnemonic::STOP;
    QList<quint32> tally = QList<quint32>();
    QList<Enu::EMnemonic> mnemonicList;

    mnemonicList << Enu::EMnemonic::STOP;
    tally.append(0);

    for(int it = 0; it < 256; it++) {
        if(mnemon == Pep::decodeMnemonic[it]) {
            tally[tallyIt]+= histogram[it];
        }
        else {
            tally.append(histogram[it]);
            tallyIt++;
            mnemon = Pep::decodeMnemonic[it];
            mnemonicList.append(mnemon);
        }
    }
    static QMetaEnum metaenum = Enu::staticMetaObject.enumerator(Enu::staticMetaObject.indexOfEnumerator("EMnemonic"));
    for(int it = 0; it < mnemonicList.size(); it++) {
        QStandardItem* instrName = new QStandardItem(QString(metaenum.valueToKey((int) mnemonicList[it])).toLower());
        QStandardItem* instrCount = new QStandardItem();
        instrCount->setData(QVariant(tally[it]), Qt::DisplayRole);
        model->insertRow(model->rowCount(), {instrName, instrCount});
        //if(Pep::addrModesMap[])
    }

}
