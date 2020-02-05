// File: memorydumppane.h
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

#ifndef MEMORYDUMPPANE_H
#define MEMORYDUMPPANE_H

#include <QScrollBar>
#include <QSet>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QWidget>
#include "colors.h"
namespace Ui {
    class MemoryDumpPane;
}
class MainMemory;
class ACPUModel;
class MemoryDumpDelegate;
class MemoryDumpPane : public QWidget {
    Q_OBJECT
    Q_DISABLE_COPY(MemoryDumpPane)
public:
    explicit MemoryDumpPane(QWidget *parent = nullptr);

    // Needs to be called after construction but before this class can be used, otherwise the class is in an incomplete state.
    void init(QSharedPointer<MainMemory> memory, QSharedPointer<ACPUModel> cpu);

    // Set the number of bytes displayed per line.
    // Must be a power of 2 between [1-16].
    // A number that is not a power of two will be rounded to the nearest power,
    // and the number will be clamped to 16.
    void setNumBytesPerLine(quint16 bytesPerLine);

    // Optionally disable the highlighting of the PC.
    // By default, the PC is highlighted
    void setHighlightPC(bool highlightPC);

    // Optionally, hide the jump to PC button.
    // By default, the jump to PC button is visible
    void showJumpToPC(bool jumpToPC = true);

    // Optionally, add a label to the top of the memory dump pane displaying the
    // widget's name.
    // By default, this title is visible.
    void showTitleLabel(bool showLabel = true);

    virtual ~MemoryDumpPane() override;

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

    QSize sizeHint() const override;
    // Post: the width of the memory dump text edit document is returned


public slots:
    void onFontChanged(QFont font);
    // Post: the font used in the memory dump

    // Handle switching styles to and from dark mode & potential re-highlighting
    void onDarkModeChanged(bool darkMode);

    // Allow memory lines to be updated whenever an address is changed.
    void onMemoryChanged(quint16 address, quint8 newValue);

    void onSimulationStarted();
    void onSimulationFinished();

private:
    Ui::MemoryDumpPane *ui;
    QStandardItemModel* data;
    quint32 lineSize;
    quint16 bytesPerLine = {8};
    QSharedPointer<MainMemory> memDevice;
    QSharedPointer<ACPUModel> cpu;
    MemoryDumpDelegate *delegate;
    const PepColors::Colors *colors;
    QList<quint16> highlightedData;
    // This is a list of bytes that are currently highlighted.

    QSet<quint16> modifiedBytes, lastModifiedBytes;
    // This is a list of bytes that were modified since the last update. This is cached for a convenient time to update
    // such as when we hit a breakpoint, the program finishes, or the end of the single step.
    // lastModifiedBytes indicates which bytes were written in the last ISA instruction.


    bool delayLastStepClear, inSimulation, highlightPC;
    // This is used to delay a clear of the QList bytesWrittenLastStep when leaving a trap that modifies bytes
    // to allow highlighting of modified bytes in trap instructions.

        // Used to highlight/unhighlight individual bytes.
    void highlightByte(quint16 memAddr, QColor foreground, QColor background);

    void mouseReleaseEvent(QMouseEvent *) override;

    void scrollToByte(quint16 address);

private slots:
    void scrollToPC();
    void scrollToSP();
    void scrollToAddress(QString string);
    void scrollToLine(int scrollBarValue);
};

/*
 * Item delegate that handles input validation of hex constants, and disables editing of address and hex dump columns.
 * Eventually, it can be extended to be signaled to enable or disable editing
 */
class MemoryDumpDelegate: public QStyledItemDelegate {
private:
    QSharedPointer<MainMemory> memDevice;
    bool canEdit;
public:
    MemoryDumpDelegate(QSharedPointer<MainMemory> memory, QObject* parent = nullptr);
    virtual ~MemoryDumpDelegate() override;
    // See http://doc.qt.io/qt-5/qstyleditemdelegate.html#subclassing-qstyleditemdelegate for explanation on the methods being reimplemented.

    // If the index is editable, create an editor that validates byte hex constants, otherwise return nullptr
    virtual QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    // Provides editor widget with starting data for editing.
    virtual void setEditorData(QWidget * editor, const QModelIndex & index) const override;
    // Ensure that editor is displayed correctly on the item view
    virtual void updateEditorGeometry(QWidget * editor,
                                      const QStyleOptionViewItem & option,
                                      const QModelIndex & index) const override;
    // Handle updating data in the model via calling the memorySection
    virtual void setModelData(QWidget *editor,
                              QAbstractItemModel *model,
                              const QModelIndex &index) const override;
    // Override painting method to allow drawing of vertical bars in dump pane.
    virtual void paint(QPainter *painter,
                       const QStyleOptionViewItem &option,
                       const QModelIndex &index ) const override;
};
#include <QObject>
#include <QItemSelectionModel>
#include <QAbstractItemModel>

/*
 * DisableEdgeSelectionModel prevents the user from selecting either
 * the rightmost or leftmost columns in an item model.
 */
class DisableEdgeSelectionModel : public QItemSelectionModel
{
    Q_OBJECT
public:
    DisableEdgeSelectionModel(QAbstractItemModel *model = Q_NULLPTR) noexcept;
    DisableEdgeSelectionModel(QAbstractItemModel *model, QObject *parent = Q_NULLPTR) noexcept;
    virtual ~DisableEdgeSelectionModel() override;
    void select(const QItemSelection &selection, QItemSelectionModel::SelectionFlags command) override;
    void select(const QModelIndex &index, SelectionFlags command) override;
};


#endif // MEMORYDUMPPANE_H
