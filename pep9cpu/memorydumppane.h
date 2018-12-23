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
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include "colors.h"
namespace Ui {
    class MemoryDumpPane;
}
class CPUControlSection;
class CPUDataSection;
class MemorySection;
class MemoryDumpDelegate;
class MemoryDumpPane : public QWidget {
    Q_OBJECT
    Q_DISABLE_COPY(MemoryDumpPane)
public:
    explicit MemoryDumpPane(QWidget *parent = 0);
    // Needs to be called after construction but before this class can be used, otherwise the class is in an incomplete state.
    void init(MemorySection *memorySection, CPUDataSection *dataSection, CPUControlSection* controlSection);
    virtual ~MemoryDumpPane();

    void refreshMemory();
    // Post: the entire memory pane is refreshed

    void refreshMemoryLines(quint16 firstByte, quint16 lastByte);
    // Post: The memory dump is refresed from the line containing startByte to the line
    // containing endByte. Called by load().

    void clearHighlight();
    // Post: Everything is unhighlighted.

    void highlight();
    // Post: The current program counter & current stack pointer are highlighted.
    // The last written bytes are highlighted.

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
    // Post: the font used in the memory dump

    // Handle switching styles to and from dark mode & potential re-highlighting
    void onDarkModeChanged(bool darkMode);

    // Allow memory lines to be updated whenever an address is changed.
    void onMemoryChanged(quint16 address, quint8, quint8);

    void onSimulationStarted();
    void onSimulationFinished();

private:
    Ui::MemoryDumpPane *ui;
    quint32 lineSize;
    QStandardItemModel* data;
    MemorySection *memorySection;
    CPUDataSection *dataSection;
    CPUControlSection *controlSection;
    MemoryDumpDelegate *delegate;
    const PepColors::Colors *colors;
    QList<quint16> highlightedData;
    // This is a list of bytes that are currently highlighted.

    QSet<quint16> modifiedBytes, lastModifiedBytes;
    // This is a list of bytes that were modified since the last update. This is cached for a convenient time to update
    // such as when we hit a breakpoint, the program finishes, or the end of the single step.
    // lastModifiedBytes indicates which bytes were written in the last ISA instruction.

    QList<quint16> bytesWrittenLastStep;
    // This is a list of bytes written last step, which is used to highlight recently modified bytes

    bool delayLastStepClear, darkModeEnabled, inSimulation;
    // This is used to delay a clear of the QList bytesWrittenLastStep when leaving a trap that modifies bytes
    // to allow highlighting of modified bytes in trap instructions.

        // Used to highlight/unhighlight individual bytes.
    void highlightByte(quint16 memAddr, QColor foreground, QColor background);

    void mouseReleaseEvent(QMouseEvent *);

    void scrollToByte(quint16 byte);

private slots:
    void scrollToPC();
    void scrollToSP();
    void scrollToAddress(QString string);
    void scrollToLine(int address);
};

/*
 * Item delegate that handles input validation of hex constants, and disables editing of address and hex dump columns.
 * Eventually, it can be extended to be signaled to enable or disable editing
 */
class MemoryDumpDelegate: public QStyledItemDelegate {
private:
    MemorySection* memorySection;
    bool canEdit;
public:
    MemoryDumpDelegate(MemorySection* memorySection, QObject* parent = 0);
    virtual ~MemoryDumpDelegate();
    // See http://doc.qt.io/qt-5/qstyleditemdelegate.html#subclassing-qstyleditemdelegate for explanation on the methods being reimplemented.

    // If the index is editable, create an editor that validates byte hex constants, otherwise return nullptr
    virtual QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    // Provides editor widget with starting data for editing.
    virtual void setEditorData(QWidget * editor, const QModelIndex & index) const override;
    // Ensure that editor is displayed correctly on the item view
    virtual void updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
    // Handle updating data in the model via calling the memorySection
    virtual void setModelData(QWidget *editor,
                                    QAbstractItemModel *model,
                                    const QModelIndex &index) const override;
};
#endif // MEMORYDUMPPANE_H