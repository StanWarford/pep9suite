#ifndef CACHEADDRESSTRANSLATOR_H
#define CACHEADDRESSTRANSLATOR_H

#include <QSharedPointer>
#include <QWidget>


class CacheMemory;

namespace Ui {
class CacheAddressTranslator;
}

class CacheAddressTranslator : public QWidget
{
    Q_OBJECT

public:
    explicit CacheAddressTranslator(QWidget *parent = nullptr);
    void init(QSharedPointer<CacheMemory> cache);
    ~CacheAddressTranslator();

public slots:
    void onCacheConfigChanged();
private slots:
    void cache_address_changed(int);
    void address_changed(int value);
private:
    Ui::CacheAddressTranslator *ui;
    QSharedPointer<CacheMemory> cache;
};

#endif // CACHEADDRESSTRANSLATOR_H
