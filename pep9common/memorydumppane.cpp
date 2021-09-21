// File: memorydumppane.cpp
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
#include <QAbstractTextDocumentLayout>
#include <QFontDialog>
#include <QStyle>
#include <QTextCharFormat>

#include "acpumodel.h"
#include "colors.h"
#include "enu.h"
#include "mainmemory.h"
#include "memorydumppane.h"
#include "pep.h"
#include "ui_memorydumppane.h"
#include <QtAlgorithms>
#include <QtCore>
#include <Qt>
#include <QClipboard>
#include <QPainter>
static QString space = "   ";

MemoryDumpPane::MemoryDumpPane(QWidget *parent) :
    QWidget(parent), ui(new Ui::MemoryDumpPane), data(new QStandardItemModel(this)), lineSize(500), memDevice(nullptr),
    cpu(nullptr), delegate(nullptr), colors(&PepColors::lightMode), highlightedData(), modifiedBytes(), lastModifiedBytes(),
    delayLastStepClear(false), inSimulation(false), highlightPC(true)
{
    ui->setupUi(this);
    ui->label->setFont(QFont(Pep::labelFont, Pep::labelFontSize));

    // Init data model later, as we need access to the memory device to perform sizing.

    // Connect scrolling events
    connect(ui->pcPushButton, &QAbstractButton::clicked, this, &MemoryDumpPane::scrollToPC);
    connect(ui->spPushButton, &QAbstractButton::clicked, this, &MemoryDumpPane::scrollToSP);
    connect(ui->scrollToLineEdit, &QLineEdit::textEdited, this, &MemoryDumpPane::scrollToAddress);
    connect(ui->tableView->verticalScrollBar(), &QScrollBar::valueChanged, this, &MemoryDumpPane::scrollToLine);
}

void MemoryDumpPane::init(QSharedPointer<MainMemory> memory, QSharedPointer<ACPUModel> cpu)
{
    this->memDevice = memory;
    this->cpu = cpu;

    setNumBytesPerLine(bytesPerLine);

    delegate = new MemoryDumpDelegate(memDevice, ui->tableView);
    ui->tableView->setItemDelegate(delegate);
    refreshMemory();
}

void MemoryDumpPane::setNumBytesPerLine(quint16 bytesPerLine)
{
    Q_ASSERT(bytesPerLine != 0);
    // Find the nearest power of two for the line size
    auto pow2 = log2f(bytesPerLine);
    pow2 = roundf(pow2);
    // Create nearest power of 2
    auto effective_line_size = 1 << (int)pow2;
    // Don't allow sizes larger than 16 for now
    if(effective_line_size >= 16) effective_line_size = 16;
    else this->bytesPerLine = (quint16) effective_line_size;
    data->clear();
    // Insert 1 column for address, 8 for memory bytes, and 1 for character dump
    data->setColumnCount(1+bytesPerLine+1);
    // Insert enough rows to hold 64k of memory
    data->setRowCount((1<<16)/bytesPerLine);
    // Set the addresses of every row now, as they will not change during execution of the program.
    for(int it = 0; it < (1<<16) /bytesPerLine; it++) {
        data->setData(data->index(it, 0), QString("%1").arg(it*bytesPerLine, 4, 16, QChar('0')).toUpper() + space);
    }

    // Hook the table view into the model, and size everything correctly
    ui->tableView->setModel(data);
    // Safe to use new inline, as it will be deleted when this class is destructed,
    ui->tableView->setSelectionModel(new DisableEdgeSelectionModel(data, this));
    ui->tableView->resizeRowsToContents();
    refreshMemory();
}

void MemoryDumpPane::setHighlightPC(bool highlightPC)
{
     this->highlightPC = highlightPC;
}

void MemoryDumpPane::showJumpToPC(bool jumpToPC)
{
    ui->pcPushButton->setEnabled(jumpToPC);
    ui->pcPushButton->setVisible(jumpToPC);
}

void MemoryDumpPane::showTitleLabel(bool showLabel)
{
    ui->label->setVisible(showLabel);
}

MemoryDumpPane::~MemoryDumpPane()
{
    delete ui;
    delete data;
    delete delegate;
}

void MemoryDumpPane::refreshMemory()
{
    // Refreshing memory is equivilant to refreshing all memory addresses.
    // Since refreshMemoryLines(quint16,quint16) is deffensive, just refresh the maximum
    // number of lines that could ever be had.
    refreshMemoryLines(0, 0xffff);
    ui->tableView->resizeColumnsToContents();
}

