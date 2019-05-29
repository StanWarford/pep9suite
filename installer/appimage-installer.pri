#Create necessary directory structure for disk image.
QMAKE_POST_LINK += \
    $${QMAKE_MKDIR} $$cleanPathQuote($$OUT_PWD/Installer) $$psc \
    $${QMAKE_MKDIR} $$cleanPathQuote($$OUT_PWD/Installer/usr) $$psc \
    $${QMAKE_MKDIR} $$cleanPathQuote($$OUT_PWD/Installer/usr/bin) $$psc \
    $${QMAKE_MKDIR} $$cleanPathQuote($$OUT_PWD/Installer/usr/lib) $$psc \
    $${QMAKE_MKDIR} $$cleanPathQuote($$OUT_PWD/Installer/usr/share) $$psc \
    $${QMAKE_MKDIR} $$cleanPathQuote($$OUT_PWD/Installer/usr/share) $$psc \
    $${QMAKE_MKDIR} $$cleanPathQuote($$OUT_PWD/Installer/usr/share/applications) $$psc \
    $${QMAKE_MKDIR} $$cleanPathQuote($$OUT_PWD/Installer/usr/share/icons) $$psc \
    $${QMAKE_MKDIR} $$cleanPathQuote($$OUT_PWD/Installer/usr/share/icons/$$LINUX_ICON_THEME) $$psc \
    $${QMAKE_MKDIR} $$cleanPathQuote($$OUT_PWD/Installer/usr/share/icons/$$LINUX_ICON_THEME/$$LINUX_ICON_SIZE) $$psc \
    $${QMAKE_MKDIR} $$cleanPathQuote($$OUT_PWD/Installer/usr/share/icons/$$LINUX_ICON_THEME/$$LINUX_ICON_SIZE/apps) $$psc \


#Copy over the executable and bundle it with its dependencies
message($$LINUX_DESKTOP_FILE_PATH/$$LINUX_DESKTOP_FILE_NAME)
QMAKE_POST_LINK += \
    $${QMAKE_COPY} $$cleanPathQuote($$OUT_PWD/$$TARGET""$$TARGET_EXT) $$cleanPathQuote($$OUT_PWD/Installer/usr/bin) $$psc \
    $${QMAKE_COPY} $$cleanPathQuote($$LINUX_ICON) $$cleanPathQuote($$OUT_PWD/Installer/usr/share/icons/$$LINUX_ICON_THEME/$$LINUX_ICON_SIZE/apps) $$psc \
    $${QMAKE_COPY} $$cleanPathQuote($$LINUX_DESKTOP_FILE_PATH/$$LINUX_DESKTOP_FILE_NAME) $$cleanPathQuote($$OUT_PWD/Installer/usr/share/applications) $$psc \

QMAKE_POST_LINK += $$cleanPathQuote($$QtDir/bin/linuxdeployqt) $$cleanPathQuote($$OUT_PWD/Installer/usr/share/applications/$$LINUX_DESKTOP_FILE_NAME) "-appimage" $$psc
