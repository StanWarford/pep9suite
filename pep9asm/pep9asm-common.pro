#TEMPLATE = lib
#TARGET = Pep9Common
#CONFIG += staticlib
QT += webenginewidgets widgets printsupport concurrent
INCLUDEPATH += $$PWD\..\pep9common
VPATH += $$PWD\..\pep9common

FORMS += \
    asmlistingpane.ui \
    asmobjectcodepane.ui \
    asmsourcecodepane.ui \
    asmtracepane.ui \
    memorytracepane.ui \

HEADERS += \
    asmargument.h \
    asmcode.h \
    asmlistingpane.h \
    asmobjectcodepane.h \
    asmprogram.h \
    asmprogrammanager.h \
    asmsourcecodepane.h \
    asmtracepane.h \
    cpphighlighter.h \
    interfaceisacpu.h \
    isaasm.h \
    memorycellgraphicsitem.h \
    memorytracepane.h \
    pepasmhighlighter.h \
    stackframefsm.h \
    typetags.h \
    stacktrace.h

SOURCES += \
    asmargument.cpp \
    asmcode.cpp \
    asmlistingpane.cpp \
    asmobjectcodepane.cpp \
    asmprogram.cpp \
    asmprogrammanager.cpp \
    asmsourcecodepane.cpp \
    asmtracepane.cpp \
    cpphighlighter.cpp \
    interfaceisacpu.cpp \
    isaasm.cpp \
    memorycellgraphicsitem.cpp \
    memorytracepane.cpp \
    pepasmhighlighter.cpp \
    stackframefsm.cpp \
    typetags.cpp \
    stacktrace.cpp