void MemoryDumpPane::refreshMemoryLines(quint16 firstByte, quint16 lastByte)
{
    // There are 8 bytes per line, so divide the byte numbers by 8 to
    // get the line number.
    quint16 firstLine = firstByte / bytesPerLine;
    quint16 lastLine = lastByte / bytesPerLine;
    quint8 tempData;
    QChar ch;
    QString memoryDumpLine;
    // Disable screen updates while re-writing all data fields to save execution time.
    bool updates = ui->tableView->updatesEnabled();
    ui->tableView->setUpdatesEnabled(false);
    // Use <= comparison, so when firstLine == lastLine that the line is stil refreshed
    for(int row = firstLine; row <= lastLine; row++) {
        memoryDumpLine.clear();
        for(int col = 0; col < bytesPerLine; col++) {
            // Only access memory if it is in range
            if(quint32(row * bytesPerLine + col) <= memDevice->maxAddress()) {
                // Use the data in the memory section to set the value in the model.
                memDevice->getByte(static_cast<quint16>(row * bytesPerLine + col), tempData);
                data->setData(data->index(row, col + 1), QString("%1").arg(tempData, 2, 16, QChar('0')).toUpper());
                ch = QChar(tempData);
                if (ch.isPrint()) {
                    memoryDumpLine.append(ch);
                }
                else {
                    memoryDumpLine.append(".");
                }
            }
            // Otherwise place a sentinel character to denote the address being inacessible.
            else {
                data->setData(data->index(row, col + 1), "zz");
                memoryDumpLine.append(".");
            }


        }
        data->setData(data->index(row, 1+bytesPerLine), memoryDumpLine.append(space));
    }
    lineSize = 0;
    for(int it = 0; it < data->columnCount(); it++) {
        lineSize += static_cast<unsigned int>(ui->tableView->columnWidth(it));
    }
    lineSize += QFontMetrics(ui->tableView->font()).boundingRect(space).width();
    ui->tableView->setUpdatesEnabled(updates);
}

void MemoryDumpPane::clearHighlight()
{
    // Don't to remove BackgroundRole & ForegroundRole from data->itemData(...) map,
    // followed by a data->setItemData(...) call.
    // Even though items were removed from the map and both calls were successful, the tableView would not remove the old highlighting.
    // Explicitly setting the field to QVariant (nothing) reutrns the field to default styling.
    while (!highlightedData.isEmpty()) {
        quint16 address = highlightedData.takeFirst();
        QStandardItem *item = data->item(address/bytesPerLine, address%bytesPerLine +1);
        item->setData(QVariant(), Qt::BackgroundRole);
        item->setData(QVariant(), Qt::ForegroundRole);
    }
}

void MemoryDumpPane::highlight()
{
    // If the SP is moved during an instruction (e.g. in microcode) it is useful
    // for the student to see where the SP is now, not where it started.
    quint16 sp = cpu->getCPURegWordCurrent(Enu::CPURegisters::SP);
    // Since the PC is modified as part of an instruction, make sure
    // to highlight the instruction being executed, not necessarily the current value
    quint16 pc = cpu->getCPURegWordStart(Enu::CPURegisters::PC);
    quint8 is;
    memDevice->getByte(pc, is);
    // Stack pointer highlighting
    if(sp == 0x0000) {
        // If the stack pointer is unitialized, don't highlight it
    }
    else {
        highlightByte(sp, colors->altTextHighlight, colors->memoryHighlightSP);
        highlightedData.append(sp);
    }
    // Program counter highlighting
    if(!highlightPC) {
        // Don't preform any PC highlighting
    }
    else if(!Pep::isUnaryMap[Pep::decodeMnemonic[is]]) {
        for(int it = 0; it < 3; it++) {
            quint16 as16 = static_cast<quint16>(pc + it);
            highlightByte(as16, colors->altTextHighlight, colors->memoryHighlightPC);
            highlightedData.append(as16);
        }
    }
    else {
        highlightByte(pc, colors->altTextHighlight, colors->memoryHighlightPC);
        highlightedData.append(pc);
    }

    for(quint16 byte : lastModifiedBytes) {
        highlightByte(byte, colors->arrowColorOn, colors->memoryHighlightChanged);
        highlightedData.append(byte);
    }

}

