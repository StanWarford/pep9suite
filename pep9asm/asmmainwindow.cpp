// File: asmmainwindow.cpp
/*
    Pep9 is a virtual machine for writing machine language and assembly
    language programs.

    Copyright (C) 2019  J. Stanley Warford & Matthew McRaven, Pepperdine University

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

#include "asmmainwindow.h"
#include "ui_asmmainwindow.h"
#include <QActionGroup>
#include <QApplication>
#include <QtConcurrent>
#include <QtCore>
#include <QDebug>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QFontDialog>
#include <QMessageBox>
#include <QPageSize>
#include <QPrinter>
#include <QPrintDialog>
#include <QSettings>
#include <QTextStream>
#include <QTextCodec>
#include <QUrl>

#include "aboutpep.h"
#include "amemorychip.h"
#include "asmargument.h"
#include "asmcode.h"
#include "asmcpupane.h"
#include "asmprogram.h"
#include "asmprogrammanager.h"
#include "asmsourcecodepane.h"
#include "asmprogramlistingpane.h"
#include "byteconverterbin.h"
#include "byteconverterchar.h"
#include "byteconverterdec.h"
#include "byteconverterhex.h"
#include "byteconverterinstr.h"
#include "darkhelper.h"
#include "asmhelpdialog.h"
#include "isacpu.h"
#include "isaasm.h"
#include "mainmemory.h"
#include "memorychips.h"
#include "memorydumppane.h"
#include "updatechecker.h"
#include "redefinemnemonicsdialog.h"
#include "registerfile.h"
#include "symboltable.h"

AsmMainWindow::AsmMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AsmMainWindow), debugState(DebugState::DISABLED), codeFont(QFont(Pep::codeFont, Pep::codeFontSize)),
    updateChecker(new UpdateChecker()), isInDarkMode(false),
    memDevice(new MainMemory(nullptr)), controlSection(new IsaCpu(AsmProgramManager::getInstance(), memDevice)),
    redefineMnemonicsDialog(new RedefineMnemonicsDialog(this)),programManager(AsmProgramManager::getInstance())

{
    // Initialize the memory subsystem
    QSharedPointer<RAMChip> ramChip(new RAMChip(1<<16, 0, memDevice.get()));
    memDevice->insertChip(ramChip, 0);
    // I/O chips will still need to be added later

    // Perform any additional setup needed for UI objects.
    ui->setupUi(this);
    // Install this class as the global event filter.
    qApp->installEventFilter(this);

    ui->memoryWidget->init(memDevice, controlSection);
    ui->memoryTracePane->init(programManager, controlSection, memDevice, controlSection->getMemoryTrace());
    // Start with the memory trace pane being invisible, as it is not needed unless
    // a program with a valid stack trace is being debugged.
    ui->memoryTracePane->setVisible(false);
    ui->assemblerPane->init(programManager);
    ui->asmProgramTracePane->init(controlSection, programManager);
    ui->asmCpuPane->init(controlSection, controlSection);
    redefineMnemonicsDialog->init(true);
    ui->executionStatisticsWidget->init(controlSection, false);

    // Create & connect all dialogs.
    helpDialog = new AsmHelpDialog(this);
    connect(helpDialog, &AsmHelpDialog::copyToSourceClicked, this, &AsmMainWindow::helpCopyToSourceClicked);
    // Load the about text and create the about dialog
    QFile aboutFile(":/help-asm/about.html");
    QString text = "";
    if(aboutFile.open(QFile::ReadOnly)) {
        text = QString(aboutFile.readAll());
    }
    QPixmap pixmap("://images/Pep9-icon.png");
    aboutPepDialog = new AboutPep(text, pixmap, this);

    connect(redefineMnemonicsDialog, &RedefineMnemonicsDialog::closed, this, &AsmMainWindow::redefine_Mnemonics_closed);
    // Byte converter setup.
    byteConverterDec = new ByteConverterDec(this);
    ui->byteConverterToolBar->addWidget(byteConverterDec);
    byteConverterHex = new ByteConverterHex(this);
    ui->byteConverterToolBar->addWidget(byteConverterHex);
    byteConverterBin = new ByteConverterBin(this);
    ui->byteConverterToolBar->addWidget(byteConverterBin);
    byteConverterChar = new ByteConverterChar(this);
    ui->byteConverterToolBar->addWidget(byteConverterChar);
    byteConverterInstr = new ByteConverterInstr(this);
    ui->byteConverterToolBar->addWidget(byteConverterInstr);
    connect(byteConverterBin, &ByteConverterBin::textEdited, this, &AsmMainWindow::slotByteConverterBinEdited);
    connect(byteConverterChar, &ByteConverterChar::textEdited, this, &AsmMainWindow::slotByteConverterCharEdited);
    connect(byteConverterDec, &ByteConverterDec::textEdited, this, &AsmMainWindow::slotByteConverterDecEdited);
    connect(byteConverterHex, &ByteConverterHex::textEdited, this, &AsmMainWindow::slotByteConverterHexEdited);

    connect(qApp, &QApplication::focusChanged, this, &AsmMainWindow::focusChanged);

    // Connect IOWidget to memory
    ui->ioWidget->bindToMemorySection(memDevice.get());
    // Connect IO events
    connect(memDevice.get(), &MainMemory::inputRequested, this, &AsmMainWindow::onInputRequested, Qt::QueuedConnection);
    connect(memDevice.get(), &MainMemory::outputWritten, this, &AsmMainWindow::onOutputReceived, Qt::QueuedConnection);

    // Connect Undo / Redo events
    connect(ui->assemblerPane, &AssemblerPane::undoAvailable, this, &AsmMainWindow::setUndoability);
    connect(ui->assemblerPane, &AssemblerPane::redoAvailable, this, &AsmMainWindow::setRedoability);
    connect(ui->ioWidget, &IOWidget::undoAvailable, this, &AsmMainWindow::setUndoability);
    connect(ui->ioWidget, &IOWidget::redoAvailable, this, &AsmMainWindow::setRedoability);

    // Connect simulation events.
    // Events that fire on simulationUpdate should be UniqueConnections, as they will be repeatedly connected and disconnected
    // via connectMicroDraw() and disconnectMicroDraw().
    connect(this, &AsmMainWindow::simulationUpdate, ui->asmCpuPane, &AsmCpuPane::onSimulationUpdate, Qt::UniqueConnection);
    connect(this, &AsmMainWindow::simulationUpdate, ui->memoryWidget, &MemoryDumpPane::updateMemory, Qt::UniqueConnection);
    connect(this, &AsmMainWindow::simulationStarted, ui->memoryWidget, &MemoryDumpPane::onSimulationStarted);
    connect(this, &AsmMainWindow::simulationStarted, ui->memoryTracePane, &NewMemoryTracePane::onSimulationStarted);
    connect(this, &AsmMainWindow::simulationStarted, ui->executionStatisticsWidget, &ExecutionStatisticsWidget::onSimulationStarted);
    connect(ui->actionSystem_Clear_CPU, &QAction::triggered, ui->executionStatisticsWidget, &ExecutionStatisticsWidget::onClear);

    connect(this, &AsmMainWindow::simulationStarted, ui->asmCpuPane, &AsmCpuPane::onSimulationUpdate, Qt::UniqueConnection);
    connect(controlSection.get(), &IsaCpu::hitBreakpoint, this, &AsmMainWindow::onBreakpointHit);

    // Clear IOWidget every time a simulation is started.
    connect(this, &AsmMainWindow::simulationStarted, ui->ioWidget, &IOWidget::onClear);
    connect(this, &AsmMainWindow::simulationStarted, ui->ioWidget, &IOWidget::onSimulationStart);

    // Post finished events to the event queue so that they are processed after simulation updates.
    connect(this, &AsmMainWindow::simulationFinished, controlSection.get(), &IsaCpu::onSimulationFinished, Qt::QueuedConnection);
    connect(this, &AsmMainWindow::simulationFinished, ui->memoryWidget, &MemoryDumpPane::onSimulationFinished, Qt::QueuedConnection);
    connect(this, &AsmMainWindow::simulationFinished, ui->memoryTracePane, &NewMemoryTracePane::onSimulationFinished, Qt::QueuedConnection);
    connect(this, &AsmMainWindow::simulationFinished, ui->asmCpuPane, &AsmCpuPane::onSimulationUpdate, Qt::QueuedConnection);
    connect(this, &AsmMainWindow::simulationFinished, ui->executionStatisticsWidget, &ExecutionStatisticsWidget::onSimulationFinished, Qt::QueuedConnection);
    // Connect MainWindow so that it can propogate simulationFinished event and clean up when execution is finished.
    connect(controlSection.get(), &IsaCpu::simulationFinished, this, &AsmMainWindow::onSimulationFinished);


    // Connect simulation events that are internal to the class.
    connect(this, &AsmMainWindow::simulationUpdate, this, &AsmMainWindow::handleDebugButtons, Qt::UniqueConnection);
    connect(this, &AsmMainWindow::simulationUpdate, this, static_cast<void(AsmMainWindow::*)()>(&AsmMainWindow::highlightActiveLines), Qt::UniqueConnection);
    connect(this, &AsmMainWindow::simulationStarted, this, &AsmMainWindow::handleDebugButtons);

    // Connect font change events.
    connect(this, &AsmMainWindow::fontChanged, helpDialog, &AsmHelpDialog::onFontChanged);
    connect(this, &AsmMainWindow::fontChanged, ui->ioWidget, &IOWidget::onFontChanged);
    connect(this, &AsmMainWindow::fontChanged, ui->assemblerPane, &AssemblerPane::onFontChanged);
    connect(this, &AsmMainWindow::fontChanged, ui->memoryWidget, &MemoryDumpPane::onFontChanged);
    connect(this, &AsmMainWindow::fontChanged, ui->asmProgramTracePane, &AsmProgramTracePane::onFontChanged);

    // Connect dark mode events.
    connect(qApp, &QGuiApplication::paletteChanged, this, &AsmMainWindow::onPaletteChanged);
    connect(this, &AsmMainWindow::darkModeChanged, helpDialog, &AsmHelpDialog::onDarkModeChanged);
    connect(this, &AsmMainWindow::darkModeChanged, ui->memoryWidget, &MemoryDumpPane::onDarkModeChanged);
    connect(this, &AsmMainWindow::darkModeChanged, ui->assemblerPane, &AssemblerPane::onDarkModeChanged);
    connect(this, &AsmMainWindow::darkModeChanged, ui->asmProgramTracePane, &AsmProgramTracePane::onDarkModeChanged);
    connect(this, &AsmMainWindow::darkModeChanged, ui->memoryTracePane, &NewMemoryTracePane::onDarkModeChanged);

    // Events that notify view of changes in model.
    // These events are disconnected whenevr "run" or "continue" are called, because they have significant overhead,
    // but the provide no benefit when running.
    // They are reconnected at the end of execution, and the receiving widgets are manually notified that changes may have occured.
    connect(memDevice.get(), &MainMemory::changed, ui->memoryWidget, &MemoryDumpPane::onMemoryChanged, Qt::ConnectionType::UniqueConnection);

    // Connect events for breakpoints
    connect(ui->actionDebug_Remove_All_Assembly_Breakpoints, &QAction::triggered, programManager, &AsmProgramManager::onRemoveAllBreakpoints);
    connect(ui->asmProgramTracePane, &AsmProgramTracePane::breakpointAdded, programManager, &AsmProgramManager::onBreakpointAdded);
    connect(ui->asmProgramTracePane, &AsmProgramTracePane::breakpointRemoved, programManager, &AsmProgramManager::onBreakpointRemoved);

    connect(programManager, &AsmProgramManager::removeAllBreakpoints, ui->assemblerPane, &AssemblerPane::onRemoveAllBreakpoints);
    connect(programManager, &AsmProgramManager::removeAllBreakpoints, ui->asmProgramTracePane, &AsmProgramTracePane::onRemoveAllBreakpoints);

    connect(programManager, &AsmProgramManager::breakpointAdded, ui->asmProgramTracePane, &AsmProgramTracePane::onBreakpointAdded);
    connect(programManager, &AsmProgramManager::breakpointRemoved, ui->asmProgramTracePane, &AsmProgramTracePane::onBreakpointRemoved);
    connect(programManager, &AsmProgramManager::breakpointAdded, ui->assemblerPane, &AssemblerPane::onBreakpointAdded);
    connect(programManager, &AsmProgramManager::breakpointRemoved, ui->assemblerPane, &AssemblerPane::onBreakpointRemoved);

    // These aren't technically slots being bound to, but this is allowed because of the new signal / slot syntax
#pragma message ("If breakpoints aren't being added / set correctly, this is probably why")
    connect(programManager, &AsmProgramManager::breakpointAdded,
            [&](quint16 address){controlSection->breakpointAdded(address);});
    connect(programManager, &AsmProgramManager::breakpointRemoved,
            [&](quint16 address){controlSection->breakpointRemoved(address);});
    connect(programManager, &AsmProgramManager::setBreakpoints,
            [&](QSet<quint16> addresses){controlSection->breakpointsSet(addresses);});
    connect(programManager, &AsmProgramManager::removeAllBreakpoints,
            [&](){controlSection->breakpointsRemoveAll();});

    // Assemble default OS.
    assembleDefaultOperatingSystem();

    // Initialize debug menu.
    handleDebugButtons();

    // Read in settings.
    readSettings();

    // Create a new ASM file so that the dialog always has a file name in it.
    on_actionFile_New_Asm_triggered();

    // Correctly show correct panes & set up buttons.
    on_actionView_Code_CPU_triggered();

    // Make sure there is always an operating system loaded.
    // Otherwise, executing a object code program upon starting the application will
    // cause a crash.
    loadOperatingSystem();
}

AsmMainWindow::~AsmMainWindow()
{
    delete ui;
    delete helpDialog;
    delete aboutPepDialog;
    memDevice.clear();
}

void AsmMainWindow::changeEvent(QEvent *e)
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
void AsmMainWindow::closeEvent(QCloseEvent *event)
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

bool AsmMainWindow::eventFilter(QObject *, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if ((keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)) {
            if(ui->asmProgramTracePane->hasFocus() || ui->debuggerTab->hasFocus()) {
                if (ui->actionDebug_Step_Over_Assembler->isEnabled()) {
                    // single step
                    on_actionDebug_Single_Step_Assembler_triggered();
                    return true;
                }
            }
            /*else if (ui->actionDebug_Stop_Debugging->isEnabled() &&
                     (ui->microcodeWidget->hasFocus() || ui->microObjectCodePane->hasFocus())) {
                ui->cpuWidget->giveFocus();

            }*/
        }
    }
    else if (event->type() == QEvent::FileOpen) {
        if (ui->actionDebug_Stop_Debugging->isEnabled()) {
            ui->statusBar->showMessage("Open failed, simulator currently debugging", 4000);
            return false;
        }
        auto fileEvent = static_cast<QFileOpenEvent *>(event)->file();
        if(fileEvent.endsWith("pepl", Qt::CaseInsensitive)) {
            loadFile(fileEvent, Enu::EPane::EListing);
            return true;
        }
        else if(fileEvent.endsWith("pepo", Qt::CaseInsensitive)) {
            loadFile(fileEvent, Enu::EPane::EObject);
            return true;
        }
        else if(fileEvent.endsWith("pep", Qt::CaseInsensitive)) {
            loadFile(fileEvent, Enu::EPane::ESource);
            return true;
        }
    }
    return false;
}

