#Create a directory for update information
!exists($$repoDir){
    QMAKE_POST_LINK += $${QMAKE_MKDIR} $$cleanPathQuote($$repoDir) $$psc
}

#Execute repository creator
QMAKE_POST_LINK += $$cleanPathQuote($$QtInstallerBin/repogen) --update-new-components -p $$cleanPathQuote($$OUT_PWD/Installer/packages) $$repoDir $$psc
