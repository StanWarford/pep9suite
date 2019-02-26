#ifndef STACKTRACE_H
#define STACKTRACE_H

#include <QObject>
#include <QStack>
#include <QSharedPointer>
#include "enu.h"
class AType;
class AMemoryDevice;
struct MemTag
{
    quint16 addr;
    QPair<Enu::ESymbolFormat, QString> type;
    operator QString() const;
};

class StackFrame
{
private:
    QStack<MemTag> stack;
public:

    class iterator;
    class reverse_iterator;
    class const_iterator;
    class const_reverse_iterator;

    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;
    const_iterator cbegin() const;
    const_iterator cend() const;

    reverse_iterator rbegin();
    reverse_iterator rend();
    const_reverse_iterator rbegin() const;
    const_reverse_iterator rend() const;
    const_reverse_iterator crbegin() const;
    const_reverse_iterator crend() const;

    bool isOrphaned = false;
    void push(MemTag tag);
    bool pop(quint16 size);
    quint16 size() const;
    quint16 numItems() const;
    operator QString() const;
};

class StackTrace
{
    QStack<QSharedPointer<StackFrame>> callStack;
    QSharedPointer<StackFrame> nextFrame;
    QString errMessage;
    bool stackIntact;
public:

    class iterator;
    class reverse_iterator;
    class const_iterator;
    class const_reverse_iterator;

    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;
    const_iterator cbegin() const;
    const_iterator cend() const;

    reverse_iterator rbegin();
    reverse_iterator rend();
    const_reverse_iterator rbegin() const;
    const_reverse_iterator rend() const;
    const_reverse_iterator crbegin() const;
    const_reverse_iterator crend() const;

    explicit StackTrace();
    void call(quint16 sp);
    void clear();
    bool ret();
    void pushLocals(quint16 start, QList<QPair<Enu::ESymbolFormat, QString> > items);
    void pushParams(quint16 start, QList<QPair<Enu::ESymbolFormat, QString> > items);
    bool popLocals(quint16 size);
    bool popParams(quint16 size);
    bool popAndOrphan(quint16 size);
    quint16 callDepth() const;
    const StackFrame& getTOS();
    operator QString() const;

    bool isStackIntact() const;
    void setStackIntact(bool value);
    QString getErrorMessage() const;
    void setErrorMessage(QString message);

};

class HeapTrace
{
    QVector<quint16> itToAddresses;
    QMap<quint16, QSharedPointer<StackFrame>> heap;
    QString errMessage;
    bool intact, addNew, isInMalloc;
public:

    class iterator;
    class reverse_iterator;
    class const_iterator;
    class const_reverse_iterator;


    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;
    const_iterator cbegin() const;
    const_iterator cend() const;

    reverse_iterator rbegin();
    reverse_iterator rend();
    const_reverse_iterator rbegin() const;
    const_reverse_iterator rend() const;
    const_reverse_iterator crbegin() const;
    const_reverse_iterator crend() const;

    explicit HeapTrace();
    void pushHeap(quint16 start, QList<QPair<Enu::ESymbolFormat, QString> > items);
    void clear();
    void setCanAddNew(bool val);
    void setHeapIntact(bool val);
    void setInMalloc(bool val);
    bool canAddNew() const;
    bool heapIntact() const;
    bool inMalloc() const;
    QString getErrorMessage() const;
    void setErrorMessage(QString message);

    operator QString() const;
};

class GlobalTrace
{
    QMap<quint16, MemTag> tags;
public:
    explicit GlobalTrace();
    void setTags(QList<QPair<quint16 /*address*/,
                  QPair<Enu::ESymbolFormat, QString> /*tag*/ > > items);
    void clear();
    const QMap<quint16, MemTag> getMemTags() const;
};

class MemoryTrace
{
    bool traceWarnings;
    // Temporary, just to test out
public:
    explicit MemoryTrace();
    void clear();
    StackTrace userStack, *activeStack;
    HeapTrace heapTrace;
    GlobalTrace globalTrace;
    bool hasTraceWarnings() const;
    void setHasTraceWarnings(bool value);
};

class StackFrame::iterator {
    StackFrame * frame;
    int idx;
public:
    typedef typename std::allocator<MemTag>::difference_type difference_type;
    typedef typename std::allocator<MemTag>::value_type value_type;
    typedef typename std::allocator<MemTag>::reference reference;
    typedef typename std::allocator<MemTag>::pointer pointer;
    //typedef std::random_access_iterator_tag iterator_category; //or another tag