void AsmMainWindow::connectViewUpdate()
{
    connect(memDevice.get(), &MainMemory::changed, ui->memoryWidget, &MemoryDumpPane::onMemoryChanged, Qt::ConnectionType::UniqueConnection);
    connect(memDevice.get(), &MainMemory::changed, ui->memoryTracePane, &NewMemoryTracePane::onMemoryChanged, Qt::ConnectionType::UniqueConnection);
    connect(this, &AsmMainWindow::simulationUpdate, ui->memoryWidget, &MemoryDumpPane::updateMemory, Qt::UniqueConnection);
    connect(this, &AsmMainWindow::simulationUpdate, ui->memoryTracePane, &NewMemoryTracePane::onMemoryChanged, Qt::UniqueConnection);
    connect(this, &AsmMainWindow::simulationUpdate, ui->asmCpuPane, &AsmCpuPane::onSimulationUpdate, Qt::UniqueConnection);
    connect(this, &AsmMainWindow::simulationUpdate, this, &AsmMainWindow::handleDebugButtons, Qt::UniqueConnection);
    connect(this, &AsmMainWindow::simulationUpdate, this, static_cast<void(AsmMainWindow::*)()>(&AsmMainWindow::highlightActiveLines), Qt::UniqueConnection);
    // If application is running, active lines shouldn't be highlighted at the begin of the instruction, as this would be misleading.
    connect(this, &AsmMainWindow::simulationStarted, this, static_cast<void(AsmMainWindow::*)()>(&AsmMainWindow::highlightActiveLines), Qt::UniqueConnection);
}

void AsmMainWindow::disconnectViewUpdate()
{
    disconnect(memDevice.get(), &MainMemory::changed, ui->memoryWidget,&MemoryDumpPane::onMemoryChanged);
    disconnect(memDevice.get(), &MainMemory::changed, ui->memoryTracePane, &NewMemoryTracePane::onMemoryChanged);
    disconnect(this, &AsmMainWindow::simulationUpdate, ui->memoryWidget, &MemoryDumpPane::updateMemory);
    disconnect(this, &AsmMainWindow::simulationUpdate, ui->memoryTracePane, &NewMemoryTracePane::onMemoryChanged);
    disconnect(this, &AsmMainWindow::simulationUpdate, ui->asmCpuPane, &AsmCpuPane::onSimulationUpdate);
    disconnect(this, &AsmMainWindow::simulationUpdate, this, &AsmMainWindow::handleDebugButtons);
    disconnect(this, &AsmMainWindow::simulationUpdate, this, static_cast<void(AsmMainWindow::*)()>(&AsmMainWindow::highlightActiveLines));
    disconnect(this, &AsmMainWindow::simulationStarted, this, static_cast<void(AsmMainWindow::*)()>(&AsmMainWindow::highlightActiveLines));
}

