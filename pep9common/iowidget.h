// File: iowidget.h
/*
    The Pep/9 suite of applications (Pep9, Pep9CPU, Pep9Micro) are
    simulators for the Pep/9 virtual machine, and allow users to
    create, simulate, and debug across various levels of abstraction.

    Copyright (C) 2018  J. Stanley Warford & Matthew McRaven, Pepperdine University

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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

    // Set the text in the batch input pane. Needed to pass
    void setBatchInput(QString text);
    // Select which tab (batch, terminal) is presented to the user.
    void setActivePane(Enu::EPane pane);
    // Return if the widget is using batch IO.
    bool inBatchMode() const;
    // Return if the widget is using interactive IO.
    bool inInteractiveMode() const;
    // If there is an outstanding input request, cancel it in a way that does
    // no generate errors.
    void cancelWaiting();
    // Address of character input / output devices MUST be set, otherwise IO will not work,
    // and the program will probably crash.
    void setInputChipAddress(quint16 address);
    void setOutputChipAddress(quint16 address);
    // Connect to needed signals/slots of passed memory device.
    void bindToMemorySection(MainMemory* memory);

    // Does the currently selected widget have any undo states?
    bool isUndoable() const;
    // Does the currently selected widget have any redo states?
    bool isRedoable() const;
    // Which editing buttons (redo, undo, copy, cut, paste) are allowed for
    // the currently selected widget?
    int editActions() const;

    void highlightOnFocus();

signals:
    void dataEntered(const QString &data);
    void undoAvailable(bool b);
    void redoAvailable(bool b);
    void inputReady(quint16 addr, quint8 val);

public slots:
    // Called whenever a memory-mapped IO generates output.
    void onOutputReceived(quint16 address, QChar data);
    // Called whenever memory-mapped IO requests input.
    void onDataRequested(quint16 address);
    // Inform the widget that a simulation has begun, so that needed
    // state may be cached. Also moves batch input to input buffer if
    // needed.
    void onSimulationStart();
    // Clear the contents of terminal, batch input/output panes.
    void onClear();
    // Change the text font for terminal, batch input/output panes.
    void onFontChanged(QFont font);

    // Following actions are applied to the currently selected pane,
    // if one is selected & the operation can be applied.
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
    quint16 charInAddr, charOutAddr;
    // Cache whether batch or terminal IO was selected when the simulation started,
    // to avoid bugs where the user switches tabs mid simulation.
    int activePane;
    // Indicies in the tab widget for the batch, terminal panes.
    static const int batch_index = 0;
    static const int terminal_index=1;

};

#endif // IOWIDGET_H
