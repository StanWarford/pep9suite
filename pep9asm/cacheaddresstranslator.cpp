#include "cacheaddresstranslator.h"
#include "ui_cacheaddresstranslator.h"

#include "cachememory.h"
CacheAddressTranslator::CacheAddressTranslator(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CacheAddressTranslator), cache(nullptr)
{
    ui->setupUi(this);
    // Connect address conversion spin boxes.
    connect(ui->tagBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &CacheAddressTranslator::cache_address_changed);
    connect(ui->lineBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &CacheAddressTranslator::cache_address_changed);
    connect(ui->offsetBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &CacheAddressTranslator::cache_address_changed);

    connect(ui->addressBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &CacheAddressTranslator::address_changed);
}

void CacheAddressTranslator::init(QSharedPointer<CacheMemory> cache)
{
    this->cache = cache;
    connect(cache.get(), &CacheMemory::configurationChanged, this, &CacheAddressTranslator::onCacheConfigChanged);
    onCacheConfigChanged();
}

CacheAddressTranslator::~CacheAddressTranslator()
{
    delete ui;
}

void CacheAddressTranslator::onCacheConfigChanged()
{
    auto tag_bits = cache->getTagSize();
    auto index_bits = cache->getIndexSize();
    auto data_bits = cache->getDataSize();

    ui->tagBox->setMaximum((1<<tag_bits) - 1);
    ui->lineBox->setMaximum((1<<index_bits) - 1);
    ui->offsetBox->setMaximum((1<<data_bits) - 1);
    ui->addressBox->setMaximum(cache->maxAddress());
}

void CacheAddressTranslator::cache_address_changed(int)
{
    auto index_bits = cache->getIndexSize();
    auto data_bits = cache->getDataSize();

    int address = 0;
    address += ui->tagBox->value() << (index_bits + data_bits);
    address += ui->lineBox->value() << (data_bits);
    address += ui->offsetBox->value();

    // Block signals during update to prevent signal cascade.
    bool old_block = ui->addressBox->blockSignals(true);
    ui->addressBox->setValue(address);
    ui->addressBox->blockSignals(old_block);
}

void CacheAddressTranslator::address_changed(int value)
{

    auto breakdown = cache->breakdownAddress(value);
    ui->tagBox->setValue(breakdown.tag);
    ui->lineBox->setValue(breakdown.index);
    ui->offsetBox->setValue(breakdown.offset);

    // Block signals during update to prevent signal cascade.
    bool old_block = ui->tagBox->blockSignals(true);
    ui->tagBox->setValue(breakdown.tag);
    ui->tagBox->blockSignals(old_block);

    old_block = ui->lineBox->blockSignals(true);
    ui->lineBox->setValue(breakdown.index);
    ui->lineBox->blockSignals(old_block);

    old_block = ui->offsetBox->blockSignals(true);
    ui->offsetBox->setValue(breakdown.offset);
    ui->offsetBox->blockSignals(old_block);
}
