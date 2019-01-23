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
    static const QPair<Enu::ESymbolFormat,QString> retType{Enu::ESymbolFormat::F_2H, "retAddr"};
    nextFrame->push({sp, retType});
    callStack.push(nextFrame);
    nextFrame = QSharedPointer<StackFrame>::create();
}

void StackTrace::ret()
{
    nextFrame = callStack.pop();
    nextFrame->pop(2);
}

void StackTrace::pushLocals(quint16 start, QList<QPair<Enu::ESymbolFormat, QString> > items)
{
    for(auto pair : items) {
        callStack.top()->push({start, pair});
        start -= Enu::tagNumBytes(pair.first);
    }
}

void StackTrace::pushParams(quint16 start, QList<QPair<Enu::ESymbolFormat, QString> > items)
{
    for(auto pair :items) {
        nextFrame->push({start,pair});
        start -= Enu::tagNumBytes(pair.first);
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

MemoryTrace::MemoryTrace()
{

}

void MemoryTrace::clear()
{
    //throw std::exception("Does not work");
}

void StackFrame::push(MemTag tag)
{
    stack.push(tag);
}

bool StackFrame::pop(quint16 size)
{
    quint16 popped = 0;
    while(popped<size) {
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
        items << QString("(%1: %2)").arg(tag->type.second).arg(tag->addr,4,16);
    }
    return items.join(", ");
}
