#include "cacheview.h"
#include "ui_cacheview.h"

#include <QDebug>

#include "cachememory.h"
#include "cachealgs.h"
#include "pep.h"

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
    ui->cacheTree->setModel(data);
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
    auto tag_bits = cache->geTagSize();
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
    // Assign values to each column
    data->itemFromIndex(data->index(entry, 0, lineIndex))->setData(c0_name, Qt::DisplayRole);
    data->itemFromIndex(data->index(entry, 1, lineIndex))->setCheckState(c1_evict);
    data->itemFromIndex(data->index(entry, 2, lineIndex))->setData(c2_address, Qt::DisplayRole);
    data->itemFromIndex(data->index(entry, 3, lineIndex))->setData(entryPtr->is_present, Qt::DisplayRole);
    data->itemFromIndex(data->index(entry, 4, lineIndex))->setData(c4_hits, Qt::DisplayRole);
}

void CacheView::refreshMemory()
{
    auto tag_bits = cache->geTagSize();

    data->clear();
    data->insertColumns(0, 5);
    data->insertRows(0, (1 << tag_bits));
    data->setHeaderData(0, Qt::Horizontal, "Tag/Index", Qt::DisplayRole);
    data->setHeaderData(1, Qt::Horizontal, "Next Eviction", Qt::DisplayRole);
    data->setHeaderData(2, Qt::Horizontal, "Address Range", Qt::DisplayRole);
    data->setHeaderData(3, Qt::Horizontal, "Present", Qt::DisplayRole);
    data->setHeaderData(4, Qt::Horizontal, "# Hits", Qt::DisplayRole);

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
    auto tag_bits = cache->geTagSize();
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
}
