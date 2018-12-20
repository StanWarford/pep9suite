#ifndef ADDRMODEDIALOG_H
#define ADDRMODEDIALOG_H

#include <QDialog>
#include <QSet>
namespace Ui {
class AddrModeDialog;
}
class AddrModeWidget;
class AddrModeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddrModeDialog(QWidget *parent = 0);
    ~AddrModeDialog();
    void addAddrMode(QString addrMode, QString addrSymbol);
    void clearAddrModes();
    QList<QPair<QString,QString>> allAddrModes();
private:
    Ui::AddrModeDialog *ui;
    QSet<AddrModeWidget*> addrWidgets;
};

#endif // ADDRMODEDIALOG_H
