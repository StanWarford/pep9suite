#include "stacktrace.h"
#include "typetags.h"
#include "symbolentry.h"
#include "enu.h"
#include <QTextStream>
#include "amemorydevice.h"

StackTrace::iterator StackTrace::begin()
{
    return iterator(*this);
}

StackTrace::iterator StackTrace::end()
{
    int size = callStack.size();
    if(!nextFrame.isNull()) ++size;
    return iterator(*this, size);
}

StackTrace::const_iterator StackTrace::begin() const
{
    return cbegin();
}

StackTrace::const_iterator StackTrace::end() const
{
    return cend();
}

StackTrace::const_iterator StackTrace::cbegin() const
{
    return const_iterator(*this);
}

StackTrace::const_iterator StackTrace::cend() const
{
    int size = callStack.size();
    if(!nextFrame.isNull()) ++size;
    return const_iterator(*this, size);
}

StackTrace::reverse_iterator StackTrace::rbegin()
{
    return reverse_iterator(*this);
}

StackTrace::reverse_iterator StackTrace::rend()
{
    return reverse_iterator(*this, -1);
}

StackTrace::const_reverse_iterator StackTrace::rbegin() const
{
    return crbegin();
}

StackTrace::const_reverse_iterator StackTrace::rend() const
{
    return crend();
}

StackTrace::const_reverse_iterator StackTrace::crbegin() const
{
    return const_reverse_iterator(*this);
}

StackTrace::const_reverse_iterator StackTrace::crend() const
{
    return const_reverse_iterator(*this, -1);
}

StackTrace::StackTrace(): callStack(), nextFrame(QSharedPointer<StackFrame>::create()), stackIntact(true)
{
    callStack.push(QSharedPointer<StackFrame>::create());
    nextFrame->isOrphaned = true;
}

void StackTrace::call(quint16 sp)
{
    static const QPair<Enu::ESymbolFormat,QString> retType{Enu::ESymbolFormat::F_2H, "retAddr"};
    // WHen a frame is being moved from the "next up" to the actual call stack, it is no longer orphaned
    nextFrame->push({sp, retType});
    nextFrame->isOrphaned = false;
    callStack.push(nextFrame);
    nextFrame = QSharedPointer<StackFrame>::create();
    nextFrame->isOrphaned = true;
}

void StackTrace::clear()
{
    callStack.clear();
    nextFrame = QSharedPointer<StackFrame>::create();
    nextFrame->isOrphaned = true;
    stackIntact = true;
    errMessage = "";
}

bool StackTrace::ret()
{
    if(callStack.isEmpty()) return false;
    nextFrame = callStack.pop();
    nextFrame->isOrphaned = true;
    return nextFrame->pop(2);
}

void StackTrace::pushLocals(quint16 start, QList<QPair<Enu::ESymbolFormat, QString> > items)
{
    if(callStack.isEmpty()) {
        auto stack = QSharedPointer<StackFrame>::create();
        stack->isOrphaned = false;
        callStack.push(stack);
    }
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
        nextFrame->isOrphaned = true;
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
        nextFrame->isOrphaned = true;
        // Orphaned frame can now be removed from call stack
        return popLocals(size);
    }
}

