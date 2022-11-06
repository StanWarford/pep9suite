// File: memorytracepane.cpp
/*
    Pep9 is a virtual machine for writing machine language and assembly
    language programs.
    
    Copyright (C) 2019  J. Stanley Warford & Matthew McRaven, Pepperdine University

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
#include "memorytracepane.h"
#include "ui_memorytracepane.h"

#include <QDebug>
#include <QFontDialog>
#include <QMessageBox>
#include <QRgb>

#include "assembler/asmprogrammanager.h"
#include "assembler/asmprogram.h"
#include "cpu/acpumodel.h"
#include "memory/amemorydevice.h"
#include "memory/mainmemory.h"
#include "stack/stacktrace.h"
#include "style/colors.h"
#include "style/fonts.h"

NewMemoryTracePane::NewMemoryTracePane(QWidget *parent): QWidget (parent), ui(new Ui::MemoryTracePane),
    colors(&PepColors::lightMode), globalVars(), runtimeStack(), heap(), extraItems(),
    graphicItemsInStackFrame(), heapFrameItemStack(),
    globalLocation(QPointF(0, 0)), stackLocation(QPointF(225, 0)),
    heapLocation (QPointF(450, 0/* - MemoryCellGraphicsItem::boxHeight*/)),
    addressToItems()
{
    ui->setupUi(this);

    ui->label->setFont(QFont(PepCore::labelFont, PepCore::labelFontSize));
    ui->warningLabel->setWordWrap(true);
    ui->graphicsView->setFont(QFont(PepCore::codeFont, PepCore::codeFontSize));

    connect(ui->spinBox, SIGNAL(valueChanged(int)), this, SLOT(zoomFactorChanged(int)));
    // ui->spinBox->hide();

    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setBackgroundBrush(QBrush(colors->backgroundFill));
}

void NewMemoryTracePane::init(const AsmProgramManager *manager, QSharedPointer<const ACPUModel> CPU,
                              QSharedPointer<const MainMemory> memorySection, QSharedPointer<const MemoryTrace> trace)
{
    this->manager = manager;
    this->cpu = CPU;
    this->memorySection = memorySection;
    this->trace = trace;
}

NewMemoryTracePane::~NewMemoryTracePane()
{
    delete ui;
}

void NewMemoryTracePane::updateTrace()
{
    // If there were trace warnings, then the stack view won't be meaningful.
    // If the trace pane is not visible (the tab it is in is invisible), still render updates.
    // If the pane is hidden (disabled & no way for the user to ever see it),
    // then updates may be skipped.
    if(trace == nullptr || trace->hasTraceWarnings() || isHidden()) return;
    updateGlobals();
    // Only render stack / heap if they are still intact.
    if(trace->activeStack->isStackIntact()) updateStack();
    else {
        ui->warningLabel->setText(trace->activeStack->getErrorMessage());
    }
    if(trace->heapTrace.heapIntact()) updateHeap();
    else {
        ui->warningLabel->setText(trace->heapTrace.getErrorMessage());
    }
    // Using main memory device, update
    for(quint16 address : memorySection->getBytesWritten()) {
        if(addressToItems.contains(address)) {
            addressToItems[address]->setModified(true);
            addressToItems[address]->updateValue();
        }
    }


    // This is time-consuming, but worthwhile to ensure scrollbars aren't going off into
    // oblivion after items are removed.
    // From the documentation for the 'itemsBoundingRect()' function:
    // Calculates and returns the bounding rect of all items on the scene.
    // This function works by iterating over all items, and because if this,
    // it can be slow for large scenes. It is unlikely that this scene become very large
    // (as in the 30,000 chips example), so this should remain performant.
    scene->setSceneRect(scene->itemsBoundingRect());

    scene->invalidate();

    // NOTE: code has been disabled, since I'm not sure this behavior is desirable.
    // Update graphicsView viewport location after the scene is repainted, else
    // the location will not be calculated correctly.
    // ui->graphicsView->centerOn(ui->graphicsView->viewport()->x(), scene->sceneRect().top());

}

void NewMemoryTracePane::highlightOnFocus()
{
    if (ui->graphicsView->hasFocus() || ui->spinBox->hasFocus()) {
        ui->label->setAutoFillBackground(true);
    }
    else {
        ui->label->setAutoFillBackground(false);
    }
}

bool NewMemoryTracePane::hasFocus()
{
    return ui->graphicsView->hasFocus() || ui->spinBox->hasFocus();
}

void NewMemoryTracePane::setFocus()
{
    ui->graphicsView->setFocus();
}

void NewMemoryTracePane::onFontChanged(QFont font)
{
    ui->graphicsView->setFont(font);
}

