#TEMPLATE = lib
#TARGET = Pep9Common
#CONFIG += staticlib
QT += widgets printsupport concurrent
INCLUDEPATH += $$PWD\..\pep9common
VPATH += $$PWD\..\pep9common

FORMS += \
    asmobjectcodepane.ui \
    asmsourcecodepane.ui \
    memorytracepane.ui \
    redefinemnemonicsdialog.ui \
    asmcpupane.ui \
    asmprogramtracepane.ui \
    asmprogramlistingpane.ui

HEADERS += \
    asmargument.h \
    asmcode.h \
    asmobjectcodepane.h \
    asmprogram.h \
    asmprogrammanager.h \
    asmsourcecodepane.h \
    cpphighlighter.h \
    interfaceisacpu.h \
    isaasm.h \
    memorycellgraphicsitem.h \
    memorytracepane.h \
    pepasmhighlighter.h \
    typetags.h \
    stacktrace.h \
    redefinemnemonicsdialog.h \
    asmcpupane.h \
    isacpu.h \
    isacpumemoizer.h \
    memoizerhelper.h \
    asmprogramtracepane.h \
    asmprogramlistingpane.h

SOURCES += \
    asmargument.cpp \
    asmcode.cpp \
    asmobjectcodepane.cpp \
    asmprogram.cpp \
    asmprogrammanager.cpp \
    asmsourcecodepane.cpp \
    cpphighlighter.cpp \
    interfaceisacpu.cpp \
    isaasm.cpp \
    memorycellgraphicsitem.cpp \
    memorytracepane.cpp \
    pepasmhighlighter.cpp \
    typetags.cpp \
    stacktrace.cpp \
    redefinemnemonicsdialog.cpp \
    asmcpupane.cpp \
    isacpu.cpp \
    isacpumemoizer.cpp \
    memoizerhelper.cpp \
    asmprogramtracepane.cpp \
    asmprogramlistingpane.cpp
