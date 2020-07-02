// File: cachereplace.cpp
/*
    Pep9 is a virtual machine for writing machine language and assembly
    language programs.

    Copyright (C) 2020  Matthew McRaven & J. Stanley Warford, Pepperdine University

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
#include "cacheview.h"
#include "ui_cacheview.h"

#include <QAction>
#include <QDebug>
#include <QMenu>
#include <QPainter>

#include "cachememory.h"
#include "cachealgs.h"
#include "pep.h"

static const int EvictedData = Qt::UserRole + 1;

// Convert an address range to a well-padded string.
QString toAddressRange(quint16 lower, quint16 upper)
{
    auto ret = QString("%1-%2")
            .arg(lower, 4, 16, QChar('0'))
            .arg(upper, 4, 16, QChar('0'));
    return ret;
}
CacheView::CacheView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CacheView), data(new QStandardItemModel(this)),
    colors(&PepColors::lightMode), cache(nullptr), eviction_collate()
{
    ui->setupUi(this);

    ui->cacheTree->setFont(Pep::codeFont);
    // Connect address conversion spin boxes.
    connect(ui->tagBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &CacheView::cachetag_changed);
    connect(ui->indexBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &CacheView::cachetag_changed);
    connect(ui->dataBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &CacheView::cachetag_changed);

    connect(ui->addressBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &CacheView::address_changed);

    // Create cache delegate
    del = new CacheViewDelegate(cache, colors);
    ui->cacheTree->setItemDelegate(del);

    // Connect model to data
    ui->cacheTree->setModel(data);

    // Handle right click events.
    ui->cacheTree->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->cacheTree, &QTreeView::customContextMenuRequested, this, &CacheView::handle_custom_menu);
}

CacheView::~CacheView()
{
    delete ui;
    if(data) delete data;
}

void CacheView::init(QSharedPointer<CacheMemory> cache)
{
    this->cache = cache;

    ui->cacheConfiguration->init(cache, true);

    // Connect cache configuration boxes.
    connect(cache.get(), &CacheMemory::configurationChanged, this, &CacheView::onCacheConfigChanged);

    onCacheConfigChanged();
    refreshMemory();
}

void CacheView::address_changed(int value)
{
    auto breakdown = cache->breakdownAddress(value);
    ui->tagBox->setValue(breakdown.tag);
    ui->indexBox->setValue(breakdown.index);
    ui->dataBox->setValue(breakdown.offset);
}

void CacheView::cachetag_changed(int /*value*/)
{
    auto index_bits = cache->getIndexSize();
    auto data_bits = cache->getDataSize();

    int address = 0;
    address += ui->tagBox->value() << (index_bits + data_bits);
    address += ui->indexBox->value() << (data_bits);
    address += ui->dataBox->value();

    ui->addressBox->setValue(address);
}

void CacheView::handle_custom_menu(const QPoint &point)
{
    // If there is no item correpsonding to the selected point, return.
    auto index = ui->cacheTree->indexAt(point);
    if(!index.isValid()) return;

    auto* item = this->data->itemFromIndex(index);
    auto* parent = item->parent();
    int cache_tag, cache_index;

    // If parent is null, we selected a cache line, not a cache entry. Return.
    if(parent == nullptr) {
        return;
    }
    // Extract the integer tag from the parent.
    else {
        cache_index  = parent->data(Qt::DisplayRole).toInt();
    }
    // If the selected row is not present, it can't be highlighted.
    if(!parent->child(index.row(), static_cast<int>(Columns::PresentColumn))->data(Qt::DisplayRole).toBool()) return;
    else {
        cache_tag = parent->child(index.row(), static_cast<int>(Columns::TagColumn))->data(Qt::DisplayRole).toInt();
    }

    auto index_bits = cache->getIndexSize();
    auto data_bits = cache->getDataSize();

    // Compute the addresses spanned by the cache entry.
    menu_tag_index = {(cache_tag<<(index_bits+data_bits)) + ((cache_index)<<data_bits),
                      (cache_tag<<(index_bits+data_bits)) + ((cache_index+1)<<data_bits) - 1};

    // Create a menu to prompt the user to show in memory pane, and make it visible.
    QMenu contextMenu(tr("Context menu"), this);
    QAction action("Show in Memory Dump...", this);
    contextMenu.addAction(&action);
    connect(&action, &QAction::triggered, this, &CacheView::accept_show_in_memory);
    contextMenu.exec(mapToGlobal(point));

}

