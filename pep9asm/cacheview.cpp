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
    ui(new Ui::CacheView), data(new QStandardItemModel(this)), cache(nullptr)
{
    ui->setupUi(this);
    QMetaObject meta = CacheAlgorithms::staticMetaObject;
    QMetaEnum metaEnum = meta.enumerator(meta.indexOfEnumerator("CacheAlgorithms"));
    for(auto keyIndex = 0; keyIndex < metaEnum.keyCount(); keyIndex++) {
        ui->replacementCombo->addItem(QString(metaEnum.key(keyIndex)));
    }
    ui->cacheTree->setFont(Pep::codeFont);
    // Connect address conversion spin boxes.
    connect(ui->tagBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &CacheView::cachetag_changed);
    connect(ui->indexBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &CacheView::cachetag_changed);
    connect(ui->dataBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &CacheView::cachetag_changed);

    connect(ui->addressBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &CacheView::address_changed);

    // Connect cache configuration boxes.

    // Create cache delegate
    del = new CacheViewDelegate(cache);
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
    // When there are more lines than ways in the cache, it means that
    if(lineItem->rowCount()>associativity){
        remove_entry entry;
        entry.root_line = line;
        entry.count = lineItem->rowCount() - associativity;
        to_delete.insert(entry);
    }

    // Display the row if it is not blank.
    ui->cacheTree->setRowHidden(line, QModelIndex(), !blank);
}

void CacheView::setRow(quint16 line, const CacheLine* linePtr, quint16 entry, const CacheEntry *entryPtr)
{
    auto index_bits = cache->getIndexSize();
    auto data_bits = cache->getDataSize();

    // Determine which values are needed in each column
    QString c0_name = "";
    Qt::CheckState c1_evict = Qt::CheckState::Unchecked;
    QString c2_address = "";
    QVariant c4_hits = "";

    if(entryPtr->is_present) {
        c0_name = QString("%1").arg(entryPtr->index);
        c2_address = QString("%1-%2")
                .arg((line<<(index_bits+data_bits)) + ((entryPtr->index)<<data_bits),4, 16)
                .arg((line<<(index_bits+data_bits)) + ((entryPtr->index+1)<<data_bits) - 1, 4, 16);
        c4_hits = entryPtr->hit_count;
    }


    quint16 eviction_candidate = linePtr->get_replacement_policy()->eviction_loohahead();
    if(eviction_candidate == entry) {
        c1_evict = Qt::CheckState::Checked;
    }

    auto lineIndex = data->index(line, 0);

    // Highlight "new" items
    if(entryPtr->is_present && data->index(entry, TagColumn, lineIndex).data() != QVariant(entry)) {
        updated_item item;
        item.root_index = line;
        item.child_row = entry;
        this->last_updated.insert(item);

        auto root_item = data->item(line);
        for(int col=0; col<data->columnCount(); col++) {
            data->itemFromIndex(data->index(entry, col, lineIndex))->setBackground(Qt::red);
        }

        // If evicting an existing row, must
        if(root_item->child(entry, Present)->data(Qt::DisplayRole).toBool()) {
            QList<QStandardItem*> new_items;
            for(int col=0; col<data->columnCount(); col++)  {
                new_items.append(new QStandardItem());
            }
            root_item->appendRow(new_items);

            for(int col=0; col<data->columnCount(); col++)  {
                data->itemFromIndex(data->index(entry, col, lineIndex))->setBackground(Qt::red);
                auto line_data = data->index(entry, col, lineIndex).data();
                data->itemFromIndex(data->index(root_item->rowCount()-1, col, lineIndex))->setData(line_data, Qt::DisplayRole);
                data->itemFromIndex(data->index(root_item->rowCount()-1, col, lineIndex))->setData(true, EvictedData);
            }
        }
    }
    else if(entryPtr->is_present && entryPtr->hit_count != data->index(entry, Hits, lineIndex).data().toInt()) {
        QFont regularFont = activeFont;
        regularFont.setBold(true);

        updated_item item;
        item.root_index = line;
        item.child_row = entry;
        this->last_updated.insert(item);

        for(int col=0; col<data->columnCount(); col++)  {
            data->itemFromIndex(data->index(entry, col, lineIndex))->setFont(regularFont);
        }
    }

    // Assign values to each column based on the present cache entry.
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
    QSet<quint16> modifiedLines = (cache->getCacheLinesTouched());
    // Caches also change with reads since the previous cycle.
    for(auto item : to_delete) {
        auto root_item = data->item(item.root_line);
        root_item->removeRows(cache->getAssociativty(), item.count);
    }
    to_delete.clear();

    for(auto item : last_updated) {
        auto root_item = data->item(item.root_index);
        for(int col = 0; col<data->columnCount(); col++) {
            auto* child = root_item->child(item.child_row, col);
            child->setBackground(Qt::white);
            QFont font = child->font();
            font.setBold(false);
            font.setStrikeOut(false);
            child->setFont(font);
        }
    }
    last_updated.clear();

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
}

