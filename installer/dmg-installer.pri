#Create necessary directory structure for disk image.
QMAKE_POST_LINK += $${QMAKE_MKDIR} $$cleanPathQuote($$OUT_PWD/Installer/dmg-installer) $$psc
#Copy over the executable and bundle it with its dependencies
QMAKE_POST_LINK += $${QMAKE_COPY_DIR} $$cleanPathQuote($$OUT_PWD/$$TARGET""$$TARGET_EXT) $$cleanPathQuote($$OUT_PWD/Installer/dmg-installer) $$psc
CONFIG(app_bundle) {
    QMAKE_POST_LINK += $$cleanPathQuote($$QtDir/bin/macdeployqt) $$cleanPathQuote($$OUT_PWD/Installer/dmg-installer/$$TARGET""$$TARGET_EXT) $$psc
    #Only sign application if a SIGN_KEY is set.
    !isEmpty(SIGN_KEY){
        QMAKE_POST_LINK += codesign -dv --verbose=4 --deep -s \"$$SIGN_KEY\" $$OUT_PWD/Installer/dmg-installer/$$TARGET""$$TARGET_EXT $$psc
    }
    else {
        message("Skipping code signing due to lack of key")
    }
}
#Use HDIUtil to make a folder into a read/write image
QMAKE_POST_LINK += hdiutil create -volname $$cleanPathQuote($$TARGET) -srcfolder $$cleanPathQuote($$OUT_PWD/Installer/dmg-installer) -attach -ov -format UDRW $$cleanPathQuote($$OUT_PWD/Installer/$$TARGET"Temp.dmg") $$psc
#Link from the read/write image to the machine's Applications folder
QMAKE_POST_LINK += ln -s /Applications $$cleanPathQuote(/Volumes/$$TARGET/Applications) $$psc
#For small files, there seems to be an issue where commands will execute out of order.
#A short pauses seems to prevent the bug of not copying over data items & making links.
QMAKE_POST_LINK += sleep 1 $$psc
#Write all data files to image
for(PACKAGE, TARGET_PACKAGES) {
    #For each target package, copy it over into the installer
    NAME = $$eval($$PACKAGE"."NAME)
    data_items = $$eval($$PACKAGE"."DATA_ITEMS)
    for(ITEM, data_items) {
        QMAKE_POST_LINK += $${QMAKE_COPY} $$cleanPathQuote($$ITEM) $$cleanPathQuote(/Volumes/$$TARGET/) $$psc
    }
}
#Needed longer pause before detaching, otherwise files not copied succesfully.
QMAKE_POST_LINK += sleep 3 $$psc
#Unmount the image, and create a new compressed, readonly image.
QMAKE_POST_LINK += hdiutil detach $$cleanPathQuote(/Volumes/$$TARGET) $$psc
QMAKE_POST_LINK += sleep 1 $$psc
QMAKE_POST_LINK += $${QMAKE_COPY} $$cleanPathQuote($$OUT_PWD/Installer/$$TARGET"Temp".dmg) $$cleanPathQuote($$OUT_PWD/Installer/$$TARGET"Temp2".dmg) $$psc
QMAKE_POST_LINK += hdiutil convert -format UDBZ -o $$cleanPathQuote($$OUT_PWD/Installer/$$OUTPUT_INSTALLER_NAME".dmg") $$cleanPathQuote($$OUT_PWD/Installer/$$TARGET"Temp2".dmg) $$psc
#Remove the temporary read/write image.
QMAKE_POST_LINK += sleep 1 $$psc
QMAKE_POST_LINK += $${QMAKE_DEL_FILE} $$cleanPathQuote($$OUT_PWD/Installer/$$TARGET"Temp".dmg) $$psc
QMAKE_POST_LINK += $${QMAKE_DEL_FILE} $$cleanPathQuote($$OUT_PWD/Installer/$$TARGET"Temp2".dmg) $$psc
#If QMAKE_POST_LINK stops working in a future version, QMAKE provides another way to add custom targets.
#Use the method described in "Adding Custom Targets" on http://doc.qt.io/qt-5/qmake-advanced-usage.html.
#Our deployment tool will be called anytime the application is sucessfully linked in release mode.

