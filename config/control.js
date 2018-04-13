function func() {

}
function Controller() {
    var widget = gui.pageById(QInstaller.Introduction); // get the introduction wizard page
    if (widget != null){
        widget.packageManagerCoreTypeChanged.connect(onPackageManagerCoreTypeChanged);
    }
}

Controller.prototype.IntroductionPageCallback  = function(){
    var widget = gui.currentPageWidget();
    if (widget != null) {
        console.log(Object.getOwnPropertyNames(widget));
        widget.findChild("PackageManagerRadioButton").visible = false;
        widget.findChild("PackageManagerRadioButton").enabled = false;
        widget.findChild("PackageManagerRadioButton").text = "";
        widget.findChild("UninstallerRadioButton").text = "Uninstall Pep9CPU"
        widget.findChild("UpdaterRadioButton").text = "Update Pep9CPU"
    }

}

Controller.prototype.LicenseAgreementPageCallback = function(){
    var widget = gui.currentPageWidget();
    if (widget != null) {
        widget.AcceptLicenseRadioButton.checked = true;
    }
}

Controller.prototype.FinishedPageCallback = function(){
}

onPackageManagerCoreTypeChanged = function(){

    var widget = gui.pageById(QInstaller.Introduction);
    if (widget != null) {
        widget.findChild("PackageManagerRadioButton").visible = false;
        if(widget.findChild("PackageManagerRadioButton").checked==true){
            widget.findChild("UninstallerRadioButton").checked = true;
        }
    }
}
