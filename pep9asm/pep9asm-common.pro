#TEMPLATE = lib
#TARGET = Pep9Common
#CONFIG += staticlib
QT += webenginewidgets widgets printsupport concurrent
INCLUDEPATH += $$PWD\..\pep9common
VPATH += $$PWD\..\pep9common

# Issues occur when referencing source files from outside the current directory
# In windows, files must use the fully qualified path in order to be found
win32: {
    SRC_PTH = $$PWD\
}
# On Max OS X or Ubuntu, qmake will not function with fully qualified paths
else: {
    SRC_PTH = ""
}
FORMS += \
    $$SRC_PTH asmlistingpane.ui \
    $$SRC_PTH asmobjectcodepane.ui \
    $$SRC_PTH asmsourcecodepane.ui \
    $$SRC_PTH asmtracepane.ui \
    $$SRC_PTH memorytracepane.ui \

HEADERS += \
    $$SRC_PTH asmargument.h \
    $$SRC_PTH asmcode.h \
    $$SRC_PTH asmlistingpane.h \
    $$SRC_PTH asmobjectcodepane.h \
    $$SRC_PTH asmprogram.h \
    $$SRC_PTH asmprogrammanager.h \
    $$SRC_PTH asmsourcecodepane.h \
    $$SRC_PTH asmtracepane.h \
    $$SRC_PTH cpphighlighter.h \
    $$SRC_PTH interfaceisacpu.h \
    $$SRC_PTH isaasm.h \
    $$SRC_PTH memorycellgraphicsitem.h \
    $$SRC_PTH memorytracepane.h \
    $$SRC_PTH pepasmhighlighter.h \
    $$SRC_PTH stackframefsm.h \
    $$SRC_PTH typetags.h \
    $$SRC_PTH stacktrace.h

SOURCES += \
    $$SRC_PTH asmargument.cpp \
    $$SRC_PTH asmcode.cpp \
    $$SRC_PTH asmlistingpane.cpp \
    $$SRC_PTH asmobjectcodepane.cpp \
    $$SRC_PTH asmprogram.cpp \
    $$SRC_PTH asmprogrammanager.cpp \
    $$SRC_PTH asmsourcecodepane.cpp \
    $$SRC_PTH asmtracepane.cpp \
    $$SRC_PTH cpphighlighter.cpp \
    $$SRC_PTH interfaceisacpu.cpp \
    $$SRC_PTH isaasm.cpp \
    $$SRC_PTH memorycellgraphicsitem.cpp \
    $$SRC_PTH memorytracepane.cpp \
    $$SRC_PTH pepasmhighlighter.cpp \
    $$SRC_PTH stackframefsm.cpp \
    $$SRC_PTH typetags.cpp \
    $$SRC_PTH stacktrace.cpp