void CacheView::accept_show_in_memory()
{
    emit requestCacheHighlighting(std::get<0>(menu_tag_index), std::get<1>(menu_tag_index));
}

void CacheView::refreshLine(quint16 line)
{
    // Determine cache parameters.
    auto tag_bits = cache->getTagSize();
    auto index_bits = cache->getIndexSize();
    auto associativity = cache->getAssociativty();

    assert(line < (1<<index_bits));

    // Make sure cache line is present, name correctly, and has the right address.
    constexpr int index_column = static_cast<int>(Columns::IndexColumn);
    auto* lineItem = data->item(line, index_column);
    if(lineItem == nullptr) {
        lineItem = new QStandardItem(QString("%1").arg(line));
        data->setItem(line, index_column, lineItem);
    }
    auto lineIndex = data->index(line, index_column);

    // Ensure the right number of rows / columns are in the cache line.
    constexpr int column_count = static_cast<int>(Columns::ColumnCount);
    if(data->columnCount(lineIndex) != column_count) lineItem->setColumnCount(column_count);
    if(data->rowCount(lineIndex) != associativity) lineItem->setRowCount(associativity);

    constexpr int address_column = static_cast<int>(Columns::AddressColumn);
    auto* addressItem = data->item(line, address_column);
    if(addressItem == nullptr) {
        addressItem = new QStandardItem();
        data->setItem(line, address_column, addressItem);
    }
    addressItem->clearData();

    auto lineEntry = cache->getCacheLine(line);
    if(!lineEntry.has_value()) {
        qDebug() << "Something went horribly wrong with a line.";
    }

    auto* linePtr = *lineEntry;
    // Determine if the entire cache line is not present.
    bool blank = false;

    for(quint16 entry = 0; entry < associativity; entry++) {
        auto entryOpt = linePtr->get_entry(entry);
        if(!entryOpt.has_value()) {
            qDebug() << "Something went horribly wrong with an entry.";
        }

        // If any cache entry is present in the line, then the entire line is displayable.
        auto entryPtr = *entryOpt;
        blank |= entryPtr->is_present;

        // Update the current row.
        setRow(line, linePtr, entry, entryPtr, false);
    }

    // Render evicted cache entries for the current cache line.
    if(auto evicted = cache->getEvictedEntry(line);
       !evicted.empty()) {
        auto root_item = data->item(line);
        int entry_number = associativity;
        // If associativty differs from row count, strikethrough will fail.
        // Ensure that this never happens.
        assert(root_item->rowCount() == associativity);

        // Conditionally choose at compile time to perform eviction collation.
        if constexpr(COLLATE_EVICTIONS) {
            // Must clear eviction collation cache each time, or entries may leak between cache lines.
            reset_eviction_collation();
            // Combine all eviction entries for the same index, and only
            // keep the most recent eviction.
            int time = 0;
            for(auto entry : evicted) {
                eviction_collate[entry.tag] = {entry, time++};
            }
            // Sort the items so that the most recent eviction is first
            using item = std::tuple<CacheEntry, int>;
            std::sort(eviction_collate.begin(), eviction_collate.end(), [](const item& lhs, const item& rhs){return std::get<1>(lhs) < std::get<1>(rhs);});
            // Iterate over all evicted items, and append an entry to the table if an index has been marked as evicted.
            for(auto evicted_entry = eviction_collate.rbegin(); evicted_entry!=eviction_collate.rend(); evicted_entry++) {
                auto &[entry, time] = *evicted_entry;
                if(!entry.is_present) continue;
                QList<QStandardItem*> new_items;
                for(int col=0; col<data->columnCount(); col++)  {
                    new_items.append(new QStandardItem());
                }
                root_item->appendRow(new_items);
                setRow(line, linePtr, entry_number, &entry, true);
                entry_number++;
            }
        }
        else {
            // Iterate over all evicted items, and append an entry to the table if an index has been marked as evicted.
            for(auto entry : evicted) {
                QList<QStandardItem*> new_items;
                for(int col=0; col<data->columnCount(); col++)  {
                    new_items.append(new QStandardItem());
                }
                root_item->appendRow(new_items);
                setRow(line, linePtr, entry_number, &entry, true);
                entry_number++;
            }
        }

    }

    // When there are more lines than ways in the cache, it means that "evicted"
    // entries have been added, an must be removed at the start of the next simulation step.
    if(lineItem->rowCount()>cache->getAssociativty()){
        remove_entry entry;
        entry.root_line = line;
        entry.count_to_remove = lineItem->rowCount() - cache->getAssociativty();
        to_delete.insert(entry);
    }

    // Display the row if it is not blank.
    ui->cacheTree->setRowHidden(line, QModelIndex(), !blank);
}