void AsmMainWindow::readSettings()
{
    QSettings settings;

    settings.beginGroup("MainWindow");

    // Restore screen dimensions
    QByteArray readGeometry = settings.value("geometry", saveGeometry()).toByteArray();
    restoreGeometry(readGeometry);

    // Restore last used font
    QVariant val = settings.value("font", codeFont);
    codeFont = qvariant_cast<QFont>(val);
    emit fontChanged(codeFont);

    //Restore last used file path
    curPath = settings.value("filePath", QDir::homePath()).toString();
    // Restore dark mode state
    onDarkModeChanged();
    settings.endGroup();
    //Handle reading for all children
    ui->assemblerPane->readSettings(settings);

}

void AsmMainWindow::writeSettings()
{
    QSettings settings;
    settings.beginGroup("MainWindow");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("font", codeFont);
    settings.setValue("filePath", curPath);
    settings.endGroup();

    //Handle writing for all children
    ui->assemblerPane->writeSettings(settings);

}

// Save methods
bool AsmMainWindow::save(Enu::EPane which)
{
    bool retVal = true;
    /* For each pane, first check if there is already a file associated with the pane.
     * if there is not, then pass control to the save as function.
     * If there is a file, then attempt save to it, then remove any modified flags from the pane if it succeded.
     */
    switch(which) {
    case Enu::EPane::ESource:
        if(ui->assemblerPane->getFileName(which).absoluteFilePath().isEmpty()) {
            retVal = saveAsFile(which);
        }
        else retVal = saveFile(ui->assemblerPane->getFileName(which).absoluteFilePath(), which);
        if(retVal) ui->assemblerPane->setModified(which, false);
        break;
    case Enu::EPane::EObject:
        if(ui->assemblerPane->getFileName(which).absoluteFilePath().isEmpty()) {
            retVal = saveAsFile(which);
        }
        if(retVal) ui->assemblerPane->setModified(which, false);
        break;
    case Enu::EPane::EListing:
        if(ui->assemblerPane->getFileName(Enu::EPane::EListing).absoluteFilePath().isEmpty()) {
            retVal = saveAsFile(Enu::EPane::EListing);
        }
        break;
    default:
        // Provided a default - even though it should never occur -
        // to silence compiler warnings.
        break;
    }
    return retVal;
}

bool AsmMainWindow::maybeSave()
{
    static QMetaEnum metaenum = QMetaEnum::fromType<Enu::EPane>();
    bool retVal = true;
    // Iterate over all the panes, and attempt to save any modified panes before closing.
    for(int it = 0; it < metaenum.keyCount(); it++) {
        retVal = retVal && maybeSave(static_cast<Enu::EPane>(metaenum.value(it)));
    }
    return retVal;
}

bool AsmMainWindow::maybeSave(Enu::EPane which)
{
    const QString dlgTitle = "Pep/9 Micro";
    const QString msgEnd = "The %1 has been modified.\nDo you want to save your changes?";
    const QString sourceText = msgEnd.arg("assembler source");
    const QString objectText = msgEnd.arg("object code");
    const QString microcodeText = msgEnd.arg("microcode");
    const auto buttons = QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel;
    QMessageBox::StandardButton ret;
    bool retVal = true;
    switch(which)
    {
    /*
     * There is no attempt to save the listing pane, since it can be recreated from the source code.
     */
    case Enu::EPane::ESource:
        if(ui->assemblerPane->isModified(which)) {
            ret = QMessageBox::warning(this, dlgTitle, sourceText, buttons);
            if (ret == QMessageBox::Save)
                retVal = save(Enu::EPane::ESource);
            else if (ret == QMessageBox::Cancel)
                retVal = false;
        }
        break;
    case Enu::EPane::EObject:
        if(ui->assemblerPane->isModified(which)) {
            ret = QMessageBox::warning(this, dlgTitle, objectText, buttons);
            if (ret == QMessageBox::Save)
                retVal = save(Enu::EPane::EObject);
            else if (ret == QMessageBox::Cancel)
                retVal = false;
        }
        break;
    default:
        break;
    }
    return retVal;
}

void AsmMainWindow::loadFile(const QString &fileName, Enu::EPane which)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Pep/9 Micro"), tr("Cannot read file %1:\n%2.")
                             .arg(fileName).arg(file.errorString()));
        return;
    }

    QTextStream in(&file);
    in.setCodec(QTextCodec::codecForName("ISO 8859-1"));
    QApplication::setOverrideCursor(Qt::WaitCursor);
    /*
     * For each pane, switch to the tab which contains that pane.
     * Give that pane focus, notify the pane of the new file's name,
     * and set the pane's text to the contents of the file.
     * Ensure that the modified (*) flag is not set.
     */
    switch(which) {
    case Enu::EPane::ESource:
        ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->assemblerTab));
        ui->assemblerPane->loadSourceFile(fileName, in.readAll());
        emit ui->actionDebug_Remove_All_Assembly_Breakpoints->trigger();
        break;
    case Enu::EPane::EObject:
        ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->assemblerTab));
        ui->assemblerPane->loadObjectFile(fileName, in.readAll());
        break;
    default:
        // Provided a default - even though it should never occur -
        // to silence compiler warnings.
        break;
    }
    curPath = QFileInfo(file).dir().absolutePath();
    statusBar()->showMessage(tr("File loaded"), 4000);
    QApplication::restoreOverrideCursor();
}

bool AsmMainWindow::saveFile(Enu::EPane which)
{
    QString fileName;
    switch(which)
    {
    //Get the filename associated with the dialog
    // With new assembler source pane, all work inside that class.
    case Enu::EPane::ESource:
        [[fallthrough]];
    case Enu::EPane::EObject:
        [[fallthrough]];
    case Enu::EPane::EListing:
        fileName = ui->assemblerPane->getFileName(which).absoluteFilePath();
        break;
    default:
        // Provided a default - even though it should never occur -
        // to silence compiler warnings.
        break;
    }
    return saveFile(fileName, which);
}

bool AsmMainWindow::saveFile(const QString &fileName, Enu::EPane which)
{

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Pep/9 Micro"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }

    QTextStream out(&file);
    out.setCodec(QTextCodec::codecForName("ISO 8859-1"));
    QApplication::setOverrideCursor(Qt::WaitCursor);

    // Messages for status bar.
    static const QString msgSource = "Source saved";
    static const QString msgObject = "Object code saved";
    static const QString msgListing = "Listing saved";
    static const QString msgMicro = "Microcode saved";
    const QString *msgOutput; // Mutable pointer to const data.
    switch(which)
    {
    // Set the output text & the pointer to the status bar message.
    case Enu::EPane::ESource:
        out << ui->assemblerPane->getPaneContents(which);
        msgOutput = &msgSource;
        break;
    case Enu::EPane::EObject:
        out << ui->assemblerPane->getPaneContents(which);
        msgOutput = &msgObject;
        break;
    case Enu::EPane::EListing:
        out << ui->assemblerPane->getPaneContents(which);
        msgOutput = &msgListing;
        break;
    default:
        // Provided a default - even though it should never occur -
        // to silence compiler warnings.
        // An invalid pane can't be saved, so return false
        return false;
    }
    QApplication::restoreOverrideCursor();
    curPath = QFileInfo(file).dir().absolutePath();
    statusBar()->showMessage(*msgOutput, 4000);
    return true;
}

