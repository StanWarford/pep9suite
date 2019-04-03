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

#include "cpumainwindow.h"
#include "ui_cpumainwindow.h"
#include <QActionGroup>
#include <QApplication>
#include <QtConcurrent>
#include <QDebug>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QFontDialog>
#include <QMessageBox>
#include <QPrinter>
#include <QPrintDialog>
#include <QSettings>
#include <QTextStream>
#include <QTextCodec>
#include <QUrl>

#include "aboutpep.h"
#include "byteconverterbin.h"
#include "byteconverterchar.h"
#include "byteconverterdec.h"
#include "byteconverterhex.h"
#include "cpupane.h"
#include "cpuhelpdialog.h"
#include "darkhelper.h"
#include "memorydumppane.h"
#include "microcode.h"
#include "microcodepane.h"
#include "microcodeprogram.h"
#include "microobjectcodepane.h"
#include "updatechecker.h"

//WIP include
#include "partialmicrocodedcpu.h"
#include "newcpudata.h"
#include "mainmemory.h"
#include "memorychips.h"
#include "symboltable.h"
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), debugState(DebugState::DISABLED), updateChecker(new UpdateChecker()), codeFont(QFont(Pep::codeFont, Pep::codeFontSize)),
    memDevice(new MainMemory(nullptr)), controlSection(new PartialMicrocodedCPU(Enu::CPUType::OneByteDataBus, memDevice)),
    dataSection(controlSection->getDataSection()),
    cpuModesGroup(new QActionGroup(this)), isInDarkMode(false)
{
    // Initialize all global maps.
    Pep::initMicroEnumMnemonMaps(Enu::CPUType::OneByteDataBus, false);


    // Initialize the memory subsystem
    QSharedPointer<RAMChip> ramChip(new RAMChip(1<<16, 0, memDevice.get()));
    memDevice->insertChip(ramChip, 0);
    // I/O chips will still need to be added later

    // Perform any additional setup needed for UI objects.
    ui->setupUi(this);
    // Install this class as the global event filter.
    qApp->installEventFilter(this);

    ui->memoryWidget->init(memDevice, controlSection);
    // In Pep9CPU, we don't work with assembly instructions, so disable PC based features
    ui->memoryWidget->setHighlightPC(false);
    ui->memoryWidget->showJumpToPC(false);
    ui->cpuWidget->init(controlSection, controlSection->getDataSection());
    ui->microcodeWidget->init(controlSection, dataSection, memDevice, false);
    ui->microobjectWidget->init(controlSection, false);

    // Create button group to hold CPU types
    cpuModesGroup->addAction(ui->actionSystem_One_Byte);
    cpuModesGroup->addAction(ui->actionSystem_Two_Byte);
    cpuModesGroup->setExclusive(true);

    // Create & connect all dialogs.
    helpDialog = new HelpDialog(this);
    connect(helpDialog, &HelpDialog::copyToMicrocodeClicked, this, &MainWindow::onCopyToMicrocodeClicked);
    // Load the about text and create the about dialog
    QFile aboutFile(":/help-cpu/about.html");
    QString text = "";
    if(aboutFile.open(QFile::ReadOnly)) {
        text = QString(aboutFile.readAll());
    }
    QPixmap pixmap("://images/Pep9cpu-icon-about.png");
    aboutPepDialog = new AboutPep(text, pixmap, this);

    // Byte converter setup.
    byteConverterDec = new ByteConverterDec();
    ui->byteConverterToolBar->addWidget(byteConverterDec);
    byteConverterHex = new ByteConverterHex();
    ui->byteConverterToolBar->addWidget(byteConverterHex);
    byteConverterBin = new ByteConverterBin();
    ui->byteConverterToolBar->addWidget(byteConverterBin);
    byteConverterChar = new ByteConverterChar();
    ui->byteConverterToolBar->addWidget(byteConverterChar);
    connect(byteConverterBin, &ByteConverterBin::textEdited, this, &MainWindow::slotByteConverterBinEdited);
    connect(byteConverterChar, &ByteConverterChar::textEdited, this, &MainWindow::slotByteConverterCharEdited);
    connect(byteConverterDec, &ByteConverterDec::textEdited, this, &MainWindow::slotByteConverterDecEdited);
    connect(byteConverterHex, &ByteConverterHex::textEdited, this, &MainWindow::slotByteConverterHexEdited);

    connect((QApplication*)QApplication::instance(), &QApplication::focusChanged, this, &MainWindow::focusChanged);

    // Connect Undo / Redo events
    connect(ui->microcodeWidget, &MicrocodePane::undoAvailable, this, &MainWindow::setUndoability);
    connect(ui->microcodeWidget, &MicrocodePane::redoAvailable, this, &MainWindow::setRedoability);

    // Connect simulation events.
    // Events that fire on simulationUpdate should be UniqueConnections, as they will be repeatedly connected and disconnected
    // via connectMicroDraw() and disconnectMicroDraw().
    connect(this, &MainWindow::simulationUpdate, ui->cpuWidget, &CpuPane::onSimulationUpdate, Qt::UniqueConnection);
    connect(this, &MainWindow::simulationUpdate, ui->memoryWidget, &MemoryDumpPane::updateMemory, Qt::UniqueConnection);
    connect(this, &MainWindow::simulationStarted, ui->memoryWidget, &MemoryDumpPane::onSimulationStarted);
    connect(controlSection.get(), &PartialMicrocodedCPU::hitBreakpoint, this, &MainWindow::onBreakpointHit);


    connect(this, &MainWindow::simulationStarted, ui->microobjectWidget, &MicroObjectCodePane::onSimulationStarted);
    connect(this, &MainWindow::simulationFinished, ui->microobjectWidget, &MicroObjectCodePane::onSimulationFinished);
    connect(this, &MainWindow::simulationFinished, controlSection.get(), &PartialMicrocodedCPU::onSimulationFinished);
    connect(this, &MainWindow::simulationFinished, ui->cpuWidget, &CpuPane::onSimulationUpdate);
    connect(this, &MainWindow::simulationFinished, ui->memoryWidget, &MemoryDumpPane::onSimulationFinished);
    // Connect MainWindow so that it can propogate simulationFinished event and clean up when execution is finished.
    connect(controlSection.get(), &PartialMicrocodedCPU::simulationFinished, this, &MainWindow::onSimulationFinished);

    // Connect simulation events that are internal to the class.
    connect(this, &MainWindow::simulationUpdate, this, &MainWindow::handleDebugButtons, Qt::UniqueConnection);
    connect(this, &MainWindow::simulationUpdate, this, static_cast<void(MainWindow::*)()>(&MainWindow::highlightActiveLines), Qt::UniqueConnection);
    connect(this, &MainWindow::simulationStarted, this, &MainWindow::handleDebugButtons);

    // Connect font change events.
    connect(this, &MainWindow::fontChanged, ui->microcodeWidget, &MicrocodePane::onFontChanged);
    connect(this, &MainWindow::fontChanged, helpDialog, &HelpDialog::onFontChanged);
    connect(this, &MainWindow::fontChanged, ui->memoryWidget, &MemoryDumpPane::onFontChanged);

    // Connect dark mode events.
    connect(this, &MainWindow::darkModeChanged, ui->microcodeWidget, &MicrocodePane::onDarkModeChanged);
    connect(this, &MainWindow::darkModeChanged, helpDialog, &HelpDialog::onDarkModeChanged);
    connect(this, &MainWindow::darkModeChanged, ui->microobjectWidget, &MicroObjectCodePane::onDarkModeChanged);
    connect(this, &MainWindow::darkModeChanged, ui->cpuWidget, &CpuPane::onDarkModeChanged);
    connect(this, &MainWindow::darkModeChanged, ui->microcodeWidget->getEditor(), &MicrocodeEditor::onDarkModeChanged);
    connect(this, &MainWindow::darkModeChanged, ui->memoryWidget, &MemoryDumpPane::onDarkModeChanged);


    connect(ui->cpuWidget, &CpuPane::appendMicrocodeLine, this, &MainWindow::appendMicrocodeLine);

    // Events that notify view of changes in model.
    // These events are disconnected whenevr "run" or "continue" are called, because they have significant overhead,
    // but the provide no benefit when running.
    // They are reconnected at the end of execution, and the receiving widgets are manually notified that changes may have occured.
    connect(memDevice.get(), &MainMemory::changed, ui->memoryWidget, &MemoryDumpPane::onMemoryChanged, Qt::ConnectionType::UniqueConnection);
    connect(ui->cpuWidget, &CpuPane::registerChanged, dataSection.get(), &NewCPUDataSection::onSetRegisterByte, Qt::ConnectionType::UniqueConnection);
    connect(dataSection.get(), &NewCPUDataSection::statusBitChanged, ui->cpuWidget, &CpuPane::onStatusBitChanged, Qt::ConnectionType::UniqueConnection);
    connect(dataSection.get(), &NewCPUDataSection::registerChanged, ui->cpuWidget, &CpuPane::onRegisterChanged, Qt::ConnectionType::UniqueConnection);
    connect(dataSection.get(), &NewCPUDataSection::memoryRegisterChanged, ui->cpuWidget, &CpuPane::onMemoryRegisterChanged, Qt::ConnectionType::UniqueConnection);

    // Connect events for breakpoints
    connect(ui->actionDebug_Remove_All_Microcode_Breakpoints, &QAction::triggered, ui->microcodeWidget, &MicrocodePane::onRemoveAllBreakpoints);

    //Pre-render memory & fix maximum widget size.
    int maxSize = ui->memoryWidget->memoryDumpWidth();
    ui->memoryWidget->setMinimumWidth(maxSize);
    ui->memoryWidget->setMaximumWidth(maxSize);

    //Initialize debug menu
    handleDebugButtons();

    //Lastly, read in settings
    readSettings();

    // Resize docking widgets because QT does a poor job of it
    // ui->memoryDockWdget->raise();

}