void CacheView::setRow(quint16 line, const CacheLine* linePtr, quint16 entry, const CacheEntry *entryPtr, bool evicted)
{
    //Extract column numbers from enum at compile time.
    constexpr int index_column = static_cast<int>(Columns::IndexColumn);
    constexpr int tag_column = static_cast<int>(Columns::TagColumn);
    constexpr int eviction_column = static_cast<int>(Columns::EvictColumn);
    constexpr int address_column = static_cast<int>(Columns::AddressColumn);
    constexpr int present_column = static_cast<int>(Columns::PresentColumn);
    constexpr int hits_column = static_cast<int>(Columns::HitsColumn);

    auto index_bits = cache->getIndexSize();
    auto data_bits = cache->getDataSize();

    /*
     * Note: Do not perform a check that entry # is less than associativity.
     * This method is used to populate "evicted" entries, and as such may be used
     * fill in entry #'s above the associativty.
     *
     * The evicted flag indicates that no highlighting/font checks should be
     * applied to the cache entry
     */

    // Determine which values are needed in each column
    QString c1_tag = "";
    Qt::CheckState c2_evict = Qt::CheckState::Unchecked;
    QString c3_address = "";
    QVariant c5_hits = "";

    // If the cache entry is present, display the index # and addresses spanned by
    // the entry in columns 0,2. Also determine hit count.
    if(entryPtr->is_present) {
        auto tag = entryPtr->tag;
        c1_tag = QString("%1").arg(tag);
        c3_address = toAddressRange((line<<(index_bits+data_bits)) + ((entryPtr->tag)<<data_bits),
                                    (line<<(index_bits+data_bits)) + ((entryPtr->tag+1)<<data_bits) - 1);
        c5_hits = entryPtr->hit_count;
    }

    // Cache hit count, so that it is easier to see from debugger.
    quint16 eviction_candidate = linePtr->get_replacement_policy()->eviction_loohahead();
    if(eviction_candidate == entry) {
        c2_evict = Qt::CheckState::Checked;
    }


    auto lineIndex = data->index(line, index_column);

    // Convert values in existing cells to usable types for comparisons.
    // These conversions will fail when the box is empty, so must check
    // ok_conv_* when attemtping to use associated integer.
    bool ok_conv_index, ok_conv_hits;
    int listedIndex = data->index(entry, tag_column, lineIndex).data().toInt(&ok_conv_index);
    int listedHits = data->index(entry, hits_column, lineIndex).data().toInt(&ok_conv_hits);

    // If evicted, indicate to Style Delegate to apply special evicted formatting.
    // Otherwise, clear evicted flags. When increasing the associativity, failing to clear
    // previous iterations' evicted flags will cause incorret strike-throughs.
    for(int col=0; col<data->columnCount(); col++) {
        data->itemFromIndex(data->index(entry, col, lineIndex))->setData(evicted, EvictedData);
    }

    // Highlight "new" items.
    // An item is "new" if it is present, and either the old value was not present,
    // or the old entry number is not the new entry number.
    if(!evicted && entryPtr->is_present && (!ok_conv_index || listedIndex != entryPtr->tag)) {

        // Make a note that special highlighting needs to be removed
        // at end of the current simulation step.
        updated_item item;
        item.root_index = line;
        item.child_row = entry;
        this->last_updated.insert(item);

        // To highlight entire row must change background of each cell in row.
        for(int col=0; col<data->columnCount(); col++) {
            data->itemFromIndex(data->index(entry, col, lineIndex))->setBackground(colors->muxCircuitRed);
        }
    }
    // Bolds "referenced" items.
    // An item is "referenced" if it is present and the hit count has changed
    // since the last time the value was updated.
    else if(!evicted && entryPtr->is_present && (!ok_conv_index || entryPtr->hit_count != listedHits)) {
        QFont regularFont = activeFont;
        regularFont.setBold(true);

        // Make a note that special font needs to be removed
        // at end of the current simulation step.
        updated_item item;
        item.root_index = line;
        item.child_row = entry;
        this->last_updated.insert(item);

        // To change font for entire row must change each cell's fonts.
        for(int col=0; col<data->columnCount(); col++)  {
            data->itemFromIndex(data->index(entry, col, lineIndex))->setFont(regularFont);
        }
    }

    // Assign values to each column based on the current cache entry.
    data->itemFromIndex(data->index(entry, tag_column, lineIndex))->setData(c1_tag, Qt::DisplayRole);
    data->itemFromIndex(data->index(entry, eviction_column, lineIndex))->setCheckState(c2_evict);
    data->itemFromIndex(data->index(entry, address_column, lineIndex))->setData(c3_address, Qt::DisplayRole);
    data->itemFromIndex(data->index(entry, present_column, lineIndex))->setData(entryPtr->is_present, Qt::DisplayRole);
    data->itemFromIndex(data->index(entry, hits_column, lineIndex))->setData(c5_hits, Qt::DisplayRole);
}

