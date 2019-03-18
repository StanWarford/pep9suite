#To change the output file name, or adjust what files are included in the ouput.

#Clean path
defineReplace(cpl){
    #Adjust the input path so that the correct slashes are used for the host shell $$psc OS
    return($$system_path($$1))
}

#Clean path with force quote
defineReplace(cpq){
    return(\"$$cpl($$1)\")
}

#Platform Specific Command Seperator
win32{
    psc="&"
    TARGET_EXT=".exe"
}
macx|linux{
    psc=";"
    TARGET_EXT=".app"
}

#Prevent Windows from trying to parse the project three times per build.
#This interferes with the deployment script, and makes debugging hard since Qt attempts to debug the optimized program.
CONFIG -= debug_and_release \
    debug_and_release_target

QtDir = $$clean_path($$[QT_INSTALL_PREFIX])
QtInstallerBin=$$clean_path($$QtDir/../../tools/Qtinstallerframework/3.0/bin)
EXECUTE_QTIFW=""
repoDir=""
PLATFORM_ICONS=""
PLATFORM_DATA=""
INSTALLER_CONFIG_FILE=""
DO_REPOGEN=""
DO_INSTALLER=""
DO_PACKAGE_COPY=""
contains(BUILD_OPT, repogen):CONFIG(release) {
    DO_REPOGEN = "true"
    DO_PACKAGE_COPY = "true"

}
contains(BUILD_OPT, installer):CONFIG(release) {
    DO_INSTALLER = "true"
    DO_PACKAGE_COPY = "true"
}
# If supposed to generate update repos & there is no repogen tool available, cause an error.
!isEmpty(DO_REPOGEN): !exists($$QtInstallerBin/repogen.exe):!exists($$QtInstallerBin/repogen) {
    warning("Aborting repogen creation, since QT Installer Framework 3.0 is not installed.")
    warning("Please run the QT maintence tool and install QT Installer Framework 3.0.")
}

# Set parameters for repo gen tools.
else:!isEmpty(DO_REPOGEN) {
    win32 {
        repoDir = $$cpq($$OUT_PWD/Repository/win32)
        INSTALLER_CONFIG_FILE=$$cpq($$PWD/../installer/packages/$$TARGET/configwin32.xml)
    }
    else:macx {
        repoDir = $$cpq($$OUT_PWD/Repository/macx)
        INSTALLER_CONFIG_FILE=$$cpq($$PWD/../installer/packages/$$TARGET/configmacx.xml)
    }
    else:linux {
        repoDir = $$cpq($$OUT_PWD/Repository/linux)
        INSTALLER_CONFIG_FILE=$$cpq($$PWD/../installer/packages/$$TARGET/configlinux.xml)
    }
    !isEmpty(DO_PACKAGE_COPY) {
        include("qtifw-packager.pri")
        unset(DO_PACKAGE_COPY)
    }
    include("qtifw-repogen.pri")
}

# Start configuration for installers

# If supposed to generate installer & there is no build tool available, cause an error.
!isEmpty(DO_INSTALLER): !exists($$QtInstallerBin/repogen.exe):!exists($$QtInstallerBin/repogen) {
    warning("Aborting installer creation, since QT Installer Framework 3.0 is not installed.")
    warning("Please run the QT maintence tool and install QT Installer Framework 3.0.")
}

# If configured, create a dmg-style installer for Mac OS.
else:macx:!isEmpty(MAC_USE_DMG_INSTALLER):!isEmpty(DO_INSTALLER) {
    include("dmg-installer.pri")
}

# Otherwise, create a typical QT-IFW installer.
else:macx:isEmpty(MAC_USE_DMG_INSTALLER):!isEmpty(DO_INSTALLER) {
    EXECUTE_QTIFW="true"
    PLATFORM_DATA=MAC_DATA
    PLATFORM_ICONS=MAC_ICONS
    DEPLOY_ARGS = ""
    !isEmpty(DO_PACKAGE_COPY) {
        include("qtifw-packager.pri")
        unset(DO_PACKAGE_COPY)
    }
    include("qtifw-installer.pri")
}
#Otherwise build the installer for windows as normal.
else:win32:!isEmpty(DO_INSTALLER){
    #Directory where the repogen tool will put its output
    EXECUTE_QTIFW="true"
    PLATFORM_DATA = WINDOWS_DATA
    PLATFORM_ICONS = WINDOWS_ICONS
    DEPLOY_ARGS = "--no-translations --no-system-d3d-compiler"
    !isEmpty(DO_PACKAGE_COPY) {
        include("qtifw-packager.pri")
        unset(DO_PACKAGE_COPY)
    }
    include("qtifw-installer.pri")
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

DISTFILES += \
    $$PWD/config/control.js \
    $$PWD/config/configlinux.xml \
    $$PWD/config/configwin32.xml \
    $$PWD/config/configmacx.xml \
    $$PWD/../installer/common/control.js \
    $$PWD/../installer/common/License.txt \
    $$PWD/../installer/common/regSetUninst.bat

FORMS += \
    $$PWD/../installer/common/ShortcutPage.ui \
    $$PWD/../installer/common/UserPage.ui
