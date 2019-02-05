#include "stacktrace.h"
#include "typetags.h"
#include "symbolentry.h"
#include "enu.h"
#include <QTextStream>
#include "amemorydevice.h"

StackTrace::StackTrace(): callStack(), nextFrame(QSharedPointer<StackFrame>::create())
{
    callStack.push(QSharedPointer<StackFrame>::create());
}

void StackTrace::call(quint16 sp)
{
    static const QPair<Enu::ESymbolFormat,QString> retType{Enu::ESymbolFormat::F_2H, "retAddr"};
    nextFrame->push({sp, retType});
    callStack.push(nextFrame);
    nextFrame = QSharedPointer<StackFrame>::create();
}

void StackTrace::clear()
{
    callStack.clear();
    nextFrame = QSharedPointer<StackFrame>::create();
}

bool StackTrace::ret()
{
    if(callStack.isEmpty()) return false;
    nextFrame = callStack.pop();
    return nextFrame->pop(2);
}

void StackTrace::pushLocals(quint16 start, QList<QPair<Enu::ESymbolFormat, QString> > items)
{
    if(callStack.isEmpty()) callStack.push(QSharedPointer<StackFrame>::create());
    for(auto pair : items) {
        start -= Enu::tagNumBytes(pair.first);
        callStack.top()->push({start, pair});
    }
}

void StackTrace::pushParams(quint16 start, QList<QPair<Enu::ESymbolFormat, QString> > items)
{
    for(auto pair :items) {
        start -= Enu::tagNumBytes(pair.first);
        nextFrame->push({start, pair});
    }
}

bool StackTrace::popLocals(quint16 size)
{
    if(callStack.isEmpty()) return false;
    return callStack.top()->pop(size);
}

bool StackTrace::popParams(quint16 size)
{
    // Handle recursive case when next frame has no contents
    if(nextFrame->size() == 0) {
        // If stack is entirely empty, return false
        if(callStack.isEmpty()) return false;
        // Otherwise take the next call stack and start popping from it
        nextFrame = callStack.pop();
        return popParams(size);
    }
    else if(size > nextFrame->size()) {
        quint16 popped = nextFrame->pop(nextFrame->size());
        return popParams(size-popped);
    }
    else {
        return nextFrame->pop(size);
    }
}

bool StackTrace::popAndOrphan(quint16 size)
{
    if(size >= nextFrame->size()) {
        return false;
    }
    else {
        nextFrame->isOrphaned = true;
        callStack.push(nextFrame);
        nextFrame = QSharedPointer<StackFrame>::create();
        // Orphaned frame can now be removed from call stack
        return popLocals(size);
    }
}

quint16 StackTrace::callDepth() const
{
    return callStack.length();
}

const StackFrame &StackTrace::getTOS()
{
    if(nextFrame->size() == 0 && callStack.size() > 0) {
        return *callStack.top().get();
    }
    else {
        return *nextFrame.get();
    }
}

StackTrace::operator QString() const
{
    QList<QString> ts;
    QString tmp = "";
    if(nextFrame->size()>0) {
        tmp = QString(*nextFrame);
    } else {
        tmp="{}";
    }
    for(auto pair : callStack) {
        if (pair->size() == 0) continue;
        ts << QString("{%1}").arg(QString(*pair));
    }
    QStringList out;
    std::reverse(ts.begin(),ts.end());
    out << tmp << ts.join(",");
    return out.join("||");
}

MemoryTrace::MemoryTrace(): userStack(StackTrace()),
    heapTrace(HeapTrace()), globalTrace(GlobalTrace())
{

}

void MemoryTrace::clear()
{
    userStack.clear();
    globalTrace.clear();
    heapTrace.clear();
}

void StackFrame::push(MemTag tag)
{
    stack.push(tag);
}

bool StackFrame::pop(quint16 size)
{
    quint16 popped = 0;
    while(popped<size && !stack.isEmpty()) {
        auto next = stack.pop();
        popped += Enu::tagNumBytes(next.type.first);
    }
    return popped == size;
}

quint16 StackFrame::size() const
{
    quint16 size=0;
    for(auto x: stack) {
        size += Enu::tagNumBytes(x.type.first);
    }
    return size;
}

StackFrame::operator QString() const
{
    QList<QString> items;
    for(auto tag = stack.rbegin(); tag!=stack.rend(); tag++) {
        items << *tag;
    }
    return items.join(", ");
}

GlobalTrace::GlobalTrace()
{

}

void GlobalTrace::setTags(QList<QPair<quint16, QPair<Enu::ESymbolFormat, QString> > > items)
{
    tags.clear();
    for(auto entry : items) {
        quint16 addr = entry.first;
        tags.insert(entry.first, {addr, entry.second});
    }
}

void GlobalTrace::clear()
{
    tags.clear();
}

const QMap<quint16, MemTag> GlobalTrace::getMemTags() const
{
    return tags;
}

MemTag::operator QString() const
{

    return QString("(%1:[%2])")
            .arg(type.second)
            .arg(addr,4,16,QChar('0'));
}

HeapTrace::HeapTrace()
{

}

void HeapTrace::pushHeap(quint16 start, QList<QPair<Enu::ESymbolFormat, QString> > items)
{
    QSharedPointer<StackFrame> frm = QSharedPointer<StackFrame>::create();
    quint16 addr = start;
    for(auto pair : items) {
        frm->push({addr, pair});
        addr += Enu::tagNumBytes(pair.first);
    }
    heap.insert(start, frm);

}

void HeapTrace::clear()
{
    heap.clear();
}

HeapTrace::operator QString() const
{
    QList<QString> items;
    for(auto frame = heap.keyBegin(); frame!=heap.keyEnd(); frame++) {
        items << QString("%1").arg(heap[(*frame)]->operator QString());
    }
    return items.join(", ");
}
