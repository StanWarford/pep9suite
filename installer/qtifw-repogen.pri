#Create a directory for update information
!exists($$repoDir){
    QMAKE_POST_LINK += $${QMAKE_MKDIR} $$cpq($$repoDir) $$psc
}

#Execute repository creator
QMAKE_POST_LINK += $$cpq($$QtInstallerBin/repogen) --update-new-components -p $$cpq($$OUT_PWD/Installer/packages) $$repoDir $$psc
