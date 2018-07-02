#ifndef IOWIDGET_H
#define IOWIDGET_H

#include <QWidget>

namespace Ui {
class IOWidget;
}
class MemorySection;
class IOWidget : public QWidget
{
    Q_OBJECT

public:
    explicit IOWidget(QWidget *parent = 0);
    ~IOWidget();

    void bindToMemorySection(MemorySection* memory);
signals:
    void dataEntered(const QString &data);
public slots:
    void onDataReceived(QChar data);
    void onDataRequested();
    void onSimulationStart();
private:
    Ui::IOWidget *ui;
    MemorySection* memory;

};

#endif // IOWIDGET_H
