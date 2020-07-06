// File: interrupthandler.cpp
/*
    The Pep/9 suite of applications (Pep9, Pep9CPU, Pep9Micro) are
    simulators for the Pep/9 virtual machine, and allow users to
    create, simulate, and debug across various levels of abstraction.

    Copyright (C) 2018\9  J. Stanley Warford & Matthew McRaven, Pepperdine University

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
