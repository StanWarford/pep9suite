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
    void batchInputToBuffer(); //Move batch input to MemorySection if needed;
    bool isUndoable() const;
    bool isRedoable() const;
    const int editActions() const;

signals:
    void dataEntered(const QString &data);

signals:
    void undoAvailable(bool b);
    void redoAvailable(bool b);

public slots:
    void onDataReceived(QChar data);
    void onDataRequested();
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

private:
    Ui::IOWidget *ui;
    MemorySection* memory;

};

#endif // IOWIDGET_H