MainWindow::~MainWindow()
{
    delete ui;
    //delete helpDialog;
    delete aboutPepDialog;
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
        // Must explicitly close dialog, otherwise it might keep
        // the entire application alive.
        helpDialog->close();
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
            if (ui->cpuWidget->hasFocus() && ui->actionDebug_Single_Step_Microcode->isEnabled()) {
                // single step or clock, depending of if currently debugging
                if (debugState == DebugState::DEBUG_MICRO) {
                    // single step
                    on_actionDebug_Single_Step_Microcode_triggered();
                }
                else {
                    // clock
                    ui->cpuWidget->clock();
                }
                return true;
            }
            else if (ui->actionDebug_Stop_Debugging->isEnabled() &&
                     (ui->microcodeWidget->hasFocus() || ui->microobjectWidget->hasFocus())) {
                ui->cpuWidget->giveFocus();
            }
        }
    }
    else if (event->type() == QEvent::FileOpen) {
        if (ui->actionDebug_Stop_Debugging->isEnabled()) {
            ui->statusBar->showMessage("Open failed, currently debugging.", 4000);
            return false;
        }
        //loadFile(static_cast<QFileOpenEvent *>(event)->file());
        return true;
    }
    return false;
}

void MainWindow::connectViewUpdate()
{
    connect(memDevice.get(), &MainMemory::changed, ui->memoryWidget, &MemoryDumpPane::onMemoryChanged, Qt::ConnectionType::UniqueConnection);
    connect(ui->cpuWidget, &CpuPane::registerChanged, dataSection.get(), &NewCPUDataSection::onSetRegisterByte, Qt::ConnectionType::UniqueConnection);
    connect(dataSection.get(), &NewCPUDataSection::statusBitChanged, ui->cpuWidget, &CpuPane::onStatusBitChanged, Qt::ConnectionType::UniqueConnection);
    connect(dataSection.get(), &NewCPUDataSection::registerChanged, ui->cpuWidget, &CpuPane::onRegisterChanged, Qt::ConnectionType::UniqueConnection);
    connect(dataSection.get(), &NewCPUDataSection::memoryRegisterChanged, ui->cpuWidget, &CpuPane::onMemoryRegisterChanged, Qt::ConnectionType::UniqueConnection);
    connect(this, &MainWindow::simulationUpdate, ui->memoryWidget, &MemoryDumpPane::updateMemory, Qt::UniqueConnection);
    connect(this, &MainWindow::simulationUpdate, ui->cpuWidget, &CpuPane::onSimulationUpdate, Qt::UniqueConnection);
    connect(this, &MainWindow::simulationUpdate, this, &MainWindow::handleDebugButtons, Qt::UniqueConnection);
    connect(this, &MainWindow::simulationUpdate, this, static_cast<void(MainWindow::*)()>(&MainWindow::highlightActiveLines), Qt::UniqueConnection);
    // If application is running, active lines shouldn't be highlighted at the begin of the instruction, as this would be misleading.
    connect(this, &MainWindow::simulationStarted, this, static_cast<void(MainWindow::*)()>(&MainWindow::highlightActiveLines), Qt::UniqueConnection);
    dataSection->setEmitEvents(true);
}

