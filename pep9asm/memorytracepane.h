// File: memorytracepane.h
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

#ifndef MEMORYTRACEPANE_H
#define MEMORYTRACEPANE_H

#include <QWidget>
#include <QGraphicsScene>
#include <QStack>
#include <QSet>
#include "memorycellgraphicsitem.h"
#include "enu.h"

namespace Ui {
    class MemoryTracePane;
}
class MainMemory;
class MemoryTrace;
class AsmProgramManager;
class ACPUModel;
class NewMemoryTracePane : public QWidget {
    Q_OBJECT
    Q_DISABLE_COPY(NewMemoryTracePane)
public:
    explicit NewMemoryTracePane(QWidget *parent = nullptr);
    // Must be called after construction but before the component is used.
    void init(const AsmProgramManager *manager, QSharedPointer<const ACPUModel> CPU,
              QSharedPointer<const MainMemory> memorySection, QSharedPointer<const MemoryTrace> trace);
    virtual ~NewMemoryTracePane() override;
    void updateTrace();

    // Post: Highlights the label based on the label window color saved in the UI file.
    void highlightOnFocus();

    // Post: returns if the pane has focus
    bool hasFocus();

    // Post: the graphics item has focus.
    void setFocus();

public slots:
    void onFontChanged(QFont font);
    void onSimulationStarted();
    void onSimulationFinished();
    // Handle switching styles to and from dark mode & potential re-highlighting
    void onDarkModeChanged(bool darkMode);
    void onMemoryChanged();
private:
    void updateGlobals();
    void updateHeap();
    void updateStack();
    void updateStatics();

    Ui::MemoryTracePane *ui;
    const PepColors::Colors *colors;
    const AsmProgramManager *manager;
    QSharedPointer<const ACPUModel> cpu;
    QSharedPointer<const MainMemory> memorySection;
    QSharedPointer<const MemoryTrace> trace;
    QGraphicsScene *scene;
    // Stack of the global variables
    QStack<MemoryCellGraphicsItem *> globalVars;
    // Stack of the stack items
    QStack<MemoryCellGraphicsItem *> runtimeStack;
    // Stack of heap items
    QStack<MemoryCellGraphicsItem *> heap;
    // Cached items from the memory view that can be re-used to reduce # of calls to new.
    QList<MemoryCellGraphicsItem *> extraItems;

    // Stack of *items used to access the stack frames.
    QStack<QGraphicsRectItem *> graphicItemsInStackFrame;
    // Stack of *items for the heap graphic frames.
    QStack<QGraphicsRectItem *> heapFrameItemStack;

    // This is the location where global items start.
    const QPointF globalLocation;
    // This is the location where stack items start.
    const QPointF stackLocation;
    // This is the location where heap items start.
    const QPointF heapLocation;

    // Maps a memory address to its associated graphics item.
    QMap<quint16, MemoryCellGraphicsItem *> addressToItems;


    void mouseReleaseEvent(QMouseEvent *) override;
    void mouseDoubleClickEvent(QMouseEvent *) override;

private slots:
    void zoomFactorChanged(int factor);

signals:
    void labelDoubleClicked(Enu::EPane pane);
};

#endif // MEMORYTRACEPANE_H
