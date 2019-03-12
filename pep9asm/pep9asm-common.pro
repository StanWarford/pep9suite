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
    redefinemnemonicsdialog.ui \
    asmcpupane.ui

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
    typetags.h \
    stacktrace.h \
    redefinemnemonicsdialog.h \
    asmcpupane.h \
    isacpu.h \
    isacpumemoizer.h

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
    typetags.cpp \
    stacktrace.cpp \
    redefinemnemonicsdialog.cpp \
    asmcpupane.cpp \
    isacpu.cpp \
    isacpumemoizer.cpp