void MainWindow::disconnectViewUpdate()
{
    disconnect(memDevice.get(), &MainMemory::changed, ui->memoryWidget,&MemoryDumpPane::onMemoryChanged);
    disconnect(ui->cpuWidget, &CpuPane::registerChanged, dataSection.get(), &NewCPUDataSection::onSetRegisterByte);
    disconnect(dataSection.get(), &NewCPUDataSection::statusBitChanged, ui->cpuWidget, &CpuPane::onStatusBitChanged);
    disconnect(dataSection.get(), &NewCPUDataSection::registerChanged, ui->cpuWidget, &CpuPane::onRegisterChanged);
    disconnect(dataSection.get(), &NewCPUDataSection::memoryRegisterChanged, ui->cpuWidget, &CpuPane::onMemoryRegisterChanged);
    disconnect(this, &MainWindow::simulationUpdate, ui->memoryWidget, &MemoryDumpPane::updateMemory);
    disconnect(this, &MainWindow::simulationUpdate, ui->cpuWidget, &CpuPane::onSimulationUpdate);
    disconnect(this, &MainWindow::simulationUpdate, this, &MainWindow::handleDebugButtons);
    disconnect(this, &MainWindow::simulationUpdate, this, static_cast<void(MainWindow::*)()>(&MainWindow::highlightActiveLines));
    disconnect(this, &MainWindow::simulationStarted, this, static_cast<void(MainWindow::*)()>(&MainWindow::highlightActiveLines));
    dataSection->setEmitEvents(false);
}

void MainWindow::readSettings()
{
    QSettings settings("cslab.pepperdine","Pep9CPU");
    settings.beginGroup("MainWindow");

    // Restore screen dimensions
    QByteArray readGeometry = settings.value("geometry", saveGeometry()).toByteArray();
    restoreGeometry(readGeometry);

    // Restore last used font
    QVariant val = settings.value("font", codeFont);
    emit fontChanged(codeFont);

    //Restore last used file path
    curPath = settings.value("filePath", QDir::homePath()).toString();
    // Restore dark mode state
    onDarkModeChanged();
    quint16 debuggerLevel = settings.value("debugLevel", 1).toInt();

    settings.endGroup();

    //Handle reading for all children
    ui->microcodeWidget->readSettings(settings);
}

void MainWindow::writeSettings()
{
    QSettings settings("cslab.pepperdine","Pep9CPU");
    settings.beginGroup("MainWindow");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("font", codeFont);
    settings.setValue("filePath", curPath);
    settings.setValue("inDarkMode", isInDarkMode);
    settings.endGroup();
    //Handle writing for all children
    ui->microcodeWidget->writeSettings(settings);
}

