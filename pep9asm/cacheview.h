#ifndef CACHEVIEW_H
#define CACHEVIEW_H

#include <QWidget>
#include <QSharedPointer>
#include <QStandardItemModel>

#include "cacheline.h"

static const quint16 TagColumn = 0;
static const quint16 PresentColumn =1;
static const quint16 Hits = 2;
static const quint16 FirstData = 3;

class CacheMemory;
namespace Ui {
class CacheView;
}

class CacheView : public QWidget
{
    Q_OBJECT

public:
    explicit CacheView(QWidget *parent = nullptr);
    virtual ~CacheView() override;

    // Needs to be called after construction but before this class can be used, otherwise the class is in an incomplete state.
    void init(QSharedPointer<CacheMemory> cache);

    // Rescale the cache view to the new cache parameters.
    void rebuild();

    // Re-render cache based on memory updates.
    void refreshMemory();
    // Post: All memory address are re-rendered.
    // When performing small updates (such as updating 10-20 addresses), updateMemory() is faster.
    // However, when all of memory must be updated, this method is prefered.

    void updateMemory();
    // Post: All memory addresses written to in internal memDevice will be updated.
    // This method is very slow when the number of updates to be performed is large.
    // If a significant portion of memory addresses were modified, refreshMemory() is more performant.
    // These addressed are accessed via memDevice->getBytesSet(), memDevice->getBytesWritten().
    // The memDevice's modified address cache will NOT be cleared.

    void highlightOnFocus();
    // Post: Highlights the label based on the label window color saved in the UI file

    bool hasFocus();
    // Post: returns if the pane has focus

public slots:
    void onFontChanged(QFont font);
    // Post: the font used in the cache view is updated to be "font".

    // Handle switching styles to and from dark mode & potential re-highlighting
    void onDarkModeChanged(bool darkMode);

    // Allow memory lines to be updated whenever an address is changed.
    void onMemoryChanged(quint16 address, quint8 newValue);

    void onSimulationStarted();
    void onSimulationFinished();

    // Whenever the cache configuration is changed, update buttons and translation boxes.
    void onCacheConfigChanged();

private slots:
    void address_changed(int value);
    void cachetag_changed(int value);
private:
    Ui::CacheView *ui;
    QStandardItemModel* data;
    QSharedPointer<CacheMemory> cache;
    bool inSimulation = false;
    void refreshLine(quint16 line);
    void setRow(quint16 line, const CacheLine* linePtr, quint16 entry, const CacheEntry* entryPtr);

};

#endif // CACHEVIEW_H
