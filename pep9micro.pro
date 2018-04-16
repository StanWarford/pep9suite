# -------------------------------------------------
# Project created by QtCreator 2009-12-01T13:18:25
# -------------------------------------------------
TEMPLATE = app
TARGET = Pep9CPU
PEP9_VERSION = 92
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
    cpucontrolsection.cpp
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
    cpucontrolsection.h
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
    aboutpep.ui
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
    rc/License.md

#Generic paths that make future parts of the code easier
QtDir = $$clean_path($$[QT_INSTALL_LIBS]/..)
QtInstallerBin=$$clean_path($$QtDir/../../tools/Qtinstallerframework/3.0/bin)
OutputInstallerName=Pep9CPU"$$PEP9_VERSION"
#All that needs to be done for mac is to run the DMG creator.
#The DMG creator will only be run in Release mode, not debug.
!CONFIG(debug,debug|release):macx{
    #For some reason, the release flag is set in both debug and release.
    #So, the above Config(...) makes it so a disk image is only built in release mode.

    #Create necessary directory structure for disk image.
    QMAKE_POST_LINK += $${QMAKE_MKDIR} $$OUT_PWD/Installer;
    #Copy over the executable and bundle it with its dependencies
    QMAKE_POST_LINK += $${QMAKE_COPY_DIR} $$OUT_PWD/Pep9CPU.app $$OUT_PWD/Installer;
    QMAKE_POST_LINK += $$QtDir/bin/macdeployqt $$OUT_PWD/Installer/Pep9CPU.app;
    #Use HDIUtil to make a folder into a read/write image
    QMAKE_POST_LINK += hdiutil create -volname Pep9CPU -srcfolder $$OUT_PWD/Installer -attach -ov -format UDRW Pep9CPUTemp.dmg;
    #Link from the read/write image to the machine's Applications folder
    QMAKE_POST_LINK += ln -s /Applications /Volumes/Pep9CPU/Applications;
    #Unmount the image, and create a new compressed, readonly image.
    QMAKE_POST_LINK += hdiutil detach /Volumes/Pep9CPU;
    QMAKE_POST_LINK += $${QMAKE_COPY} $$OUT_PWD/Pep9CPUTemp.dmg $$OUT_PWD/Pep9CPUTemp2.dmg;
    QMAKE_POST_LINK += hdiutil convert -format UDBZ -o $$OUT_PWD/$$OutputInstallerName"Mac.dmg" $$OUT_PWD/Pep9CPUTemp2.dmg;
    #Remove the temporary read/write image.
    QMAKE_POST_LINK += $${QMAKE_DEL_FILE} $$OUT_PWD/Pep9CPUTemp.dmg;
    QMAKE_POST_LINK += $${QMAKE_DEL_FILE} $$OUT_PWD/Pep9CPUTemp2.dmg;
    #If QMAKE_POST_LINK stops working in a future version, QMAKE provides another way to add custom targets.
    #Use the method described in "Adding Custom Targets" on http://doc.qt.io/qt-5/qmake-advanced-usage.html.
    #Our deployment tool will be called anytime the application is sucessfully linked in release mode.
}

#Otherwise if the target is windows, but no installer framework exists
else:!CONFIG(debug,debug|release):win32:!exists($$QtInstallerBin/repogen.exe){
    warning("Aborting installer creations, since QT Installer Framework 3.0 is not installed.")
    warning("Please run the QT maintence tool and install QT Installer Framework 3.0.")
}
    #Otherwise build the installer for windows as normal.
else:!CONFIG(debug,debug|release):win32{
    repoDir=$$OUT_PWD/Repository/win32
    #Create installer directory structure
    QMAKE_POST_LINK += $${QMAKE_MKDIR} \"$$OUT_PWD/Installer\" & \
        $${QMAKE_MKDIR} \"$$OUT_PWD/Installer/packages\" & \
        $${QMAKE_MKDIR} \"$$OUT_PWD/Installer/packages/pep9cpu\" & \
        $${QMAKE_MKDIR} \"$$OUT_PWD/Installer/packages/pep9cpu/meta\" & \
        $${QMAKE_MKDIR} \"$$OUT_PWD/Installer/packages/pep9cpu/data\" & \
        $${QMAKE_MKDIR} \"$$OUT_PWD/Installer/config\" &
    #Create a directory for update information
    !exists($$repoDir){
        QMAKE_POST_LINK += $${QMAKE_MKDIR} \"$$repoDir\" &
    }
    #Copy over files needed to create installer
    QMAKE_POST_LINK += $${QMAKE_COPY} \"$$shell_path($$PWD\config\configwin32.xml)\" \"$$shell_path($$OUT_PWD/Installer/config/config.xml)\" & \
        $${QMAKE_COPY} \"$$shell_path($$PWD/images/icon.ico)\" \"$$shell_path($$OUT_PWD/Installer/config)\" & \
        $${QMAKE_COPY} \"$$shell_path($$PWD/images/Pep9cpu-icon.png)\" \"$$shell_path($$OUT_PWD/Installer/config)\" & \
        $${QMAKE_COPY} \"$$shell_path($$PWD/packages/pep9cpu/package.xml)\" \"$$shell_path($$OUT_PWD/Installer/packages/pep9cpu/meta)\" & \
        $${QMAKE_COPY} \"$$shell_path($$PWD/packages/pep9cpu/License.txt)\" \"$$shell_path($$OUT_PWD/Installer/packages/pep9cpu/meta)\" & \
        $${QMAKE_COPY} \"$$shell_path($$PWD/packages/pep9cpu/installscript.js)\" \"$$shell_path($$OUT_PWD/Installer/packages/pep9cpu/meta)\" & \
        $${QMAKE_COPY} \"$$shell_path($$PWD/config/control.js)\" \"$$shell_path($$OUT_PWD/Installer/config)\" &
    #Copy over executable
    QMAKE_POST_LINK +=  $${QMAKE_COPY} \"$$shell_path($$OUT_PWD/Pep9CPU.exe)\" \"$$shell_path($$OUT_PWD/Installer/packages/pep9cpu/data)\" &
    #Execute windeployqt to copy over needed binaries
    #Need to prune extra unneeded libraries, but the first goal is to get a working standalone program
    QMAKE_POST_LINK += \"$$QtDir/bin/windeployqt\" --no-translations --no-system-d3d-compiler \"$$OUT_PWD/Installer/packages/pep9cpu/data/Pep9CPU.exe\" &
    #Execute repository creator
    QMAKE_POST_LINK += \"$$QtInstallerBin/repogen\" --update-new-components -p $$OUT_PWD/Installer/packages $$repoDir &
    #Create installer
    QMAKE_POST_LINK += \"$$QtInstallerBin/binarycreator\" -c \"$$OUT_PWD/Installer/config/config.xml\" -p \"$$OUT_PWD/Installer/packages\" \
 \"$$OUT_PWD/Installer/$$OutputInstallerName"Win"\" &
}

#Since there is no native QT deploy tool for Linux, one must be added in the project configuration
#This condition is to make sure that a tool was provided as an argument to qmake
else:linux:isEmpty(LINUX_DEPLOY){
    warning("Attempting a Linux build, but no path to the build tool was provided")
}

#Then linuxdeployqt is available, and it should be used to make a working installer for linux.
else:linux{
    message("This is where the linux build code will go")
}