// Save methods
bool MainWindow::save()
{
    bool retVal = true;
    /* For each pane, first check if there is already a file associated with the pane.
     * if there is not, then pass control to the save as function.
     * If there is a file, then attempt save to it, then remove any modified flags from the pane if it succeded.
     */
    if(QFileInfo(ui->microcodeWidget->getCurrentFile()).absoluteFilePath().isEmpty()) {
        retVal = saveAsFile();
    }
    else retVal = saveFile(ui->microcodeWidget->getCurrentFile().fileName());
    if(retVal) {
        // If an invalid program is saved, it might be incorrectly "run"
        // due to caching in microcode pane. Clearing the cached microprogram
        // will work around this bug.
        ui->microcodeWidget->clearProgram();
        ui->microcodeWidget->setModifiedFalse();
    }
    return retVal;
}

bool MainWindow::maybeSave()
{
    const QString dlgTitle = "Pep/9 CPU";
    const QString msg = "The microcode has been modified.\nDo you want to save your changes?";
    const auto buttons = QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel;
    QMessageBox::StandardButton ret;
    bool retVal = true;
    if(ui->microcodeWidget->isModified()) {
        ret = QMessageBox::warning(this, dlgTitle, msg, buttons);
        if (ret == QMessageBox::Save)
            retVal = save();
        else if (ret == QMessageBox::Cancel)
            retVal = false;
    }
    return retVal;

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
    ui->microcodeWidget->setFocus();
    ui->microcodeWidget->setCurrentFile(fileName);
    ui->microcodeWidget->setMicrocode(in.readAll());
    ui->microcodeWidget->setModifiedFalse();
    curPath = QFileInfo(file).dir().absolutePath();
    statusBar()->showMessage(tr("File loaded"), 4000);
    QApplication::restoreOverrideCursor();
    emit ui->actionDebug_Remove_All_Microcode_Breakpoints->trigger();
}

bool MainWindow::saveFile()
{
    QString fileName;
    fileName = QFileInfo(ui->microcodeWidget->getCurrentFile()).absoluteFilePath();
    return saveFile(fileName);
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

    // Messages for status bar.
    static const QString msgMicro = "Microcode saved";
    const QString *msgOutput; // Mutable pointer to const data.
    out << ui->microcodeWidget->getMicrocodeText();
    msgOutput = &msgMicro;
    QApplication::restoreOverrideCursor();
    curPath = QFileInfo(file).dir().absolutePath();
    statusBar()->showMessage(*msgOutput, 4000);
    return true;
}

bool MainWindow::saveAsFile()
{
    // Default filenames for each pane.
    static const QString defMicroFile = "untitled.pepcpu";
    QString usingFile;

    // Titles for each pane.
    static const QString titleBase = "Save %1";
    static const QString microTitle = titleBase.arg("Microcode");


    // Patterns for source code files.
    static const QString microTypes = "Pep/9 Microcode (*.pepcpu *.txt)";

    // Pick the filename
    if(ui->microcodeWidget->getCurrentFile().fileName().isEmpty()) {
        usingFile = QDir(curPath).absoluteFilePath(defMicroFile);
    }
    else usingFile = ui->microcodeWidget->getCurrentFile().fileName();

    QString fileName = QFileDialog::getSaveFileName(
                this,
                microTitle,
                usingFile,
                microTypes);

    if (fileName.isEmpty()) {
        return false;
    }
    else if (saveFile(fileName)) {
        // If the file is successfully saved, then change the file associated with the pane.
        ui->microcodeWidget->setCurrentFile(fileName);
        return true;
    }
    else return false;
}

QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void MainWindow::print()
{
    // Don't use a pointer for the text, because toPlainText() returns a temporary object
    QString text;
    const QString title = "Print Microcode";

    QPrinter printer(QPrinter::HighResolution);
    QTextDocument document;

    // Create a highlighter independent of the microcodewidget's highlighter,
    // so that we may force it to use light mode colors.
    PepMicroHighlighter mcHi(Enu::CPUType::TwoByteDataBus,
                                                   true, PepColors::lightMode, &document);
    mcHi.forceAllFeatures(true);
    document.setPlainText(ui->microcodeWidget->toPlainText());
    mcHi.rehighlight();

    QPrintDialog *dialog = new QPrintDialog(&printer, this);
    dialog->setWindowTitle(title);
    if (dialog->exec() == QDialog::Accepted) {
        // printer.setPaperSize(QPrinter::Letter);
        // Must set margins, or printer might default to 2cm
        printer.setPageMargins(1, 1, 1, 1, QPrinter::Inch);
        // The document will pick the system default font.
        // It is not monospaced, and it will not print well.
        document.setDefaultFont(this->codeFont);
        // If the document page size is not specified, then Windows mught try
        // and fit the entire document on a single page.
        document.setPageSize(printer.paperSize(QPrinter::Point));
        // qDebug() << document.pageCount();
        document.print(&printer);
    }
    dialog->deleteLater();
}

