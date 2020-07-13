# -------------------------------------------------
# Project created by Matthew McRaven, 07/05/2020
# -------------------------------------------------
QT += widgets printsupport concurrent

TEMPLATE = lib
CONFIG += staticlib
TARGET = pep-core-common

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

RESOURCES += \
    common-helpresources.qrc \
    common-resources.qrc

include(about/about.pri)
include(converters/converters.pri)
include(cpu/cpu.pri)
include(io/io.pri)
include(memory/memory.pri)
include(pep/pep.pri)
include(style/style.pri)
include(symbol/symbol.pri)
include(update/update.pri)