void MemoryDumpPane::updateMemory()
{
    QList<quint16> list;
    QSet<quint16> linesToBeUpdated;
    // Don't clear the memDevice's written / set bytes, since other UI components might
    // need access to them.
    // However, must clear the local cache of modified bytes, or there is the potential to over-highlight.
    modifiedBytes.clear();
    modifiedBytes.unite(memDevice->getBytesSet());
    modifiedBytes.unite(memDevice->getBytesWritten());
    lastModifiedBytes = memDevice->getBytesWritten();
    list = modifiedBytes.toList();
    while(!list.isEmpty()) {
        linesToBeUpdated.insert(list.takeFirst() / bytesPerLine);
    }
    list = linesToBeUpdated.toList();
    std::sort(list.begin(), list.end());

    for(auto x: list) {
        // Multiply by 8 to convert from line # to address of first byte on a line.
        refreshMemoryLines(x * bytesPerLine, x * bytesPerLine);
    }
    ui->tableView->resizeColumnsToContents();

}

void MemoryDumpPane::scrollToTop()
{
    ui->tableView->scrollToTop();
}

void MemoryDumpPane::highlightOnFocus()
{
    if (hasFocus()) {
        ui->label->setAutoFillBackground(true);
    }
    else {
        ui->label->setAutoFillBackground(false);
    }
}

bool MemoryDumpPane::hasFocus()
{
    return ui->tableView->hasFocus() || ui->scrollToLineEdit->hasFocus();
}

void MemoryDumpPane::copy()
{
    auto sModel = ui->tableView->selectionModel();
    auto indices = sModel->selectedIndexes();
    QMap<int, QStringList> map;
    // Find all selected items, and associate them with the row from which the came.
    for(auto index : indices) {
        // Allow any text to be copied, even though copying the address
        // or text representation seems like a useless feature.
        if(!map.contains(index.row())) map[index.row()] = QStringList();
        // First and last columns have hidden white space to help with graphics.
        // Remove it before appending it to a string.
        map[index.row()].append(index.data().toString().trimmed());

    }
    // Turn the text of the selected rows into space delimited strings.
    QStringList lines;
    for(auto line : map) {
        lines.append(line.join(" "));
    }
    // Join all selected rows into a newline delimited string.
    QApplication::clipboard()->setText(lines.join("\n"));
}

QSize MemoryDumpPane::sizeHint() const
{
    int tableSize = static_cast<int>(lineSize);
    int extraPad = 10;
    return QSize(tableSize + extraPad, QWidget::sizeHint().height());
}

void MemoryDumpPane::onFontChanged(QFont font)
{
    ui->tableView->setFont(font);
    ui->scrollToLineEdit->setFont(font);
    ui->tableView->resizeColumnsToContents();
    lineSize = 0;
    for(int it = 0; it < data->columnCount(); it++) {
        lineSize += static_cast<unsigned int>(ui->tableView->columnWidth(it));
    }
    lineSize += QFontMetrics(font).boundingRect(space).width();
    ui->tableView->adjustSize();
    setMaximumWidth(sizeHint().width());
}

void MemoryDumpPane::onDarkModeChanged(bool darkMode)
{
    if(darkMode) colors = &PepColors::darkMode;
    else colors = &PepColors::lightMode;
    // Explicitly rehighlight if in simulation, otherwise old highlighting colors will still be used until the next cycle.
    if(inSimulation) {
        clearHighlight();
        highlight();
    }
}

void MemoryDumpPane::onMemoryChanged(quint16 address, quint8)
{
    // Do not use address+1 or address-1, as an address at the end of a line
    // would incorrectly trigger a refresh of an adjacent line.
    // Refresh memoryLines(...) will work correctly if both start and end addresses are the same.
    modifiedBytes.insert(address);
    this->refreshMemoryLines(address, address);
    //ui->tableView->resizeColumnsToContents();
}

void MemoryDumpPane::onSimulationStarted()
{
    inSimulation = true;
}

void MemoryDumpPane::onSimulationFinished()
{
    inSimulation = false;
    refreshMemory();
}

void MemoryDumpPane::highlightByte(quint16 memAddr, QColor foreground, QColor background)
{
    // Rows contain 8 bytes of memory.
    // The first column is an address, so the first byte in a row is in column one.
    QModelIndex index = data->index(memAddr/bytesPerLine, memAddr%bytesPerLine +1);
    // Set style data of item from parameters.
    data->setData(index, foreground ,Qt::ForegroundRole);
    data->setData(index, background, Qt::BackgroundRole);
}

void MemoryDumpPane::mouseReleaseEvent(QMouseEvent *)
{
    ui->tableView->setFocus();
}

