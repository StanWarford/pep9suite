#Now that all confiugration flags have been set, execute

QMAKE_POST_LINK += $$cpq($$QtInstallerBin/binarycreator) -c $$cpq($$OUT_PWD/Installer/config/config.xml) -p $$cpq($$OUT_PWD/Installer/packages) \
$$cpq($$OUT_PWD/Installer/$$OUTPUT_INSTALLER_NAME) $$psc

