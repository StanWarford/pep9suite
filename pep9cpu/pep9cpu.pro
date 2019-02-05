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

# Mac icon/plist
ICON = images/icon.icns
QMAKE_INFO_PLIST = app.plist
QMAKE_MAC_SDK = macosx10.14

#Windows RC file for icon:
RC_FILE = pep9resources.rc

FORMS += \
    cpuhelpdialog.ui \
    cpumainwindow.ui \

HEADERS += \
    cpuhelpdialog.h \
    cpumainwindow.h \
    partialmicrocodedcpu.h \
    partialmicrocodedmemoizer.h

SOURCES += \
    cpuhelpdialog.cpp \
    cpumainwindow.cpp \
    main.cpp \
    partialmicrocodedcpu.cpp \
    partialmicrocodedmemoizer.cpp

RESOURCES += \
    cpu_helpresources.qrc \
    pep9cpu-resources.qrc \
    ../pep9common/dark_style.qrc \
    ../pep9common/pep9common-resources.qrc

DISTFILES += \
    pep9resources.rc

INCLUDEPATH += $$PWD/../pep9common
VPATH += $$PWD
VPATH += $$PWD/../pep9common

include(../pep9common/pep9common.pro)
include(pep9cpu-common.pro)