void MainWindow::debugButtonEnableHelper(const int which)
{
    // Only allow formatting of code if there exists a user-built program
    // to format from.
    bool formatMicrocode = !ui->microcodeWidget->getMicrocodeProgram().isNull();
    // Crack the parameter using DebugButtons to properly enable and disable all buttons related to debugging and running.
    // Build Actions
    ui->actionBuild_Microcode->setEnabled(which & DebugButtons::BUILD_MICRO);
    ui->actionEdit_Remove_Error_Microcode->setEnabled(which & DebugButtons::BUILD_MICRO);
    ui->actionEdit_Format_Microcode->setEnabled((which & DebugButtons::BUILD_MICRO) && formatMicrocode);

    // Debug & Run Actions
    ui->actionBuild_Run->setEnabled(which & DebugButtons::RUN);
    ui->actionDebug_Start_Debugging->setEnabled(which & DebugButtons::DEBUG);
    ui->actionDebug_Continue->setEnabled(which & DebugButtons::CONTINUE);
    ui->actionDebug_Restart_Debugging->setEnabled(which & DebugButtons::RESTART);
    ui->actionDebug_Stop_Debugging->setEnabled(which & DebugButtons::STOP);
    ui->actionDebug_Single_Step_Microcode->setEnabled(which & DebugButtons::SINGLE_STEP_MICRO);

    // System Actions
    ui->actionSystem_One_Byte->setEnabled(which & DebugButtons::SWITCH_BUSES);
    ui->actionSystem_Two_Byte->setEnabled(which & DebugButtons::SWITCH_BUSES);

    //File open & new actions
    ui->actionFile_New_Microcode->setEnabled(which & DebugButtons::OPEN_NEW);
    ui->actionFile_Open->setEnabled(which & DebugButtons::OPEN_NEW);
}

void MainWindow::highlightActiveLines()
{
    // always highlight the current microinstruction
    ui->microcodeWidget->updateSimulationView();
    ui->microobjectWidget->highlightCurrentInstruction();
    ui->memoryWidget->clearHighlight();
    ui->memoryWidget->highlight();
}

bool MainWindow::initializeSimulation()
{
    // Load microprogram into the micro control store
    if (ui->microcodeWidget->microAssemble()) {
        ui->statusBar->showMessage("MicroAssembly succeeded", 4000);
        if(ui->microcodeWidget->getMicrocodeProgram()->hasMicrocode() == false)
        {
            ui->statusBar->showMessage("No microcode program to build", 4000);
            return false;
        }
        ui->microobjectWidget->setObjectCode(ui->microcodeWidget->getMicrocodeProgram(), nullptr);
        controlSection->setMicrocodeProgram(ui->microcodeWidget->getMicrocodeProgram());
    }
    else {
        ui->statusBar->showMessage("MicroAssembly failed", 4000);
        return false;
    }

    // If there are preconditions, then clear memory & CPU
    if(ui->microcodeWidget->getMicrocodeProgram()->hasUnitPre()) {
        // Clear data models & application views
        controlSection->onResetCPU();
        controlSection->initCPU();
        ui->cpuWidget->clearCpu();
        on_actionSystem_Clear_Memory_triggered();
        for(auto line : ui->microcodeWidget->getMicrocodeProgram()->getObjectCode()) {
            if(line->hasUnitPre()) {
                static_cast<UnitPreCode*>(line)->setUnitPre(dataSection.get());
            }
        }
    }
    else {
        // If there are no preconditions, the do not clear anything
    }

    // Don't allow the microcode pane to be edited while the program is running
    ui->microcodeWidget->setReadOnly(true);
    return true;
}

void MainWindow::onUpdateCheck(int val)
{
    val = (int)val; //Ugly way to get rid of unused paramter warning without actually modifying the parameter.
    //Dummy to handle update checking code
}

// File MainWindow triggers
void MainWindow::on_actionFile_New_Microcode_triggered()
{
    //Try to save the microcode pane before clearing it & the micro-object-code pane.
    if (maybeSave()) {
        ui->actionDebug_Remove_All_Microcode_Breakpoints->trigger();
        ui->microcodeWidget->setFocus();
        ui->microcodeWidget->setMicrocode("");
        ui->microcodeWidget->setCurrentFile("");
        ui->microobjectWidget->setObjectCode();
        emit ui->actionDebug_Remove_All_Microcode_Breakpoints->trigger();
    }
}

void MainWindow::on_actionFile_Open_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(
                this,
                "Open text file",
                curPath,
                "Pep/9 CPU files (*.pepcpu *.txt)");
    // Depending on the file ending, change which pane will be loaded into.
    if (!fileName.isEmpty()) {
        if(maybeSave()) {
            loadFile(fileName);
        }
        curPath = QFileInfo(fileName).absolutePath();
    }
}

bool MainWindow::on_actionFile_Save_Microcode_triggered()
{
    return save();
}

bool MainWindow::on_actionFile_Save_Microcode_As_triggered()
{
    return saveAsFile();
}

void MainWindow::on_actionFile_Print_Microcode_triggered()
{
    print();
}

// Edit MainWindow triggers

void MainWindow::on_actionEdit_Undo_triggered()
{
    // Other Pep9Cpu panes do not support undo
    if (ui->microcodeWidget->hasFocus() && ui->actionDebug_Start_Debugging->isEnabled()) {
        ui->microcodeWidget->undo();
    }
}

void MainWindow::on_actionEdit_Redo_triggered()
{
    // Other Pep9Cpu panes do not support redo
    if (ui->microcodeWidget->hasFocus() && ui->actionDebug_Start_Debugging->isEnabled()) {
        ui->microcodeWidget->redo();
    }
}

