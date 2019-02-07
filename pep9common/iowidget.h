#ifndef IOWIDGET_H
#define IOWIDGET_H

#include <QWidget>
#include "enu.h"
namespace Ui {
class IOWidget;
}
class MainMemory;
class IOWidget : public QWidget
{
    Q_OBJECT

public:
    explicit IOWidget(QWidget *parent = nullptr);
    ~IOWidget();

    void setBatchInput(QString text);
    void setActivePane(Enu::EPane pane);
    void cancelWaiting();
    // Address of character input / output devices MUST be set, otherwise IO will not work,
    // and the program will probably crash.
    void setInputChipAddress(quint16 address);
    void setOutputChipAddress(quint16 address);
    void bindToMemorySection(MainMemory* memory);
    void batchInputToBuffer(); //Move batch input to AMemoryDevice if needed;
    bool isUndoable() const;
    bool isRedoable() const;
    int editActions() const;

signals:
    void dataEntered(const QString &data);
    void undoAvailable(bool b);
    void redoAvailable(bool b);
    void inputReady(quint16 addr, quint8 val);

public slots:
    void onDataReceived(quint16 address, QChar data);
    void onDataRequested(quint16 address);
    void onSimulationStart();
    void onClear();
    void onFontChanged(QFont font);

    void copy() const;
    void cut();
    void paste();
    void undo();
    void redo();

private slots:
    void onSetRedoability(bool b);
    void onSetUndoability(bool b);
    void onInputReady(QString value);
private:
    Ui::IOWidget *ui;
    MainMemory* memory;
    quint16 iChipAddr, oChipAddr;

};

#endif // IOWIDGET_H