void CacheView::onDarkModeChanged(bool darkMode)
{
    // TODO: respond to color changes after determining how to highlight
}

void CacheView::onMemoryChanged(quint16 address, quint8 /*newValue*/)
{
    auto breakdown = cache->breakdownAddress(address);
    refreshLine(breakdown.tag);
}

void CacheView::onSimulationStarted()
{
    inSimulation = true;
}

void CacheView::onSimulationFinished()
{
    inSimulation = false;
}

void CacheView::onCacheConfigChanged()
{
    auto tag_bits = cache->getTagSize();
    auto index_bits = cache->getIndexSize();
    auto data_bits = cache->getDataSize();
    auto associativity = cache->getAssociativty();

    ui->tagBox->setMaximum((1<<tag_bits) - 1);
    ui->tagBits->setValue(tag_bits);
    ui->indexBox->setMaximum((1<<index_bits) - 1);
    ui->indexBits->setValue(index_bits);
    ui->dataBox->setMaximum((1<<data_bits) - 1);
    ui->dataBits->setValue(data_bits);
    ui->associativityNum->setValue(associativity);
    ui->replacementCombo->setCurrentIndex(ui->replacementCombo->findText(cache->getCacheAlgorithm()));
    if(cache->getAllocationPolicy() == Cache::WriteAllocationPolicy::WriteAllocate) {
        ui->writeAllocationCombo->setCurrentIndex(0);
    } else {
        ui->writeAllocationCombo->setCurrentIndex(1);
    }

    // Ensure that data model is properly sized for cache configuration.
    data->insertColumns(0, 5);
    data->setRowCount((1 << tag_bits));

    // Must set headers after clearing, or headers will be removed.
    data->setHeaderData(0, Qt::Horizontal, "Tag/Index", Qt::DisplayRole);
    data->setHeaderData(1, Qt::Horizontal, "Next Eviction", Qt::DisplayRole);
    data->setHeaderData(2, Qt::Horizontal, "Address Range", Qt::DisplayRole);
    data->setHeaderData(3, Qt::Horizontal, "Present", Qt::DisplayRole);
    data->setHeaderData(4, Qt::Horizontal, "# Hits", Qt::DisplayRole);
}

bool CacheView::remove_entry::operator==(const CacheView::remove_entry &rhs) const
{
    return (this->root_line == rhs.root_line) && (this->count == rhs.count);
}

bool CacheView::updated_item::operator==(const CacheView::updated_item &rhs) const
{
    return (this->root_index == rhs.root_index) && (this->child_row == rhs.child_row);
}

CacheViewDelegate::CacheViewDelegate(QSharedPointer<CacheMemory> memory, QObject *parent): QStyledItemDelegate(parent),
    memDevice(memory)
{

}

CacheViewDelegate::~CacheViewDelegate()
{

}

QWidget *CacheViewDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // Nothing is editable by the user.
    return nullptr;
}

void CacheViewDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    // Nothing is editable by the ueser.
}

void CacheViewDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const
{
    // Pass geometry information to the editor.
    editor->setGeometry(option.rect);
}

void CacheViewDelegate::setModelData(QWidget *editor, QAbstractItemModel *, const QModelIndex &index) const
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
#pragma message("TODO: Handle dark/light mode.")
        painter->save();
        painter->setPen(Qt::black);
        painter->drawLine(option.rect.left(),  option.rect.top() + option.rect.height()/2,
                          option.rect.right(), option.rect.top() + option.rect.height()/2);
        painter->restore();

    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }


}
