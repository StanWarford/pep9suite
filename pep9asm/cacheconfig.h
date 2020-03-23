#ifndef CACHECONFIG_H
#define CACHECONFIG_H

#include <QWidget>
#include <QSharedPointer>

class CacheMemory;


namespace Ui {
class CacheConfig;
}

class CacheConfig : public QWidget
{
    Q_OBJECT

public:
    explicit CacheConfig(QWidget *parent = nullptr);
    ~CacheConfig();
    void init(QSharedPointer<CacheMemory> cache, bool enableCacheChanges);
    // Change if configuration boxes are read
    void setReadOnly(bool readOnly);

public slots:
    void onCacheConfigChanged();

private:
    Ui::CacheConfig *ui;
    QSharedPointer<CacheMemory> cache;
    bool valuesChanged = false;
    bool enableCacheChanges = true;
    // Determine if the "Update Configuration" button should be enabled or not
    void updateButtonRefresh();
    // Compute the offset bits from the index, tag bits. Also compute the maximum
    // value that may be placed in either tag or index fields.
    void updateAddressBits();
    // Number of bits needed to represent a memory address.
    static const quint16 memory_bits = 16;

    // Respond to values in configuration being changed.
private slots:
    void on_tagBits_valueChanged(int newValue);
    void on_indexBits_valueChanged(int newValue);
    void on_offsetBits_valueChanged(int newValue);
    void on_associativityNum_valueChanged(int newValue);
    void on_replacementCombo_currentIndexChanged(int);
    void on_writeAllocationCombo_currentIndexChanged(int);
    void on_updateButton_pressed();
};

#endif // CACHECONFIG_H
