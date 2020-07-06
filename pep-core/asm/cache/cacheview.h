// File: cacheview.h
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
#ifndef CACHEVIEW_H
#define CACHEVIEW_H

#include <QWidget>
#include <QSharedPointer>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QSet>

#include "cache/cacheline.h"
#include "style/colors.h"



class CacheMemory;
namespace Ui {
    class CacheView;
}

class CacheViewDelegate;

/*
 * When an index has been evicted multiple times during a single simulation step (like during a trap instruction),
 * only the most recent eviction for each simulation step is kept. Otherwise, dozens to hundreds of eviction entries may flood the screen.
 * This behavior may change in the future, so it may currently be enabled or disabled at compile time using the following macro.
 */
static const bool COLLATE_EVICTIONS = true;

/*
 * The following class graphically represents the state of a cache device.
 */
class CacheView : public QWidget
{
    enum class Columns {
        IndexColumn = 0,
        TagColumn=1, EvictColumn=2, AddressColumn=3,
        PresentColumn=4, HitsColumn=5, ColumnCount
    };
    Q_OBJECT

public:
    explicit CacheView(QWidget *parent = nullptr);
    virtual ~CacheView() override;

    // Needs to be called after construction but before this class can be used, otherwise the class is in an incomplete state.
    void init(QSharedPointer<CacheMemory> cache);

    // Rescale the cache view to the new cache parameters.
    void rebuild();

    void clearOldHighlights();

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
signals:
    void requestCacheHighlighting(quint16 lower, quint16 upper);
    // Signal to other memory views that a range of addresses has been highlighted.

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

    // Determine if a right click context menu needs to be made for a cache entry.
    // If yes, cache the addressed spanned in menu_tag_index.
    void handle_custom_menu(const QPoint& point);

    // Emit requestCacheHighlighting with the most recently selected addresses.
    void accept_show_in_memory();

    void onExpandChanged(const QModelIndex &index);
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
    uint friend qHash(const CacheView::updated_item &key) {return
                static_cast<uint>((key.root_index<<16) + key.child_row);
    }
    QSet<updated_item> last_updated;

    /*
     * Structs, functions, and members used to track which lines in the cache view
     * have "evicted" entries appended to the end. These evicted entries need to be
     * removed at the start of the next simulation step. It indicates to remove the last
     * "count_to_remove" lines from the corresponding root entry.
     */
    struct remove_entry{
        quint16 root_line;
        quint16 count_to_remove;
        bool operator==(const remove_entry& rhs) const;
    };
    // Must define qHash to use QSet.
    uint friend qHash(const CacheView::remove_entry &key) {
        return static_cast<uint>((key.root_line<<16) + key.count_to_remove);
    }
    QSet<remove_entry> to_delete;

    // When an index has been evicted multiple times during a single simulation step, only show the most recent eviction.
    // This prevents users from being overwhelmed by a large number of evictions in a single step.
    void reset_eviction_collation();
    mutable QVector<std::tuple<CacheEntry, int>> eviction_collate;

    // Cache the last row that was right-clicked, since the information is hard to recover from the context menu's events.
    std::tuple<quint16, quint16> menu_tag_index;

};

/*
 * Item delegate that handles input validation of hex constants, and disables editing of address and hex dump columns.
 * Eventually, it can be extended to be signaled to enable or disable editing
 */
class CacheViewDelegate: public QStyledItemDelegate {
private:
    QSharedPointer<CacheMemory> memDevice;
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