void MainWindow::on_actionEdit_Cut_triggered()
{
    if (ui->microcodeWidget->hasFocus() && ui->actionDebug_Start_Debugging->isEnabled()) {
        ui->microcodeWidget->cut();
    }
}

void MainWindow::on_actionEdit_Copy_triggered()
{
    if (ui->microcodeWidget->hasFocus()) {
        ui->microcodeWidget->copy();
    }
    // other panes should not be able to copy
}

void MainWindow::on_actionEdit_Paste_triggered()
{
    if (ui->microcodeWidget->hasFocus() && ui->actionDebug_Start_Debugging->isEnabled()) {
        ui->microcodeWidget->paste();
    }
}

void MainWindow::on_actionEdit_UnComment_Line_triggered()
{
    if (!ui->actionDebug_Stop_Debugging->isEnabled()) { // we are not debugging
        ui->microcodeWidget->unCommentSelection();
    }
}

void MainWindow::on_actionEdit_Format_Microcode_triggered()
{
    if (ui->microcodeWidget->microAssemble()) {
        ui->microcodeWidget->setMicrocode(ui->microcodeWidget->getMicrocodeProgram()->format());
    }
}

void MainWindow::on_actionEdit_Remove_Error_Microcode_triggered()
{
    ui->microcodeWidget->removeErrorMessages();
}

void MainWindow::on_actionEdit_Font_triggered()
{
    bool ok = false;
    QFont font  = QFontDialog::getFont(&ok, codeFont, this, "Set Source Code Font");
    if(ok) {
        codeFont = font;
        emit fontChanged(codeFont);
    }
}

void MainWindow::on_actionEdit_Reset_font_to_Default_triggered()
{
    codeFont = QFont(Pep::codeFont, Pep::codeFontSize);
    emit fontChanged(codeFont);
}

bool MainWindow::on_actionBuild_Microcode_triggered()
{
    if(ui->microcodeWidget->microAssemble()) {
        ui->statusBar->showMessage("MicroAssembly succeeded", 4000);
        ui->microobjectWidget->setObjectCode(ui->microcodeWidget->getMicrocodeProgram(), nullptr);
        return true;
    }
    else {
        ui->statusBar->showMessage("MicroAssembly failed", 4000);
        return false;
    }
}

//Build Events

void MainWindow::on_actionBuild_Run_triggered()
{
    if(!initializeSimulation()) return;
    debugState = DebugState::RUN;
    if (initializeSimulation()) {
        disconnectViewUpdate();
        emit simulationStarted();
        ui->memoryWidget->updateMemory();
        memDevice->clearBytesSet();
        memDevice->clearBytesWritten();
        controlSection->onSimulationStarted();
        controlSection->onRun();
        // Make sure to highlight modified memory addresses to make it clear to the user
        // what has been modified over the course of execution.
        highlightActiveLines();
        connectViewUpdate();

    }
    else {
        // If the simulation can't be initialized, revert to default state.
        debugState = DebugState::DISABLED;
   }
   // If somehow the simulation is not finished, then make sure to terminate it.
   if(debugState != DebugState::DISABLED) onSimulationFinished();
}

// Debug slots

void MainWindow::handleDebugButtons()
{
    quint8 byte;
    memDevice->getByte(controlSection->getCPURegWordStart(Enu::CPURegisters::PC), byte);
    Enu::EMnemonic mnemon = Pep::decodeMnemonic[byte];
    bool enable_into = (mnemon == Enu::EMnemonic::CALL) || Pep::isTrapMap[mnemon];
    // Disable button stepping if waiting on IO
    int enabledButtons = 0;
    switch(debugState)
    {
    case DebugState::DISABLED:
        enabledButtons = DebugButtons::RUN| DebugButtons::DEBUG;
        enabledButtons |= DebugButtons::BUILD_MICRO;
        enabledButtons |= DebugButtons::OPEN_NEW | DebugButtons::SWITCH_BUSES;
        break;
    case DebugState::RUN:
        enabledButtons = DebugButtons::STOP | DebugButtons::INTERRUPT;
        break;
    case DebugState::DEBUG_MICRO:
        enabledButtons = DebugButtons::INTERRUPT | DebugButtons::STOP | DebugButtons::RESTART | DebugButtons::CONTINUE;
        enabledButtons |= DebugButtons::SINGLE_STEP_MICRO;
        break;
    case DebugState::DEBUG_RESUMED:
        enabledButtons = DebugButtons::INTERRUPT | DebugButtons::STOP | DebugButtons::RESTART;
        break;
    default:
        break;
    }
    debugButtonEnableHelper(enabledButtons);
}

bool MainWindow::on_actionDebug_Start_Debugging_triggered()
{  
    connectViewUpdate();
    if(!initializeSimulation()) return false;
    else {
        debugState = DebugState::DEBUG_MICRO;
        emit simulationStarted();
        controlSection->onSimulationStarted();
        memDevice->clearBytesSet();
        memDevice->clearBytesWritten();
        controlSection->enableDebugging();
        ui->cpuWidget->startDebugging();
        ui->memoryWidget->updateMemory();
        // Force re-highlighting on memory to prevent last run's data
        // from remaining highlighted. Otherwise, if the last program
        // was "run", then every byte that it modified will be highlighted
        // upon starting the simulation.
        highlightActiveLines();
        return true;
    }
}

