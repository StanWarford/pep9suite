#ifndef IOWIDGET_H
#define IOWIDGET_H

#include <QWidget>

namespace Ui {
class IOWidget;
}

class IOWidget : public QWidget
{
    Q_OBJECT

public:
    explicit IOWidget(QWidget *parent = 0);
    ~IOWidget();

private:
    Ui::IOWidget *ui;
};

#endif // IOWIDGET_H