    iterator(StackFrame&);
    iterator(StackFrame&, int offset);
    iterator (const iterator&);
    ~iterator();

    iterator& operator=(const iterator&);
    bool operator==(const iterator&) const;
    bool operator!=(const iterator&) const;
    bool operator<(const iterator&) const; //optional
    bool operator>(const iterator&) const; //optional
    bool operator<=(const iterator&) const; //optional
    bool operator>=(const iterator&) const; //optional

    iterator& operator++();
    iterator& operator--(); //optional

    reference operator*();
    pointer operator->();
    //reference operator[](size_type) const; //optional
};

class StackFrame::reverse_iterator{
    StackFrame * frame;
    int idx;
public:
    typedef typename std::allocator<MemTag>::difference_type difference_type;
    typedef typename std::allocator<MemTag>::value_type value_type;
    typedef typename std::allocator<MemTag>::reference reference;
    typedef typename std::allocator<MemTag>::pointer pointer;
    //typedef std::random_access_iterator_tag iterator_category; //or another tag

    reverse_iterator(StackFrame&);
    reverse_iterator(StackFrame&, int offset);
    reverse_iterator (const reverse_iterator&);
    ~reverse_iterator();

    reverse_iterator& operator=(const reverse_iterator&);
    bool operator==(const reverse_iterator&) const;
    bool operator!=(const reverse_iterator&) const;
    bool operator<(const reverse_iterator&) const; //optional
    bool operator>(const reverse_iterator&) const; //optional
    bool operator<=(const reverse_iterator&) const; //optional
    bool operator>=(const reverse_iterator&) const; //optional

    reverse_iterator& operator++();
    reverse_iterator& operator--(); //optional

    reference operator*();
    pointer operator->();

};

class StackFrame::const_iterator {
    const StackFrame * frame;
    int idx;
public:
    typedef typename std::allocator<const MemTag>::difference_type difference_type;
    typedef typename std::allocator<const MemTag>::value_type value_type;
    typedef typename std::allocator<const MemTag>::const_reference reference;
    typedef typename std::allocator<const MemTag>::pointer pointer;
    //typedef std::random_access_iterator_tag iterator_category; //or another tag

    const_iterator(const StackFrame&);
    const_iterator(const StackFrame&, int offset);
    const_iterator (const const_iterator&);
    ~const_iterator();

    const_iterator& operator=(const const_iterator&);
    bool operator==(const const_iterator&) const;
    bool operator!=(const const_iterator&) const;
    bool operator<(const const_iterator&) const; //optional
    bool operator>(const const_iterator&) const; //optional
    bool operator<=(const const_iterator&) const; //optional
    bool operator>=(const const_iterator&) const; //optional

    const_iterator& operator++();
    const_iterator& operator--(); //optional

    reference operator*() const;
    pointer operator->() const;
    //reference operator[](size_type) const; //optional
};

class StackFrame::const_reverse_iterator {
    const StackFrame * frame;
    int idx;
public:
    typedef typename std::allocator<const MemTag>::difference_type difference_type;
    typedef typename std::allocator<const MemTag>::value_type value_type;
    typedef typename std::allocator<const MemTag>::const_reference reference;
    typedef typename std::allocator<const MemTag>::pointer pointer;
    //typedef std::random_access_iterator_tag iterator_category; //or another tag

    const_reverse_iterator(const StackFrame&);
    const_reverse_iterator(const StackFrame&, int offset);
    const_reverse_iterator (const const_reverse_iterator&);
    ~const_reverse_iterator();

    const_reverse_iterator& operator=(const const_reverse_iterator&);
    bool operator==(const const_reverse_iterator&) const;
    bool operator!=(const const_reverse_iterator&) const;
    bool operator<(const const_reverse_iterator&) const; //optional
    bool operator>(const const_reverse_iterator&) const; //optional
    bool operator<=(const const_reverse_iterator&) const; //optional
    bool operator>=(const const_reverse_iterator&) const; //optional

    const_reverse_iterator& operator++();
    const_reverse_iterator& operator--(); //optional

    reference operator*() const;
    pointer operator->() const;
    //reference operator[](size_type) const; //optional
};

class StackTrace::iterator {
    StackTrace * trace;
    int idx;
public:
    typedef typename std::allocator<StackFrame>::difference_type difference_type;
    typedef typename std::allocator<StackFrame>::value_type value_type;
    typedef typename std::allocator<StackFrame>::reference reference;
    typedef typename std::allocator<StackFrame>::pointer pointer;
    //typedef std::random_access_iterator_tag iterator_category; //or another tag