void MemoryDumpPane::scrollToByte(quint16 address)
{
    // Rows contain 8 bytes of memory.
    // The first column is an address, so the first byte in a row is in column one.
    disconnect(ui->tableView->verticalScrollBar(), &QScrollBar::valueChanged, this, &MemoryDumpPane::scrollToLine);
    ui->tableView->scrollTo(data->index(address/bytesPerLine, address%bytesPerLine + 1), QAbstractItemView::ScrollHint::PositionAtTop);
    connect(ui->tableView->verticalScrollBar(), &QScrollBar::valueChanged, this, &MemoryDumpPane::scrollToLine, Qt::UniqueConnection);
}

void MemoryDumpPane::scrollToPC()
{
    quint16 value = cpu->getCPURegWordStart(Enu::CPURegisters::PC);
    // Separate 0x from rest of string, so that the x does not get capitalized.
    QString str = "0x" + QString("%1").arg(value, 4, 16, QLatin1Char('0')).toUpper();
    ui->scrollToLineEdit->setText(str);
    scrollToAddress(str);
}

void MemoryDumpPane::scrollToSP()
{
    quint16 value = cpu->getCPURegWordCurrent(Enu::CPURegisters::SP);
    // Separate 0x from rest of string, so that the x does not get capitalized.
    QString str = "0x" + QString("%1").arg(value, 4, 16, QLatin1Char('0')).toUpper();
    ui->scrollToLineEdit->setText(str);
    scrollToAddress(str);
}

void MemoryDumpPane::scrollToAddress(QString string)
{
    bool ok;
    int address;
    if (string.startsWith("0x", Qt::CaseInsensitive)) {
        address = string.toInt(&ok, 16);
        if (ok) {
            if (address > 65535) {
                ui->scrollToLineEdit->setText("0xFFFF");
            } else {
                scrollToByte(static_cast<quint16>(address));
            }
        }
        else {
            ui->scrollToLineEdit->setText("0x");
        }
    }
    else {
        ui->scrollToLineEdit->setText("0x");
    }
}

void MemoryDumpPane::scrollToLine(int /*scrollBarValue*/)
{
    // The scrollbar value does not have an intuitive relation to
    // topmost visible row in the table view.
    // Calling rowAt(0) accounts for the view's movement through
    // table data, and returns the index of the topmost visible row.
    // Each row contains 8 bytes, so 8*index gives first byte of row
    // Also, separate 0x from rest of string, so that the x does not get capitalized.
    QString str = "0x" + QString("%1").arg(ui->tableView->rowAt(0) * bytesPerLine, 4, 16, QLatin1Char('0')).toUpper();
    ui->scrollToLineEdit->setText(str);
}

MemoryDumpDelegate::MemoryDumpDelegate(QSharedPointer<MainMemory> memory, QObject *parent): QStyledItemDelegate(parent),
    memDevice(memory), canEdit(true)
{

}

MemoryDumpDelegate::~MemoryDumpDelegate()
{

}

QWidget *MemoryDumpDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // The first and last columns are not user editable, so do not create an editor.
    if(index.column() == 0 || index.column() == index.model()->columnCount() - 1 || !canEdit) return nullptr;
    // Otherwise, defer to QStyledItemDelegate's implementation, which returns a LineEdit
    QLineEdit *line = qobject_cast<QLineEdit*>(QStyledItemDelegate::createEditor(parent, option, index));
    // Apply a validator, so that a user cannot input anything other than a one byte hexadecimal constant
    line->setValidator(new QRegExpValidator(QRegExp("[a-fA-F0-9][a-fA-F0-9]|[a-fA-F0-9]"), line));
    return line;
}

void MemoryDumpDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    // The default value in the line edit should be the text currently in that cell.
    QString value = index.model()->data(index, Qt::EditRole).toString();
    QLineEdit *line = static_cast<QLineEdit*>(editor);
    line->setText(value);
}

void MemoryDumpDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const
{
    // Pass geometry information to the editor.
    editor->setGeometry(option.rect);
}