void NewMemoryTracePane::onSimulationStarted()
{
    globalVars.clear();
    runtimeStack.clear();
    heap.clear();
    addressToItems.clear();
    graphicItemsInStackFrame.clear();
    heapFrameItemStack.clear();
    scene->clear();
    MemoryCellGraphicsItem* ptr = nullptr;
    qreal globaly = globalLocation.y();
    ui->warningLabel->clear();
    // Don't attempt to render items if there are trace tag warnings.
    if(manager->getUserProgram().isNull() ||
      !manager->getUserProgram()->getTraceInfo()->hadTraceTags ||
       manager->getUserProgram()->getTraceInfo()->staticTraceError) {
        setVisible(false);
        return;
    }
    else{
        setVisible(true);
    }
    if(trace->hasTraceWarnings()) {
        ui->warningLabel->setText("Can't render trace due to trace tag errors.");
        return;
    }
    // Add global symbols
    quint16 num = static_cast<quint16>(trace->globalTrace.getMemTags().size());
    globaly -= num * MemoryCellGraphicsItem::boxHeight;
    for(auto tag : trace->globalTrace.getMemTags()) {
        ptr = new MemoryCellGraphicsItem(memorySection.get(),
                                         tag.addr, tag.type.second,
                                         tag.type.first,
                                         static_cast<int>(globalLocation.x()),
                                         static_cast<int>(globaly));
        ptr->updateValue();
        scene->addItem(ptr);
        globalVars.push(ptr);
        addressToItems.insert(tag.addr, ptr);
        if(ptr->getNumBytes() == 2) {
            addressToItems.insert((tag.addr + 1) % memorySection->maxAddress(), ptr);
        }
        globaly += MemoryCellGraphicsItem::boxHeight;
    }

    updateStatics();
    updateTrace();
    scene->invalidate(); // redraw the scene!

}

void NewMemoryTracePane::onSimulationFinished()
{
    // Nothing in need of cleaning up
    updateTrace();
}

void NewMemoryTracePane::onDarkModeChanged(bool darkMode)
{
    if(darkMode) {
        colors = &PepColors::darkMode;
    }
    else {
        colors = &PepColors::lightMode;
    }
    for(auto item : this->addressToItems) {
        item->setColorTheme(*colors);
    }
    // Pen to draw dark border
    QPen pen(colors->textColor);
    pen.setWidth(4);
    for(auto item : this->graphicItemsInStackFrame) {
        item->setPen(pen);
    }
    for(auto item : this->heapFrameItemStack) {
        item->setPen(pen);
    }
    ui->graphicsView->setBackgroundBrush(QBrush(colors->backgroundFill));
    updateStatics();
}

void NewMemoryTracePane::onMemoryChanged()
{
    updateTrace();
}

void NewMemoryTracePane::updateGlobals()
{
    // Update the value of all global items
    for(MemoryCellGraphicsItem* item : globalVars) {
        item->setModified(false);
        item->updateValue();
    }
}

