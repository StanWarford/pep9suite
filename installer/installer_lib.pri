##########################
# Global installer defines
##########################
# Qt binary directory (where qmake is stored for the current kit).
QtDir = $$clean_path($$[QT_INSTALL_PREFIX])
# Directory where the Qt installer framework is installed.
# Choose the highest published version of the tools if possible,
# but we support back to 3.0.
exists($$QtDir/../../tools/Qtinstallerframework/3.1/bin) {
    QtInstallerBin = $$QtDir/../../tools/Qtinstallerframework/3.1/bin
}
else {
    QtInstallerBin = $$clean_path($$QtDir/../../tools/Qtinstallerframework/3.0/bin)
}


################################
# Global installer configuration
################################
# Deterime platform specific command separator (psc) and
# determine target extensions (TARGET_EXT) for various platforms.
win32 {
    #Prevent Windows from trying to parse the project three times per build.
    #This interferes with the deployment script, and makes debugging hard since Qt attempts to debug the optimized program.
    CONFIG -= debug_and_release \
    debug_and_release_target
    psc = "&"
    TARGET_EXT = ".exe"
}
else: macx {
    psc = ";"
    CONFIG(app_bundle){
        TARGET_EXT = ".app"
    }
    else{
        TARGET_EXT = ""
    }
}
else: linux {
    psc = ";"
    TARGET_EXT = ""
}

##############################
#Replace function declarations
##############################

# Clean path. Necessary before passing path as argument in qmake command,
# as batch scripts like "copy" will break if non-system paths are used
# (e.g. a path with '/' is used in Windows instead of '\')
defineReplace(cleanPath){
    # Adjust the input path so that the correct slashes are used for the host shell $$psc OS
    return($$system_path($$1))
}

# Clean path with force quote.
defineReplace(cleanPathQuote){
    return(\"$$cleanPath($$1)\")
}

# Return the name of the platform deploy tool.
defineReplace(platformDeployTool) {
    win32 {
        return("windeployqt")
    }
    else: macx {
        return("macdeployqt")
    }
    else: linux {
        return("linuxdeployqt")
    }
}

# Create extra Qt library arguments for deploy tools.
defineReplace(extraLibArgs) {
    outList = ""
    for(library, $$1) {
        outList += "-"$$library
    }
    return($$join(outList, " "))
}

###########################
#Test function declarations
###########################

# Check if repository generation tools exist, warn if they do not.
defineTest(detectRepogen) {
    !exists($$1/repogen.exe): !exists($$1//repogen) {
        warning("Aborting repogen creation, since QT Installer Framework 3.1 is not installed.")
        warning("Please run the QT maintence tool and install QT Installer Framework 3.1.")
        return(false)
    }
    return(true)
}

# Check if deployment tools exist, warn if they do not.
defineTest(detectDeploy) {
    #true:{
    win32: macx{
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
        return(false)
    }
    return(true)

}

# Print a message iff dbgMsg is set to "true".
# Useful for conditionally printing help messages in installer framework.
defineTest(debugMessage) {
    contains(dbgMsg, "true") {
        message($$1)
        return(true)
    }
    return(false)
}