bool AsmMainWindow::saveAsFile(Enu::EPane which)
{
    // Default filenames for each pane.
    static const QString defSourceFile = "untitled.pep";
    static const QString defObjectFile = "untitled.pepo";
    static const QString defListingFile = "untitled.pepl";
    QString usingFile;

    // Titles for each pane.
    static const QString titleBase = "Save %1";
    static const QString sourceTitle = titleBase.arg("Assembler Source Code");
    static const QString objectTitle = titleBase.arg("Object Code");
    static const QString listingTitle = titleBase.arg("Assembler Listing");
    const QString *usingTitle;

    // Patterns for source code files.
    static const QString sourceTypes = "Pep/9 Source (*.pep *.txt)";
    static const QString objectTypes = "Pep/9 Object (*.pepo *.txt)";
    static const QString listingTypes = "Pep/9 Listing (*.pepl)";
    const QString *usingTypes;

    QFileInfo file;
    /*
     * For each pane, get the file associated with each pane, or the default if there is none.
     * Set the title and source code patterns for the pane.
     */
    switch(which)
    {
    case Enu::EPane::ESource:
        file = ui->assemblerPane->getFileName(which);
        if(file.fileName().isEmpty()) {
            usingFile = QDir(curPath).absoluteFilePath(defSourceFile);
        }
        else usingFile = file.fileName();
        usingTitle = &sourceTitle;
        usingTypes = &sourceTypes;
        break;
    case Enu::EPane::EObject:
        file = ui->assemblerPane->getFileName(which);
        if(file.fileName().isEmpty()) {
            usingFile = QDir(curPath).absoluteFilePath(defObjectFile);
        }
        else usingFile = file.fileName();
        usingTitle = &objectTitle;
        usingTypes = &objectTypes;
        break;
    case Enu::EPane::EListing:
        file = ui->assemblerPane->getFileName(which);
        if(file.fileName().isEmpty()) {
            usingFile = QDir(curPath).absoluteFilePath(defListingFile);
        }
        else usingFile = file.fileName();
        usingTitle = &listingTitle;
        usingTypes = &listingTypes;
        break;
    default:
        // Provided a default - even though it should never occur -
        // to silence compiler warnings.
        // An invalid pane can't be saved, so return false.
        return false;
    }

    QString fileName = QFileDialog::getSaveFileName(
                this,
                *usingTitle,
                usingFile,
                *usingTypes);
    if (fileName.isEmpty()) {
        return false;
    }
    else if (saveFile(fileName, which)) {
        // If the file is successfully saved, then change the file associated with the pane.
        QFileInfo fileInfo = QFileInfo(fileName);
        switch(which)
        {
        case Enu::EPane::ESource:
            ui->assemblerPane->setFileName(which, fileInfo);
            break;
        case Enu::EPane::EObject:
            ui->assemblerPane->setFileName(which, fileInfo);
            break;
        case Enu::EPane::EListing:
            ui->assemblerPane->setFileName(which, fileInfo);
            break;
        default:
            // Provided a default - even though it should never occur -
            // to silence compiler warnings.
            break;
        }
        return true;
    }
    else return false;
}

QString AsmMainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void AsmMainWindow::print(Enu::EPane which)
{
    //Don't use a pointer for the text, because toPlainText() returns a temporary object
    QString text;
    const QString base = "Print %1";
    const QString source = base.arg("Assembler Source Code");
    const QString object = base.arg("Object Code");
    const QString listing = base.arg("Assembler Listing");
    const QString micro = base.arg("Microcode");
    const QString *title = &source; //Pointer to the title of the dialog
    /*
     * Store text-to-print and dialog title based on which pane is to be printed
     */
    QPrinter printer(QPrinter::HighResolution);
    QTextDocument document;
    // Create highlighters (which may or may not be needed),
    // so that the documents may be properly highlighted.
    QSyntaxHighlighter* hi = nullptr;
    PepASMHighlighter* asHi;
    switch(which)
    {
    case Enu::EPane::ESource:
        title = &source;
        document.setPlainText(ui->assemblerPane->getPaneContents(which));
        asHi = new PepASMHighlighter(PepColors::lightMode, &document);
        hi = asHi;
        hi->rehighlight();
        break;
    case Enu::EPane::EObject:
        title = &object;
        document.setPlainText(ui->assemblerPane->getPaneContents(which));
        break;
    case Enu::EPane::EListing:
        title = &listing;
        document.setPlainText(ui->assemblerPane->getPaneContents(which));
        asHi = new PepASMHighlighter(PepColors::lightMode, &document);
        hi = asHi;
        hi->rehighlight();
        break;
    default:
        // Provided a default - even though it should never occur -
        // to silence compiler warnings.
        break;
    }
    QPrintDialog *dialog = new QPrintDialog(&printer, this);

    dialog->setWindowTitle(*title);
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
    if(hi != nullptr) {
        hi->deleteLater();
    }
}

void AsmMainWindow::assembleDefaultOperatingSystem()
{
    // Need to assemble operating system.
    QString defaultOSText = Pep::resToString(":/help-asm/figures/pep9os.pep", false);
    // If there is text, attempt to assemble it
    if(!defaultOSText.isEmpty()) {
        QSharedPointer<AsmProgram> prog;
        auto elist = QList<QPair<int, QString>>();
        IsaAsm assembler(*programManager);
        if(assembler.assembleOperatingSystem(defaultOSText, true, prog, elist)) {
            programManager->setOperatingSystem(prog);
        }
        // If the operating system failed to assembly, we can't progress any further.
        // All application functionality depends on the operating system being defined.
        else {
            qDebug() << "OS failed to assemble.";
            auto textList = defaultOSText.split("\n");
            for(auto errorPair : elist) {
                qDebug() << textList[errorPair.first] << errorPair.second << endl;
            }
            throw std::logic_error("The default operating system failed to assemble.");
        }
    }
    // If the operating system couldn't be found, we can't progress any further.
    // All application functionality depends on the operating system being defined.
    else {
        throw std::logic_error("Could not find default operating system.");
    }
}

// Helper function that turns hexadecimal object code into a vector of unsigned characters, which is easier to copy into memory.
QVector<quint8> convertObjectCodeToIntArray(QString line, bool &success)
{
    success = true;
    bool ok = true;
    quint8 temp;
    QVector<quint8> output;
    for(QString byte : line.split(" ")) {
        // toShort(...) should never throw any errors, so there should be no concerns if byte is not a hex constant.
        temp = static_cast<quint8>(byte.toShort(&ok, 16));
        // There could be a loss in precision if given text outside the range of an uchar but in range of a ushort.
        if(ok && byte.length()>0) output.append(temp);
        else ok = false;
    }
    return output;
}

void AsmMainWindow::loadOperatingSystem()
{
    QVector<quint8> values;
    quint16 startAddress;
    values = programManager->getOperatingSystem()->getObjectCode();
    startAddress = programManager->getOperatingSystem()->getBurnAddress();
    // Get addresses for I/O chips
    auto osSymTable = programManager->getOperatingSystem()->getSymbolTable();
    quint16 charIn, charOut;
    charIn = static_cast<quint16>(osSymTable->getValue("charIn")->getValue());
    charOut = static_cast<quint16>(osSymTable->getValue("charOut")->getValue());
    ui->ioWidget->setInputChipAddress(charIn);
    ui->ioWidget->setOutputChipAddress(charOut);

    // Construct main memory according to the current configuration of the operating system.
    QList<MemoryChipSpec> list;
    // Make sure RAM will fill any accidental gaps in the memory map by making it go
    // right up to the start of the operating system.
    list.append({AMemoryChip::ChipTypes::RAM, 0, startAddress});
    list.append({AMemoryChip::ChipTypes::ROM, startAddress, static_cast<quint32>(values.length())});
    // Character input / output ports are only 1 byte wide by design.
    list.append({AMemoryChip::ChipTypes::IDEV, charIn, 1});
    list.append({AMemoryChip::ChipTypes::ODEV, charOut, 1});
    memDevice->constructMemoryDevice(list);

    memDevice->autoUpdateMemoryMap(true);
    // Make sure nothing is in memory before loading operating system.
    memDevice->clearMemory();
    memDevice->loadValues(programManager->getOperatingSystem()->getBurnAddress(), values);
}

bool AsmMainWindow::loadObjectCodeProgram()
{
    // Get the object code, and convert it to an integer array.
    QString lines = ui->assemblerPane->getPaneContents(Enu::EPane::EObject);
    QVector<quint8> data;
    // Temp to track each conversion, and tracker for cumulative success.
    bool temp, convertSuccess = true;
    // If there are no lines, there is no data.
    bool hadData = !lines.isEmpty();
    for(auto line : lines.split("\n")) {
        // Check if line is empty, if it is, stop processing line.
        QString trimmed = line.trimmed();
        if(trimmed.isEmpty()){
            continue;
        } else {
            hadData = true;
            data.append(convertObjectCodeToIntArray(line, temp));
            // Keep track of rolling success status.
            convertSuccess &= temp;
        }

    }
    // If there was no data or a data conversion failed, display an error message.
    if(!hadData || !convertSuccess) {
        ui->statusBar->showMessage("Load failed", 4000);
    }
    // Otherwise load the values correctly.
    else {
        // Imitate the behavior of Pep/9.
        // Always copy bytes to memory starting at 0x0000, instead of invoking the loader.
        memDevice->loadValues(0, data);
        ui->statusBar->showMessage("Load succeeded", 4000);
    }
    return hadData;
}

void AsmMainWindow::set_Obj_Listing_filenames_from_Source()
{
    ui->assemblerPane->setFilesFromSource();
}

