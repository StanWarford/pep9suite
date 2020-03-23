#include "cacheview.h"
#include "ui_cacheview.h"

#include <QDebug>
#include <QPainter>

#include "cachememory.h"
#include "cachealgs.h"
#include "pep.h"

static const int EvictedData = Qt::UserRole + 1;

CacheView::CacheView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CacheView), data(new QStandardItemModel(this)),
    colors(&PepColors::lightMode), cache(nullptr)
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

void CacheView::refreshLine(quint16 line)
{
    // Determine cache parameters.
    auto tag_bits = cache->getTagSize();
    auto index_bits = cache->getIndexSize();
    auto data_bits = cache->getDataSize();
    auto associativity = cache->getAssociativty();

    assert(line < (1<<tag_bits));

    // Make sure cache line is present, name correctly, and has the right address.
    auto* lineItem = data->item(line, 0);
    if(lineItem == nullptr) {
        lineItem = new QStandardItem(QString("%1").arg(line));
        data->setItem(line, 0, lineItem);
    }
    auto lineIndex = data->index(line, 0);

    // Ensure the right number of rows / columns are in the cache line.
    if(data->columnCount(lineIndex) != 5) lineItem->setColumnCount(5);
    if(data->rowCount(lineIndex) != associativity) lineItem->setRowCount(associativity);

    auto* addressItem = data->item(line, 2);
    if(addressItem == nullptr) {
        addressItem = new QStandardItem();
        data->setItem(line, 2, addressItem);
    }
    addressItem->setText(QString("%1-%2")
                         .arg(line<<(index_bits+data_bits),4, 16)
                         .arg((((line+1)<<(index_bits+data_bits)) - 1), 4, 16));

    auto lineEntry = cache->getCacheLine(line);
    if(!lineEntry.has_value()) {
        qDebug() << "Something went horribly wrong with a line.";
    }

    auto* linePtr = lineEntry.value();
    // Determine if the entire cache line is not present.
    bool blank = false;

    for(int entry = 0; entry < associativity; entry++) {
        auto entryOpt = lineEntry.value()->get_entry(entry);
        if(!entryOpt.has_value()) {
            qDebug() << "Something went horribly wrong with an entry.";
        }

        // If any cache entry is present in the line, then the entire line is displayable.
        auto entryPtr = entryOpt.value();
        blank |= entryPtr->is_present;

        setRow(line, linePtr, entry, entryPtr);
    }

    // Render evicted cache entries for the current cache line.
    if(auto evicted = cache->getEvictedEntry(line);
           !evicted.empty()) {
        auto root_item = data->item(line);
        int entry_number = root_item->rowCount();
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

    // When there are more lines than ways in the cache, it means that "evicted"
    // entries have been added, an must be removed at the start of the next simulation step.
    if(lineItem->rowCount()>cache->getAssociativty()){
        remove_entry entry;
        entry.root_line = line;
        entry.count = lineItem->rowCount() - cache->getAssociativty();
        to_delete.insert(entry);
    }

    // Display the row if it is not blank.
    ui->cacheTree->setRowHidden(line, QModelIndex(), !blank);
}

void CacheView::setRow(quint16 line, const CacheLine* linePtr, quint16 entry, const CacheEntry *entryPtr, bool evicted)
{
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
    QString c0_name = "";
    Qt::CheckState c1_evict = Qt::CheckState::Unchecked;
    QString c2_address = "";
    QVariant c4_hits = "";

    // If the cache entry is present, display the index # and addresses spanned by
    // the entry in columns 0,2. Also determine hit count.
    if(entryPtr->is_present) {
        c0_name = QString("%1").arg(entryPtr->index);
        c2_address = QString("%1-%2")
                .arg((line<<(index_bits+data_bits)) + ((entryPtr->index)<<data_bits),4, 16)
                .arg((line<<(index_bits+data_bits)) + ((entryPtr->index+1)<<data_bits) - 1, 4, 16);
        c4_hits = entryPtr->hit_count;
    }

    // Cache hit count, so that it is easier to see from debugger.
    quint16 eviction_candidate = linePtr->get_replacement_policy()->eviction_loohahead();
    if(eviction_candidate == entry) {
        c1_evict = Qt::CheckState::Checked;
    }

    auto lineIndex = data->index(line, 0);

    // Convert values in existing cells to usable types for comparisons.
    // These conversions will fail when the box is empty, so must check
    // ok_conv_* when attemtping to use associated integer.
    bool ok_conv_index, ok_conv_hits;
    int listedIndex = data->index(entry, TagColumn, lineIndex).data().toInt(&ok_conv_index);
    quint32 listedHits = data->index(entry, Hits, lineIndex).data().toInt(&ok_conv_hits);

    // If evicted, indicate to Style Delegate to apply special evicted formatting.
    if(evicted) {
        // Unlike following branches, do not track properties of special entries.
        // Tracking # of evicted entry rows is done at the point where the rows are added.
        for(int col=0; col<data->columnCount(); col++) {
            data->itemFromIndex(data->index(entry, col, lineIndex))->setData(true, EvictedData);
        }
    }
    // Highlight "new" items.
    // An item is "new" if it is present, and either the old value was not present,
    // or the old entry number is not the new entry number.
    else if(entryPtr->is_present && (!ok_conv_index || listedIndex != entryPtr->index)) {

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
    else if(entryPtr->is_present && (!ok_conv_index || entryPtr->hit_count != listedHits)) {
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
    data->itemFromIndex(data->index(entry, TagColumn, lineIndex))->setData(c0_name, Qt::DisplayRole);
    data->itemFromIndex(data->index(entry, EvictColumn, lineIndex))->setCheckState(c1_evict);
    data->itemFromIndex(data->index(entry, Address, lineIndex))->setData(c2_address, Qt::DisplayRole);
    data->itemFromIndex(data->index(entry, Present, lineIndex))->setData(entryPtr->is_present, Qt::DisplayRole);
    data->itemFromIndex(data->index(entry, Hits, lineIndex))->setData(c4_hits, Qt::DisplayRole);
}

void CacheView::refreshMemory()
{
    // Refresh each cache line individually.
    auto tag_bits = cache->getTagSize();

    for(int line = 0; line < 1<<tag_bits; line++) {
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
    modifiedLines.unite(QSet<quint16>(keys.begin(), keys.end()));

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
    // TODO: respond to font change.
    activeFont = font;
    // DIsable font stylization used to indicate cache access.
    // This includes bolding, italics.
    activeFont.setBold(false);
    activeFont.setItalic(false);
    activeFont.setStrikeOut(false);
    ui->cacheTree->setFont(activeFont);
    // Propogate event to child.
    ui->cacheConfiguration->onFontChanged(font);
}

void CacheView::onDarkModeChanged(bool darkMode)
{
    // TODO: respond to color changes after determining how to highlight
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
    // W do not have a write-allocate cache (by default),
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
    data->setColumnCount(5);
    data->setRowCount((1 << tag_bits));

    last_updated.clear();
    to_delete.clear();

    // Must set headers after clearing, or headers will be removed.
    data->setHeaderData(0, Qt::Horizontal, "Tag/Index", Qt::DisplayRole);
    data->setHeaderData(1, Qt::Horizontal, "Next Eviction", Qt::DisplayRole);
    data->setHeaderData(2, Qt::Horizontal, "Address Range", Qt::DisplayRole);
    data->setHeaderData(3, Qt::Horizontal, "Present", Qt::DisplayRole);
    data->setHeaderData(4, Qt::Horizontal, "# Hits", Qt::DisplayRole);
}

void CacheView::onSimulationStep()
{
    // Purge cache entries that were swapped out during the previous simulation step.
    for(auto item : to_delete) {
        auto root_item = data->item(item.root_line);
        root_item->removeRows(cache->getAssociativty(), item.count);
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
    last_updated.clear();

    // Refresh any cache lines that changed since the start of the last simulation step.
    updateMemory();

}

bool CacheView::remove_entry::operator==(const CacheView::remove_entry &rhs) const
{
    return (this->root_line == rhs.root_line) && (this->count == rhs.count);
}

bool CacheView::updated_item::operator==(const CacheView::updated_item &rhs) const
{
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