    iterator(StackTrace&);
    iterator(StackTrace&, int offset);
    iterator (const iterator&);
    ~iterator();

    iterator& operator=(const iterator&);
    bool operator==(const iterator&) const;
    bool operator!=(const iterator&) const;
    bool operator<(const iterator&) const; //optional
    bool operator>(const iterator&) const; //optional
    bool operator<=(const iterator&) const; //optional
    bool operator>=(const iterator&) const; //optional

    iterator& operator++();
    iterator& operator--(); //optional

    reference operator*();
    pointer operator->();
    //reference operator[](size_type) const; //optional
};

class StackTrace::reverse_iterator {
    StackTrace * trace;
    int idx;
public:
    typedef typename std::allocator<StackFrame>::difference_type difference_type;
    typedef typename std::allocator<StackFrame>::value_type value_type;
    typedef typename std::allocator<StackFrame>::reference reference;
    typedef typename std::allocator<StackFrame>::pointer pointer;
    //typedef std::random_access_iterator_tag iterator_category; //or another tag

    reverse_iterator(StackTrace&);
    reverse_iterator(StackTrace&, int offset);
    reverse_iterator (const reverse_iterator&);
    ~reverse_iterator();

    reverse_iterator& operator=(const reverse_iterator&);
    bool operator==(const reverse_iterator&) const;
    bool operator!=(const reverse_iterator&) const;
    bool operator<(const reverse_iterator&) const; //optional
    bool operator>(const reverse_iterator&) const; //optional
    bool operator<=(const reverse_iterator&) const; //optional
    bool operator>=(const reverse_iterator&) const; //optional

    reverse_iterator& operator++();
    reverse_iterator& operator--(); //optional

    reference operator*();
    pointer operator->();
    //reference operator[](size_type) const; //optional
};

class StackTrace::const_iterator {
    const StackTrace * trace;
    int idx;
public:
    typedef typename std::allocator<const StackFrame>::difference_type difference_type;
    typedef typename std::allocator<const StackFrame>::value_type value_type;
    typedef typename std::allocator<const StackFrame>::const_reference reference;
    typedef typename std::allocator<const StackFrame>::pointer pointer;
    //typedef std::random_access_iterator_tag iterator_category; //or another tag

    const_iterator(const StackTrace&);
    const_iterator(const StackTrace&, int offset);
    const_iterator (const const_iterator&);
    ~const_iterator();

    const_iterator& operator=(const const_iterator&);
    bool operator==(const const_iterator&) const;
    bool operator!=(const const_iterator&) const;
    bool operator<(const const_iterator&) const; //optional
    bool operator>(const const_iterator&) const; //optional
    bool operator<=(const const_iterator&) const; //optional
    bool operator>=(const const_iterator&) const; //optional

    const_iterator& operator++();
    const_iterator& operator--(); //optional

    reference operator*() const;
    pointer operator->() const;
    //reference operator[](size_type) const; //optional
};

class StackTrace::const_reverse_iterator {
    const StackTrace * trace;
    int idx;
public:
    typedef typename std::allocator<const StackFrame>::difference_type difference_type;
    typedef typename std::allocator<const StackFrame>::value_type value_type;
    typedef typename std::allocator<const StackFrame>::const_reference reference;
    typedef typename std::allocator<const StackFrame>::pointer pointer;
    //typedef std::random_access_iterator_tag iterator_category; //or another tag

    const_reverse_iterator(const StackTrace&);
    const_reverse_iterator(const StackTrace&, int offset);
    const_reverse_iterator (const const_reverse_iterator&);
    ~const_reverse_iterator();

    const_reverse_iterator& operator=(const const_reverse_iterator&);
    bool operator==(const const_reverse_iterator&) const;
    bool operator!=(const const_reverse_iterator&) const;
    bool operator<(const const_reverse_iterator&) const; //optional
    bool operator>(const const_reverse_iterator&) const; //optional
    bool operator<=(const const_reverse_iterator&) const; //optional
    bool operator>=(const const_reverse_iterator&) const; //optional

    const_reverse_iterator& operator++();
    const_reverse_iterator& operator--(); //optional

    reference operator*() const;
    pointer operator->() const;
    //reference operator[](size_type) const; //optional
};

class HeapTrace::iterator {
    HeapTrace * trace;
    int idx;
public:
    typedef typename std::allocator<StackFrame>::difference_type difference_type;
    typedef typename std::allocator<StackFrame>::value_type value_type;
    typedef typename std::allocator<StackFrame>::reference reference;
    typedef typename std::allocator<StackFrame>::pointer pointer;
    //typedef std::random_access_iterator_tag iterator_category; //or another tag

