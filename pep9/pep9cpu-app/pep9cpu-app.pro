# -------------------------------------------------
# Project created by Matthew McRaven, 12/30/2018
# -------------------------------------------------
TEMPLATE = app
TARGET = Pep9CPU
QT += widgets webenginewidgets printsupport concurrent
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
    ../../pep-core/common/common-helpresources.qrc \
    ../../pep-core/common/common-resources.qrc \
    ../pep9cpu-lib/pep9cpu-helpresources.qrc \
    ../pep9cpu-lib/pep9cpu-resources.qrc

FORMS += \
    cpuhelpdialog.ui \
    cpumainwindow.ui

HEADERS += \
    cpuhelpdialog.h \
    cpumainwindow.h

SOURCES += \
    cpuhelpdialog.cpp \
    cpumain.cpp \
    cpumainwindow.cpp

#Windows RC file for icon:
RC_FILE = pep9resources.rc
# Mac icon/plist
ICON = ../pep9cpu-lib/images/icon.icns
QMAKE_INFO_PLIST = app.plist

#Add this include to the bottom of your project to enable automated installer creation
#Include the definitions file that sets all variables needed for the InstallerConfig Script
include("installer-config.pri")

#Include and run the installer config script
include("../../installer/installer-creator.pri")

# Link against special libraries needed for dark mode.
include(../../mac-objc-fix.pri)

# Link against Pep core code
unix|win32: LIBS += -L$$OUT_PWD/../../pep-core/common/ -lpep-core-common

INCLUDEPATH += $$PWD/../../pep-core/common
DEPENDPATH += $$PWD/../../pep-core/common

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../pep-core/common/pep-core-common.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../pep-core/common/libpep-core-common.a

unix|win32: LIBS += -L$$OUT_PWD/../../pep-core/cpu/ -lpep-core-cpu

INCLUDEPATH += $$PWD/../../pep-core/cpu
DEPENDPATH += $$PWD/../../pep-core/cpu

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../pep-core/cpu/pep-core-cpu.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../../pep-core/cpu/libpep-core-cpu.a

# Link against shared code for Pep9CPU.

unix|win32: LIBS += -L$$OUT_PWD/../pep9def-lib/ -lpep9def-lib

INCLUDEPATH += $$PWD/../pep9def-lib
DEPENDPATH += $$PWD/../pep9def-lib

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../pep9def-lib/pep9def-lib.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../pep9def-lib/libpep9def-lib.a

unix|win32: LIBS += -L$$OUT_PWD/../pep9cpu-lib/ -lpep9cpu-lib

INCLUDEPATH += $$PWD/../pep9cpu-lib
DEPENDPATH += $$PWD/../pep9cpu-lib

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../pep9cpu-lib/pep9cpu-lib.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../pep9cpu-lib/libpep9cpu-lib.a

