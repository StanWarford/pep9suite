#ifndef EXECUTIONSTATISTICSWIDGET_H
#define EXECUTIONSTATISTICSWIDGET_H

#include <QWidget>
#include "interfaceisacpu.h"
#include <QStandardItemModel>
namespace Ui {
class ExecutionStatisticsWidget;
}

class ExecutionStatisticsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ExecutionStatisticsWidget(QWidget *parent = nullptr);
    void init(QSharedPointer<InterfaceISACPU> cpu, bool showCycles);
    ~ExecutionStatisticsWidget();

public slots:
    void onClear();
    void onSimulationStarted();
    void onSimulationFinished();

private:
    Ui::ExecutionStatisticsWidget *ui;
    QSharedPointer<InterfaceISACPU> cpu;
    QStandardItemModel* model;
    void fillModel(const QVector<quint32> histogram);
};

#endif // EXECUTIONSTATISTICSWIDGET_H