void CacheView::reset_eviction_collation()
{
    for(auto& item : eviction_collate) {
        item = {CacheEntry(), 0};
    }
}

void CacheView::refreshMemory()
{
    // Iteratively refresh each cache line, and ensure columns
    // are large enough to fit line.
    auto index_bits = cache->getIndexSize();

    for(int line = 0; line < 1<<index_bits; line++) {
        refreshLine(line);
    }
    for(int it = 0; it < data->columnCount(); it++) {
        ui->cacheTree->resizeColumnToContents(it);
    }

}

void CacheView::updateMemory()
{

    // Don't clear the memDevice's written / set bytes, since other UI components might
    // need access to them.
    QSet<quint16> modifiedLines = cache->getCacheLinesTouched();
    auto keys = cache->getAllEvictedEntries().keys();
    // Qt has a new method for joining sets and is deprecating the old method.
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    modifiedLines.unite(QSet<quint16>(keys.begin(), keys.end()));
#else
    modifiedLines.unite(QSet<quint16>::fromList(keys));
#endif

    for(auto x: modifiedLines) {
        refreshLine(x);
    }
}

void CacheView::highlightOnFocus()
{
    if (hasFocus()) {
        ui->cacheLabel->setAutoFillBackground(true);
    }
    else {
        ui->cacheLabel->setAutoFillBackground(false);
    }
}

bool CacheView::hasFocus()
{
    return this->isAncestorOf(focusWidget());
}

void CacheView::onFontChanged(QFont font)
{
    activeFont = font;
    // Disable font stylization used to indicate cache access.
    // This includes bolding, italics.
    activeFont.setBold(false);
    activeFont.setItalic(false);
    activeFont.setStrikeOut(false);
    ui->cacheTree->setFont(activeFont);
    // Propogate event to child.
    ui->cacheConfiguration->onFontChanged(font);
    // Adjust columns to fit new font.
    for(int it = 0; it < data->columnCount(); it++) {
        ui->cacheTree->resizeColumnToContents(it);
    }
}

void CacheView::onDarkModeChanged(bool darkMode)
{
    if(darkMode) {
        colors = &PepColors::darkMode;
        del->changeColors(colors);
    } else {
        colors = &PepColors::lightMode;
        del->changeColors(colors);
    }
    // Propogate event to child.
    ui->cacheConfiguration->onDarkModeChanged(darkMode);
}

void CacheView::onMemoryChanged(quint16 /*address*/, quint8 /*newValue*/)
{
    // This function is only triggered on writes, not reads.
    // We do not have a write-allocate cache (by default),
    // we really only care about reads.
    // Updating only the line at value written to "address" is insufficient for
    // synchronizing cache visualization and cache model--reads must be accounted for.
    // Do not refresh until simulation step completes; only track that write occured.
    // This performance optimization was determined to be necessary from profiling.
    //updateMemory();
}

void CacheView::onSimulationStarted()
{
    inSimulation = true;
    ui->cacheConfiguration->setReadOnly(true);
}

void CacheView::onSimulationFinished()
{
    inSimulation = false;
    ui->cacheConfiguration->setReadOnly(false);
}