void MainWindow::on_actionDebug_Stop_Debugging_triggered()
{
    connectViewUpdate();
    highlightActiveLines();
    debugState = DebugState::DISABLED;
    ui->microcodeWidget->clearSimulationView();
    ui->microobjectWidget->clearSimulationView();
    ui->microcodeWidget->setReadOnly(false);
    ui->cpuWidget->stopDebugging();
    handleDebugButtons();
    ui->memoryWidget->updateMemory();
    controlSection->onSimulationFinished();
    emit simulationFinished();
}

void MainWindow::on_actionDebug_Interupt_Execution_triggered()
{
    on_actionDebug_Stop_Debugging_triggered();
}

void MainWindow::on_actionDebug_Continue_triggered()
{
    debugState = DebugState::DEBUG_RESUMED;
    handleDebugButtons();
    disconnectViewUpdate();
    controlSection->onRun();
    if(controlSection->hadErrorOnStep()) {
        return; // we'll just return here instead of letting it fail and go to the bottom
    }
    connectViewUpdate();
    if(controlSection->stoppedForBreakpoint()) {
        emit simulationUpdate();
        QApplication::processEvents();
        highlightActiveLines();
    }
}

void MainWindow::on_actionDebug_Restart_Debugging_triggered()
{
    on_actionDebug_Stop_Debugging_triggered();
    on_actionDebug_Start_Debugging_triggered();
}

void MainWindow::on_actionDebug_Single_Step_Microcode_triggered()
{
    debugState = DebugState::DEBUG_MICRO;
    controlSection->onMCStep();
    emit simulationUpdate();
}

void MainWindow::onMicroBreakpointHit()
{
    debugState = DebugState::DEBUG_MICRO;
}

// System MainWindow triggers
void MainWindow::on_actionSystem_Clear_CPU_triggered()
{
    controlSection->onResetCPU();
    ui->cpuWidget->clearCpu();
}

void MainWindow::on_actionSystem_Clear_Memory_triggered()
{
    memDevice->clearMemory();
    ui->memoryWidget->refreshMemory();
}

void MainWindow::on_actionSystem_One_Byte_triggered()
{
    Pep::initMicroEnumMnemonMaps(Enu::OneByteDataBus, false);
    controlSection->setCPUType(Enu::OneByteDataBus);
    ui->microobjectWidget->initCPUModelState();
    ui->microcodeWidget->onCPUTypeChanged(Enu::OneByteDataBus);
    ui->cpuWidget->onCPUTypeChanged();

}

void MainWindow::on_actionSystem_Two_Byte_triggered()
{
    Pep::initMicroEnumMnemonMaps(Enu::TwoByteDataBus, false);
    controlSection->setCPUType(Enu::TwoByteDataBus);
    ui->microobjectWidget->initCPUModelState();
    ui->microcodeWidget->onCPUTypeChanged(Enu::TwoByteDataBus);
    ui->cpuWidget->onCPUTypeChanged();
}

void MainWindow::onSimulationFinished()
{
    QString errorString;
    on_actionDebug_Stop_Debugging_triggered();

    QVector<AMicroCode*> prog = ui->microcodeWidget->getMicrocodeProgram()->getObjectCode();
    bool hadPostTest = false;
    for (AMicroCode* x : prog) {
        if(x->hasUnitPost()) hadPostTest = true;
        if (x->hasUnitPost() && !((UnitPostCode*)x)->testPostcondition(dataSection.get(), errorString)) {
             ((UnitPostCode*)x)->testPostcondition(dataSection.get(), errorString);
             ui->microcodeWidget->appendMessageInSourceCodePaneAt(-1, errorString);
             QMessageBox::warning(this, "Pep/9 CPU", "Failed unit test");
             ui->statusBar->showMessage("Failed unit test", 4000);
             return;
         }
    }
    if(controlSection->hadErrorOnStep()) {
        ui->statusBar->showMessage("Execution Failed", 4000);
        QMessageBox::critical(
          this,
          tr("Pep/9 CPU"),
          controlSection->getErrorMessage());        
    }
    else if(hadPostTest) ui->statusBar->showMessage("Passed unit test", 4000);
    else ui->statusBar->showMessage("Execution Finished", 4000);
}

void MainWindow::onDarkModeChanged()
{
    isInDarkMode = inDarkMode();
    emit darkModeChanged(isInDarkMode, styleSheet());
}

// help:
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

void MainWindow::on_actionHelp_One_Byte_Examples_triggered()
{
    helpDialog->show();
    helpDialog->selectItem("One-byte Bus Examples");
}

