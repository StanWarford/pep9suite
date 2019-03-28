#Create installer directory structure
#These will be ignored if the target already exists
QMAKE_POST_LINK += $${QMAKE_MKDIR} $$cpq($$OUT_PWD/Installer) $$psc \
    $${QMAKE_MKDIR} $$cpq($$OUT_PWD/Installer/packages) $$psc \
    $${QMAKE_MKDIR} $$cpq($$OUT_PWD/Installer/packages/$$TARGET) $$psc \
    $${QMAKE_MKDIR} $$cpq($$OUT_PWD/Installer/packages/$$TARGET/meta) $$psc \
    $${QMAKE_MKDIR} $$cpq($$OUT_PWD/Installer/packages/$$TARGET/data) $$psc \
    $${QMAKE_MKDIR} $$cpq($$OUT_PWD/Installer/config) $$psc
#Copy over files needed to create installer
QMAKE_POST_LINK += $${QMAKE_COPY} $$INSTALLER_CONFIG_FILE $$cpq($$OUT_PWD/Installer/config/config.xml) $$psc \ #Copy Platform dependant config file
    $${QMAKE_COPY} $$cpq($$PWD/../installer/common/control.js) $$cpq($$OUT_PWD/Installer/config/) $$psc #Copy over installer control script

for(PACKAGE, TARGET_PACKAGES) {
    #For each target package, copy it over into the installer
    NAME = $$eval($$PACKAGE"."NAME)
    meta_items = $$eval($$PACKAGE"."META_ITEMS)
    data_items = $$eval($$PACKAGE"."DATA_ITEMS)
    for(ITEM, meta_items) {
        QMAKE_POST_LINK += $${QMAKE_COPY} $$cpq($$ITEM) $$cpq($$OUT_PWD/Installer/packages/$$NAME/meta/) $$psc
    }
    for(ITEM, data_items) {
        QMAKE_POST_LINK += $${QMAKE_COPY} $$cpq($$ITEM) $$cpq($$OUT_PWD/Installer/packages/$$NAME/data/) $$psc
    }
}


#Copy over needed icons as set in defs file
for(name, UNIVERSAL_ICONS){
    QMAKE_POST_LINK += $${QMAKE_COPY} $$cpq($$PATH_PREFIX/$$name) $$cpq($$OUT_PWD/Installer/config) $$psc
}

win32:{
    for(name, WINDOWS_ICONS){
        QMAKE_POST_LINK += $${QMAKE_COPY} $$cpq($$PATH_PREFIX/$$name) $$cpq($$OUT_PWD/Installer/config) $$psc
    }
    #Copy over script that installs application with registry.
    QMAKE_POST_LINK +=  $${QMAKE_COPY} $$cpq($$PWD/../installer/common/regSetUninst.bat) $$cpq($$OUT_PWD/Installer/packages/$$TARGET/data) $$psc
    #Copy over executable to data directory
    QMAKE_POST_LINK +=  $${QMAKE_COPY} $$cpq($$OUT_PWD/$$TARGET$$TARGET_EXT) $$cpq($$OUT_PWD/Installer/packages/$$TARGET/data) $$psc
    #Execute windeployqt to copy needed binaries (dlls, etc).
    #See documentation here:
    #http://doc.qt.io/qt-5/windows-deployment.html
    QMAKE_POST_LINK += $$cpq($$QtDir/bin/windeployqt) $$DEPLOY_ARGS $$cpq($$OUT_PWD/Installer/packages/$$TARGET/data/$$TARGET$$TARGET_EXT) $$psc
}

else:macx{
    for(name, MAC_ICONS){
        QMAKE_POST_LINK += $${QMAKE_COPY} $$cpq($$PATH_PREFIX/$$name) $$cpq($$OUT_PWD/Installer/config) $$psc
    }
    #Copy over executable to data directory
    QMAKE_POST_LINK +=  $${QMAKE_COPY_DIR} $$cpq($$OUT_PWD/$$TARGET$$TARGET_EXT) $$cpq($$OUT_PWD/Installer/packages/$$TARGET/data) $$psc
    #Execute macdeployqt to copy needed binaries (dlls, etc).
    #See documentation here:
    #http://doc.qt.io/qt-5/windows-deployment.html
    CONFIG(app_bundle){
        QMAKE_POST_LINK += $$cpq($$QtDir/bin/macdeployqt) $$DEPLOY_ARGS $$cpq($$OUT_PWD/Installer/packages/$$TARGET/data/$$TARGET$$TARGET_EXT) $$psc
    }
}

