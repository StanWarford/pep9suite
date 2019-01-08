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
    $$SRC_PTH cpupane.ui \
    $$SRC_PTH microcodepane.ui \
    $$SRC_PTH microobjectcodepane.ui \

HEADERS += \
    $$SRC_PTH amccpumodel.h \
    $$SRC_PTH cpupane.h \
    $$SRC_PTH cpugraphicsitems.h \
    $$SRC_PTH disableselectionmodel.h \
    $$SRC_PTH microasm.h \
    $$SRC_PTH microcode.h \
    $$SRC_PTH microcodeeditor.h \
    $$SRC_PTH microcodepane.h \
    $$SRC_PTH microcodeprogram.h \
    $$SRC_PTH microobjectcodepane.h \
    $$SRC_PTH pepmicrohighlighter.h \
    $$SRC_PTH rotatedheaderview.h \
    $$SRC_PTH shapes_one_byte_data_bus.h \
    $$SRC_PTH shapes_two_byte_data_bus.h \
    $$SRC_PTH specification.h \
    $$SRC_PTH tristatelabel.h \
    $$SRC_PTH newcpudata.h

SOURCES += \
    $$SRC_PTH amccpumodel.cpp \
    $$SRC_PTH cpupane.cpp \
    $$SRC_PTH cpugraphicsitems.cpp \
    $$SRC_PTH disableselectionmodel.cpp \
    $$SRC_PTH microasm.cpp \
    $$SRC_PTH microcode.cpp \
    $$SRC_PTH microcodeeditor.cpp \
    $$SRC_PTH microcodepane.cpp \
    $$SRC_PTH microcodeprogram.cpp \
    $$SRC_PTH microobjectcodepane.cpp \
    $$SRC_PTH pepmicrohighlighter.cpp \
    $$SRC_PTH rotatedheaderview.cpp \
    $$SRC_PTH specification.cpp \
    $$SRC_PTH tristatelabel.cpp \
    $$SRC_PTH newcpudata.cpp
