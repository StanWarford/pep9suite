#dbgMsg = "true"
include(installer_lib.pri)

# Pre-declare any condition variables and set them all to sensible defaults
DO_REPOGEN = false
DO_PACKAGE_COPY = false
LINUX_USE_APPIMAGE = false
MAC_USE_DMG = false
DO_INSTALLER = false

# Begin installer code
contains(DEPLOY_OPT, repogen):CONFIG(release) {
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
    !linux{
        error("appimg_installer is not a valid option on platforms other than linux.")
    }
    !detectDeploy($$QtInstallerBin) {
        error("Aborting due to failure to find linuxdeployqt executable.")
    }
    LINUX_USE_APPIMAGE = true
    DO_INSTALLER = true
    DO_PACKAGE_COPY = true
}
# If a dmg image output type was selected.
else: contains(DEPLOY_OPT, dmg_installer): CONFIG(release) {
    debugMessage("Selecting dmg_installer installer.")
    !macx{
        error("dmg_installer is not a valid option on platforms other than linux.")
    }
    !detectDeploy($$QtInstallerBin) {
        error("Aborting due to failure to find macdeployqt executable.")
    }
    MAC_USE_DMG = true
    DO_INSTALLER = true
    DO_PACKAGE_COPY = true
}
# If a standard QTIFW installer was selected.
else: contains(DEPLOY_OPT, qtifw_installer): CONFIG(release) {
    debugMessage("Selecting qtifw_installer.")
    !detectDeploy($$QtInstallerBin) {
        error("Aborting due to failure to find $$platformDeployTool() executable.")
    }
    linux {
        error("Only appimg_installer deployment is supported on linux.")
    }
    DO_INSTALLER = true
    DO_PACKAGE_COPY = true
}

# If doing repository generation or creating an installer, a QTIFW installer config is needed.
$$DO_INSTALLER | $$DO_REPOGEN {
    debugMessage("Setting installer config")
    win32 {
        INSTALLER_CONFIG_FILE=$$cleanPathQuote($$INSTALLER_CONFIG_PATH/configwin32.xml)
    }
    else:macx {
        INSTALLER_CONFIG_FILE=$$cleanPathQuote($$INSTALLER_CONFIG_PATH/configmacx.xml)
    }
    else:linux {
        INSTALLER_CONFIG_FILE=$$cleanPathQuote($$INSTALLER_CONFIG_PATH/configlinux.xml)
    }
}
# Set parameters for repo gen tools.
$$DO_REPOGEN {
    debugMessage("Performing repository generation.")
    win32 {
        repoDir = $$cleanPathQuote($$OUT_PWD/Repository/win32)
    }
    else:macx {
        repoDir = $$cleanPathQuote($$OUT_PWD/Repository/macx)
    }
    else:linux {
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
macx: $$MAC_USE_DMG: $$DO_INSTALLER {
    debugMessage("Creating Mac OS DMG installer.")
    DEPLOY_ARGS = $$extraLibArgs(EXTRA_LIBS)
    include(dmg-installer.pri)
}

# Otherwise, create a typical QT-IFW installer.
else: macx: !$$MAC_USE_DMG: $$DO_INSTALLER {
    debugMessage("Creating Mac OS QTIFW installer.")
    PLATFORM_DATA = MAC_DATA
    PLATFORM_ICONS = MAC_ICONS
    DEPLOY_ARGS = $$extraLibArgs(extraLibs)
    $$DO_PACKAGE_COPY {
        include(qtifw-packager.pri)
        DO_PACKAGE_COPY = false
    }
    include(qtifw-installer.pri)
}

#Otherwise build the installer for windows as normal.
else: win32: $$DO_INSTALLER {
    debugMessage("Creating Windows QTIFW installer.")
    PLATFORM_DATA = WINDOWS_DATA
    PLATFORM_ICONS = WINDOWS_ICONS
    DEPLOY_ARGS = "--no-translations --no-system-d3d-compiler "$$extraLibArgs(EXTRA_LIBS)
    $$DO_PACKAGE_COPY {
        include(qtifw-packager.pri)
        DO_PACKAGE_COPY = false
    }
    include(qtifw-installer.pri)
}

#Then linuxdeployqt is available, and it should be used to make a working installer for linux.
else: linux: $$DO_INSTALLER: $$LINUX_USE_APPIMAGE {
    message("Creating Linux QTIFW AppImage installer.")
    include(appimage-installer.pri)
}
else: linux: $$DO_INSTALLER:: !$$LINUX_USE_APPIMAGE {
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
