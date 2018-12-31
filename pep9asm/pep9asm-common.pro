#TEMPLATE = lib
#TARGET = Pep9Common
#CONFIG += staticlib
QT += webenginewidgets widgets printsupport concurrent
INCLUDEPATH += $$PWD\..\pep9common

# Always prefix files with their fully qualified paths.
# Otherwise, including projects can't find the files.


FORMS += \
    $$PWD\asmlistingpane.ui \
    $$PWD\asmobjectcodepane.ui \
    $$PWD\asmsourcecodepane.ui \
	$$PWD\asmtracepane.ui \
    $$PWD\memorytracepane.ui \

HEADERS += \
    $$PWD\asmargument.h \
    $$PWD\asmcode.h \
    $$PWD\asmlistingpane.h \
    $$PWD\asmobjectcodepane.h \
    $$PWD\asmprogram.h \
    $$PWD\asmprogrammanager.h \
    $$PWD\asmsourcecodepane.h \
	$$PWD\asmtracepane.h \
    $$PWD\cpphighlighter.h \
    $$PWD\isaasm.h \
    $$PWD\memorycellgraphicsitem.h \
    $$PWD\memorytracepane.h \
    $$PWD\pepasmhighlighter.h \
    $$PWD\stackframefsm.h \

SOURCES += \
    $$PWD\asmargument.cpp \
    $$PWD\asmcode.cpp \
    $$PWD\asmlistingpane.cpp \
    $$PWD\asmobjectcodepane.cpp \
    $$PWD\asmprogram.cpp \
    $$PWD\asmprogrammanager.cpp \
    $$PWD\asmsourcecodepane.cpp \
	$$PWD\asmtracepane.cpp \
    $$PWD\cpphighlighter.cpp \
    $$PWD\isaasm.cpp \
    $$PWD\memorycellgraphicsitem.cpp \
    $$PWD\memorytracepane.cpp \
    $$PWD\pepasmhighlighter.cpp \
    $$PWD\stackframefsm.cpp \
