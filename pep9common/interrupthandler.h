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
