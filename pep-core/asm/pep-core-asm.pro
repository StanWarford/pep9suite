# -------------------------------------------------
# Project created by Matthew McRaven, 07/05/2020
# -------------------------------------------------
QT -= gui
QT += widgets printsupport concurrent

TEMPLATE = lib
CONFIG += staticlib
TARGET = pep-core-asm

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Default rules for deployment.
unix {
    target.path = $$[QT_INSTALL_PLUGINS]/generic
}
!isEmpty(target.path): INSTALLS += target

#Prevent Windows from trying to parse the project three times per build.
CONFIG -= debug_and_release \
    debug_and_release_target
#Flag for enabling C++17 features.
#Due to support for C++17 features being added before the standard was finalized, and the placeholder text of "C++1z" has remained
CONFIG += c++17
win32{
    #MSVC doesn't recognize c++1z flag, so use the MSVC specific flag here
    win32-msvc*: QMAKE_CXXFLAGS += /std:c++17
    #Flags needed to generate PDB information in release. Necessary information to profile program.
    #Flags also picked to provide a ~15% speed increase in release mode (at the cost of increased compile times).
    QMAKE_LFLAGS_RELEASE +=/MAP
    QMAKE_CFLAGS_RELEASE += /MD /zi
    QMAKE_LFLAGS_RELEASE +=/debug /opt:ref
}

include(assembler/assembler.pri)
include(cache/cache.pri)
include(cpu/cpu.pri)
include(debugger/debugger.pri)
include(editor/editor.pri)
include(highlight/highlight.pri)
include(stack/stack.pri)
include(stats/stats.pri)

# Link against "common" dependencies.
unix|win32: LIBS += -L$$OUT_PWD/../common/ -lpep-core-common

INCLUDEPATH += $$PWD/../common
DEPENDPATH += $$PWD/../common

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../common/pep-core-common.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../common/libpep-core-common.a
