// File: memorydumppane.h
/*
    Pep9 is a virtual machine for writing machine language and assembly
    language programs.
    
    Copyright (C) 2018  J. Stanley Warford, Pepperdine University

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

#ifndef MEMORYDUMPPANE_H
#define MEMORYDUMPPANE_H

#include <QWidget>
#include <QScrollBar>
#include <QSet>

namespace Ui {
    class MemoryDumpPane;
}
class MemorySection;
class CPUDataSection;
class MemoryDumpPane : public QWidget {
    Q_OBJECT
    Q_DISABLE_COPY(MemoryDumpPane)
public:
    explicit MemoryDumpPane(QWidget *parent = 0);
    //Needs to be called before this class can be used
    void init(MemorySection *memory, CPUDataSection *data);
    virtual ~MemoryDumpPane();

    void refreshMemory();
    // Post: the entire memory pane is refreshed

    void refreshMemoryLines(quint16 firstByte, quint16 lastByte);
    // Post: The memory dump is refresed from the line containing startByte to the line
    // containing endByte. Called by load().

    void clearHighlight();
    // Post: Everything is unhighlighted.

    void highlightPC_SP();
    // Post: The current program counter & current stack pointer are highlighted.

    void highlightLastWritten();
    // Post: The byte or word written last cycle is highlighted.

    void cacheModifiedBytes();
    // Post: Changed bytes from Sim are added to the QSet modifiedBytes

    void updateMemory();
    // Post: Memory displays are updated using the changedMemoryAddrss qlist in sim

    void scrollToTop();
    // Post: Memory dump is scrolled to the top left corner

    void highlightOnFocus();
    // Post: Highlights the label based on the label window color saved in the UI file

    bool hasFocus();
    // Post: returns if the pane has focus

    void copy();
    // Post: selected text in the text edit is copied to the clipboard

    int memoryDumpWidth();
    // Post: the width of the memory dump text edit document is returned


public slots:
    void onFontChanged(QFont font);
    void onDarkModeChanged(bool darkMode);
    void onMemoryChanged(quint16 address, quint8, quint8);

private:
    quint32 lineSize;
    Ui::MemoryDumpPane *ui;
    MemorySection *memory;
    CPUDataSection *data;
    QList<quint16> highlightedData;
    // This is a list of bytes that are currently highlighted.

    QSet<quint16> modifiedBytes;
    // This is a list of bytes that were modified since the last update. This is cached for a convenient time to update
    // such as when we hit a breakpoint, the program finishes, or the end of the single step.

    QList<quint16> bytesWrittenLastStep;
    // This is a list of bytes written last step, which is used to highlight recently modified bytes

    bool delayLastStepClear, darkModeEnabled;
    // This is used to delay a clear of the QList bytesWrittenLastStep when leaving a trap that modifies bytes
    // to allow highlighting of modified bytes in trap instructions.

    void highlightByte(quint16 memAddr, QColor foreground, QColor background);
    // Used to highlight/unhighlight individual bytes.

    void mouseReleaseEvent(QMouseEvent *);

    void scrollToByte(quint16 byte);


private slots:
    void scrollToPC();
    void scrollToSP();
    void scrollToAddress(QString string);
};

#endif // MEMORYDUMPPANE_H