void CacheView::onCacheConfigChanged()
{
    auto tag_bits = cache->getTagSize();
    auto index_bits = cache->getIndexSize();
    auto data_bits = cache->getDataSize();

    ui->cacheConfiguration->onCacheConfigChanged();

    ui->tagBox->setMaximum((1<<tag_bits) - 1);
    ui->indexBox->setMaximum((1<<index_bits) - 1);
    ui->dataBox->setMaximum((1<<data_bits) - 1);

    // Ensure that data model is properly sized for cache configuration.
    data->setColumnCount(static_cast<int>(Columns::ColumnCount));
    data->setRowCount((1 << index_bits));
    // Note: Each row in data must be resized to match the associativity of the cache,
    // but this will be handled within ::setRow(...).

    last_updated.clear();
    to_delete.clear();

    // Must set headers after clearing, or headers will be removed.
    data->setHeaderData(static_cast<int>(Columns::IndexColumn), Qt::Horizontal, "Index", Qt::DisplayRole);
    data->setHeaderData(static_cast<int>(Columns::TagColumn), Qt::Horizontal, "Tag", Qt::DisplayRole);
    data->setHeaderData(static_cast<int>(Columns::EvictColumn), Qt::Horizontal, "Next Victim", Qt::DisplayRole);
    data->setHeaderData(static_cast<int>(Columns::AddressColumn), Qt::Horizontal, "Address Range", Qt::DisplayRole);
    data->setHeaderData(static_cast<int>(Columns::PresentColumn), Qt::Horizontal, "Valid", Qt::DisplayRole);
    data->setHeaderData(static_cast<int>(Columns::HitsColumn), Qt::Horizontal, "# References", Qt::DisplayRole);

    // Ensure that eviction collation array is large enough to contain all evicted entries.
    eviction_collate.resize(1<<tag_bits);
    reset_eviction_collation();
}

void CacheView::onSimulationStep()
{
    // Purge cache entries that were swapped out during the previous simulation step.
    for(auto item : to_delete) {
        auto root_item = data->item(item.root_line);
        root_item->removeRows(cache->getAssociativty(), item.count_to_remove);
    }
    to_delete.clear();

    // Undo stylization applied to reference, swapped in cache entries.
    for(auto item : last_updated) {
        auto root_item = data->item(item.root_index);
        for(int col = 0; col<data->columnCount(); col++) {
            auto* child = root_item->child(item.child_row, col);
            auto roles = data->itemData(child->index());
            // Must remove item roles from child, since setItemData(...) will
            // not modify unlisted roles.
            child->clearData();
            // Prevent old / unused roles from accumulating in QMap.
            // Accumulation of these roles hinders switching from light <-> dark mode.
            roles.remove(Qt::FontRole);
            roles.remove(Qt::BackgroundRole);
            // Set the item data with the special stylization removed.
            // Will not modify item roles not in "roles" map. See:
            //      https://doc.qt.io/qt-5/qabstractitemmodel.html#setItemData
            data->setItemData(child->index(), roles);
        }
    }
    // Now that items has been updated, may clear list.
    last_updated.clear();

    // Refresh any cache lines that changed since the start of the last simulation step.
    updateMemory();

}

bool CacheView::remove_entry::operator==(const CacheView::remove_entry &rhs) const
{
    // Removal entries are equvilant if they describe the same root item and remove the same number of rows.
    return (this->root_line == rhs.root_line) && (this->count_to_remove == rhs.count_to_remove);
}

bool CacheView::updated_item::operator==(const CacheView::updated_item &rhs) const
{
    // items are equivilant if they share the same root and row,
    return (this->root_index == rhs.root_index) && (this->child_row == rhs.child_row);
}

CacheViewDelegate::CacheViewDelegate(QSharedPointer<CacheMemory> memory, const PepColors::Colors *colors,
                                     QObject *parent): QStyledItemDelegate(parent),
     memDevice(memory), colors(colors)
{

}

CacheViewDelegate::~CacheViewDelegate()
{

}

void CacheViewDelegate::changeColors(const PepColors::Colors *colors)
{
   this->colors = colors;
}

QWidget *CacheViewDelegate::createEditor(QWidget* , const QStyleOptionViewItem &, const QModelIndex &) const
{
    // Nothing is editable by the user.
    return nullptr;
}

void CacheViewDelegate::setEditorData(QWidget* , const QModelIndex &) const
{
    // Nothing is editable by the ueser.
}

void CacheViewDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const
{
    // Pass geometry information to the editor.
    editor->setGeometry(option.rect);
}

void CacheViewDelegate::setModelData(QWidget *, QAbstractItemModel *, const QModelIndex &) const
{
    // Editor can't cause a change in data.
}

void CacheViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{

    QStyledItemDelegate::paint(painter, option, index);
    if(index.data(EvictedData).toBool()) {

        // Must draw item before box, otherwise line will be overwritten.
        QStyledItemDelegate::paint(painter, option, index);

        // Draw a solid line through the evicted cache lines.
        // Save/restore since this operation modifies painter.
        painter->save();
        painter->setPen(colors->textColor);
        painter->drawLine(option.rect.left(),  option.rect.top() + option.rect.height()/2,
                          option.rect.right(), option.rect.top() + option.rect.height()/2);
        painter->restore();

    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }


}
