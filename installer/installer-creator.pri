#Should debug messages be printed by the installer?
#dbgMsg = "true"

# Library of functions & definitions needed
include(installer_lib.pri)

# Pre-declare any condition variables and set them all to sensible defaults
# This prevents
DO_REPOGEN = false
DO_PACKAGE_COPY = false
LINUX_USE_APPIMAGE = false
MAC_USE_DMG = false
ALL_USE_QTIFW_INSTALLER = false

######################
# Begin installer code
######################
# Should online updater repositories be generated?
contains(DEPLOY_OPT, repogen): CONFIG(release) {
    debugMessage("Initializing repogen.")
    !detectRepogen($$QtInstallerBin) {
        error("Aborting due to failure to find repogen executable.")
    }
    DO_REPOGEN = true
    DO_PACKAGE_COPY = true

}


# Detect if multiple installer types were picked.
# If multiples were selected, warn user and default to qtifw_installer
options = $$find(DEPLOY_OPT, "appimg_installer") $$find(DEPLOY_OPT, "dmg_installer") $$find(DEPLOY_OPT, "qtifw_installer")
!count(options, 0) : !count(options, 1)  {
    warning("Too many installer types specified, defaulting to qtifw_installer.")
    INSTALLER_OPT -= "appimg_installer"
    INSTALLER_OPT -= "dmg_installer"
    INSTALLER_OPT += "qtifw_installer"
}

# If an app image output type was selected.
contains(DEPLOY_OPT, appimg_installer): CONFIG(release) {
    debugMessage("Selecting appimg_installer installer.")
    # AppImages only work on linux, so error if a non-linux system is trying to use them.
    !linux {
        error("appimg_installer is not a valid option on platforms other than linux.")
    }
    # If the Qt deployment tools can't be detected, abort.
    !detectDeploy($$QtInstallerBin) {
        error("Aborting due to failure to find linuxdeployqt executable.")
    }
    LINUX_USE_APPIMAGE = true
    ALL_USE_QTIFW_INSTALLER = false
    DO_PACKAGE_COPY = true
}
# If a dmg image output type was selected.
else: contains(DEPLOY_OPT, dmg_installer): CONFIG(release) {
    debugMessage("Selecting dmg_installer installer.")
    # Installing from a DMG is only available on Mac OS.
    !macx{
        error("dmg_installer is not a valid option on platforms other than Mac OS.")
    }
    # If the Qt deployment tools can't be detected, abort.
    !detectDeploy($$QtInstallerBin) {
        error("Aborting due to failure to find macdeployqt executable.")
    }
    MAC_USE_DMG = true
    ALL_USE_QTIFW_INSTALLER = false
    DO_PACKAGE_COPY = true
}
# If a standard QT-IFW installer was selected.
else: contains(DEPLOY_OPT, qtifw_installer): CONFIG(release) {
    debugMessage("Selecting qtifw_installer.")
    !detectDeploy($$QtInstallerBin) {
        error("Aborting due to failure to find $$platformDeployTool() executable.")
    }
    linux {
        error("Only appimg_installer deployment is supported on linux.")
    }
    ALL_USE_QTIFW_INSTALLER = true
    DO_PACKAGE_COPY = true
}

# If doing repository generation or creating a QT-IFW installer, a QT-IFW installer config is needed.
$$ALL_USE_QTIFW_INSTALLER | $$DO_REPOGEN {
    debugMessage("Setting installer config.")
    win32 {
        INSTALLER_CONFIG_FILE = $$cleanPathQuote($$INSTALLER_CONFIG_PATH/configwin32.xml)
    }
    else: macx {
        INSTALLER_CONFIG_FILE = $$cleanPathQuote($$INSTALLER_CONFIG_PATH/configmacx.xml)
    }
    else: linux {
        INSTALLER_CONFIG_FILE = $$cleanPathQuote($$INSTALLER_CONFIG_PATH/configlinux.xml)
    }
}
# Set parameters for repo gen tools.
$$DO_REPOGEN {
    debugMessage("Performing repository generation.")
    win32 {
        repoDir = $$cleanPathQuote($$OUT_PWD/Repository/win32)
    }
    else: macx {
        repoDir = $$cleanPathQuote($$OUT_PWD/Repository/macx)
    }
    else: linux {
        error("Repository generation is not supported on Linux.")
        #repoDir = $$cleanPathQuote($$OUT_PWD/Repository/linux)
    }
    DEPLOY_ARGS = $$extraLibArgs(EXTRA_LIBS)
    $$DO_PACKAGE_COPY {
        include(qtifw-packager.pri)
        DO_PACKAGE_COPY = false
    }
    include(qtifw-repogen.pri)
}

# Start configuration for installers

# If configured, create a dmg-style installer for Mac OS.
macx: $$MAC_USE_DMG {
    debugMessage("Creating Mac OS DMG installer.")
    # Additional libraries like XML will not be picked up automatically.
    # Therefore, tell the deploy tool about all linked Qt libraries.
    DEPLOY_ARGS = $$extraLibArgs(QT)
    include(dmg-installer.pri)
}

# Otherwise, create a typical QT-IFW installer for Mac OS.
else: macx: !$$MAC_USE_DMG: $$ALL_USE_QTIFW_INSTALLER {
    debugMessage("Creating Mac OS QTIFW installer.")
    PLATFORM_DATA = MAC_DATA
    PLATFORM_ICONS = MAC_ICONS
    # Additional libraries like XML will not be picked up automatically.
    # Therefore, tell the deploy tool about all linked Qt libraries.
    DEPLOY_ARGS = $$extraLibArgs(QT)
    $$DO_PACKAGE_COPY {
        include(qtifw-packager.pri)
        DO_PACKAGE_COPY = false
    }
    include(qtifw-installer.pri)
}

# Build a QT-IFW installer for Windows as normal.
else: win32: $$ALL_USE_QTIFW_INSTALLER {
    debugMessage("Creating Windows QTIFW installer.")
    PLATFORM_DATA = WINDOWS_DATA
    PLATFORM_ICONS = WINDOWS_ICONS
    # Translations & d3d compiler are unused by our application, but
    # use a significant amount of space, so we do not include them.
    # Additional libraries like XML will not be picked up automatically.
    # Therefore, tell the deploy tool about all linked Qt libraries.
    DEPLOY_ARGS = "--no-translations --no-system-d3d-compiler "$$extraLibArgs(QT)
    $$DO_PACKAGE_COPY {
        include(qtifw-packager.pri)
        DO_PACKAGE_COPY = false
    }
    include(qtifw-installer.pri)
}

# If configured, use linuxdeployqt & AppImage to construct a working Linux application.
else: linux: $$LINUX_USE_APPIMAGE {
    debugMessage("Creating Linux AppImage installer.")
    # No need to set DEPLOY_ARGS, as
    include(appimage-installer.pri)
}
# Default QT-IFW installer is not supported at this time.
else: linux: !$$LINUX_USE_APPIMAGE: $$ALL_USE_QTIFW_INSTALLER {
    error("Linux only supports appimg_installers.")
}

DISTFILES += \
    $$PWD/config/control.js \
    $$PWD/config/configlinux.xml \
    $$PWD/config/configwin32.xml \
    $$PWD/config/configmacx.xml \
    $$PWD/../installer/common/control.js \
    $$PWD/../installer/common/License.txt \
    $$PWD/../installer/common/regSetUninst.bat \
    $$PWD/options

FORMS += \
    $$PWD/../installer/common/ShortcutPage.ui \
    $$PWD/../installer/common/UserPage.ui
