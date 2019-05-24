#include "interrupthandler.h"

InterruptHandler::InterruptHandler(QObject *parent) : QObject(parent), handlerMap(), interruptList()
{

}

void InterruptHandler::registerHandler(Interrupts which, std::function<void ()> handler)
{
    handlerMap.insert(which, handler);
}

void InterruptHandler::handleQueuedInterrupts()
{
    while(!interruptList.isEmpty()) {
        auto inter = interruptList.takeFirst();
        emit handlingInterrupt(inter);
        // Guard against case where no handler has been registered.
        if(handlerMap.contains(inter)) {
            handlerMap[inter]();
        }

    }
}

void InterruptHandler::clearQueuedInterrupts()
{
    interruptList.clear();
}

void InterruptHandler::interupt(Interrupts which)
{
    interruptList.append(which);
}
