#ifndef ADDRMODEWIDGET_H
#define ADDRMODEWIDGET_H

#include <QWidget>

namespace Ui {
class AddrModeWidget;
}

class AddrModeWidget : public QWidget
{
    Q_OBJECT

public:
    AddrModeWidget(QString addrMode, QString addrSymbol, QWidget *parent = 0);
    explicit AddrModeWidget(QWidget *parent = 0);
    virtual ~AddrModeWidget();
    void setAddrMode(QString addrMode);
    void setAddrSymbol(QString addrSymbol);

    QString addrSymbol() const;
    QString addrMode() const;

private:
    Ui::AddrModeWidget *ui;
};

#endif // ADDRMODEWIDGET_H
