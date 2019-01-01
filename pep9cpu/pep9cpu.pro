# -------------------------------------------------
# Project created by Matthew McRaven, 12/30/2018
# -------------------------------------------------
TEMPLATE = app
TARGET = Pep9CPU
#Prevent Windows from trying to parse the project three times per build.
CONFIG -= debug_and_release \
    debug_and_release_target
#Flag for enabling C++17 features.
#Due to support for C++17 features being added before the standard was finalized, and the placeholder text of "C++1z" has remained
CONFIG += c++1z
win32{
    #MSVC doesn't recognize c++1z flag, so use the MSVC specific flag here
    win32-msvc*: QMAKE_CXXFLAGS += /std:c++17
    #Flags needed to generate PDB information in release. Necessary information to profile program.
    #Flags also picked to provide a ~15% speed increase in release mode (at the cost of increased compile times).
    QMAKE_LFLAGS_RELEASE +=/MAP
    QMAKE_CFLAGS_RELEASE -= O2
    QMAKE_CFLAGS_RELEASE += /O3 /MD /zi
    QMAKE_LFLAGS_RELEASE +=/debug /opt:ref
}
QT += widgets webenginewidgets printsupport concurrent

VPATH += $$PWD/../pep9common
INCLUDEPATH += $$PWD/../pep9common

FORMS += \
    cpumainwindow.ui \
#    helpdialog.ui \

HEADERS += \
    cpumainwindow.h \
#   helpdialog.h \
    partialmicrocodedcpu.h \
    partialmicrocodedmemoizer.h

SOURCES += \
    cpumainwindow.cpp \
#   helpdialog.cpp \
    main.cpp \
    partialmicrocodedcpu.cpp \
    partialmicrocodedmemoizer.cpp

include(../pep9common/pep9common.pro)
include(pep9cpu-common.pro)
