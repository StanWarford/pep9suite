# -------------------------------------------------
# Project created by Matthew McRaven, 12/30/2018
# -------------------------------------------------
TEMPLATE = app
TARGET = Pep9
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
    asmmainwindow.ui \
    asmhelpdialog.ui \

HEADERS += \
    asmmainwindow.h \
    asmhelpdialog.h

SOURCES += \
    asmmainwindow.cpp \
    asmhelpdialog.cpp \
    main.cpp

RESOURCES += \
    ../pep9asm/pep9asm-helpresources.qrc \
    ../pep9asm/pep9asm-resources.qrc \
    ../pep9common/pep9common-resources.qrc \
    ../pep9common/pep9common-helpresources.qrc

INCLUDEPATH += $$PWD/../pep9common

#Include own directory in VPATH, otherwise qmake might accidentally import files with
#the same name from other directories.
VPATH += $$PWD
VPATH += $$PWD/../pep9common
include(../pep9common/pep9common.pro)
include(../pep9asm/pep9asm-common.pro)

#Add this include to the bottom of your project to enable automated installer creation
#Include the definitions file that sets all variables needed for the InstallerConfig Script
include("installer-config.pri")

#Lastly, include and run the installer config script
include("../installer/installer-creator.pri")
