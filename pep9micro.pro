# -------------------------------------------------
# Project created by QtCreator 2009-12-01T13:18:25
# -------------------------------------------------
TEMPLATE = app
TARGET = Pep9Micro
#Prevent Windows from trying to parse the project three times per build.
CONFIG -= debug_and_release \
    debug_and_release_target

QT += webenginewidgets
QT += widgets
QT += printsupport
QT += concurrent

# Mac icon/plist
ICON = images/icon.icns
QMAKE_INFO_PLIST = app.plist
QMAKE_MAC_SDK = macosx10.13

#Windows RC file for icon:
RC_FILE = pep9resources.rc

SOURCES += main.cpp \
    mainwindow.cpp \
    byteconverterhex.cpp \
    byteconverterdec.cpp \
    byteconverterchar.cpp \
    byteconverterbin.cpp \
    mainmemory.cpp \
    pep.cpp \
    microcodepane.cpp \
    cpupane.cpp \
    helpdialog.cpp \
    pephighlighter.cpp \
    microcodeeditor.cpp \
    memoryitemdelegate.cpp \
    objectcodepane.cpp \
    code.cpp \
    asm.cpp \
    tristatelabel.cpp \
    specification.cpp \
    byteconverterinstr.cpp \
    aboutpep.cpp \
    cpugraphicsitems.cpp \
    updatechecker.cpp \
    microcodeprogram.cpp \
    rotatedheaderview.cpp \
    disableselectionmodel.cpp \
    colors.cpp \
    cpudatasection.cpp \
    cpucontrolsection.cpp \
    addrmodedialog.cpp \
    addrmodewidget.cpp \
    symbolentry.cpp \
    symboltable.cpp \
    symbolvalue.cpp \
    iowidget.cpp \
    inputpane.cpp \
    outputpane.cpp \
    terminalpane.cpp \
    iodialog.cpp \
    memorysection.cpp \
    cpumemoizer.cpp
HEADERS += mainwindow.h \
    byteconverterhex.h \
    byteconverterdec.h \
    byteconverterchar.h \
    byteconverterbin.h \
    mainmemory.h \
    pep.h \
    microcodepane.h \
    cpupane.h \
    helpdialog.h \
    pephighlighter.h \
    enu.h \
    microcodeeditor.h \
    memoryitemdelegate.h \
    objectcodepane.h \
    code.h \
    asm.h \
    tristatelabel.h \
    specification.h \
    byteconverterinstr.h \
    aboutpep.h \
    cpugraphicsitems.h \
    shapes_one_byte_data_bus.h \
    shapes_two_byte_data_bus.h \
    updatechecker.h \
    microcodeprogram.h \
    rotatedheaderview.h \
    disableselectionmodel.h \
    colors.h \
    cpudatasection.h \
    cpucontrolsection.h \
    addrmodedialog.h \
    addrmodewidget.h \
    symbolentry.h \
    symboltable.h \
    symbolvalue.h \
    iowidget.h \
    inputpane.h \
    outputpane.h \
    terminalpane.h \
    iodialog.h \
    memorysection.h \
    cpumemoizer.h
FORMS += mainwindow.ui \
    byteconverterhex.ui \
    byteconverterdec.ui \
    byteconverterchar.ui \
    byteconverterbin.ui \
    mainmemory.ui \
    microcodepane.ui \
    cpupane.ui \
    helpdialog.ui \
    objectcodepane.ui \
    byteconverterinstr.ui \
    aboutpep.ui \
    addrmodedialog.ui \
    addrmodewidget.ui \
    iowidget.ui \
    inputpane.ui \
    outputpane.ui \
    terminalpane.ui \
    iodialog.ui
OTHER_FILES += help/images/registeraddresssignals.png \
    help/figures/exer1204.pepcpu \
    help/figures/exer1206.pepcpu \
    images/Pep9cpu-icon.png \
    images/Pep9cpu-icon-about.png \
    help/usingpep9cpu.html \
    help/pep9reference.html \
    help/pep9os.txt \
    help/microcode.html \
    help/exercises.html \
    help/examples.html \
    help/debugging.html \
    help/cpu.html
RESOURCES += pep9cpuresources.qrc \
    helpresources.qrc \
    dark_style.qrc

DISTFILES += \
    package/package.xml \
    packages/package.xml \
    packages/pep9cpu/meta/package.xml \
    packages/pep9cpu/package.xml \
    packages/pep9cpu/License.txt \
    packages/pep9cpu/control.qs \
    config/configwin32.xml \
    config/configlinux.xml \
    config/control.js \
    packages/pep9cpu/installscript.js \
    rc/License.md \
    ProjectDefs.pri \
    help/osunalignedsymbols.txt \
    help/osunalignedsymbols.txt

#Add this include to the bottom of your project to enable automated installer creation
#Include the definitions file that sets all variables needed for the InstallerConfig Script
include("ProjectDefs.pri")

#Lastly, include and run the installer config script
include("Installer/InstallerConfig.pri")


