# Defines copied in to including namespace
QtDir = $$clean_path($$[QT_INSTALL_PREFIX])
QtInstallerBin=$$clean_path($$QtDir/../../tools/Qtinstallerframework/3.0/bin)
# Deterime platform specific comman separator, target extensions for various platforms.
win32 {
    #Prevent Windows from trying to parse the project three times per build.
    #This interferes with the deployment script, and makes debugging hard since Qt attempts to debug the optimized program.
    CONFIG -= debug_and_release \
    debug_and_release_target
    psc="&"
    TARGET_EXT=".exe"
}
else:macx {
    psc=";"
    CONFIG(app_bundle){
        TARGET_EXT=".app"
    }
    else{
        TARGET_EXT=""
    }
}
else:linux {
    psc=";"
    TARGET_EXT=""
}

#Begin function declarations

#Clean path
defineReplace(cleanPath){
    #Adjust the input path so that the correct slashes are used for the host shell $$psc OS
    return($$system_path($$1))
}

#Clean path with force quote
defineReplace(cleanPathQuote){
    return(\"$$cleanPath($$1)\")
}

# Return the name of the platform deploy tool
defineReplace(platformDeployTool) {
    win32 {
        return("windeployqt")
    }
    else:macx {
        return("macdeployqt")
    }
    else:linux {
        return("linuxdeployqt")
    }
}
defineReplace(extraLibArgs) {
    outList = ""
    for(library, $$1) {
        outList += "-"$$library
    }
    return($$join(outList," "))
}
# If repogen isn't detected, throw a warning
defineTest(detectRepogen) {
    #true:{
    !exists($$1/repogen.exe):!exists($$1//repogen) {
        warning("Aborting repogen creation, since QT Installer Framework 3.1 is not installed.")
        warning("Please run the QT maintence tool and install QT Installer Framework 3.1.")
        return(false)
    }
    return(true)
}

# If repogen isn't detected, throw a warning
defineTest(detectDeploy) {
    #true:{
    win32:macx{
        exists($$1/windeployqt.exe) {
            return(true)
        }
        exists($$1/macdeployqt) {
            return(true)
        }
        warning("Aborting installer creation, since QT Installer Framework 3.1 is not installed.")
        warning("Please run the QT maintence tool and install QT Installer Framework 3.1.")
        return(false)
    }
    else: linux: !exists($$[QT_INSTALL_BINS]/linuxdeployqt) {
        message($$[QT_INSTALL_BINS])
        warning("Aborting installer creation, since linuxdeployqt is not installed.")
        warning("Please follow the build instructions listed for https://github.com/probonopd/linuxdeployqt.")
        warning("Alternatively, check out https://github.com/StanWarford/pep9suite/wiki/Generating-the-Installer-&-Update-Files-from-Qt-Creator.")
        return(false)
    }
    return(true)

}

defineTest(debugMessage) {
    contains(dbgMsg, "true") {
        message($$1)
        return(true)
    }
    return(false)
}