void AsmMainWindow::debugButtonEnableHelper(const int which)
{
    // Crack the parameter using DebugButtons to properly enable
    // and disable all buttons related to debugging and running.
    // Build Actions
    ui->actionBuild_Assemble->setEnabled(which & DebugButtons::BUILD_ASM);
    ui->actionEdit_Remove_Error_Assembler->setEnabled(which & DebugButtons::BUILD_ASM);
    ui->actionEdit_Format_Assembler->setEnabled((which & DebugButtons::BUILD_ASM));
    ui->actionBuild_Load_Object->setEnabled(which & DebugButtons::BUILD_ASM);

    // Debug & Run Actions
    ui->actionBuild_Run->setEnabled(which & DebugButtons::RUN);
    ui->actionBuild_Execute->setEnabled(which);
    ui->actionBuild_Run_Object->setEnabled(which & DebugButtons::RUN_OBJECT);
    ui->actionDebug_Start_Debugging->setEnabled(which & DebugButtons::DEBUG);
    ui->actionDebug_Start_Debugging_Object->setEnabled(which & DebugButtons::DEBUG_OBJECT);
    ui->actionDebug_Start_Debugging_Loader->setEnabled(which & DebugButtons::DEBUG_LOADER);
    ui->actionDebug_Interupt_Execution->setEnabled(which & DebugButtons::INTERRUPT);
    ui->actionDebug_Continue->setEnabled(which & DebugButtons::CONTINUE);
    ui->actionDebug_Stop_Debugging->setEnabled(which & DebugButtons::STOP);
    ui->actionDebug_Single_Step_Assembler->setEnabled(which & DebugButtons::STEP_OVER_ASM);
    ui->actionDebug_Step_Over_Assembler->setEnabled(which & DebugButtons::STEP_OVER_ASM);
    ui->actionDebug_Step_Into_Assembler->setEnabled(which & DebugButtons::STEP_INTO_ASM);
    ui->actionDebug_Step_Out_Assembler->setEnabled(which & DebugButtons::STEP_OUT_ASM);

    // File open & new actions
    ui->actionFile_New_Asm->setEnabled(which & DebugButtons::OPEN_NEW);
    ui->actionFile_Open->setEnabled(which & DebugButtons::OPEN_NEW);

    // Operating Assembly / Mnemonic Redefinition actions
    ui->actionSystem_Redefine_Mnemonics->setEnabled(which & DebugButtons::INSTALL_OS);
    ui->actionSystem_Reinstall_Default_OS->setEnabled(which & DebugButtons::INSTALL_OS);
    ui->actionSystem_Assemble_Install_New_OS->setEnabled(which & DebugButtons::INSTALL_OS);

    // System actions
    ui->actionSystem_Clear_CPU->setEnabled(which & DebugButtons::CLEAR);
    ui->actionSystem_Clear_Memory->setEnabled(which & DebugButtons::CLEAR);

    // If the user starts simulating while the redefine mnemonics dialog is open,
    // force it to close so that the user can't change any mnemonics at runtime.
    // Also explictly call redefine_Mnemonics_closed(), since QDialog::closed is
    // not emitted when QDialog::hide() is invoked.
    if(!(which & DebugButtons::INSTALL_OS) && redefineMnemonicsDialog->isVisible()) {
        redefineMnemonicsDialog->hide();
        redefine_Mnemonics_closed();
    }
}

void AsmMainWindow::highlightActiveLines()
{
    ui->memoryWidget->clearHighlight();
    ui->memoryWidget->highlight();
    ui->asmProgramTracePane->updateSimulationView();
}

bool AsmMainWindow::initializeSimulation()
{
    // Clear data models & application views
    controlSection->onResetCPU();
    controlSection->initCPU();
    ui->asmCpuPane->clearCpu();

    // No longer emits simulationStarted(), as this could trigger extra screen painting that is unwanted.
    return true;
}

void AsmMainWindow::onUpdateCheck(int val)
{
    val = (int)val; //Ugly way to get rid of unused paramter warning without actually modifying the parameter.
    // Dummy to handle update checking code
}

// File MainWindow triggers
void AsmMainWindow::on_actionFile_New_Asm_triggered()
{
    // Try to save source code before clearing it, the object code pane, and the listing pane.
    if (maybeSave(Enu::EPane::ESource)) {
        ui->actionDebug_Remove_All_Assembly_Breakpoints->trigger();
        ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->assemblerTab));
        ui->assemblerPane->newProject();
        ui->asmProgramTracePane->clearSourceCode();
        programManager->setUserProgram(nullptr);
        ui->ioWidget->onClear();
        emit ui->actionDebug_Remove_All_Assembly_Breakpoints->trigger();
        handleDebugButtons();
    }
}

void AsmMainWindow::on_actionFile_Open_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(
                this,
                "Open text file",
                curPath,
                "Pep/9 files (*.pep *.pepo *.pepl *.txt)");
    // If we don't recognize an extension, assume it is an assembler source document
    Enu::EPane which = Enu::EPane::ESource;
    // Depending on the file ending, change which pane will be loaded into.
    if (!fileName.isEmpty()) {
        if(fileName.endsWith("pep", Qt::CaseInsensitive)) which = Enu::EPane::ESource;
        else if(fileName.endsWith("pepo", Qt::CaseInsensitive)) which = Enu::EPane::EObject;
        else if(fileName.endsWith("pepl", Qt::CaseInsensitive)) which = Enu::EPane::EListing;
        if(maybeSave(which)) {
            loadFile(fileName, which);
        }
        curPath = QFileInfo(fileName).absolutePath();
    }
}

bool AsmMainWindow::on_actionFile_Save_Asm_triggered()
{
    return save(Enu::EPane::ESource);
}

bool AsmMainWindow::on_actionFile_Save_Asm_Source_As_triggered()
{
    return saveAsFile(Enu::EPane::ESource);
}

bool AsmMainWindow::on_actionFile_Save_Object_Code_As_triggered()
{
    return saveAsFile(Enu::EPane::EObject);
}

bool AsmMainWindow::on_actionFile_Save_Assembler_Listing_As_triggered()
{
    return saveAsFile(Enu::EPane::EListing);
}

void AsmMainWindow::on_actionFile_Print_Assembler_Source_triggered()
{
    print(Enu::EPane::ESource);
}

void AsmMainWindow::on_actionFile_Print_Object_Code_triggered()
{
    print(Enu::EPane::EObject);
}

void AsmMainWindow::on_actionFile_Print_Assembler_Listing_triggered()
{
    print(Enu::EPane::EListing);
}

// Edit MainWindow triggers

void AsmMainWindow::on_actionEdit_Undo_triggered()
{
    if(ui->assemblerPane->isAncestorOf(focusWidget())) {
        ui->assemblerPane->undo();
    }
    else if (ui->ioWidget->isAncestorOf(QApplication::focusWidget())) {
        ui->ioWidget->undo();
    }
}

void AsmMainWindow::on_actionEdit_Redo_triggered()
{
    if(ui->assemblerPane->isAncestorOf(focusWidget())) {
        ui->assemblerPane->redo();
    }
    else if (ui->ioWidget->isAncestorOf(QApplication::focusWidget())) {
        ui->ioWidget->redo();
    }
}

void AsmMainWindow::on_actionEdit_Cut_triggered()
{
    if(ui->assemblerPane->isAncestorOf(focusWidget())) {
        ui->assemblerPane->cut();
    }
    else if (ui->ioWidget->isAncestorOf(QApplication::focusWidget())) {
        ui->ioWidget->cut();
    }
}

void AsmMainWindow::on_actionEdit_Copy_triggered()
{
    if (helpDialog->hasFocus()) {
        helpDialog->copy();
    }
    else if(ui->assemblerPane->isAncestorOf(focusWidget())) {
        ui->assemblerPane->copy();
    }
    else if (ui->ioWidget->isAncestorOf(QApplication::focusWidget())) {
        ui->ioWidget->copy();
    }
    else if(ui->memoryWidget->hasFocus()) {
        ui->memoryWidget->copy();
    }
    // other panes should not be able to copy
}

void AsmMainWindow::on_actionEdit_Paste_triggered()
{
    if(ui->assemblerPane->isAncestorOf(focusWidget())) {
        ui->assemblerPane->paste();
    }
    else if (ui->ioWidget->isAncestorOf(QApplication::focusWidget())) {
        ui->ioWidget->paste();
    }
    // other panes should not be able to paste
}

void AsmMainWindow::on_actionEdit_Format_Assembler_triggered()
{
    ui->assemblerPane->formatAssemblerCode();
}

void AsmMainWindow::on_actionEdit_Remove_Error_Assembler_triggered()
{
    ui->assemblerPane->removeErrorMessages();
}

void AsmMainWindow::on_actionEdit_Font_triggered()
{
    bool ok = false;
    QFont font  = QFontDialog::getFont(&ok, codeFont, this, "Set Source Code Font");
    if(ok) {
        codeFont = font;
        emit fontChanged(codeFont);
    }
}

void AsmMainWindow::on_actionEdit_Reset_font_to_Default_triggered()
{
    codeFont = QFont(Pep::codeFont, Pep::codeFontSize);
    emit fontChanged(codeFont);
}

