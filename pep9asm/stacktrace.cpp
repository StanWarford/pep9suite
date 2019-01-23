#include "stacktrace.h"
#include "typetags.h"
#include "symbolentry.h"
#include "enu.h"
#include <QTextStream>
StackTrace::StackTrace(): callStack(), nextFrame(QSharedPointer<StackFrame>::create())
{
    callStack.push(QSharedPointer<StackFrame>::create());
}

void StackTrace::call(quint16 sp)
{
    static const QSharedPointer<LiteralPrimitiveType> alpha = QSharedPointer<LiteralPrimitiveType>::create("retAddr", Enu::ESymbolFormat::F_2H);
    nextFrame->push({sp, alpha});
    callStack.push(nextFrame);
    nextFrame = QSharedPointer<StackFrame>::create();
}

void StackTrace::ret()
{
    nextFrame = callStack.pop();
    nextFrame->pop(2);
}

void StackTrace::pushLocals(quint16 sp, QList<QSharedPointer<AType> > items)
{
    for(auto symbol : items) {
        callStack.top()->push({sp,symbol});
        sp -= symbol->size();
    }
}

void StackTrace::pushParams(quint16 sp, QList<QSharedPointer<AType> > items)
{
    for(auto symbol : items) {
        nextFrame->push({sp,symbol});
        sp -= symbol->size();
    }
}

void StackTrace::popLocals(quint16 size)
{
    callStack.top()->pop(size);
}

void StackTrace::popParams(quint16 size)
{
    nextFrame->pop(size);
}

quint16 StackTrace::callDepth() const
{
    return callStack.length();
}

StackTrace::operator QString() const
{
    QList<QString> ts;
    QString out = "";
    if(nextFrame->size()>0) {
        out = QString(*nextFrame)+"||";
    }
    for(auto pair : callStack) {
        if (pair->size() == 0) continue;
        ts << QString("{%1}").arg(QString(*pair));
    }
    return out + ts.join(",");
}

MemoryTrace::MemoryTrace()
{

}

void MemoryTrace::clear()
{
    //throw std::exception("Does not work");
}

void StackFrame::push(QPair<quint16, QSharedPointer<AType> > symbol)
{
    stack.push(symbol);
}

bool StackFrame::pop(quint16 size)
{
    quint16 popped = 0;
    for (QMutableVectorIterator it(stack); it.hasNext() && popped<=size;) {
        auto next = it.next();
        popped += next.second->size();
        it.remove();
    }
    return popped == size;
}

quint16 StackFrame::size() const
{
    quint16 size=0;
    for(auto x: stack) {
        size += x.second->size();
    }
    return size;
}


StackFrame::operator QString() const
{
    QList<QString> items;
    for(auto pair : stack) {
        items << QString("(%1,%2)").arg(pair.first).arg((qint64) pair.second.get());
    }
    return items.join(", ");
}
