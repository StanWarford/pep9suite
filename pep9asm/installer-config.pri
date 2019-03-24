#Installer Configuration
#Pep9 Version
PEPVER=92
#Name of the installation tool
OUTPUT_INSTALLER_NAME=$$TARGET"-Installer-"$$PEPVER
#Internal version number, unused as of 5/2/2018
VERSION_NUMBER = "1_0"
#If you want to use a DMG based installer for Mac, put anything in this field
#If you want to use a QT Installer Framework Base Installer, set the field to ""
MAC_USE_DMG_INSTALLER = "true"

#Data Configuration
#Look for all files below starting in this directory
PATH_PREFIX = $$PWD
#Each of the following variables should be a space separated list
UNIVERSAL_ICONS = ""
WINDOWS_ICONS = "images/icon.ico" "images/Pep9-icon.png"
MAC_ICONS = "images/icon.icns" "images/Pep9-icon.png"
LINUX_ICONS = "" #No implementation for Linux

#One of your target packages must always be $$TARGET
MAIN_PACKAGE.NAME = $$TARGET
MAIN_PACKAGE.META_ITEMS += $$PWD/../installer/common/ShortcutPage.ui \
    $$PWD/../installer/common/UserPage.ui \
    $$PWD/../installer/common/License.txt \
    $$PWD/../installer/packages/$$TARGET/install.js \
    $$PWD/../installer/packages/$$TARGET/package.xml
MAIN_PACKAGE.DATA_ITEMS += "$$PWD/../LICENSE"

TARGET_PACKAGES += MAIN_PACKAGE

DISTFILES += \
    $$PWD/../installer/packages/pep9/install.js \
    $$PWD/../installer/packages/pep9/package.xml \
    $$PWD/../installer/packages/pep9/configlinux.xml \
    $$PWD/../installer/packages/pep9/configwin32.xml \
    $$PWD/../installer/packages/pep9/configmacx.xml