//Build Events
bool AsmMainWindow::on_actionBuild_Assemble_triggered()
{
    ui->assemblerPane->assembleAsProgram();
    if(ui->assemblerPane->getAssemblerOutput()->success){
        programManager->setUserProgram(ui->assemblerPane->getAssemblerOutput()->prog);
        ui->asmProgramTracePane->onRemoveAllBreakpoints();
        controlSection->breakpointsRemoveAll();
        ui->asmProgramTracePane->setProgram(ui->assemblerPane->getAssemblerOutput()->prog);
        set_Obj_Listing_filenames_from_Source();
        ui->statusBar->showMessage("Assembly succeeded", 4000);
        handleDebugButtons();
        return true;
    }
    else {
#pragma message("May be redundant")
        ui->assemblerPane->clearPane(Enu::EPane::EObject);
        ui->assemblerPane->clearPane(Enu::EPane::EListing);
        ui->asmProgramTracePane->clearSourceCode();
        ui->asmProgramTracePane->onRemoveAllBreakpoints();
        // ui->pepCodeTraceTab->setCurrentIndex(0); // Make source code pane visible
        loadObjectCodeProgram();
        ui->statusBar->showMessage("Assembly failed", 4000);
        return false;
    }

}

void AsmMainWindow::on_actionBuild_Load_Object_triggered()
{
    loadOperatingSystem();
    loadObjectCodeProgram();
    ui->memoryWidget->refreshMemory();
    ui->memoryWidget->clearHighlight();
}

void AsmMainWindow::on_actionBuild_Execute_triggered()
{
    // Do not load operating system, as this will erase existing bytes in memory.
    debugState = DebugState::RUN;
    if (initializeSimulation()) {
        disconnectViewUpdate();
        emit simulationStarted();
        ui->memoryWidget->clearHighlight();
        ui->memoryWidget->refreshMemory();
        controlSection->onSimulationStarted();
        controlSection->onRun();
        connectViewUpdate();
    }
    else {
        debugState = DebugState::DISABLED;
    }
    // If the simulator finished, then propogate that information to connect components.
    if(controlSection->getExecutionFinished()) {
        debugState = DebugState::DISABLED;
        onSimulationFinished();
        emit simulationFinished();
    }
    // Otherwise, the simulator paused execution, so don't explicitly terminate
    // the simulator.
    else {
        handleDebugButtons();
        emit simulationUpdate();
    }
}

void AsmMainWindow::on_actionBuild_Run_triggered()
{
    if(!on_actionBuild_Assemble_triggered()) return;
    loadOperatingSystem();
    loadObjectCodeProgram();
    debugState = DebugState::RUN;
    if (initializeSimulation()) {
        ui->asmProgramTracePane->startSimulationView();
        disconnectViewUpdate();
        memDevice->clearBytesSet();
        memDevice->clearBytesWritten();
        emit simulationStarted();
        ui->memoryWidget->updateMemory();
        ui->memoryTracePane->updateTrace();
        controlSection->onSimulationStarted();
        controlSection->onRun();
        connectViewUpdate();

    }
    else {
        debugState = DebugState::DISABLED;
    }
    // If the simulator finished, then propogate that information to connect components.
    if(controlSection->getExecutionFinished()) {
        debugState = DebugState::DISABLED;
        onSimulationFinished();
    }
    // Otherwise, the simulator paused execution, so don't explicitly terminate
    // the simulator.
    else {
        handleDebugButtons();
        emit simulationUpdate();
    }
}

void AsmMainWindow::on_actionBuild_Run_Object_triggered()
{
    // Must load operating system, as this will not be done by
    // on_actionBuild_Execute_triggered().
    loadOperatingSystem();
    if(loadObjectCodeProgram()) {
        on_actionBuild_Execute_triggered();
    }
}

// Debug slots

void AsmMainWindow::handleDebugButtons()
{
    bool enable_into = controlSection->canStepInto();
    // Disable button stepping if waiting on IO
    bool waiting_io =
            // If the simulation is running
            debugState != DebugState::DISABLED
            // and there is an operating system
            && !programManager->getOperatingSystem().isNull()
            // and it defined the character input device
            && !programManager->getOperatingSystem()->getSymbolTable()->getValue("charIn").isNull();
    if(waiting_io) {
       quint16 address = programManager->getOperatingSystem()->getSymbolTable()->getValue("charIn")->getValue();
       InputChip *chip = static_cast<InputChip*>(memDevice->chipAt(address));
       waiting_io &= chip->waitingForInput(address-chip->getBaseAddress());
    }
    int enabledButtons = 0;
    switch(debugState)
    {
    case DebugState::DISABLED:
        enabledButtons = DebugButtons::RUN | DebugButtons::RUN_OBJECT| DebugButtons::DEBUG | DebugButtons::DEBUG_OBJECT | DebugButtons::DEBUG_LOADER;
        enabledButtons |= DebugButtons::BUILD_ASM;
        enabledButtons |= DebugButtons::OPEN_NEW | DebugButtons::INSTALL_OS;
        enabledButtons |= DebugButtons::CLEAR;
        break;
    case DebugState::RUN:
        enabledButtons = DebugButtons::STOP | DebugButtons::INTERRUPT;
        break;
    case DebugState::DEBUG_ISA:
        enabledButtons = DebugButtons::INTERRUPT | DebugButtons::STOP | DebugButtons::CONTINUE*(!waiting_io);
        enabledButtons |= DebugButtons::STEP_OUT_ASM*(!waiting_io);
        enabledButtons |= DebugButtons::STEP_OVER_ASM*(!waiting_io);
        enabledButtons |= DebugButtons::STEP_INTO_ASM*(enable_into * !waiting_io);
        break;
    case DebugState::DEBUG_RESUMED:
        enabledButtons = DebugButtons::INTERRUPT | DebugButtons::STOP;
        break;
    default:
        break;
    }
    debugButtonEnableHelper(enabledButtons);
}

bool AsmMainWindow::on_actionDebug_Start_Debugging_triggered()
{  
    if(!on_actionBuild_Assemble_triggered()) return false;
    // The function on_actionDebug_Start_Debugging_Object_triggered() does not switch to
    // the debugger tab. This is intentional, since object code does not necessarily
    // correspond to the last assembled program.
    else if(!on_actionDebug_Start_Debugging_Object_triggered()) return false;
    ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->debuggerTab));
    return true;

}

bool AsmMainWindow::on_actionDebug_Start_Debugging_Object_triggered()
{
    loadOperatingSystem();
    loadObjectCodeProgram();
    connectViewUpdate();
    debugState = DebugState::DEBUG_ISA;
    ui->asmProgramTracePane->startSimulationView();
    if(initializeSimulation()) {
        emit simulationStarted();
        controlSection->onSimulationStarted();
        controlSection->enableDebugging();
        controlSection->breakpointsSet(programManager->getBreakpoints());
        memDevice->clearBytesSet();
        memDevice->clearBytesWritten();
        ui->memoryWidget->updateMemory();
        // Force re-highlighting on memory to prevent last run's data
        // from remaining highlighted. Otherwise, if the last program
        // was "run", then every byte that it modified will be highlighted
        // upon starting the simulation.
        highlightActiveLines();
        ui->memoryTracePane->updateTrace();
        // Give focus to the trace pane so that once can immediately
        // start hitting "return" to trigger single steps.
        ui->asmProgramTracePane->setFocus(Qt::FocusReason::MouseFocusReason);
        return true;
    }
    return false;
}

bool AsmMainWindow::on_actionDebug_Start_Debugging_Loader_triggered()
{
    if(!on_actionBuild_Assemble_triggered()) return false;
    memDevice->clearMemory();
    loadOperatingSystem();
    // Copy object code to batch input pane and make it the active input pane
    QString objcode = ui->assemblerPane->getPaneContents(Enu::EPane::EObject);
    // Replace all new line characters with spaces
    objcode = objcode.replace('\n', ' ');
    ui->ioWidget->setBatchInput(objcode);
    ui->ioWidget->setActivePane(Enu::EPane::EBatchIO);
    if(!on_actionDebug_Start_Debugging_Object_triggered()) return false;
    ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->debuggerTab));
    quint16 sp, pc;
    memDevice->readWord(programManager->getOperatingSystem()->getBurnValue() - 9, sp);
    memDevice->readWord(programManager->getOperatingSystem()->getBurnValue() - 3, pc);
    // Write SP, PC to simulator
    controlSection->getRegisterBank().writePCStart(pc);
    controlSection->getRegisterBank().writeRegisterWord(Enu::CPURegisters::PC, pc);
    controlSection->getRegisterBank().writeRegisterWord(Enu::CPURegisters::SP, sp);
    // Memory has been cleared, but will not display as such unless explicitly refreshed.
    ui->memoryWidget->refreshMemory();
    // Give focus to the trace pane so that once can immediately
    // start hitting "return" to trigger single steps.
    ui->asmProgramTracePane->setFocus(Qt::FocusReason::MouseFocusReason);
    emit simulationUpdate();
    return true;
}

void AsmMainWindow::on_actionDebug_Stop_Debugging_triggered()
{
    connectViewUpdate();
    highlightActiveLines();
    debugState = DebugState::DISABLED;
    // Handle case of execution being canceled during IO
    memDevice->clearIO();
    reenableUIAfterInput();
    ui->ioWidget->cancelWaiting();
    ui->asmProgramTracePane->clearSimulationView();
    handleDebugButtons();
    controlSection->onSimulationFinished();
    emit simulationFinished();
}

