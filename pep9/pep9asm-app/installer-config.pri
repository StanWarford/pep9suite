#Installer Configuration

#Pep9 Version
PEPVER=942
#Name of the installation tool
OUTPUT_INSTALLER_NAME=$$TARGET"-Installer-"$$PEPVER
#Internal version number, unused as of 5/2/2018
VERSION_NUMBER = "1_0"

#Data Configuration
#Look for all files below starting in this directory
PATH_PREFIX = $$PWD
#Each of the following variables should be a space separated list
UNIVERSAL_ICONS = ""
WINDOWS_ICONS = "../pep9asm-lib/images/icon.ico" "../pep9asm-lib/images/Pep9-icon.png"
MAC_ICONS = "../pep9asm-lib/images/icon.icns" "../pep9asm-lib/images/Pep9-icon.png"
LINUX_ICONS = "" #No implementation for Linux

# Path where the installer config files may be found
INSTALLER_CONFIG_PATH = $$PWD/../../installer/packages/pep9

#  Needed for appimage in linux
LINUX_ICON = $$PWD/../pep9asm-lib/images/Pep9-icon.png
LINUX_ICON_THEME = "hicolor"
LINUX_ICON_SIZE = "512x512"
LINUX_DESKTOP_FILE_PATH = $$PWD/../../installer/packages/pep9
LINUX_DESKTOP_FILE_NAME = "Pep9.desktop"

#One of your target packages must always be $$TARGET
MAIN_PACKAGE.NAME = $$TARGET
MAIN_PACKAGE.META_ITEMS += $$PWD/../../installer/common/ShortcutPage.ui \
    $$PWD/../../installer/common/UserPage.ui \
    $$PWD/../../installer/common/License.txt \
    $$PWD/../../installer/packages/$$TARGET/install.js \
    $$PWD/../../installer/packages/$$TARGET/package.xml
MAIN_PACKAGE.DATA_ITEMS += "$$PWD/../../LICENSE"

TARGET_PACKAGES += MAIN_PACKAGE

DISTFILES += \
    $$PWD/../../installer/packages/pep9/install.js \
    $$PWD/../../installer/packages/pep9/package.xml \
    $$PWD/../../installer/packages/pep9/configlinux.xml \
    $$PWD/../../installer/packages/pep9/configwin32.xml \
    $$PWD/../../installer/packages/pep9/configmacx.xml