void MainWindow::on_actionHelp_Two_Byte_Examples_triggered()
{
    helpDialog->show();
    helpDialog->selectItem("Two-byte Bus Examples");
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

void MainWindow::slotByteConverterBinEdited(const QString &str)
{
    if (str.length() > 0) {
        bool ok;
        int data = str.toInt(&ok, 2);
        byteConverterChar->setValue(data);
        byteConverterDec->setValue(data);
        byteConverterHex->setValue(data);
    }
}

void MainWindow::slotByteConverterCharEdited(const QString &str)
{
    if (str.length() > 0) {
        int data = static_cast<int>(str[0].toLatin1());
        byteConverterBin->setValue(data);
        byteConverterDec->setValue(data);
        byteConverterHex->setValue(data);
    }
}

void MainWindow::slotByteConverterDecEdited(const QString &str)
{
    if (str.length() > 0) {
        bool ok;
        int data = str.toInt(&ok, 10);
        byteConverterBin->setValue(data);
        byteConverterChar->setValue(data);
        byteConverterHex->setValue(data);
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
                byteConverterBin->setValue(data);
                byteConverterChar->setValue(data);
                byteConverterDec->setValue(data);
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

// Focus Coloring. Activates and deactivates undo/redo/cut/copy/paste actions contextually
void MainWindow::focusChanged(QWidget *oldFocus, QWidget *)
{
    // Unhighlight the old widget.
    if(ui->microcodeWidget->isAncestorOf(oldFocus)) {
        ui->microcodeWidget->highlightOnFocus();
    }
    else if(ui->microobjectWidget->isAncestorOf(oldFocus)) {
        ui->microobjectWidget->highlightOnFocus();
    }
    else if(ui->memoryWidget->isAncestorOf(oldFocus)) {
        ui->memoryWidget->highlightOnFocus();
    }
    else if(ui->cpuWidget->isAncestorOf(oldFocus)) {
        ui->cpuWidget->highlightOnFocus();
    }

    // Highlight the newly focused widget.
    int which = 0;
    if (ui->microcodeWidget->hasFocus()) {
        which = Enu::EditButton::COPY | Enu::EditButton::CUT | Enu::EditButton::PASTE;
        which |= Enu::EditButton::UNDO*ui->microcodeWidget->isUndoable() | Enu::EditButton::REDO*ui->microcodeWidget->isRedoable();
        ui->microcodeWidget->highlightOnFocus();
    }
    else if (ui->memoryWidget->hasFocus()) {
        which = 0;
        ui->memoryWidget->highlightOnFocus();
    }
    else if (ui->cpuWidget->hasFocus()) {
        which = 0;
        ui->cpuWidget->highlightOnFocus();
    }
    else if (ui->microobjectWidget->hasFocus()) {
        which = Enu::EditButton::COPY;
        ui->microobjectWidget->highlightOnFocus();
    }

    ui->actionEdit_Undo->setEnabled(which & Enu::EditButton::UNDO);
    ui->actionEdit_Redo->setEnabled(which & Enu::EditButton::REDO);
    ui->actionEdit_Cut->setEnabled(which & Enu::EditButton::CUT);
    ui->actionEdit_Copy->setEnabled(which & Enu::EditButton::COPY);
    ui->actionEdit_Paste->setEnabled(which & Enu::EditButton::PASTE);
}

void MainWindow::setUndoability(bool b)
{
    if (ui->microcodeWidget->hasFocus()) {
        ui->actionEdit_Undo->setEnabled(b);
    }
    else if (ui->memoryWidget->hasFocus()) {
        ui->actionEdit_Undo->setEnabled(false);
    }
    else if (ui->cpuWidget->hasFocus()) {
        ui->actionEdit_Undo->setEnabled(false);
    }
    else if (ui->microobjectWidget->hasFocus()) {
        ui->actionEdit_Redo->setDisabled(true);
    }
}

void MainWindow::setRedoability(bool b)
{
    if (ui->microcodeWidget->hasFocus()) {
        ui->actionEdit_Redo->setEnabled(b);
    }
    else if (ui->memoryWidget->hasFocus()) {
        ui->actionEdit_Redo->setEnabled(false);
    }
    else if (ui->cpuWidget->hasFocus()) {
        ui->actionEdit_Redo->setEnabled(false);
    }
    else if (ui->microobjectWidget->hasFocus()) {
        ui->actionEdit_Redo->setDisabled(true);
    }
}

void MainWindow::appendMicrocodeLine(QString line)
{
    ui->microcodeWidget->appendMessageInSourceCodePaneAt(-2, line);
}

void MainWindow::onCopyToMicrocodeClicked()
{
    if(controlSection->getInSimulation() || controlSection->getInDebug()) {
        QMessageBox::warning(this, "Help Warning", "Can't copy to microcode when a simulation is running");
        return;
    }
    if(controlSection->getCPUType() != helpDialog->getExamplesModel()) {
        if(helpDialog->getExamplesModel() == Enu::CPUType::OneByteDataBus) {
            on_actionSystem_One_Byte_triggered();
        }
        else if(helpDialog->getExamplesModel() == Enu::CPUType::TwoByteDataBus) {
            on_actionSystem_Two_Byte_triggered();
        }
    }
    QString code = helpDialog->getExampleText();
    helpDialog->hide();
    if(code.isEmpty()) return;
    emit ui->actionDebug_Remove_All_Microcode_Breakpoints->trigger();
    ui->microcodeWidget->setMicrocode(code);
    on_actionBuild_Microcode_triggered();
    statusBar()->showMessage("Copied to microcode", 4000);
    ui->microcodeWidget->microAssemble();
    ui->microobjectWidget->setObjectCode(ui->microcodeWidget->getMicrocodeProgram(), nullptr);
}

void MainWindow::onBreakpointHit(Enu::BreakpointTypes type)
{
    switch(type) {
    case Enu::BreakpointTypes::MICROCODE:
        onMicroBreakpointHit();
        break;
    }
}
