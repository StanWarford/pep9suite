#include "cacheconfig.h"
#include "ui_cacheconfig.h"
#include "cachealgs.h"
#include "cachememory.h"

#include <QMessageBox>

CacheConfig::CacheConfig(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CacheConfig)
{
    ui->setupUi(this);
}

CacheConfig::~CacheConfig()
{
    delete ui;
}

void CacheConfig::init(QSharedPointer<CacheMemory> cache, bool enableCacheChanges)
{
    this->cache = cache;
    this->enableCacheChanges = enableCacheChanges;
    ui->updateButton->setEnabled(enableCacheChanges);
    ui->updateButton->setVisible(enableCacheChanges);

    QMetaObject meta = CacheAlgorithms::staticMetaObject;
    QMetaEnum metaEnum = meta.enumerator(meta.indexOfEnumerator("CacheAlgorithms"));
    for(auto keyIndex = 0; keyIndex < metaEnum.keyCount(); keyIndex++) {
        ui->replacementCombo->addItem(QString(metaEnum.key(keyIndex)));
    }
}

void CacheConfig::setReadOnly(bool readOnly)
{
    bool enabled = ~readOnly;
    ui->tagBits->setEnabled(enabled);
    ui->indexBits->setEnabled(enabled);
    // # of offset bits calculated from
    //ui->offsetBits->setEnabled(false);
    ui->writeAllocationCombo->setEnabled(enabled);
    ui->replacementCombo->setEnabled(enabled);
    ui->associativityNum->setEnabled(enabled);
    ui->updateButton->setEnabled(enabled & enableCacheChanges);
}

void CacheConfig::onCacheConfigChanged()
{
    auto tag_bits = cache->getTagSize();
    auto index_bits = cache->getIndexSize();
    auto data_bits = cache->getDataSize();
    auto associativity = cache->getAssociativty();

    ui->tagBits->setValue(tag_bits);
    ui->indexBits->setValue(index_bits);
    ui->offsetBits->setValue(data_bits);
    ui->associativityNum->setValue(associativity);
    ui->replacementCombo->setCurrentIndex(ui->replacementCombo->findText(cache->getCacheAlgorithm()));
    if(cache->getAllocationPolicy() == Cache::WriteAllocationPolicy::NoWriteAllocate) {
        ui->writeAllocationCombo->setCurrentIndex(0);
    } else {
        ui->writeAllocationCombo->setCurrentIndex(1);
    }

    // Anytime the cache is re-configured, it is cleared.
    updateButtonRefresh();
    valuesChanged = false;
}

void CacheConfig::onFontChanged(QFont)
{
    // No operation.
    // Added to mantain consistent interface across widgets.
}

void CacheConfig::onDarkModeChanged(bool)
{
    // No operation.
    // Added to mantain consistent interface across widgets.
}

void CacheConfig::updateButtonRefresh()
{
    if(enableCacheChanges) {
        ui->updateButton->setEnabled(valuesChanged);
    }

}

void CacheConfig::updateAddressBits()
{
    quint16 tag = ui->tagBits->value();
    quint16 index = ui->indexBits->value();
    // Compute size of offset from current tag, index bits.
    quint16 remaining = memory_bits - index - tag;

    // Place upper bound on tag.
    ui->tagBits->setMaximum(tag + remaining - 1);
    // Place upper bound on index.
    ui->indexBits->setMaximum(index + remaining - 1);
    // Set number of data bits.
    ui->offsetBits->setValue(remaining);
}

void CacheConfig::on_tagBits_valueChanged(int newValue)
{
    if(cache->getTagSize() != newValue) valuesChanged |= true;
    updateButtonRefresh();
    updateAddressBits();
}

void CacheConfig::on_indexBits_valueChanged(int newValue)
{
    if(cache->getIndexSize() != newValue) valuesChanged |= true;
    updateButtonRefresh();
    updateAddressBits();
}

void CacheConfig::on_offsetBits_valueChanged(int /*newValue*/)
{
    // No operation, since offset is computed from tag, index bits.
}

void CacheConfig::on_associativityNum_valueChanged(int newValue)
{
    if(cache->getAssociativty() != newValue) valuesChanged |= true;
    updateButtonRefresh();
    updateAddressBits();
}

void CacheConfig::on_replacementCombo_currentIndexChanged(int)
{
    // Text in combo box is set via the CacheAlgorithms enum, which should be
    // identical to at least one caching algorithm used by the cache.
    if(cache->getCacheAlgorithm() != ui->replacementCombo->currentText()) valuesChanged |= true;
    updateButtonRefresh();
}

void CacheConfig::on_writeAllocationCombo_currentIndexChanged(int)
{
    Cache::WriteAllocationPolicy policy;
    // Convert from true/false string to enumerated write allocation policy.
    if(ui->writeAllocationCombo->currentIndex() == 0) {
        policy = Cache::WriteAllocationPolicy::NoWriteAllocate;
    } else {
        policy = Cache::WriteAllocationPolicy::WriteAllocate;
    }
    if(cache->getAllocationPolicy() != policy) valuesChanged |= true;
    updateButtonRefresh();
}

void CacheConfig::on_updateButton_pressed()
{
    if(valuesChanged) {
        Cache::CacheConfiguration config;

        // No math needs to be done to confirm that bits are correct.
        // If the bits do not sum correctly, then Cache::resize(...) will return
        // that the new configuration is invalid.
        config.tag_bits = ui->tagBits->value();
        config.index_bits = ui->indexBits->value();
        config.data_bits = ui->offsetBits->value();
        config.associativity = ui->associativityNum->value();

        // Convert from write allocation being true / false to the enumerated value.
        if(ui->writeAllocationCombo->currentIndex() == 0) {
            config.write_allocation = Cache::WriteAllocationPolicy::NoWriteAllocate;
        } else {
            config.write_allocation = Cache::WriteAllocationPolicy::WriteAllocate;
        }

        // Use Qt enumeration objects to convert index in combo box to associated
        // enumerated item.
        QMetaObject meta = CacheAlgorithms::staticMetaObject;
        QMetaEnum metaEnum = meta.enumerator(meta.indexOfEnumerator("CacheAlgorithms"));
        using algorithm = CacheAlgorithms::CacheAlgorithms;
        algorithm alg = static_cast<algorithm>(metaEnum.value(ui->replacementCombo->currentIndex()));

        // Should be moved to a factory in CacheAlgs, but so far this code is only needed in
        // this place.
        switch(alg){
        case algorithm::LRU:
            config.policy = QSharedPointer<LRUFactory>::create(config.associativity);
            break;
        case algorithm::MRU:
            config.policy = QSharedPointer<MRUFactory>::create(config.associativity);
            break;
        case algorithm::BPLRU:
            config.policy = QSharedPointer<BPLRUFactory>::create(config.associativity);
            break;
        case algorithm::LFU:
            config.policy = QSharedPointer<LFUFactory>::create(config.associativity);
            break;
        case algorithm::MFU:
            config.policy = QSharedPointer<MFUFactory>::create(config.associativity);
            break;
        case algorithm::FIFO:
            config.policy = QSharedPointer<FIFOFactory>::create(config.associativity);
            break;
        case algorithm::Random:
            config.policy = QSharedPointer<RandomFactory>::create(config.associativity);
            break;
        }

        bool success = cache->resizeCache(config);
        if(!success) {
            QMessageBox::warning(this, "Cache Configuration Invalid", "Cache did not configure correctly.");
        }
        onCacheConfigChanged();

        valuesChanged = false;
    }
    updateButtonRefresh();
}