void NewMemoryTracePane::updateHeap()
{
    // Pen to draw dark border
    QPen pen(colors->textColor);
    pen.setWidth(4);
    // Y location where bold outline should be drawn.
    int frameBase = static_cast<int>(heapLocation.y());
    // Number of bytes allocated by the current call to malloc, or 0 otherwise.
    quint16 frameItemCount = 0;
    // Since we want to allocate any previously missed frames,
    // iterate from the oldest frame to the newest frame,
    // and shift up any old frame to make room for new ones.
    for(auto stackFrame = trace->heapTrace.begin();
        stackFrame != trace->heapTrace.end(); ++stackFrame) {
        // Reset starting y location for each iteration, or multiple frames may be allocated on top of each other
        int yLoc = static_cast<int>(heapLocation.y()) - MemoryCellGraphicsItem::boxHeight;
        // Number of cells in current frame.
        frameItemCount = stackFrame->numItems();
        // If a frame has 0 items, it is already allocated (or doesn't matter), otherwise, it might not be allocated.
        bool frameAdded = frameItemCount == 0;
        for(auto graphicsItem : heap){
            // If the first address of any arbitrary item in the current frame is found on the
            // heap, assume the entire frame has already been sucessfully placed on the heap.
            if(graphicsItem->getAddress() == stackFrame->begin()->addr) {
                frameAdded = true;
                break;
            }
        }
        // If the frame hasn't been added yet and the heap is still
        // able to be added to (i.e. it is intact), there's work to do.
        if(!frameAdded && trace->heapTrace.canAddNew()){
            // First, shift up all existing stack entries by the size of this stack frame.
            for(auto item : heap) {
                item->moveBy(0, 0 - frameItemCount * MemoryCellGraphicsItem::boxHeight);
            }
            // Shift up the frame outlines by the size of this stack frame
            for(auto frame: heapFrameItemStack) {
                frame->moveBy(0, 0 - frameItemCount * MemoryCellGraphicsItem::boxHeight);
            }
            // Add the cells from this frame to the heap
            for(auto memTag = stackFrame->rbegin();
                memTag != stackFrame->rend(); ++memTag) {
                MemoryCellGraphicsItem* item = new MemoryCellGraphicsItem(memorySection.get(), memTag->addr,
                                                  memTag->type.second, memTag->type.first,
                                                  static_cast<int>(heapLocation.x()),
                                                                          yLoc);
                item->setColorTheme(*colors);
                addressToItems.insert(item->getAddress(), item);
                if(item->getNumBytes() == 2) addressToItems.insert(item->getAddress() + 1, item);
                heap.append(item);
                scene->addItem(item);
                yLoc -= MemoryCellGraphicsItem::boxHeight;
            }
            // Add the bolded frame
            QGraphicsRectItem * rectItem = new QGraphicsRectItem(heapLocation.x() - 2, frameBase,
                              static_cast<qreal>(MemoryCellGraphicsItem::boxWidth + 4),
                              - static_cast<qreal>(MemoryCellGraphicsItem::boxHeight * stackFrame->numItems()), nullptr);
            scene->addItem(rectItem);
            rectItem->setPen(pen);
            rectItem->setZValue(1.0); // This moves the frame to the front
            heapFrameItemStack.push(rectItem);
        }

    }
    // Update the contents of every cell in the memory view
    for (auto item : heap) {
        item->setModified(false);
        item->updateValue();
        item->setBackgroundColor(colors->backgroundFill);
    }
    // If currently in malloc, and there are items to highlight, highlight (in green) the last added frame.
    if(trace->heapTrace.inMalloc()
            && trace->heapTrace.crbegin() != trace->heapTrace.crend()) {
        for(int it = 0; it < trace->heapTrace.crbegin()->numItems(); it++) {
            heap[heap.size() - 1 - it]->setBackgroundColor(Qt::green);
        }
    }
}

