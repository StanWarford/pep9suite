// File: interrupthandler.h
/*
    The Pep/9 suite of applications (Pep9, Pep9CPU, Pep9Micro) are
    simulators for the Pep/9 virtual machine, and allow users to
    create, simulate, and debug across various levels of abstraction.

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
#ifndef INTERRUPTHANDLER_H
#define INTERRUPTHANDLER_H

#include <functional>
#include <QMap>
#include <QObject>
enum class Interrupts
{
    BREAKPOINT_ASM, BREAKPOINT_MICRO, MMIO
};

/*
 * The InterruptHandler class provides the ability to queue interupts for a CPU and register handlers for each callback.
 * This callback mechanism means that all classes deriving from ACPUModel can handle exceptions, statuses, and breakpoints
 * in a more generic way.
 */
class InterruptHandler : public QObject
{
    Q_OBJECT
    QMap<Interrupts, std::function<void(void)>> handlerMap;
    QList<Interrupts> interruptList;

public:
    explicit InterruptHandler(QObject *parent = nullptr);
    void registerHandler(Interrupts which, std::function<void(void)> handler);
    // For each enqueued interupt, call the associated callback handler and remove from list.
    // If no handler exists for the interupt, nothing will be done.
    void handleQueuedInterrupts();
    // If any interrupts have been queued, remove them without processing.
    void clearQueuedInterrupts();

signals:
    // Notify listeners that which's handler is about to be called.
    void handlingInterrupt(Interrupts which);

public slots:
    void interupt(Interrupts which);
};

#endif // INTERRUPTHANDLER_H