    iterator(HeapTrace&);
    iterator(HeapTrace&, int offset);
    iterator (const iterator&);
    ~iterator();

    iterator& operator=(const iterator&);
    bool operator==(const iterator&) const;
    bool operator!=(const iterator&) const;
    bool operator<(const iterator&) const; //optional
    bool operator>(const iterator&) const; //optional
    bool operator<=(const iterator&) const; //optional
    bool operator>=(const iterator&) const; //optional

    iterator& operator++();
    iterator& operator--(); //optional

    reference operator*();
    pointer operator->();
    //reference operator[](size_type) const; //optional
};

class HeapTrace::reverse_iterator {
    HeapTrace * trace;
    int idx;
public:
    typedef typename std::allocator<StackFrame>::difference_type difference_type;
    typedef typename std::allocator<StackFrame>::value_type value_type;
    typedef typename std::allocator<StackFrame>::reference reference;
    typedef typename std::allocator<StackFrame>::pointer pointer;
    //typedef std::random_access_iterator_tag iterator_category; //or another tag

    reverse_iterator(HeapTrace&);
    reverse_iterator(HeapTrace&, int offset);
    reverse_iterator (const reverse_iterator&);
    ~reverse_iterator();

    reverse_iterator& operator=(const reverse_iterator&);
    bool operator==(const reverse_iterator&) const;
    bool operator!=(const reverse_iterator&) const;
    bool operator<(const reverse_iterator&) const; //optional
    bool operator>(const reverse_iterator&) const; //optional
    bool operator<=(const reverse_iterator&) const; //optional
    bool operator>=(const reverse_iterator&) const; //optional

    reverse_iterator& operator++();
    reverse_iterator& operator--(); //optional

    reference operator*();
    pointer operator->();
    //reference operator[](size_type) const; //optional
};

class HeapTrace::const_iterator {
    const HeapTrace * trace;
    int idx;
public:
    typedef typename std::allocator<const StackFrame>::difference_type difference_type;
    typedef typename std::allocator<const StackFrame>::value_type value_type;
    typedef typename std::allocator<const StackFrame>::const_reference reference;
    typedef typename std::allocator<const StackFrame>::pointer pointer;
    //typedef std::random_access_iterator_tag iterator_category; //or another tag

    const_iterator(const HeapTrace&);
    const_iterator(const HeapTrace&, int offset);
    const_iterator (const const_iterator&);
    ~const_iterator();

    const_iterator& operator=(const const_iterator&);
    bool operator==(const const_iterator&) const;
    bool operator!=(const const_iterator&) const;
    bool operator<(const const_iterator&) const; //optional
    bool operator>(const const_iterator&) const; //optional
    bool operator<=(const const_iterator&) const; //optional
    bool operator>=(const const_iterator&) const; //optional

    const_iterator& operator++();
    const_iterator& operator--(); //optional

    reference operator*() const;
    pointer operator->() const;
    //reference operator[](size_type) const; //optional
};

class HeapTrace::const_reverse_iterator {
    const HeapTrace * trace;
    int idx;
public:
    typedef typename std::allocator<const StackFrame>::difference_type difference_type;
    typedef typename std::allocator<const StackFrame>::value_type value_type;
    typedef typename std::allocator<const StackFrame>::const_reference reference;
    typedef typename std::allocator<const StackFrame>::pointer pointer;
    //typedef std::random_access_iterator_tag iterator_category; //or another tag

    const_reverse_iterator(const HeapTrace&);
    const_reverse_iterator(const HeapTrace&, int offset);
    const_reverse_iterator (const const_reverse_iterator&);
    ~const_reverse_iterator();

    const_reverse_iterator& operator=(const const_reverse_iterator&);
    bool operator==(const const_reverse_iterator&) const;
    bool operator!=(const const_reverse_iterator&) const;
    bool operator<(const const_reverse_iterator&) const; //optional
    bool operator>(const const_reverse_iterator&) const; //optional
    bool operator<=(const const_reverse_iterator&) const; //optional
    bool operator>=(const const_reverse_iterator&) const; //optional

    const_reverse_iterator& operator++();
    const_reverse_iterator& operator--(); //optional

    reference operator*() const;
    pointer operator->() const;
    //reference operator[](size_type) const; //optional
};

#endif // STACKTRACE_H
