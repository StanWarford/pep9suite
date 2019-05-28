#Now that all confiugration flags have been set, execute
QMAKE_POST_LINK += $$cleanPathQuote($$QtInstallerBin/binarycreator) -c $$cleanPathQuote($$OUT_PWD/Installer/config/config.xml) \
    -p $$cleanPathQuote($$OUT_PWD/Installer/packages) \
    $$cleanPathQuote($$OUT_PWD/Installer/$$OUTPUT_INSTALLER_NAME) $$psc