void NewMemoryTracePane::updateStack()
{
    // Pen to draw dark border
    QPen pen(colors->textColor);
    pen.setWidth(4);

    // Pointer to item being rendered.
    MemoryCellGraphicsItem * item = nullptr;
    // Items that are being added to the runtime stack this cycle.
    QList<MemoryCellGraphicsItem *> newItems = {};
    // Mantain cache of usable rectangle to avoid useless memory allocations
    QStack<QGraphicsRectItem *> itemCache = graphicItemsInStackFrame;
    graphicItemsInStackFrame.clear();
    // Iterator to traverse the items already on the runtime stack.
    QStack<MemoryCellGraphicsItem *>::iterator rtit = runtimeStack.begin();
    // Iterator to traverse the unused graphics items that are lying around.
    QList<MemoryCellGraphicsItem *>::iterator exit = extraItems.begin();
    int yLoc = static_cast<int>(stackLocation.y()) - MemoryCellGraphicsItem::boxHeight;
    int frameBase;
    for(auto stackFrame = trace->activeStack->cbegin();
        stackFrame != trace->activeStack->cend(); ++stackFrame) {
        // Store the bottom Y value of this stack frame,
        // so that a bold outline might be drawn around it.
        frameBase = yLoc + MemoryCellGraphicsItem::boxHeight;
        for(auto memTag = stackFrame->begin();
            memTag != stackFrame->end(); ++memTag) {

            // Exhaust items on the runtime stack.
            if(rtit != runtimeStack.cend()) {
                // If the current tag does not match the item at the current
                // position on the runtime stack, extra cleanup work todo
                if((*rtit)->getAddress() != memTag->addr) {
                    // Remove old entry's address from the lookup map,
                    // else a modifcation to that address might effect this one.
                    addressToItems.remove((*rtit)->getAddress());
                    if((*rtit)->getNumBytes() == 2) addressToItems.remove((*rtit)->getAddress() + 1);
                    (*rtit)->updateContents(memTag->addr, memTag->type.second, memTag->type.first, yLoc);
                    // Add updated cell to address lookup map.
                    addressToItems.insert((*rtit)->getAddress(), (*rtit));
                    if((*rtit)->getNumBytes() == 2) addressToItems.insert((*rtit)->getAddress() + 1, (*rtit));
                }
                // Set the item whose contents are to be updated.
                item = *rtit;
                ++rtit;
            }

            // Then exhaust extra cached items.
            else if(exit != extraItems.end()) {
                // Take an item from the extra list and update its contents.
                item = *exit;
                item->updateContents(memTag->addr, memTag->type.second, memTag->type.first, yLoc);
                item->setColorTheme(*colors);
                // Add updated cell to address lookup map.
                addressToItems.insert(item->getAddress(), item);
                if(item->getNumBytes() == 2) addressToItems.insert(item->getAddress() + 1, item);
                ++exit;
                // Remove item from extra list, and append it to the list of newly added items this update pass.
                extraItems.pop_front();
                newItems.append(item);
                scene->addItem(item);

            }

            // Then start creating new items
            else {
                // No cached items, so must allocate some via new.
                item = new MemoryCellGraphicsItem(memorySection.get(), memTag->addr,
                                                  memTag->type.second, memTag->type.first,
                                                  static_cast<int>(stackLocation.x()), yLoc);
                item->setColorTheme(*colors);
                // Add updated cell to address lookup map.
                addressToItems.insert(item->getAddress(), item);
                if(item->getNumBytes() == 2) addressToItems.insert(item->getAddress() + 1, item);
                newItems.append(item);
                scene->addItem(item);
            }

            // Adjust location of next stack item to account for the most recently processed item
            yLoc -= MemoryCellGraphicsItem::boxHeight;
            item->setModified(false);
            item->updateValue();
        }

        // If a frame is orphaned or incomplete, it should not be outlined.
        if(stackFrame->isOrphaned == false) {
            QGraphicsRectItem * item;
            // If there are no leftover frame outlines, start creating new ones
            if(itemCache.isEmpty()) {
            item = new QGraphicsRectItem(stackLocation.x() - 2, frameBase,
                              static_cast<qreal>(MemoryCellGraphicsItem::boxWidth + 4),
                              - static_cast<qreal>(MemoryCellGraphicsItem::boxHeight * stackFrame->numItems()), nullptr);
                scene->addItem(item);
            }
            // Otherwise reuse an old outline.
            else {
                item = itemCache.takeFirst();
                item->setRect(stackLocation.x() - 2, frameBase,
                              static_cast<qreal>(MemoryCellGraphicsItem::boxWidth + 4),
                              - static_cast<qreal>(MemoryCellGraphicsItem::boxHeight * stackFrame->numItems()));
            }
            item->setPen(pen);
            graphicItemsInStackFrame.push(item);
            item->setZValue(1.0); // This moves the stack frame to the front
        }
    }

    // Remove items from rendering on runtime stack if they have been popped.
    // Cache the items, to save calls to new.
    while(rtit != runtimeStack.cend()) {
        addressToItems.remove((*rtit)->getAddress());
        if((*rtit)->getNumBytes() == 2) addressToItems.remove((*rtit)->getAddress() + 1);
        scene->removeItem(*rtit);
        extraItems.append(*rtit);
        // Don't need to delete items, as they have been sucessfully appended to extraItems.
        rtit = runtimeStack.erase(rtit);
    }

    // Delete additional frame outlines. These should be cached, but the performance gain should be minimal
    while(!itemCache.isEmpty()) {
        QGraphicsRectItem * item = itemCache.takeFirst();
        scene->removeItem(item);
        delete item;
    }

    // Move newItems to runtime stack.
    // Don't move them until after updating stack,
    // since adding items would confuse our non-mutable iterators.
    for(auto item : newItems) {
        runtimeStack.push(item);
    }
}

void NewMemoryTracePane::updateStatics()
{
    // Add lines under stack
    scene->addLine(stackLocation.x() - MemoryCellGraphicsItem::boxWidth * 0.2, stackLocation.y(),
                   stackLocation.x() + MemoryCellGraphicsItem::boxWidth * 1.2, stackLocation.y(),
                   QPen(QBrush(colors->textColor, Qt::SolidPattern), 2, Qt::SolidLine));
    int dist = static_cast<int>(MemoryCellGraphicsItem::boxWidth * 1.2 - MemoryCellGraphicsItem::boxWidth * 1.4);
    for (int i = static_cast<int>(MemoryCellGraphicsItem::boxWidth * 1.2); i > dist; i = i - 10) {
        scene->addLine(stackLocation.x() + i - 10, stackLocation.y() + 10,
                       stackLocation.x() + i, stackLocation.y() + 1,
                       QPen(QBrush(colors->textColor, Qt::SolidPattern), 1, Qt::SolidLine));
    }

}

void NewMemoryTracePane::mouseReleaseEvent(QMouseEvent *)
{
    ui->graphicsView->setFocus();
}

void NewMemoryTracePane::mouseDoubleClickEvent(QMouseEvent *)
{
    emit labelDoubleClicked(PepCore::EPane::EMemoryTrace);
}

void NewMemoryTracePane::zoomFactorChanged(int factor)
{
    auto adjusted_factor = ((double)factor)/old_factor;
    ui->graphicsView->scale(adjusted_factor, adjusted_factor);
    old_factor = factor;
}
