#ifndef STACKTRACE_H
#define STACKTRACE_H

#include <QObject>
#include <QStack>
#include <QSharedPointer>
#include "enu.h"
class AType;
struct MemTag
{
    quint16 addr;
    QPair<Enu::ESymbolFormat, QString> type;
};

class StackFrame
{
private:
    QStack<MemTag> stack;
public:
    void push(MemTag tag);
    bool pop(quint16 size);
    quint16 size() const;
    operator QString() const;
};

class StackTrace
{
    QStack<QSharedPointer<StackFrame>> callStack;
    QSharedPointer<StackFrame> nextFrame;
public:
    explicit StackTrace();
    void call(quint16 sp);
    void ret();
    void pushLocals(quint16 start, QList<QPair<Enu::ESymbolFormat, QString> > items);
    void pushParams(quint16 start, QList<QPair<Enu::ESymbolFormat, QString> > items);
    void popLocals(quint16 size);
    void popParams(quint16 size);
    quint16 callDepth() const;
    operator QString() const;
};

class HeapTrace
{

};

class GlobalTrace
{

};

class MemoryTrace
{
    // Temporary, just to test out
public:
    explicit MemoryTrace();
    void clear();
    StackTrace userStack, osStack, *activeStack;
    HeapTrace heapTrace;
    GlobalTrace globalTrace;
};



#endif // STACKTRACE_H
