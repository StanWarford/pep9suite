#TEMPLATE = lib
#TARGET = Pep9Common
#CONFIG += staticlib
QT += widgets printsupport concurrent
INCLUDEPATH += $$PWD\..\pep9common
VPATH += $$PWD\..\pep9common

FORMS += \
    asmobjectcodepane.ui \
    asmsourcecodepane.ui \
    cacheaddresstranslator.ui \
    executionstatisticswidget.ui \
    memorytracepane.ui \
    redefinemnemonicsdialog.ui \
    asmcpupane.ui \
    asmprogramtracepane.ui \
    asmprogramlistingpane.ui \
    assemblerpane.ui

HEADERS += \
    asmargument.h \
    asmcode.h \
    asmobjectcodepane.h \
    asmprogram.h \
    asmprogrammanager.h \
    asmsourcecodepane.h \
    cacheaddresstranslator.h \
    cpphighlighter.h \
    executionstatisticswidget.h \
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
    asmprogramlistingpane.h \
    assemblerpane.h

SOURCES += \
    asmargument.cpp \
    asmcode.cpp \
    asmobjectcodepane.cpp \
    asmprogram.cpp \
    asmprogrammanager.cpp \
    asmsourcecodepane.cpp \
    cacheaddresstranslator.cpp \
    cachealgs.cpp \
    cpphighlighter.cpp \
    executionstatisticswidget.cpp \
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
    asmprogramlistingpane.cpp \
    assemblerpane.cpp

# Cache components
FORMS += \
    cacheconfig.ui \
    cacheview.ui

HEADERS += \
    cache.h \
    cachealgs.h \
    cacheconfig.h \
    cacheline.h \
    cachememory.h \
    cachereplace.h \
    cacheview.h

SOURCES += \
    cacheconfig.cpp \
    cacheline.cpp \
    cachememory.cpp \
    cachereplace.cpp \
    cacheview.cpp