void MemoryDumpDelegate::setModelData(QWidget *editor, QAbstractItemModel *, const QModelIndex &index) const
{
    // Get text from editor and convert it to a integer.
    QLineEdit *line = static_cast<QLineEdit*>(editor);
    QString strValue = line->text();
    bool ok;
    quint64 intValue = static_cast<quint64>(strValue.toInt(&ok, 16));
    // Number of bytes is equal to the number of columns, less the address and data column.
    quint16 bytesPerLine = index.model()->columnCount() - 2;
    // Use column - 1 since the first column is the address.
    quint16 addr = static_cast<quint16>(index.row()*bytesPerLine + index.column() - 1);
    // Even though there is a regexp validator in place, validate data again.
    if(ok && intValue< 1<<16) {
        // Instead of inserting data directly into the item model, notify the MemorySection of a change.
        // The memory section will signal back to the MemoryDump to update the value that changed.
        // This is done to avoid an infinite loop of signals between the memory section and the item model.
        memDevice->setByte(addr, static_cast<quint8>(intValue));
    }
}

void MemoryDumpDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{

    // Use default pen color, as it should be aware of the current theme.
    int margin = option.fontMetrics.boundingRect(space).width();
    // The first column has a bar on the right.
    if(index.column() == 0) {

        QStyleOptionViewItem style(option);
        // Prevent text from being hidden, since the new display rectangle will be too
        // small to hold all of the text.
        style.textElideMode = Qt::TextElideMode::ElideNone;
        // Add space between first column and next column.
        style.rect.adjust(0, 0, -margin, 0);
        QStyledItemDelegate::paint(painter, style, index);
        // Draw a line at the  midpoint of the old and new positions.
        style.rect.adjust(0, 0, margin/2 - 1, 0);
        painter->drawLine(style.rect.topRight(), style.rect.bottomRight());
    }
    // The last column has a bar on the left.
    else if(index.column() == index.model()->columnCount() - 1) {
        QStyleOptionViewItem style(option);
        // Prevent text from being hidden, since the new display rectangle will be too
        // small to hold all of the text.
        style.textElideMode = Qt::TextElideMode::ElideNone;
        // Add space between last column and previous column.
        style.rect.adjust(margin, 0, 0, 0);
        QStyledItemDelegate::paint(painter, style, index);
        // Draw a line at the  midpoint of the old and new positions.
        style.rect.adjust(-margin/2, 0, 0, 0);
        painter->drawLine(style.rect.topLeft(), style.rect.bottomLeft());
    }
    // Otherwise perfom default drawing with given parameters.
    else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

DisableEdgeSelectionModel::DisableEdgeSelectionModel(QAbstractItemModel *model) noexcept :
    QItemSelectionModel (model)
{

}

DisableEdgeSelectionModel::DisableEdgeSelectionModel(QAbstractItemModel *model, QObject *parent) noexcept :
    QItemSelectionModel (model, parent)
{

}

DisableEdgeSelectionModel::~DisableEdgeSelectionModel()
{

}

void DisableEdgeSelectionModel::select(const QItemSelection &selection, QItemSelectionModel::SelectionFlags command)
{
    QItemSelection newSelection;
    // For every selection range, remove any invalid indicies.
    for(auto item : selection) {
        // If the leftmost edge is in the last column, the selection
        // only spans the last column, so ignore it.
        if(item.topLeft().column() == model()->columnCount() - 1) continue;
        // If the rightmost edge is in the first column, the selection
        // only spans the first column, so ignore it.
        else if(item.bottomRight().column() == 0) continue;
        // If the selection spans the first and lasts columns, remove both columns.
        else if(item.topLeft().column() == 0 &&
                item.bottomRight().column() == model()->columnCount() - 1) {
            newSelection.append(QItemSelectionRange(model()->index(item.left(), 1), model()->index(item.right(), model()->columnCount() - 2)));
        }
        // If the selection extends only into the first column, remove first column from selection.
        else if(item.topLeft().column() == 0) {
            newSelection.append(QItemSelectionRange(model()->index(item.left(), 1), item.bottomRight()));
        }
        // If the selection extends only into the last column, remove last column from selection.
        else if(item.bottomRight().column() == model()->columnCount() - 1) {
            newSelection.append(QItemSelectionRange(item.topLeft(), model()->index(item.right(), model()->columnCount() - 2)));
        }
        // Otherwise, no change to the selection is needed.
        else{
            newSelection.append(item);
        }
    }
    QItemSelectionModel::select(newSelection, command);
}

void DisableEdgeSelectionModel::select(const QModelIndex &index, QItemSelectionModel::SelectionFlags command)
{
    // Don't allow first or last columns to be selected
    if(index.column() == 0 ||
            index.column() == model()->columnCount() - 1) {
        return;
    }
    else {
        QItemSelectionModel::select(index, command);
    }
}
