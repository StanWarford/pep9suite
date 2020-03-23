#ifndef CACHEVIEW_H
#define CACHEVIEW_H

#include <QWidget>
#include <QSharedPointer>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QSet>

#include "cacheline.h"
#include "colors.h"

static const quint16 TagColumn = 0;
static const quint16 EvictColumn = 1;
static const quint16 Address = 2;
static const quint16 Present = 3;
static const quint16 Hits = 4;

class CacheMemory;
namespace Ui {
    class CacheView;
}

class CacheViewDelegate;

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

    // Clear the modified indicators and remove purged entries.
    // Separates the simulation "stepping" and clearing available
    void onSimulationStep();

private slots:
    void address_changed(int value);
    void cachetag_changed(int value);
private:
    Ui::CacheView *ui;
    QStandardItemModel* data;

    CacheViewDelegate* del;
    const PepColors::Colors *colors;
    QSharedPointer<CacheMemory> cache;
    bool inSimulation = false;
    QFont activeFont;
    void refreshLine(quint16 line);
    void setRow(quint16 line, const CacheLine* linePtr, quint16 entry, const CacheEntry* entryPtr, bool evicted=false);

    /*
     * Structs, functions, and members used to track which lines in the cache view
     * have additional stylization that needs to be removed at the start of the
     * next simulation step.
     */
    struct updated_item {
        quint16 root_index;
        quint16 child_row;
        bool operator==(const updated_item& rhs) const;
    };
    uint friend qHash(const CacheView::updated_item &key) {return (key.root_index<<16) + key.child_row;}
    QSet<updated_item> last_updated;

    /*
     * Structs, functions, and members used to track which lines in the cache view
     * have "evicted" entries appended to the end. These evicted entries need to be
     * removed at the start of the next simulation step.
     */
    struct remove_entry{
        quint16 root_line;
        quint16 count;
        bool operator==(const remove_entry& rhs) const;
    };

    uint friend qHash(const CacheView::remove_entry &key) {return (key.root_line<<16) + key.count;}
    QSet<remove_entry> to_delete;

};

/*
 * Item delegate that handles input validation of hex constants, and disables editing of address and hex dump columns.
 * Eventually, it can be extended to be signaled to enable or disable editing
 */
class CacheViewDelegate: public QStyledItemDelegate {
private:
    QSharedPointer<CacheMemory> memDevice;
    bool canEdit;
    const PepColors::Colors *colors;
public:
    CacheViewDelegate(QSharedPointer<CacheMemory> memory, const PepColors::Colors *colors, QObject* parent = nullptr);
    virtual ~CacheViewDelegate() override;
    void changeColors(const PepColors::Colors *colors);
    // See http://doc.qt.io/qt-5/qstyleditemdelegate.html#subclassing-qstyleditemdelegate for explanation on the methods being reimplemented.

    // If the index is editable, create an editor that validates byte hex constants, otherwise return nullptr
    virtual QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    // Provides editor widget with starting data for editing.
    virtual void setEditorData(QWidget * editor, const QModelIndex & index) const override;
    // Ensure that editor is displayed correctly on the item view
    virtual void updateEditorGeometry(QWidget * editor,
                                      const QStyleOptionViewItem & option,
                                      const QModelIndex & index) const override;
    // Handle updating data in the model via calling the memorySection
    virtual void setModelData(QWidget *editor,
                              QAbstractItemModel *model,
                              const QModelIndex &index) const override;
    // Override painting method to allow drawing of vertical bars in dump pane.
    virtual void paint(QPainter *painter,
                       const QStyleOptionViewItem &option,
                       const QModelIndex &index ) const override;
};

#endif // CACHEVIEW_H
