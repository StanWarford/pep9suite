#TEMPLATE = lib
#TARGET = Pep9Common
#CONFIG += staticlib
QT += webenginewidgets widgets printsupport concurrent
INCLUDEPATH += $$PWD\..\pep9common

# Always prefix files with their fully qualified paths.
# Otherwise, including projects can't find the files.

FORMS += \
	$$PWD\cpupane.ui \
    $$PWD\memorydumppane.ui \
    $$PWD\microcodepane.ui \
    $$PWD\microobjectcodepane.ui \

HEADERS += \
    $$PWD\amccpumodel.h \
    $$PWD\cpupane.h \
    $$PWD\cpugraphicsitems.h \
    $$PWD\disableselectionmodel.h \
    $$PWD\memorydumppane.h \
    $$PWD\microasm.h \
    $$PWD\microcode.h \
    $$PWD\microcodeeditor.h \
    $$PWD\microcodepane.h \
    $$PWD\microcodeprogram.h \
    $$PWD\microobjectcodepane.h \
    $$PWD\pepmicrohighlighter.h \
    $$PWD\rotatedheaderview.h \
    $$PWD\shapes_one_byte_data_bus.h \
    $$PWD\shapes_two_byte_data_bus.h \
    $$PWD\specification.h \
    $$PWD\tristatelabel.h \
    $$PWD\newcpudata.h

SOURCES += \
    $$PWD\amccpumodel.cpp \
    $$PWD\cpupane.cpp \
    $$PWD\cpugraphicsitems.cpp \
    $$PWD\disableselectionmodel.cpp \
    $$PWD\memorydumppane.cpp \
    $$PWD\microasm.cpp \
    $$PWD\microcode.cpp \
    $$PWD\microcodeeditor.cpp \
    $$PWD\microcodepane.cpp \
    $$PWD\microcodeprogram.cpp \
    $$PWD\microobjectcodepane.cpp \
    $$PWD\pepmicrohighlighter.cpp \
    $$PWD\rotatedheaderview.cpp \
    $$PWD\specification.cpp \
    $$PWD\tristatelabel.cpp \
    $$PWD\newcpudata.cpp
