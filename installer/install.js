/*
 * Introduction
 * License Agreement
 * Target Directory
 * Component Select
 * Start Button
 * Select packages
 * Perform Install
 * Finished page
 * Add desktop Entry
 */

function Controller()
{
    installer.autoAcceptMessageBoxes();
    installer.setMessageBoxAutomaticAnswer("stopProcessesForUpdates", QMessageBox.Ignore);
    installer.installationFinished.connect(function() {
        gui.clickButton(buttons.NextButton);
    })
	installer.setValue("isOffline", installer.isOfflineOnly() ? "true" : "false");
}


Controller.prototype.IntroductionPageCallback = function()
{
	var widget = gui.currentPageWidget();
	if(!installer.isInstaller()) {
			widget.findChild("PackageManagerRadioButton").visible = false;
			widget.findChild("UpdaterRadioButton").visible = false;
			widget.findChild("UninstallerRadioButton").checked = true;
			gui.clickButton(buttons.NextButton);
	} 
	else {
		gui.clickButton(buttons.NextButton);
	}
}

Controller.prototype.ComponentSelectionPageCallback = function()
{
    gui.clickButton(buttons.NextButton);
}

Controller.prototype.LicenseAgreementPageCallback = function()
{
    gui.currentPageWidget().AcceptLicenseRadioButton.setChecked(true);
    gui.clickButton(buttons.NextButton);
}

// Target Directory Page
Controller.prototype.TargetDirectoryPageCallback = function()
{
	var dir = installer.value("TargetDir")
	if(installer.fileExists(dir) | installer.fileExists(dir+"/Pep9-Updater.exe")) {
		console.log("Attempting to automatically uninstall old versions of Pep/9.")
		var ret = installer.execute(dir + "/Pep9-Updater.exe", "--script=" + dir + "/uninstall.js");
		gui.clickButton(buttons.CancelButton); // automatically click the Next button
	}
	else {
		gui.clickButton(buttons.NextButton); // automatically click the Next button
	}

}

Controller.prototype.StartMenuDirectoryPageCallback = function()
{
    gui.clickButton(buttons.NextButton);
}

Controller.prototype.ReadyForInstallationPageCallback = function()
{
    gui.clickButton(buttons.NextButton);
}

Controller.prototype.PerformInstallationPageCallback = function()
{
    gui.clickButton(buttons.NextButton);
}

Controller.prototype.FinishedPageCallback = function()
{
	// Prevent application from being launched.
	gui.currentPageWidget().RunItCheckBox.setChecked(false);
    gui.clickButton(buttons.FinishButton);
}

// Add Desktop Entry
Controller.prototype.DynamicShortcutPageCallback = function()
{
	gui.clickButton(buttons.NextButton); // automatically click the Next button
}