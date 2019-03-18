#Create necessary directory structure for disk image.
QMAKE_POST_LINK += $${QMAKE_MKDIR} $$cpq($$OUT_PWD/Installer/dmg-installer)$$psc
#Copy over the executable and bundle it with its dependencies
QMAKE_POST_LINK += $${QMAKE_COPY_DIR} $$cpq($$OUT_PWD/$$TARGET".app") $$cpq($$OUT_PWD/Installer/dmg-installer)$$psc
QMAKE_POST_LINK += $$cpq($$QtDir/bin/macdeployqt) $$cpq($$OUT_PWD/Installer/dmg-installer/$$TARGET".app")$$psc
#Use HDIUtil to make a folder into a read/write image
QMAKE_POST_LINK += hdiutil create -volname $$TARGET -srcfolder $$cpq($$OUT_PWD/Installer/dmg-installer) -attach -ov -format UDRW $$OUT_PWD/Installer/$$TARGET"Temp.dmg"$$psc
#Link from the read/write image to the machine's Applications folder
QMAKE_POST_LINK += ln -s /Applications /Volumes/$$TARGET/Applications$$psc

#Write all data files to image
for(PACKAGE, TARGET_PACKAGES) {
    #For each target package, copy it over into the installer
    NAME = $$eval($$PACKAGE"."NAME)
    data_items = $$eval($$PACKAGE"."DATA_ITEMS)
    for(ITEM, data_items) {
        QMAKE_POST_LINK += $${QMAKE_COPY} $$cpq($$ITEM) $$cpq(/Volumes/$$TARGET/) $$psc
    }
}
#Unmount the image, and create a new compressed, readonly image.
QMAKE_POST_LINK += hdiutil detach /Volumes/$$TARGET$$psc
QMAKE_POST_LINK += $${QMAKE_COPY} $$cpq($$OUT_PWD/Installer/$$TARGET"Temp".dmg) $$cpq($$OUT_PWD/Installer/$$TARGET"Temp2".dmg)$$psc
QMAKE_POST_LINK += hdiutil convert -format UDBZ -o $$cpq($$OUT_PWD/Installer/$$OUTPUT_INSTALLER_NAME".dmg") $$cpq($$OUT_PWD/Installer/$$TARGET"Temp2".dmg)$$psc
#Remove the temporary read/write image.
QMAKE_POST_LINK += $${QMAKE_DEL_FILE} $$cpq($$OUT_PWD/Installer/$$TARGET"Temp".dmg)$$psc
QMAKE_POST_LINK += $${QMAKE_DEL_FILE} $$cpq($$OUT_PWD/Installer/$$TARGET"Temp2".dmg)$$psc
#If QMAKE_POST_LINK stops working in a future version, QMAKE provides another way to add custom targets.
#Use the method described in "Adding Custom Targets" on http://doc.qt.io/qt-5/qmake-advanced-usage.html.
#Our deployment tool will be called anytime the application is sucessfully linked in release mode.

