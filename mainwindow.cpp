// File: mainwindow.cpp
/*
    Pep9CPU is a CPU simulator for executing microcode sequences to
    implement instructions in the instruction set of the Pep/9 computer.

    Copyright (C) 2010  J. Stanley Warford, Pepperdine University

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "cpucontrolsection.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QSettings>
#include <QMessageBox>
#include <QTextStream>
#include <QTextCodec>
#include <QFileDialog>
#include <QDesktopServices>
#include <QUrl>
#include <QtConcurrent>
#include <QDebug>
#include <QFontDialog>
#include "memorysection.h"
#include "cpudatasection.h"
#include "cpucontrolsection.h"
#include "code.h"
#include <iodialog.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), updateChecker(new UpdateChecker()),
    memorySection(MemorySection::getInstance()), dataSection(CPUDataSection::getInstance()),
    controlSection(CPUControlSection::getInstance())
{
    Pep::initMicroEnumMnemonMaps();
    Pep::initEnumMnemonMaps();
    Pep::initMnemonicMaps();
    Pep::initAddrModesMap();
    Pep::initDecoderTables();
    ui->setupUi(this);
    // connect and begin update checker
    connect(updateChecker, &UpdateChecker::updateInformation, this, &MainWindow::onUpdateCheck);
    auto x = QtConcurrent::run(updateChecker,&UpdateChecker::beginUpdateCheck);
    auto y = new IODialog(this);
    y->setVisible(true);
    //Connect Models to necessary components

    mainMemory = new MainMemory(ui->mainSplitter);
    delete ui->memoryFrame;
    microcodePane = new MicrocodePane(ui->codeSplitter);
    delete ui->microcodeFrame;
    objectCodePane = new ObjectCodePane(ui->codeSplitter);
    //objectCodePane->hide();
    delete ui->objectCodeFrame;
    //cpuPaneOneByteDataBus = new CpuPane(Enu::OneByteDataBus, ui->mainSplitter);
    //ui->mainSplitter->insertWidget(1, cpuPaneOneByteDataBus);
    //cpuPane = cpuPaneOneByteDataBus;


    cpuPaneTwoByteDataBus = new CpuPane(Enu::TwoByteDataBus, ui->mainSplitter);
    //cpuPaneTwoByteDataBus->hide();
    ui->mainSplitter->insertWidget(1, cpuPaneTwoByteDataBus);
    cpuPane = cpuPaneTwoByteDataBus;

    delete ui->cpuFrame;

    QList<int> list;
    list.append(3000);
    list.append(1);
    ui->codeSplitter->setSizes(list);

    ui->mainSplitter->insertWidget(0, mainMemory);
    ui->mainSplitter->insertWidget(1, cpuPane);

    helpDialog = new HelpDialog(this);
    aboutPepDialog = new AboutPep(this);

    // Byte converter setup
    byteConverterDec = new ByteConverterDec(this);
    ui->byteConverterToolBar->addWidget(byteConverterDec);
    byteConverterHex = new ByteConverterHex(this);
    ui->byteConverterToolBar->addWidget(byteConverterHex);
    byteConverterBin = new ByteConverterBin(this);
    ui->byteConverterToolBar->addWidget(byteConverterBin);
    byteConverterChar = new ByteConverterChar(this);
    ui->byteConverterToolBar->addWidget(byteConverterChar);
    ui->byteConverterToolBar->setWindowTitle("Byte Converter");
    connect(byteConverterDec, &ByteConverterDec::textEdited, this, &MainWindow::slotByteConverterDecEdited);
    connect(byteConverterHex, &ByteConverterHex::textEdited, this, &MainWindow::slotByteConverterHexEdited);
    connect(byteConverterBin, &ByteConverterBin::textEdited, this, &MainWindow::slotByteConverterBinEdited);
    connect(byteConverterChar, &ByteConverterChar::textEdited, this, &MainWindow::slotByteConverterCharEdited);

    connect(helpDialog, &HelpDialog::copyToMicrocodeClicked, this, &MainWindow::helpCopyToMicrocodeButtonClicked);

    connect(qApp->instance(), SIGNAL(focusChanged(QWidget*, QWidget*)), this, SLOT(focusChanged(QWidget*, QWidget*)));
    connect(microcodePane, &MicrocodePane::undoAvailable, this, &MainWindow::setUndoability);
    connect(microcodePane, &MicrocodePane::redoAvailable, this, &MainWindow::setRedoability);

    connect(cpuPane, &CpuPane::updateSimulation, this, &MainWindow::updateSimulation);
    connect(cpuPane, &CpuPane::simulationFinished, this, &MainWindow::simulationFinished);
    connect(cpuPane, &CpuPane::stopSimulation, this, &MainWindow::stopSimulation);
    connect(cpuPane, &CpuPane::writeByte, this, &MainWindow::updateMemAddress);

    //Connect events that pass on CPU Feature changes
    //Pep::initEnumMnemonMaps();
    //Connect Simulation events
    connect(this, &MainWindow::beginSimulation,this->objectCodePane,&ObjectCodePane::onBeginSimulation);
    connect(this, &MainWindow::endSimulation,this->objectCodePane,&ObjectCodePane::onEndSimulation);
    connect(this, &MainWindow::endSimulation,this->controlSection,&CPUControlSection::onSimulationFinished);
    //Connect font change events
    connect(this, &MainWindow::fontChanged, microcodePane, &MicrocodePane::onFontChanged);
    connect(this, &MainWindow::fontChanged, helpDialog, &HelpDialog::onFontChanged);
    connect(this, &MainWindow::darkModeChanged, microcodePane, &MicrocodePane::onDarkModeChanged);
    connect(this, &MainWindow::darkModeChanged, helpDialog, &HelpDialog::onDarkModeChanged);
    connect(this, &MainWindow::darkModeChanged, objectCodePane, &ObjectCodePane::onDarkModeChanged);
    connect(this, &MainWindow::darkModeChanged, cpuPaneTwoByteDataBus, &CpuPane::onDarkModeChanged);
    connect(this, &MainWindow::darkModeChanged, microcodePane->getEditor(), &MicrocodeEditor::onDarkModeChanged);
    connect(this, &MainWindow::darkModeChanged, mainMemory, &MainMemory::onDarkModeChange);
    qApp->installEventFilter(this);
    //Load Style sheets
    QFile f(":qdarkstyle/dark_style.qss");
    f.open(QFile::ReadOnly | QFile::Text);
    QTextStream ts(&f);
    darkStyle= ts.readAll();
    lightStyle = this->styleSheet();
    connect(cpuPane, &CpuPane::appendMicrocodeLine, this, &MainWindow::appendMicrocodeLine);
    readSettings();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

// Protected closeEvent
void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        writeSettings();
        event->accept();
    }
    else {
        event->ignore();
    }
}

bool MainWindow::eventFilter(QObject *, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if ((keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)) {
            if (cpuPane->hasFocus()) {
                // single step or clock, depending
                if (ui->actionSystem_Stop_Debugging->isEnabled()) {
                    // single step
                    cpuPane->singleStep();
                }
                else {
                    // clock
                    cpuPane->clock();
                }
                return true;
            }
            else if (ui->actionSystem_Stop_Debugging->isEnabled() &&
                     (microcodePane->hasFocus() || objectCodePane->hasFocus())) {
                cpuPane->giveFocus();
            }
        }
    }
    else if (event->type() == QEvent::FileOpen) {
        if (ui->actionSystem_Stop_Debugging->isEnabled()) {
            ui->statusBar->showMessage("Open failed, currently debugging.", 4000);
            return false;
        }
        loadFile(static_cast<QFileOpenEvent *>(event)->file());
        return true;
    }
    return false;
}

void MainWindow::readSettings()
{
    QSettings settings("cslab.pepperdine","PEP9CPU");
    QDesktopWidget *desktop = QApplication::desktop();
    int width = static_cast<int>(desktop->width() * 0.80);
    int height = static_cast<int>(desktop->height() * 0.70);
    int screenWidth = desktop->width();
    int screenHeight = desktop->height();
    settings.beginGroup("MainWindow");
    QPoint pos = settings.value("pos", QPoint((screenWidth - width) / 2, (screenHeight - height) / 2)).toPoint();
    QSize size = settings.value("size", QSize(width, height)).toSize();
    if (Pep::getSystem() == "Mac") {
        pos.setY(pos.y() + 20); // Every time the app launches, it seems OSX moves the window 20 pixels up the screen, so we compensate here.
    }
    else if (Pep::getSystem() == "Linux") { // Linux has a similar issue, so compensate here.
        pos.setY(pos.y() - 20);
    }
    if (pos.x() > width || pos.x() < 0 || pos.y() > height || pos.y() < 0) {
        pos = QPoint(0, 0);
    }
    QVariant val = settings.value("font",codeFont);
    if(val.canConvert<QFont>())
    {
        codeFont = qvariant_cast<QFont>(val);
    }
    emit fontChanged(codeFont);
    resize(size);
    move(pos);

    curPath = settings.value("filePath", QDir::homePath()).toString();
    settings.endGroup();
    //Handle reading for all children
    microcodePane->readSettings(settings);
}

void MainWindow::writeSettings()
{
    QSettings settings("cslab.pepperdine","PEP9CPU");
    settings.beginGroup("MainWindow");
    settings.setValue("pos", pos());
    settings.setValue("size", size());
    settings.setValue("filePath", curPath);
    settings.setValue("font",codeFont);
    settings.endGroup();
    //Handle writing for all children
    microcodePane->writeSettings(settings);
}

// Save methods
bool MainWindow::save()
{
    if (curFile.isEmpty()) {
        return on_actionFile_Save_As_triggered();
    }
    else {
        return saveFile(curFile);
    }
}

bool MainWindow::maybeSave()
{
    if (microcodePane->isModified()) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, "Pep/9 CPU",
                                   "The microcode has been modified.\n"
                                   "Do you want to save your changes?",
                                   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save)
            return save();
        else if (ret == QMessageBox::Cancel)
            return false;
    }
    return true;
}

void MainWindow::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Pep/9 CPU"), tr("Cannot read file %1:\n%2.")
                             .arg(fileName).arg(file.errorString()));
        return;
    }

    QTextStream in(&file);
    in.setCodec(QTextCodec::codecForName("ISO 8859-1"));
    QApplication::setOverrideCursor(Qt::WaitCursor);
    // Set source code pane text
    microcodePane->setMicrocode(in.readAll());
    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File loaded"), 4000);
    QApplication::restoreOverrideCursor();
}

bool MainWindow::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Pep/9 CPU"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }

    QTextStream out(&file);
    out.setCodec(QTextCodec::codecForName("ISO 8859-1"));
    QApplication::setOverrideCursor(Qt::WaitCursor);
    out << Pep::addCycleNumbers(microcodePane->getMicrocode());
    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
    statusBar()->showMessage("Microcode saved", 4000);
    return true;
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    microcodePane->setFilename(QFileInfo(fileName).fileName());
    curFile = fileName;
    microcodePane->setModifiedFalse();

    if (!fileName.isEmpty()) {
        curPath = QFileInfo(fileName).path();
    }
}

QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void MainWindow::onUpdateCheck(int val)
{
    val = (int)val; //Ugly way to get rid of unused paramter warning without actually modifying the parameter.
    //Dummy to handle update checking code
    QMessageBox::information(this,"help","me");
}

// File MainWindow triggers
void MainWindow::on_actionFile_New_triggered()
{
    if (maybeSave()) {
        microcodePane->setMicrocode("");
        objectCodePane->setObjectCode();
        setCurrentFile("");
    }
}

void MainWindow::on_actionFile_Open_triggered()
{
    if (maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(
                    this,
                    "Open text file",
                    curPath,
                    "Text files (*.pepcpu *.txt)");
        if (!fileName.isEmpty()) {
            loadFile(fileName);
            curPath = QFileInfo(fileName).path();
        }
    }
}

bool MainWindow::on_actionFile_Save_triggered()
{
    if (curFile.isEmpty()) {
        return on_actionFile_Save_As_triggered();
    }
    else {
        return saveFile(curFile);
    }
}

bool MainWindow::on_actionFile_Save_As_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(
                this,
                "Save Microcode",
                curFile.isEmpty() ? curPath + "/untitled.pepcpu" : curPath + "/" + strippedName(curFile),
                "Pep9 Source (*.pepcpu *.txt)");
    if (fileName.isEmpty()) {
        return false;
    }
    return saveFile(fileName);
}

// Edit MainWindow triggers
void MainWindow::on_actionEdit_Undo_triggered()
{
    if (microcodePane->hasFocus() && ui->actionSystem_Start_Debugging->isEnabled()) {
        microcodePane->undo();
    }
    // other panes should not be able to undo
}

void MainWindow::on_actionEdit_Redo_triggered()
{
    if (microcodePane->hasFocus() && ui->actionSystem_Start_Debugging->isEnabled()) {
        microcodePane->redo();
    }
    // other panes should not be able to redo
}

void MainWindow::on_actionEdit_Cut_triggered()
{
    if (microcodePane->hasFocus() && ui->actionSystem_Start_Debugging->isEnabled()) {
        microcodePane->cut();
    }
    // other panes should not be able to cut
}

void MainWindow::on_actionEdit_Copy_triggered()
{
    if (microcodePane->hasFocus()) {
        microcodePane->copy();
    }
    else if (objectCodePane->hasFocus()) {
        objectCodePane->copy();
    }
    else if (helpDialog->hasFocus()) {
        helpDialog->copy();
    }
    // other panes should not be able to copy
}

void MainWindow::on_actionEdit_Paste_triggered()
{
    if (microcodePane->hasFocus() && ui->actionSystem_Start_Debugging->isEnabled()) {
        microcodePane->paste();
    }
    // other panes should not be able to paste
}

void MainWindow::on_actionEdit_UnComment_Line_triggered()
{
    if (!ui->actionSystem_Stop_Debugging->isEnabled()) { // we are not debugging
        microcodePane->unCommentSelection();
    }
}

void MainWindow::on_actionEdit_Auto_Format_Microcode_triggered()
{
    //Should format correctly anytime assembly works
    if (microcodePane->microAssemble()) {
        microcodePane->setMicrocode(microcodePane->getMicrocodeProgram()->format());
    }
}

void MainWindow::on_actionEdit_Remove_Error_Messages_triggered()
{
    microcodePane->removeErrorMessages();
}

void MainWindow::on_actionEdit_Font_triggered()
{
    bool ok = false ;
    QFont font  = QFontDialog::getFont(&ok,codeFont, this, "Set Source Code Font");
    if(ok)
    {
        codeFont = font;
        emit fontChanged(codeFont);
    }
}

void MainWindow::on_actionEdit_Reset_font_to_Default_triggered()
{
    codeFont = QFont(Pep::codeFont,Pep::codeFontSize);
    emit fontChanged(codeFont);
}

// System MainWindow triggers
void MainWindow::on_actionSystem_Run_triggered()
{
    if (on_actionSystem_Start_Debugging_triggered()) {
        cpuPane->run();
    }
}

bool MainWindow::on_actionSystem_Start_Debugging_triggered()
{
    emit beginSimulation();
    MicrocodeProgram* prog;
    if (microcodePane->microAssemble()) {
        ui->statusBar->showMessage("MicroAssembly succeeded", 4000);
        objectCodePane->setObjectCode(microcodePane->getMicrocodeProgram(),nullptr);
        controlSection->setMicrocodeProgram(microcodePane->getMicrocodeProgram());
        prog = microcodePane->getMicrocodeProgram();
        controlSection->onDebuggingStarted();
        bool hasUnitPre = prog->hasUnitPre();
        if(hasUnitPre)
        {
            // setup preconditions
            controlSection->onClearCPU();
            mainMemory->clearMemory();
            cpuPane->clearCpu();
            controlSection->initCPUStateFromPreconditions();
        }
    }
    else {
        ui->statusBar->showMessage("MicroAssembly failed", 4000);
        return false;
    }

    // prevent simulation from starting if there's nothing to simulate
    bool hasMicrocode = prog->hasMicrocode(); //The compiler might warn here, but the only way to get here is if microassembly succeded, and thus there is a valid program
    if (!hasMicrocode) {
        return false;
    }

    // enable the actions available while we're debugging
    ui->actionSystem_Stop_Debugging->setEnabled(true);

    // disable actions related to editing/starting debugging
    ui->actionSystem_Run->setEnabled(false);
    ui->actionSystem_Start_Debugging->setEnabled(false);
    microcodePane->setReadOnly(true);

    cpuPane->startDebugging();
    return true;
}

void MainWindow::on_actionSystem_Stop_Debugging_triggered()
{
    microcodePane->clearSimulationView();
    objectCodePane->clearSimulationView();
    // disable the actions available while we're debugging
    ui->actionSystem_Stop_Debugging->setEnabled(false);

    // enable actions related to editing/starting debugging
    ui->actionSystem_Run->setEnabled(true);
    ui->actionSystem_Start_Debugging->setEnabled(true);
    microcodePane->setReadOnly(false);

    cpuPane->stopDebugging();
    emit endSimulation();
}

void MainWindow::on_actionSystem_Clear_CPU_triggered()
{
    controlSection->onClearCPU();
    cpuPane->clearCpu();
}

void MainWindow::on_actionSystem_Clear_Memory_triggered()
{
    mainMemory->clearMemory();
    controlSection->onClearMemory();
}

void MainWindow::on_actionDark_Mode_triggered()
{

    if(ui->actionDark_Mode->isChecked())
    {
        this->setStyleSheet(darkStyle);
    }
    else
    {
        this->setStyleSheet(lightStyle);
    }
    emit darkModeChanged(ui->actionDark_Mode->isChecked());
}


// help:
void MainWindow::on_actionHelp_UsingPep9CPU_triggered()
{
    helpDialog->show();
    helpDialog->selectItem("Using Pep/9 CPU");
}

void MainWindow::on_actionHelp_InteractiveUse_triggered()
{
    helpDialog->show();
    helpDialog->selectItem("Interactive Use");
}

void MainWindow::on_actionHelp_MicrocodeUse_triggered()
{
    helpDialog->show();
    helpDialog->selectItem("Microcode Use");
}

void MainWindow::on_actionHelp_DebuggingUse_triggered()
{
    helpDialog->show();
    helpDialog->selectItem("Debugging Use");
}

void MainWindow::on_actionHelp_Pep9Reference_triggered()
{
    helpDialog->show();
    helpDialog->selectItem("Pep/9 Reference");
}

void MainWindow::on_actionHelp_Examples_triggered()
{
    helpDialog->show();
    helpDialog->selectItem("Examples");
}

void MainWindow::on_actionHelp_triggered()
{
    if (!helpDialog->isHidden()) {
        // give it focus again:
        helpDialog->hide();
        helpDialog->show();
    }
    else {
        helpDialog->show();
    }
}

void MainWindow::on_actionHelp_About_Pep9CPU_triggered()
{
    aboutPepDialog->show();
}

void MainWindow::on_actionHelp_About_Qt_triggered()
{
    QDesktopServices::openUrl(QUrl("http://www.qt.io/"));
}

// Byte Converter slots
void MainWindow::slotByteConverterDecEdited(const QString &str)
{
    if (str.length() > 0) {
        bool ok;
        int data = str.toInt(&ok, 10);
        byteConverterHex->setValue(data);
        byteConverterBin->setValue(data);
        byteConverterChar->setValue(data);
    }
}

void MainWindow::slotByteConverterHexEdited(const QString &str)
{
    if (str.length() >= 2) {
        if (str.startsWith("0x")) {
            QString hexPart = str;
            hexPart.remove(0, 2);
            if (hexPart.length() > 0) {
                bool ok;
                int data = hexPart.toInt(&ok, 16);
                byteConverterDec->setValue(data);
                byteConverterBin->setValue(data);
                byteConverterChar->setValue(data);
            }
            else {
                // Exactly "0x" remains, so do nothing
            }
        }
        else {
            // Prefix "0x" was mangled
            byteConverterHex->setValue(-1);
        }
    }
    else {
        // Prefix "0x" was shortened
        byteConverterHex->setValue(-1);
    }
}

void MainWindow::slotByteConverterBinEdited(const QString &str)
{
    if (str.length() > 0) {
        bool ok;
        int data = str.toInt(&ok, 2);
        byteConverterDec->setValue(data);
        byteConverterHex->setValue(data);
        byteConverterChar->setValue(data);
    }
}

void MainWindow::slotByteConverterCharEdited(const QString &str)
{
    if (str.length() > 0) {
        int data = (int)str[0].toLatin1();
        byteConverterDec->setValue(data);
        byteConverterHex->setValue(data);
        byteConverterBin->setValue(data);
    }
}

// Focus Coloring. Activates and deactivates undo/redo/cut/copy/paste actions contextually
void MainWindow::focusChanged(QWidget *, QWidget *)
{
    microcodePane->highlightOnFocus();
    mainMemory->highlightOnFocus();
    objectCodePane->highlightOnFocus();
    cpuPane->highlightOnFocus();

    if (microcodePane->hasFocus()) {
        ui->actionEdit_Undo->setDisabled(!microcodePane->isUndoable());
        ui->actionEdit_Redo->setDisabled(!microcodePane->isRedoable());
        ui->actionEdit_Cut->setDisabled(false);
        ui->actionEdit_Copy->setDisabled(false);
        ui->actionEdit_Paste->setDisabled(false);
    }
    else if (mainMemory->hasFocus()) {
        ui->actionEdit_Undo->setDisabled(true);
        ui->actionEdit_Redo->setDisabled(true);
        ui->actionEdit_Cut->setDisabled(true);
        ui->actionEdit_Copy->setDisabled(true);
        ui->actionEdit_Paste->setDisabled(true);
    }
    else if (cpuPane->hasFocus()) {
        ui->actionEdit_Undo->setDisabled(true);
        ui->actionEdit_Redo->setDisabled(true);
        ui->actionEdit_Cut->setDisabled(true);
        ui->actionEdit_Copy->setDisabled(true);
        ui->actionEdit_Paste->setDisabled(true);
    }
    else if (objectCodePane->hasFocus()) {
        ui->actionEdit_Undo->setDisabled(true);
        ui->actionEdit_Redo->setDisabled(true);
        ui->actionEdit_Cut->setDisabled(true);
        ui->actionEdit_Copy->setDisabled(false);
        ui->actionEdit_Paste->setDisabled(true);
    }
}

void MainWindow::setUndoability(bool b)
{
    if (microcodePane->hasFocus()) {
        ui->actionEdit_Undo->setDisabled(!b);
    }
    else if (mainMemory->hasFocus()) {
        ui->actionEdit_Redo->setDisabled(true);
    }
    else if (cpuPane->hasFocus()) {
        ui->actionEdit_Redo->setDisabled(true);
    }
    else if (objectCodePane->hasFocus()) {
        ui->actionEdit_Redo->setDisabled(true);
    }
}

void MainWindow::setRedoability(bool b)
{
    if (microcodePane->hasFocus()) {
        ui->actionEdit_Redo->setDisabled(!b);
    }
    else if (mainMemory->hasFocus()) {
        ui->actionEdit_Redo->setDisabled(true);
    }
    else if (cpuPane->hasFocus()) {
        ui->actionEdit_Redo->setDisabled(true);
    }
    else if (objectCodePane->hasFocus()) {
        ui->actionEdit_Redo->setDisabled(true);
    }
}

void MainWindow::updateSimulation()
{
    microcodePane->updateSimulationView();
    objectCodePane->highlightCurrentInstruction();
}

void MainWindow::stopSimulation()
{
    on_actionSystem_Stop_Debugging_triggered();

}

void MainWindow::simulationFinished()
{
    QString errorString;

    on_actionSystem_Stop_Debugging_triggered();

    QVector<Code*> prog = microcodePane->getMicrocodeProgram()->getObjectCode();
    for (Code* x : prog) {
        if (x->hasUnitPost()&&!((UnitPostCode*)x)->testPostcondition(dataSection, errorString)) {
            ((UnitPostCode*)x)->testPostcondition(dataSection, errorString);
            microcodePane->appendMessageInSourceCodePaneAt(-1, errorString);
            QMessageBox::warning(this, "Pep9CPU", "Failed unit test");
            return;
        }
    }
    // feature, not a bug: we will display the "passed unit test" even
    // on the empty case - no postconditions

    ui->statusBar->showMessage("Passed unit test", 4000);
}

void MainWindow::appendMicrocodeLine(QString line)
{
    microcodePane->appendMessageInSourceCodePaneAt(-2, line);
}

void MainWindow::helpCopyToMicrocodeButtonClicked()
{
    if (maybeSave()) {
        microcodePane->setMicrocode(helpDialog->getExampleText());
        microcodePane->microAssemble();
        objectCodePane->setObjectCode(microcodePane->getMicrocodeProgram(),nullptr);
        helpDialog->hide();
        statusBar()->showMessage("Copied to microcode", 4000);
    }
}

void MainWindow::updateMemAddress(int address)
{
    mainMemory->setMemAddress(address, memorySection->getMemoryByte(address));
    mainMemory->showMemEdited(address);
}