void AsmMainWindow::on_actionDebug_Single_Step_Assembler_triggered()
{
    quint8 is;
    quint16 addr = controlSection->getCPURegWordStart(Enu::CPURegisters::PC);
    memDevice->getByte(addr, is);
    if(controlSection->canStepInto()
            && Pep::isTrapMap[Pep::decodeMnemonic[is]]) {
        on_actionDebug_Step_Over_Assembler_triggered();
    } else if (controlSection->canStepInto()) {
        on_actionDebug_Step_Into_Assembler_triggered();
    } else {
        on_actionDebug_Step_Over_Assembler_triggered();
    }

}

void AsmMainWindow::on_actionDebug_Interupt_Execution_triggered()
{
    // Enable debugging in CPU and then temporarily pause execution.
    controlSection->enableDebugging();
    controlSection->forceBreakpoint(Enu::BreakpointTypes::ASSEMBLER);
    connectViewUpdate();
    debugState = DebugState::DEBUG_ISA;
    highlightActiveLines();
    handleDebugButtons();
    // Switch to debugger tab if it is not already visibile.
    ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->debuggerTab));
}

void AsmMainWindow::on_actionDebug_Continue_triggered()
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

void AsmMainWindow::on_actionDebug_Step_Over_Assembler_triggered()
{
    ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->debuggerTab));
    //Disconnect any drawing functions, since very many steps might execute, and it would be wasteful to update the UI
    disconnectViewUpdate();
    // Since stepOver() could loop infinitely, disable all stepping buttons
    // until this step finishes.
    debugState = DebugState::DEBUG_RESUMED;
    handleDebugButtons();
    controlSection->stepOver();
    // The step has finished. The program may have been canceled
    // during the execution of that step, so only transition to
    // debugging at the ISA level if the simulation is still ongoing.
    if(debugState != DebugState::DISABLED) {
            // Actions will be refreshed on simulationUpdate.
            debugState = DebugState::DEBUG_ISA;
    }
    connectViewUpdate();
    emit simulationUpdate();
}

void AsmMainWindow::on_actionDebug_Step_Into_Assembler_triggered()
{
    debugState = DebugState::DEBUG_ISA;
    ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->debuggerTab));
    controlSection->stepInto();
    emit simulationUpdate();
}

void AsmMainWindow::on_actionDebug_Step_Out_Assembler_triggered()
{
    debugState = DebugState::DEBUG_ISA;
    ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->debuggerTab));
    //Disconnect any drawing functions, since very many steps might execute, and it would be wasteful to update the UI
    disconnectViewUpdate();
    // Since stepOut() could loop infinitely, disable all stepping buttons
    // until this step finishes.
    debugState = DebugState::DEBUG_RESUMED;
    handleDebugButtons();
    controlSection->stepOut();
    // The step has finished. The program may have been canceled
    // during the execution of that step, so only transition to
    // debugging at the ISA level if the simulation is still ongoing.
    if(debugState != DebugState::DISABLED) {
            // Actions will be refreshed on simulationUpdate.
            debugState = DebugState::DEBUG_ISA;
    }
    connectViewUpdate();
    emit simulationUpdate();
}

void AsmMainWindow::onASMBreakpointHit()
{
    debugState = DebugState::DEBUG_ISA;
    ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->debuggerTab));
}

void AsmMainWindow::onPaletteChanged(const QPalette &)
{
    onDarkModeChanged();
}

// System MainWindow triggers
void AsmMainWindow::on_actionSystem_Clear_CPU_triggered()
{
    controlSection->onResetCPU();
    ui->asmCpuPane->clearCpu();
}

void AsmMainWindow::on_actionSystem_Clear_Memory_triggered()
{
    memDevice->clearMemory();
    ui->memoryWidget->refreshMemory();
    ui->memoryWidget->clearHighlight();

}

void AsmMainWindow::on_actionSystem_Assemble_Install_New_OS_triggered()
{
    auto output = programManager->assembleOS(ui->assemblerPane->getPaneContents(Enu::EPane::ESource),false);
    if(output->success) {
        programManager->setOperatingSystem(output->prog);
        ui->assemblerPane->setPanesFromProgram(*output);
        ui->asmProgramTracePane->onRemoveAllBreakpoints();
        controlSection->breakpointsRemoveAll();
        set_Obj_Listing_filenames_from_Source();
        ui->statusBar->showMessage("Assembly succeeded, OS installed", 4000);
    }
    else {
        ui->assemblerPane->clearPane(Enu::EPane::EObject);
        ui->assemblerPane->clearPane(Enu::EPane::EListing);
        ui->asmProgramTracePane->clearSourceCode();
        ui->asmProgramTracePane->onRemoveAllBreakpoints();
        ui->statusBar->showMessage("Assembly failed, previous OS remains", 4000);
    }
    loadOperatingSystem();
    ui->memoryWidget->refreshMemory();
}

void AsmMainWindow::on_actionSystem_Reinstall_Default_OS_triggered()
{
    assembleDefaultOperatingSystem();
    loadOperatingSystem();
    ui->memoryWidget->refreshMemory();
}

void AsmMainWindow::on_actionSystem_Redefine_Mnemonics_triggered()
{
    redefineMnemonicsDialog->show();
}

void AsmMainWindow::redefine_Mnemonics_closed()
{
    // Propogate ASM-level instruction definition changes across the application.
    ui->assemblerPane->rebuildHighlightingRules();
    ui->asmProgramTracePane->rebuildHighlightingRules();
}

void AsmMainWindow::onSimulationFinished()
{
    QString errorString;
    on_actionDebug_Stop_Debugging_triggered();

    if(controlSection->hadErrorOnStep()) {
        QMessageBox::critical(
          this,
          tr("Pep/9 Micro"),
          controlSection->getErrorMessage());
        ui->statusBar->showMessage("Execution failed", 4000);
    }
    else ui->statusBar->showMessage("Execution finished", 4000);

}

void AsmMainWindow::onDarkModeChanged()
{
    isInDarkMode = inDarkMode();
    emit darkModeChanged(isInDarkMode, styleSheet());
}

// help:
void AsmMainWindow::on_actionHelp_triggered()
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

void AsmMainWindow::on_actionHelp_Writing_Programs_triggered()
{
    helpDialog->show();
    helpDialog->selectItem("Writing Programs");
}

void AsmMainWindow::on_actionHelp_Machine_Language_triggered()
{
    helpDialog->show();
    helpDialog->selectItem("Machine Language");
}

void AsmMainWindow::on_actionHelp_Assembly_Language_triggered()
{
    helpDialog->show();
    helpDialog->selectItem("Assembly Language");
}

void AsmMainWindow::on_actionHelp_Debugging_Assembly_triggered()
{
    helpDialog->show();
    helpDialog->selectItem("Debugging Programs");
}

void AsmMainWindow::on_actionHelp_Writing_Trap_Handlers_triggered()
{
    helpDialog->show();
    helpDialog->selectItem("Writing Trap Handlers");
}

void AsmMainWindow::on_actionHelp_Pep9Reference_triggered()
{
    helpDialog->show();
    helpDialog->selectItem("Pep/9 Reference");
}

void AsmMainWindow::on_actionHelp_Examples_triggered()
{
    helpDialog->show();
    helpDialog->selectItem("Examples");
}

void AsmMainWindow::on_actionHelp_Pep9_Operating_System_triggered()
{
    helpDialog->show();
    helpDialog->selectItem("Pep/9 Operating System");
}

void AsmMainWindow::on_actionHelp_About_Pep9_triggered()
{
    aboutPepDialog->show();
}

void AsmMainWindow::on_actionHelp_About_Qt_triggered()
{
    QDesktopServices::openUrl(QUrl("http://www.qt.io/"));
}

//Handle hiding and showing of different sections of the application.
void AsmMainWindow::on_actionView_Code_Only_triggered()
{
    ui->horizontalSplitter->widget(0)->show();
    ui->horizontalSplitter->widget(1)->hide();
    ui->horizontalSplitter->widget(2)->hide();
    ui->actionView_Code_Only->setDisabled(true);
    ui->actionView_Code_CPU->setDisabled(false);
    ui->actionView_Code_CPU_Memory->setDisabled(false);
}

void AsmMainWindow::on_actionView_Code_CPU_triggered()
{
    ui->horizontalSplitter->widget(0)->show();
    ui->horizontalSplitter->widget(1)->show();
    ui->horizontalSplitter->widget(2)->hide();
    ui->actionView_Code_Only->setDisabled(false);
    ui->actionView_Code_CPU->setDisabled(true);
    ui->actionView_Code_CPU_Memory->setDisabled(false);
    QList<int> list;
    list.append(3000);
    list.append(1);
    list.append(1);
    ui->horizontalSplitter->setSizes(list);
}

