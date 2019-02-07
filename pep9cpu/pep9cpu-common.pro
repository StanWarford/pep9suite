#TEMPLATE = lib
#TARGET = Pep9Common
#CONFIG += staticlib
QT += webenginewidgets widgets printsupport concurrent
INCLUDEPATH += $$PWD\..\pep9common
VPATH += $$PWD\..\pep9common

FORMS += \
    cpupane.ui \
    microcodepane.ui \
    microobjectcodepane.ui \

HEADERS += \
    cpupane.h \
    cpugraphicsitems.h \
    disableselectionmodel.h \
    interfacemccpu.h \
    microasm.h \
    microcode.h \
    microcodeeditor.h \
    microcodepane.h \
    microcodeprogram.h \
    microobjectcodepane.h \
    pepmicrohighlighter.h \
    rotatedheaderview.h \
    shapes_one_byte_data_bus.h \
    shapes_two_byte_data_bus.h \
    specification.h \
    tristatelabel.h \
    newcpudata.h \
    registerfile.h

SOURCES += \
    cpupane.cpp \
    cpugraphicsitems.cpp \
    disableselectionmodel.cpp \
    interfacemccpu.cpp \
    microasm.cpp \
    microcode.cpp \
    microcodeeditor.cpp \
    microcodepane.cpp \
    microcodeprogram.cpp \
    microobjectcodepane.cpp \
    pepmicrohighlighter.cpp \
    rotatedheaderview.cpp \
    specification.cpp \
    tristatelabel.cpp \
    newcpudata.cpp \
    registerfile.cpp
