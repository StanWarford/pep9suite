# Create necessary directory structure for disk image.
# We adhere to the Filesystem Hierarchy Standard, so that AppImages run correctly.
$$LINUX_DEPLOY_FHS {
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
        $${QMAKE_MKDIR} $$cleanPathQuote($$OUT_PWD/Installer/usr/share/icons/$$LINUX_ICON_THEME/$$LINUX_ICON_SIZE/apps) $$psc



    debugMessage($$LINUX_DESKTOP_FILE_PATH/$$LINUX_DESKTOP_FILE_NAME)
    # Copy over the executable and bundle it with its dependencies
    # Then, copy over the icon and correct .desktop file.
    QMAKE_POST_LINK += \
        $${QMAKE_COPY} $$cleanPathQuote($$OUT_PWD/$$TARGET""$$TARGET_EXT) $$cleanPathQuote($$OUT_PWD/Installer/usr/bin) $$psc \
        $${QMAKE_COPY} $$cleanPathQuote($$LINUX_ICON) $$cleanPathQuote($$OUT_PWD/Installer/usr/share/icons/$$LINUX_ICON_THEME/$$LINUX_ICON_SIZE/apps) $$psc \
        $${QMAKE_COPY} $$cleanPathQuote($$LINUX_DESKTOP_FILE_PATH/$$LINUX_DESKTOP_FILE_NAME) $$cleanPathQuote($$OUT_PWD/Installer/usr/share/applications) $$psc
    # If any extra libraries are needed on linux, copy them over now.
    for(library, LINUX_EXTRA_COPY_LIBS) {
        QMAKE_POST_LINK += $${QMAKE_COPY} $$cleanPathQuote($$library) $$cleanPathQuote($$OUT_PWD/Installer/usr/lib) $$psc
    }
}
$$LINUX_USE_APPIMAGE {
    # Pass -unsupported-allow-new-glibc so that we can compile on newer versions of Linux.
    # Pass -extra-plugins=iconengines,platformthemes/libqgtk3.so so that we have proper icon support in Linux.
    # Pass -qmake=.. to correctly deploy Qt libraries.
    QMAKE_POST_LINK += $$cleanPathQuote($$QtDir/bin/linuxdeployqt) $$cleanPathQuote($$OUT_PWD/Installer/usr/share/applications/$$LINUX_DESKTOP_FILE_NAME) "-qmake=$$QtDir/qmake" "-appimage -unsupported-allow-new-glibc -extra-plugins=iconengines,platformthemes/libqgtk3.so" $$psc
}
