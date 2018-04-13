// File: mainmemory.h
/*
    Pep9CPU is a CPU simulator for executing microcode sequences to
    implement instructions in the instruction set of the Pep/9 computer.

    Copyright (C) 2010  J. Stanley Warford, Pepperdine University

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
#ifndef MAINMEMORY_H
#define MAINMEMORY_H

#include <QWidget>
#include <QTableWidgetItem>

namespace Ui {
    class MainMemory;
}

class CPUDataSection;
class MainMemory : public QWidget {
    Q_OBJECT
public:
    MainMemory(QWidget *parent = 0);
    ~MainMemory();

    // Populate memory items in the table
    void populateMemoryItems();

    // Refresh the memory to reflect the Sim::Mem object
    void refreshMemory();

    void setMemAddress(int memAddress, int value);

    void setMemPrecondition(int memAddress, int value);
    bool testMemPostcondition(int memAddress, int value);

    void clearMemory();
    // Post: sets all memory to 0x00;

    // scrolls to modified cell and highlights it
    void showMemEdited(int address);

    // highlights all modified bytes in the current view
    void hightlightModifiedBytes();

    void scrollToAddress(int address); // used to display the most recently modified cell

public slots:
    // Highlights the label based on the label window color saved in the UI file
    void highlightOnFocus();
    void onMemoryValueChanged(quint16 address, quint8 oldVal, quint8 newVal);
    // Returns if the table has focus
    bool hasFocus();

private slots:
    // Slot called when the vertical scroll bar changes.
    // Will cause the table to scroll through memory.
    void sliderMoved(int pos);

    // Fires when the data in an item changes, used to store the value into Sim::Mem[]
    void cellDataChanged(QTableWidgetItem* item);

    void scrollToChanged(QString string); // used to scroll to the cell corresponding to the line edit at the bottom

protected:
    void changeEvent(QEvent *e);
    void resizeEvent(QResizeEvent *);

    // this is for the table widget, which sends the scroll
    // event to the scroll bar, which handles scrolling the table widget
    bool eventFilter(QObject *, QEvent *e);

private:
    Ui::MainMemory *ui;

    // List of all the rows currently in the table
    QStringList rows;


    int	highlightedIndex;
    int	currentMemoryOffset;
    char mem[0x10000];
    CPUDataSection* dataSection;
    enum { CELL_COUNT = 30};

    int oldRowCount;

};

#endif // MAINMEMORY_H