quint16 StackTrace::callDepth() const
{
    return static_cast<quint16>(callStack.length());
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

bool StackTrace::isStackIntact() const
{
    return stackIntact;
}

void StackTrace::setStackIntact(bool value)
{
    stackIntact = value;
}

QString StackTrace::getErrorMessage() const
{
    return errMessage;
}

void StackTrace::setErrorMessage(QString message)
{
    errMessage = message;
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

MemoryTrace::MemoryTrace(): traceWarnings(false), userStack(StackTrace()),
    heapTrace(HeapTrace()), globalTrace(GlobalTrace())
{

}

void MemoryTrace::clear()
{
    userStack.clear();
    globalTrace.clear();
    heapTrace.clear();
    traceWarnings = false;
}

bool MemoryTrace::hasTraceWarnings() const
{
    return traceWarnings;
}

void MemoryTrace::setHasTraceWarnings(bool value)
{
    traceWarnings = value;
}

StackFrame::iterator StackFrame::begin()
{
    return iterator(*this);
}

StackFrame::iterator StackFrame::end()
{
    int size = this->stack.size();
    return iterator(*this, size);
}

StackFrame::const_iterator StackFrame::begin() const
{
    return cbegin();
}

StackFrame::const_iterator StackFrame::end() const
{
   return cend();
}

StackFrame::const_iterator StackFrame::cbegin() const
{
    return const_iterator(*this);
}

StackFrame::const_iterator StackFrame::cend() const
{
    int size = this->stack.size();
    return const_iterator(*this, size);
}

StackFrame::reverse_iterator StackFrame::rbegin()
{
    return reverse_iterator(*this);
}

StackFrame::reverse_iterator StackFrame::rend()
{
    return reverse_iterator(*this, -1);
}

StackFrame::const_reverse_iterator StackFrame::rbegin() const
{
    return crbegin();
}

StackFrame::const_reverse_iterator StackFrame::rend() const
{
    return crend();
}

StackFrame::const_reverse_iterator StackFrame::crbegin() const
{
    return const_reverse_iterator(*this);
}

StackFrame::const_reverse_iterator StackFrame::crend() const
{
    return const_reverse_iterator(*this, -1);
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
    if(this->size() == 0) isOrphaned = true;
    return popped == size;
}

quint16 StackFrame::size() const
{
    quint16 size = 0;
    for(auto x : stack) {
        size += Enu::tagNumBytes(x.type.first);
    }
    return size;
}

quint16 StackFrame::numItems() const
{
    return static_cast<quint16>(stack.size());
}

StackFrame::operator QString() const
{
    QList<QString> items;
    for(auto tag = stack.rbegin(); tag!=stack.rend(); tag++) {
        items << *tag;
    }
    return items.join(", ");
}

GlobalTrace::GlobalTrace(): tags()
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

HeapTrace::iterator HeapTrace::begin()
{
    return iterator(*this);
}

HeapTrace::iterator HeapTrace::end()
{
    int size = heap.size();
    return iterator(*this, size);
}

HeapTrace::const_iterator HeapTrace::begin() const
{
    return cbegin();
}

HeapTrace::const_iterator HeapTrace::end() const
{
    return cend();
}

HeapTrace::const_iterator HeapTrace::cbegin() const
{
    return const_iterator(*this);
}

HeapTrace::const_iterator HeapTrace::cend() const
{
    int size = heap.size();
    return const_iterator(*this, size);
}

HeapTrace::reverse_iterator HeapTrace::rbegin()
{
    return reverse_iterator(*this);
}

HeapTrace::reverse_iterator HeapTrace::rend()
{
    return reverse_iterator(*this, -1);
}

HeapTrace::const_reverse_iterator HeapTrace::rbegin() const
{
    return crbegin();
}

HeapTrace::const_reverse_iterator HeapTrace::rend() const
{
    return crend();
}

HeapTrace::const_reverse_iterator HeapTrace::crbegin() const
{
    return const_reverse_iterator(*this);
}

HeapTrace::const_reverse_iterator HeapTrace::crend() const
{
    return const_reverse_iterator(*this, -1);
}

HeapTrace::HeapTrace():  itToAddresses(), heap(), intact(true), addNew(true), isInMalloc(false)
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
    itToAddresses.append(start);
    heap.insert(start, frm);

}

void HeapTrace::clear()
{
    heap.clear();
    itToAddresses.clear();
    intact = true;
    addNew = true;
    isInMalloc = false;
    errMessage = "";
}

void HeapTrace::setCanAddNew(bool val)
{
    addNew = val;
}

void HeapTrace::setHeapIntact(bool val)
{
    intact = val;
}

void HeapTrace::setInMalloc(bool val)
{
    isInMalloc = val;
}

bool HeapTrace::canAddNew() const
{
    return addNew;
}

bool HeapTrace::heapIntact() const
{
    return intact;
}

bool HeapTrace::inMalloc() const
{
    return isInMalloc;
}

QString HeapTrace::getErrorMessage() const
{
    return errMessage;
}

void HeapTrace::setErrorMessage(QString message)
{
    errMessage = message;
}

HeapTrace::operator QString() const
{
    QList<QString> items;
    for(auto frame = heap.keyBegin(); frame!=heap.keyEnd(); frame++) {
        items << QString("%1").arg(heap[(*frame)]->operator QString());
    }
    return items.join(", ");
}

/*
 * Stack trace iterators
 */
StackTrace::iterator::iterator(StackTrace & trace): trace(&trace), idx(0)
{

}

StackTrace::iterator::iterator(StackTrace & trace, int offset): trace(&trace), idx(offset)
{

}

StackTrace::iterator::iterator(const StackTrace::iterator & rhs): trace(rhs.trace),
    idx(rhs.idx)
{

}

StackTrace::iterator::~iterator()
{
    // Nothing to clean up with this iterator
}

StackTrace::iterator &StackTrace::iterator::operator=(const StackTrace::iterator & rhs)
{
    if(*this != rhs){
        this->trace = rhs.trace;
        this->idx = rhs.idx;
    }
    return *this;
}

bool StackTrace::iterator::operator==(const StackTrace::iterator & rhs) const
{
    return this->trace == rhs.trace
            && this->idx == rhs.idx;
}

bool StackTrace::iterator::operator!=(const StackTrace::iterator &rhs) const
{
    return !(*this == rhs);
}

bool StackTrace::iterator::operator<(const StackTrace::iterator & rhs) const
{
    return this->trace == rhs.trace
            && this->idx < rhs.idx;
}

bool StackTrace::iterator::operator>(const StackTrace::iterator & rhs) const
{
    return this->trace == rhs.trace
            && this->idx > rhs.idx;
}

bool StackTrace::iterator::operator<=(const StackTrace::iterator & rhs) const
{
    return this->trace == rhs.trace
            && this->idx <= rhs.idx;
}

bool StackTrace::iterator::operator>=(const StackTrace::iterator & rhs) const
{
    return this->trace == rhs.trace
            && this->idx >= rhs.idx;
}

StackTrace::iterator &StackTrace::iterator::operator++()
{
    idx++;
    return *this;
}

StackTrace::iterator &StackTrace::iterator::operator--()
{
    idx--;
    return *this;
}

StackTrace::iterator::reference StackTrace::iterator::operator*()
{
    if(idx < trace->callStack.size()) {
        return *trace->callStack[idx].get();
    }
    else {
        return *trace->nextFrame.get();
    }
}

StackTrace::iterator::pointer StackTrace::iterator::operator->()
{
    if(idx < trace->callStack.size()) {
        return trace->callStack[idx].get();
    }
    else {
        return trace->nextFrame.get();
    }
}

StackTrace::reverse_iterator::reverse_iterator(StackTrace & trace): trace(&trace)
{
    int size = trace.callStack.size() - 1;
    if(!trace.nextFrame.isNull()) size++;
    idx = size;
}

StackTrace::reverse_iterator::reverse_iterator(StackTrace & trace, int offset):
    trace(&trace), idx(offset)
{

}

StackTrace::reverse_iterator::reverse_iterator(const StackTrace::reverse_iterator & rhs):
    trace(rhs.trace), idx(rhs.idx)
{

}

StackTrace::reverse_iterator::~reverse_iterator()
{
    // Nothing to clean up with this reverse_iterator
}

StackTrace::reverse_iterator &StackTrace::reverse_iterator::operator=(const StackTrace::reverse_iterator & rhs)
{
    if(*this != rhs){
        this->trace = rhs.trace;
        this->idx = rhs.idx;
    }
    return *this;
}

bool StackTrace::reverse_iterator::operator==(const StackTrace::reverse_iterator & rhs) const
{
    return this->trace == rhs.trace
            && this->idx == rhs.idx;
}

bool StackTrace::reverse_iterator::operator!=(const StackTrace::reverse_iterator &rhs) const
{
    return !(*this == rhs);
}

bool StackTrace::reverse_iterator::operator<(const StackTrace::reverse_iterator & rhs) const
{
    return this->trace == rhs.trace
            && this->idx > rhs.idx;
}

bool StackTrace::reverse_iterator::operator>(const StackTrace::reverse_iterator & rhs) const
{
    return this->trace == rhs.trace
            && this->idx < rhs.idx;
}

bool StackTrace::reverse_iterator::operator<=(const StackTrace::reverse_iterator & rhs) const
{
    return this->trace == rhs.trace
            && this->idx >= rhs.idx;
}

bool StackTrace::reverse_iterator::operator>=(const StackTrace::reverse_iterator & rhs) const
{
    return this->trace == rhs.trace
            && this->idx <= rhs.idx;
}

StackTrace::reverse_iterator &StackTrace::reverse_iterator::operator++()
{
    idx--;
    return *this;
}

StackTrace::reverse_iterator &StackTrace::reverse_iterator::operator--()
{
    idx++;
    return *this;
}

StackTrace::reverse_iterator::reference StackTrace::reverse_iterator::operator*()
{
    if(idx < trace->callStack.size()) {
        return *trace->callStack[idx].get();
    }
    else {
        return *trace->nextFrame.get();
    }
}

StackTrace::reverse_iterator::pointer StackTrace::reverse_iterator::operator->()
{
    if(idx < trace->callStack.size()) {
        return trace->callStack[idx].get();
    }
    else {
        return trace->nextFrame.get();
    }
}

StackTrace::const_iterator::const_iterator(const StackTrace & trace): trace(&trace), idx(0)
{

}

StackTrace::const_iterator::const_iterator(const StackTrace & trace, int offset):
    trace(&trace), idx(offset)
{

}

StackTrace::const_iterator::const_iterator(const StackTrace::const_iterator & rhs):
    trace(rhs.trace), idx(rhs.idx)
{

}

StackTrace::const_iterator::~const_iterator()
{
    // Nothing to clean up with this iterator
}

StackTrace::const_iterator &StackTrace::const_iterator::operator=(const StackTrace::const_iterator & rhs)
{
    if(*this != rhs){
        this->trace = rhs.trace;
        this->idx = rhs.idx;
    }
    return *this;
}

bool StackTrace::const_iterator::operator==(const StackTrace::const_iterator & rhs) const
{
    return this->trace == rhs.trace
            && this->idx == rhs.idx;
}

bool StackTrace::const_iterator::operator!=(const StackTrace::const_iterator &rhs) const
{
    return !(*this == rhs);
}

bool StackTrace::const_iterator::operator<(const StackTrace::const_iterator & rhs) const
{
    return this->trace == rhs.trace
            && this->idx < rhs.idx;
}

bool StackTrace::const_iterator::operator>(const StackTrace::const_iterator & rhs) const
{
    return this->trace == rhs.trace
            && this->idx > rhs.idx;
}

bool StackTrace::const_iterator::operator<=(const StackTrace::const_iterator & rhs) const
{
    return this->trace == rhs.trace
            && this->idx <= rhs.idx;
}

bool StackTrace::const_iterator::operator>=(const StackTrace::const_iterator & rhs) const
{
    return this->trace == rhs.trace
            && this->idx >= rhs.idx;
}

StackTrace::const_iterator &StackTrace::const_iterator::operator++()
{
    idx++;
    return *this;
}

StackTrace::const_iterator &StackTrace::const_iterator::operator--()
{
    idx--;
    return *this;
}

StackTrace::const_iterator::reference StackTrace::const_iterator::operator*() const
{
    if(idx < trace->callStack.size()) {
        return *trace->callStack[idx].get();
    }
    else {
        return *trace->nextFrame.get();
    }
}

StackTrace::const_iterator::pointer StackTrace::const_iterator::operator->() const
{
    if(idx < trace->callStack.size()) {
        return trace->callStack[idx].get();
    }
    else {
        return trace->nextFrame.get();
    }
}

StackTrace::const_reverse_iterator::const_reverse_iterator(const StackTrace & trace):
    trace(&trace)
{
    int size = trace.callStack.size() - 1;
    if(!trace.nextFrame.isNull()) size++;
    idx = size;
}

StackTrace::const_reverse_iterator::const_reverse_iterator(const StackTrace & trace, int offset):
    trace(&trace), idx(offset)
{

}

StackTrace::const_reverse_iterator::const_reverse_iterator(const StackTrace::const_reverse_iterator & rhs):
    trace(rhs.trace), idx(rhs.idx)
{

}

StackTrace::const_reverse_iterator::~const_reverse_iterator()
{
    // Nothing to clean up with this iterator
}

StackTrace::const_reverse_iterator &StackTrace::const_reverse_iterator::operator=(const StackTrace::const_reverse_iterator & rhs)
{
    if(*this != rhs){
        this->trace = rhs.trace;
        this->idx = rhs.idx;
    }
    return *this;
}

bool StackTrace::const_reverse_iterator::operator==(const StackTrace::const_reverse_iterator & rhs) const
{
    return this->trace == rhs.trace
            && this->idx == rhs.idx;
}

bool StackTrace::const_reverse_iterator::operator!=(const StackTrace::const_reverse_iterator &rhs) const
{
    return !(*this == rhs);
}

bool StackTrace::const_reverse_iterator::operator<(const StackTrace::const_reverse_iterator & rhs) const
{
    return this->trace == rhs.trace
            && this->idx > rhs.idx;
}

bool StackTrace::const_reverse_iterator::operator>(const StackTrace::const_reverse_iterator & rhs) const
{
    return this->trace == rhs.trace
            && this->idx < rhs.idx;
}

bool StackTrace::const_reverse_iterator::operator<=(const StackTrace::const_reverse_iterator & rhs) const
{
    return this->trace == rhs.trace
            && this->idx >= rhs.idx;
}

bool StackTrace::const_reverse_iterator::operator>=(const StackTrace::const_reverse_iterator & rhs) const
{
    return this->trace == rhs.trace
            && this->idx <= rhs.idx;
}

StackTrace::const_reverse_iterator &StackTrace::const_reverse_iterator::operator++()
{
    idx--;
    return *this;
}

StackTrace::const_reverse_iterator &StackTrace::const_reverse_iterator::operator--()
{
    idx++;
    return *this;
}

StackTrace::const_reverse_iterator::reference StackTrace::const_reverse_iterator::operator*() const
{
    if(idx < trace->callStack.size()) {
        return *trace->callStack[idx].get();
    }
    else {
        return *trace->nextFrame.get();
    }
}

StackTrace::const_reverse_iterator::pointer StackTrace::const_reverse_iterator::operator->() const
{
    if(idx < trace->callStack.size()) {
        return trace->callStack[idx].get();
    }
    else {
        return trace->nextFrame.get();
    }
}


/*
 * Iterators for a stack frame
 */
StackFrame::iterator::iterator(StackFrame & frame): frame(&frame),
    idx(0)
{

}

StackFrame::iterator::iterator(StackFrame & frame, int offset):
    frame(&frame), idx(offset)
{

}

StackFrame::iterator::iterator(const StackFrame::iterator & other):
    frame(other.frame), idx(other.idx)
{

}

StackFrame::iterator::~iterator()
{
    // No cleanup to do, this doesn't own any memory
}

StackFrame::iterator &StackFrame::iterator::operator=(const StackFrame::iterator & rhs)
{
    if(*this != rhs) {
        this->frame = rhs.frame;
        this->idx = rhs.idx;
    }
    return *this;
}

bool StackFrame::iterator::operator==(const StackFrame::iterator & rhs) const
{
    return this->frame == rhs.frame
            && this->idx == rhs.idx;
}

bool StackFrame::iterator::operator!=(const StackFrame::iterator & rhs) const
{
    return !(*this == rhs);
}

bool StackFrame::iterator::operator<(const StackFrame::iterator & rhs) const
{
    return this->frame == rhs.frame
            && this->idx < rhs.idx;
}

bool StackFrame::iterator::operator>(const StackFrame::iterator & rhs) const
{
    return this->frame == rhs.frame
            && this->idx > rhs.idx;
}

bool StackFrame::iterator::operator<=(const StackFrame::iterator & rhs) const
{
    return this->frame == rhs.frame
            && this->idx <= rhs.idx;
}

bool StackFrame::iterator::operator>=(const StackFrame::iterator & rhs) const
{
    return this->frame == rhs.frame
            && this->idx >= rhs.idx;
}

StackFrame::iterator &StackFrame::iterator::operator++()
{
    idx++;
    return *this;
}

StackFrame::iterator &StackFrame::iterator::operator--()
{
    idx--;
    return *this;
}

StackFrame::iterator::reference StackFrame::iterator::operator*()
{
    return frame->stack[idx];
}

StackFrame::iterator::pointer StackFrame::iterator::operator->()
{
    return &frame->stack[idx];
}

StackFrame::reverse_iterator::reverse_iterator(StackFrame & frame): frame(&frame)
{
    int size = frame.stack.size() - 1;
    idx = size;
}

StackFrame::reverse_iterator::reverse_iterator(StackFrame & frame, int offset):
    frame(&frame), idx(offset)
{

}

StackFrame::reverse_iterator::reverse_iterator(const StackFrame::reverse_iterator & other):
    frame(other.frame), idx(other.idx)
{

}

StackFrame::reverse_iterator::~reverse_iterator()
{
    // No cleanup to do, this doesn't own any memory
}

StackFrame::reverse_iterator &StackFrame::reverse_iterator::operator=(const StackFrame::reverse_iterator & rhs)
{
    if(*this != rhs) {
        this->frame = rhs.frame;
        this->idx = rhs.idx;
    }
    return *this;
}

bool StackFrame::reverse_iterator::operator==(const StackFrame::reverse_iterator & rhs) const
{
    return this->frame == rhs.frame
            && this->idx == rhs.idx;
}

bool StackFrame::reverse_iterator::operator!=(const StackFrame::reverse_iterator & rhs) const
{
    return !(*this == rhs);
}

bool StackFrame::reverse_iterator::operator<(const StackFrame::reverse_iterator & rhs) const
{
    return this->frame == rhs.frame
            && this->idx > rhs.idx;
}

bool StackFrame::reverse_iterator::operator>(const StackFrame::reverse_iterator & rhs) const
{
    return this->frame == rhs.frame
            && this->idx < rhs.idx;
}

bool StackFrame::reverse_iterator::operator<=(const StackFrame::reverse_iterator & rhs) const
{
    return this->frame == rhs.frame
            && this->idx >= rhs.idx;
}

bool StackFrame::reverse_iterator::operator>=(const StackFrame::reverse_iterator & rhs) const
{
    return this->frame == rhs.frame
            && this->idx <= rhs.idx;
}

StackFrame::reverse_iterator &StackFrame::reverse_iterator::operator++()
{
    idx--;
    return *this;
}

StackFrame::reverse_iterator &StackFrame::reverse_iterator::operator--()
{
    idx++;
    return *this;
}

StackFrame::reverse_iterator::reference StackFrame::reverse_iterator::operator*()
{
    return frame->stack[idx];
}

StackFrame::reverse_iterator::pointer StackFrame::reverse_iterator::operator->()
{
    return &frame->stack[idx];
}

StackFrame::const_iterator::const_iterator(const StackFrame & frame): frame(&frame),
    idx(0)
{

}

StackFrame::const_iterator::const_iterator(const StackFrame & frame, int offset):
    frame(&frame), idx(offset)
{

}

StackFrame::const_iterator::const_iterator(const StackFrame::const_iterator & other):
    frame(other.frame), idx(other.idx)
{

}

StackFrame::const_iterator::~const_iterator()
{
    // No cleanup to do, this doesn't own any memory
}

StackFrame::const_iterator &StackFrame::const_iterator::operator=(const StackFrame::const_iterator & rhs)
{
    if(*this != rhs) {
        this->frame = rhs.frame;
        this->idx = rhs.idx;
    }
    return *this;
}

bool StackFrame::const_iterator::operator==(const StackFrame::const_iterator & rhs) const
{
    return this->frame == rhs.frame
            && this->idx == rhs.idx;
}

bool StackFrame::const_iterator::operator!=(const StackFrame::const_iterator & rhs) const
{
    return !(*this == rhs);
}

bool StackFrame::const_iterator::operator<(const StackFrame::const_iterator & rhs) const
{
    return this->frame == rhs.frame
            && this->idx < rhs.idx;
}

bool StackFrame::const_iterator::operator>(const StackFrame::const_iterator & rhs) const
{
    return this->frame == rhs.frame
            && this->idx > rhs.idx;
}

bool StackFrame::const_iterator::operator<=(const StackFrame::const_iterator & rhs) const
{
    return this->frame == rhs.frame
            && this->idx <= rhs.idx;
}

bool StackFrame::const_iterator::operator>=(const StackFrame::const_iterator & rhs) const
{
    return this->frame == rhs.frame
            && this->idx >= rhs.idx;
}

StackFrame::const_iterator &StackFrame::const_iterator::operator++()
{
    idx++;
    return *this;
}

StackFrame::const_iterator &StackFrame::const_iterator::operator--()
{
    idx--;
    return *this;
}

StackFrame::const_iterator::reference StackFrame::const_iterator::operator*() const
{
    return frame->stack.at(idx);
}

StackFrame::const_iterator::pointer StackFrame::const_iterator::operator->() const
{
    return &frame->stack.at(idx);
}

StackFrame::const_reverse_iterator::const_reverse_iterator(const StackFrame & frame):
    frame(&frame)
{
    int size = frame.stack.size() - 1;
    idx = size;
}

StackFrame::const_reverse_iterator::const_reverse_iterator(const StackFrame & frame, int offset):
    frame(&frame), idx(offset)
{

}

StackFrame::const_reverse_iterator::const_reverse_iterator(const StackFrame::const_reverse_iterator & other):
    frame(other.frame), idx(other.idx)
{

}

StackFrame::const_reverse_iterator::~const_reverse_iterator()
{
    // No cleanup to do, this doesn't own any memory
}

StackFrame::const_reverse_iterator &StackFrame::const_reverse_iterator::operator=(const StackFrame::const_reverse_iterator & rhs)
{
    if(*this != rhs) {
        this->frame = rhs.frame;
        this->idx = rhs.idx;
    }
    return *this;
}

bool StackFrame::const_reverse_iterator::operator==(const StackFrame::const_reverse_iterator & rhs) const
{
    return this->frame == rhs.frame
            && this->idx == rhs.idx;
}

bool StackFrame::const_reverse_iterator::operator!=(const StackFrame::const_reverse_iterator & rhs) const
{
    return !(*this == rhs);
}

bool StackFrame::const_reverse_iterator::operator<(const StackFrame::const_reverse_iterator & rhs) const
{
    return this->frame == rhs.frame
            && this->idx > rhs.idx;
}

bool StackFrame::const_reverse_iterator::operator>(const StackFrame::const_reverse_iterator & rhs) const
{
    return this->frame == rhs.frame
            && this->idx < rhs.idx;
}

bool StackFrame::const_reverse_iterator::operator<=(const StackFrame::const_reverse_iterator & rhs) const
{
    return this->frame == rhs.frame
            && this->idx >= rhs.idx;
}

bool StackFrame::const_reverse_iterator::operator>=(const StackFrame::const_reverse_iterator & rhs) const
{
    return this->frame == rhs.frame
            && this->idx <= rhs.idx;
}

StackFrame::const_reverse_iterator &StackFrame::const_reverse_iterator::operator++()
{
    idx--;
    return *this;
}

StackFrame::const_reverse_iterator &StackFrame::const_reverse_iterator::operator--()
{
    idx++;
    return *this;
}

StackFrame::const_reverse_iterator::reference StackFrame::const_reverse_iterator::operator*() const
{
    return frame->stack.at(idx);
}

StackFrame::const_reverse_iterator::pointer StackFrame::const_reverse_iterator::operator->() const
{
    return &frame->stack.at(idx);
}

HeapTrace::iterator::iterator(HeapTrace & trace): trace(&trace), idx(0)
{

}

HeapTrace::iterator::iterator(HeapTrace & trace, int offset): trace(&trace), idx(offset)
{

}

HeapTrace::iterator::iterator(const HeapTrace::iterator & rhs): trace(rhs.trace),
    idx(rhs.idx)
{

}

HeapTrace::iterator::~iterator()
{
    // Nothing to clean up with this iterator
}

HeapTrace::iterator &HeapTrace::iterator::operator=(const HeapTrace::iterator & rhs)
{
    if(*this != rhs){
        this->trace = rhs.trace;
        this->idx = rhs.idx;
    }
    return *this;
}

bool HeapTrace::iterator::operator==(const HeapTrace::iterator & rhs) const
{
    return this->trace == rhs.trace
            && this->idx == rhs.idx;
}

bool HeapTrace::iterator::operator!=(const HeapTrace::iterator &rhs) const
{
    return !(*this == rhs);
}

bool HeapTrace::iterator::operator<(const HeapTrace::iterator & rhs) const
{
    return this->trace == rhs.trace
            && this->idx < rhs.idx;
}

bool HeapTrace::iterator::operator>(const HeapTrace::iterator & rhs) const
{
    return this->trace == rhs.trace
            && this->idx > rhs.idx;
}

bool HeapTrace::iterator::operator<=(const HeapTrace::iterator & rhs) const
{
    return this->trace == rhs.trace
            && this->idx <= rhs.idx;
}

bool HeapTrace::iterator::operator>=(const HeapTrace::iterator & rhs) const
{
    return this->trace == rhs.trace
            && this->idx >= rhs.idx;
}

HeapTrace::iterator &HeapTrace::iterator::operator++()
{
    idx++;
    return *this;
}

HeapTrace::iterator &HeapTrace::iterator::operator--()
{
    idx--;
    return *this;
}

HeapTrace::iterator::reference HeapTrace::iterator::operator*()
{
    return *trace->heap[trace->itToAddresses[idx]].get();
}

HeapTrace::iterator::pointer HeapTrace::iterator::operator->()
{
    return trace->heap[trace->itToAddresses[idx]].get();
}

HeapTrace::reverse_iterator::reverse_iterator(HeapTrace & trace): trace(&trace)
{
    int size = trace.itToAddresses.size() - 1;
    idx = size;
}

HeapTrace::reverse_iterator::reverse_iterator(HeapTrace & trace, int offset):
    trace(&trace), idx(offset)
{

}

HeapTrace::reverse_iterator::reverse_iterator(const HeapTrace::reverse_iterator & rhs):
    trace(rhs.trace), idx(rhs.idx)
{

}

HeapTrace::reverse_iterator::~reverse_iterator()
{
    // Nothing to clean up with this reverse_iterator
}

HeapTrace::reverse_iterator &HeapTrace::reverse_iterator::operator=(const HeapTrace::reverse_iterator & rhs)
{
    if(*this != rhs){
        this->trace = rhs.trace;
        this->idx = rhs.idx;
    }
    return *this;
}

bool HeapTrace::reverse_iterator::operator==(const HeapTrace::reverse_iterator & rhs) const
{
    return this->trace == rhs.trace
            && this->idx == rhs.idx;
}

bool HeapTrace::reverse_iterator::operator!=(const HeapTrace::reverse_iterator &rhs) const
{
    return !(*this == rhs);
}

bool HeapTrace::reverse_iterator::operator<(const HeapTrace::reverse_iterator & rhs) const
{
    return this->trace == rhs.trace
            && this->idx > rhs.idx;
}

bool HeapTrace::reverse_iterator::operator>(const HeapTrace::reverse_iterator & rhs) const
{
    return this->trace == rhs.trace
            && this->idx < rhs.idx;
}

bool HeapTrace::reverse_iterator::operator<=(const HeapTrace::reverse_iterator & rhs) const
{
    return this->trace == rhs.trace
            && this->idx >= rhs.idx;
}

bool HeapTrace::reverse_iterator::operator>=(const HeapTrace::reverse_iterator & rhs) const
{
    return this->trace == rhs.trace
            && this->idx <= rhs.idx;
}

HeapTrace::reverse_iterator &HeapTrace::reverse_iterator::operator++()
{
    --idx;
    return *this;
}

HeapTrace::reverse_iterator &HeapTrace::reverse_iterator::operator--()
{
    ++idx;
    return *this;
}

HeapTrace::reverse_iterator::reference HeapTrace::reverse_iterator::operator*()
{
    return *trace->heap[trace->itToAddresses[idx]].get();
}

HeapTrace::reverse_iterator::pointer HeapTrace::reverse_iterator::operator->()
{
    return trace->heap[trace->itToAddresses[idx]].get();
}

HeapTrace::const_iterator::const_iterator(const HeapTrace & trace): trace(&trace), idx(0)
{

}

HeapTrace::const_iterator::const_iterator(const HeapTrace & trace, int offset):
    trace(&trace), idx(offset)
{

}

HeapTrace::const_iterator::const_iterator(const HeapTrace::const_iterator & rhs):
    trace(rhs.trace), idx(rhs.idx)
{

}

HeapTrace::const_iterator::~const_iterator()
{
    // Nothing to clean up with this iterator
}

HeapTrace::const_iterator &HeapTrace::const_iterator::operator=(const HeapTrace::const_iterator & rhs)
{
    if(*this != rhs){
        this->trace = rhs.trace;
        this->idx = rhs.idx;
    }
    return *this;
}

bool HeapTrace::const_iterator::operator==(const HeapTrace::const_iterator & rhs) const
{
    return this->trace == rhs.trace
            && this->idx == rhs.idx;
}

bool HeapTrace::const_iterator::operator!=(const HeapTrace::const_iterator &rhs) const
{
    return !(*this == rhs);
}

bool HeapTrace::const_iterator::operator<(const HeapTrace::const_iterator & rhs) const
{
    return this->trace == rhs.trace
            && this->idx < rhs.idx;
}

bool HeapTrace::const_iterator::operator>(const HeapTrace::const_iterator & rhs) const
{
    return this->trace == rhs.trace
            && this->idx > rhs.idx;
}

bool HeapTrace::const_iterator::operator<=(const HeapTrace::const_iterator & rhs) const
{
    return this->trace == rhs.trace
            && this->idx <= rhs.idx;
}

bool HeapTrace::const_iterator::operator>=(const HeapTrace::const_iterator & rhs) const
{
    return this->trace == rhs.trace
            && this->idx >= rhs.idx;
}

HeapTrace::const_iterator &HeapTrace::const_iterator::operator++()
{
    ++idx;
    return *this;
}

HeapTrace::const_iterator &HeapTrace::const_iterator::operator--()
{
    --idx;
    return *this;
}

HeapTrace::const_iterator::reference HeapTrace::const_iterator::operator*() const
{
    return *trace->heap[trace->itToAddresses[idx]].get();
}

HeapTrace::const_iterator::pointer HeapTrace::const_iterator::operator->() const
{
    return trace->heap[trace->itToAddresses[idx]].get();
}

HeapTrace::const_reverse_iterator::const_reverse_iterator(const HeapTrace & trace):
    trace(&trace)
{
    int size = trace.heap.size() - 1;
    idx = size;
}

HeapTrace::const_reverse_iterator::const_reverse_iterator(const HeapTrace & trace, int offset):
    trace(&trace), idx(offset)
{

}

HeapTrace::const_reverse_iterator::const_reverse_iterator(const HeapTrace::const_reverse_iterator & rhs):
    trace(rhs.trace), idx(rhs.idx)
{

}

HeapTrace::const_reverse_iterator::~const_reverse_iterator()
{
    // Nothing to clean up with this iterator
}

HeapTrace::const_reverse_iterator &HeapTrace::const_reverse_iterator::operator=(const HeapTrace::const_reverse_iterator & rhs)
{
    if(*this != rhs){
        this->trace = rhs.trace;
        this->idx = rhs.idx;
    }
    return *this;
}

bool HeapTrace::const_reverse_iterator::operator==(const HeapTrace::const_reverse_iterator & rhs) const
{
    return this->trace == rhs.trace
            && this->idx == rhs.idx;
}

bool HeapTrace::const_reverse_iterator::operator!=(const HeapTrace::const_reverse_iterator &rhs) const
{
    return !(*this == rhs);
}

bool HeapTrace::const_reverse_iterator::operator<(const HeapTrace::const_reverse_iterator & rhs) const
{
    return this->trace == rhs.trace
            && this->idx > rhs.idx;
}

bool HeapTrace::const_reverse_iterator::operator>(const HeapTrace::const_reverse_iterator & rhs) const
{
    return this->trace == rhs.trace
            && this->idx < rhs.idx;
}

bool HeapTrace::const_reverse_iterator::operator<=(const HeapTrace::const_reverse_iterator & rhs) const
{
    return this->trace == rhs.trace
            && this->idx >= rhs.idx;
}

bool HeapTrace::const_reverse_iterator::operator>=(const HeapTrace::const_reverse_iterator & rhs) const
{
    return this->trace == rhs.trace
            && this->idx <= rhs.idx;
}

HeapTrace::const_reverse_iterator &HeapTrace::const_reverse_iterator::operator++()
{
    --idx;
    return *this;
}

HeapTrace::const_reverse_iterator &HeapTrace::const_reverse_iterator::operator--()
{
    ++idx;
    return *this;
}

HeapTrace::const_reverse_iterator::reference HeapTrace::const_reverse_iterator::operator*() const
{
    return *trace->heap[trace->itToAddresses[idx]].get();
}

HeapTrace::const_reverse_iterator::pointer HeapTrace::const_reverse_iterator::operator->() const
{
    return trace->heap[trace->itToAddresses[idx]].get();
}