void AsmMainWindow::on_actionView_Code_CPU_Memory_triggered()
{
    ui->memoryWidget->refreshMemory();
    ui->horizontalSplitter->widget(0)->show();
    ui->horizontalSplitter->widget(1)->show();
    ui->horizontalSplitter->widget(2)->show();
    ui->actionView_Code_Only->setDisabled(false);
    ui->actionView_Code_CPU->setDisabled(false);
    ui->actionView_Code_CPU_Memory->setDisabled(true);
    QList<int> list;
    list.append(3000);
    list.append(1);
    list.append(3000);
    ui->horizontalSplitter->setSizes(list);

}

void AsmMainWindow::on_actionView_Assembler_Tab_triggered()
{
    ui->tabWidget->setCurrentWidget(ui->assemblerTab);
}

void AsmMainWindow::on_actionView_Debugger_Tab_triggered()
{
    ui->tabWidget->setCurrentWidget(ui->debuggerTab);
}

// Byte Converter slots

void AsmMainWindow::slotByteConverterBinEdited(const QString &str)
{
    if (str.length() > 0) {
        bool ok;
        int data = str.toInt(&ok, 2);
        byteConverterChar->setValue(data);
        byteConverterDec->setValue(data);
        byteConverterHex->setValue(data);
        byteConverterInstr->setValue(data);

    }
}

void AsmMainWindow::slotByteConverterCharEdited(const QString &str)
{
    if (str.length() > 0) {
        int data = static_cast<int>(str[0].toLatin1());
        byteConverterBin->setValue(data);
        byteConverterDec->setValue(data);
        byteConverterHex->setValue(data);
        byteConverterInstr->setValue(data);
    }
}

void AsmMainWindow::slotByteConverterDecEdited(const QString &str)
{
    if (str.length() > 0) {
        bool ok;
        int data = str.toInt(&ok, 10);
        byteConverterBin->setValue(data);
        byteConverterChar->setValue(data);
        byteConverterHex->setValue(data);
        byteConverterInstr->setValue(data);
    }
}

void AsmMainWindow::slotByteConverterHexEdited(const QString &str)
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
                byteConverterInstr->setValue(data);
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
void AsmMainWindow::focusChanged(QWidget *oldFocus, QWidget *)
{
    // Unhighlight the old widget.
    if(ui->memoryWidget->isAncestorOf(oldFocus)) {
        ui->memoryWidget->highlightOnFocus();
    }
    else if(ui->assemblerPane->isAncestorOf(oldFocus)) {
        ui->assemblerPane->highlightOnFocus();
    }
    else if(ui->asmProgramTracePane->isAncestorOf(oldFocus)) {
        ui->asmProgramTracePane->highlightOnFocus();
    }
    else if(ui->asmCpuPane->isAncestorOf(oldFocus)) {
        ui->asmCpuPane->highlightOnFocus();
    }
    else if (ui->ioWidget->isAncestorOf(oldFocus)) {
        ui->ioWidget->highlightOnFocus();
    }
    else if (ui->executionStatisticsWidget->isAncestorOf(oldFocus)) {
        ui->executionStatisticsWidget->highlightOnFocus();
    }

    // Highlight the newly focused widget.
    int which = 0;
    if (ui->asmCpuPane->hasFocus()) {
        which = 0;
        ui->asmCpuPane->highlightOnFocus();
    }
    else if (ui->memoryWidget->hasFocus()) {
        which = Enu::EditButton::COPY;
        ui->memoryWidget->highlightOnFocus();
    }
    else if (ui->assemblerPane->isAncestorOf(focusWidget())) {
        which = ui->assemblerPane->enabledButtons();
        ui->assemblerPane->highlightOnFocus();
    }
    else if (ui->ioWidget->isAncestorOf(QApplication::focusWidget())) {
        which = ui->ioWidget->editActions();
        ui->ioWidget->highlightOnFocus();
    }
    else if (ui->asmProgramTracePane->hasFocus()) {
        ui->asmProgramTracePane->highlightOnFocus();
        which = 0;
    }
    else if (ui->executionStatisticsWidget->hasFocus()) {
        ui->executionStatisticsWidget->highlightOnFocus();
        which = 0;
    }

    ui->actionEdit_Undo->setEnabled(which & Enu::EditButton::UNDO);
    ui->actionEdit_Redo->setEnabled(which & Enu::EditButton::REDO);
    ui->actionEdit_Cut->setEnabled(which & Enu::EditButton::CUT);
    ui->actionEdit_Copy->setEnabled(which & Enu::EditButton::COPY);
    ui->actionEdit_Paste->setEnabled(which & Enu::EditButton::PASTE);
}

void AsmMainWindow::setUndoability(bool b)
{
    if (ui->memoryWidget->hasFocus()) {
        ui->actionEdit_Undo->setEnabled(false);
    }
    else if (ui->assemblerPane->isAncestorOf(focusWidget())) {
        ui->actionEdit_Undo->setEnabled(ui->assemblerPane->isUndoable() && b);
    }
    else if (ui->ioWidget->isAncestorOf(QApplication::focusWidget())) {
        ui->actionEdit_Undo->setEnabled(b);
    }
    else if (ui->asmCpuPane->hasFocus()) {
        ui->actionEdit_Undo->setEnabled(false);
    }
    else if (ui->memoryTracePane->hasFocus()) {
        ui->actionEdit_Undo->setEnabled(false);
    }
}

void AsmMainWindow::setRedoability(bool b)
{
    if (ui->memoryWidget->hasFocus()) {
        ui->actionEdit_Redo->setEnabled(false);
    }
    else if (ui->assemblerPane->isAncestorOf(focusWidget())) {
        ui->actionEdit_Redo->setEnabled(ui->assemblerPane->isRedoable() && b);
    }
    else if (ui->ioWidget->isAncestorOf(QApplication::focusWidget())) {
        ui->actionEdit_Redo->setEnabled(b);
    }
    else if (ui->asmCpuPane->hasFocus()) {
        ui->actionEdit_Redo->setEnabled(false);
    }
    else if (ui->memoryTracePane->hasFocus()) {
        ui->actionEdit_Redo->setEnabled(false);
    }
}

void AsmMainWindow::helpCopyToSourceClicked()
{
    helpDialog->hide();
    ui->ioWidget->onClear();
    Enu::EPane destPane, inputPane;
    QString input;
    QString code = helpDialog->getCode(destPane, inputPane, input);
    if(code.isEmpty()) {
        return;
    }
    else {
        switch(destPane)
        {
        case Enu::EPane::ESource:
            if(maybeSave(Enu::EPane::ESource)) {
                ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->assemblerTab));
                ui->assemblerPane->loadSourceFile("", code);
                emit ui->actionDebug_Remove_All_Assembly_Breakpoints->trigger();
                statusBar()->showMessage("Copied to assembler source code", 4000);
            }
            break;
        case Enu::EPane::EObject:
            if(maybeSave(Enu::EPane::EObject)) {
                ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->assemblerTab));
                ui->assemblerPane->loadObjectFile("", code);
                statusBar()->showMessage("Copied to assembler object code", 4000);
            }
            break;
        default:
            // No other panes allow copying help into them.
            return;

    }
    }

    switch(inputPane)
    {
    case Enu::EPane::ETerminal:
        // It doesn't make sense to put input text into a terminal, so ignore input text.
        ui->ioWidget->setActivePane(Enu::EPane::ETerminal);
        break;
    case Enu::EPane::EBatchIO:
        ui->ioWidget->setBatchInput(input);
        ui->ioWidget->setActivePane(Enu::EPane::EBatchIO);
        break;
    default:
        break;
    }
}

void AsmMainWindow::onOutputReceived(quint16 address, quint8 value)
{
    ui->ioWidget->onOutputReceived(address, value);
}

void AsmMainWindow::onInputRequested(quint16 address)
{
    handleDebugButtons();
    connectViewUpdate();
    ui->tabWidget->setCurrentWidget(ui->debuggerTab);
    // Must highlight lines with both the assembler and microcode debugger visible.
    // This is because of a bug in centerCursor() which doesn't center the cursor
    // iff the widget has never been visible.
    auto cw = ui->tabWidget->currentWidget();
    ui->tabWidget->setCurrentWidget(ui->assemblerTab);
    highlightActiveLines();
    ui->tabWidget->setCurrentWidget(ui->debuggerTab);
    highlightActiveLines();
    ui->tabWidget->setCurrentWidget(cw);
    // End fix for centerCursor()

    emit simulationUpdate();
    statusBar()->showMessage("Input requested", 4000);
    ui->ioWidget->onDataRequested(address);
    handleDebugButtons();
    reenableUIAfterInput();
    disconnectViewUpdate();
}

void AsmMainWindow::onBreakpointHit(Enu::BreakpointTypes type)
{
    ui->memoryWidget->refreshMemory();
    ui->memoryTracePane->updateTrace();
    switch(type) {
    case Enu::BreakpointTypes::ASSEMBLER:
        onASMBreakpointHit();
        break;
    default:
        // Don't handle other kinds of breakpoints if they are generated.
        return;
    }
}

void AsmMainWindow::reenableUIAfterInput()
{

}
