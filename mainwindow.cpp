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
#include "asmargument.h"
#include "asmcode.h"
#include "asmprogram.h"
#include "asmprogrammanager.h"
#include "asmsourcecodepane.h"
#include "asmlistingpane.h"
#include "byteconverterbin.h"
#include "byteconverterchar.h"
#include "byteconverterdec.h"
#include "byteconverterhex.h"
#include "byteconverterinstr.h"
#include "cpudatasection.h"
#include "cpucontrolsection.h"
#include "cpupane.h"
#include "helpdialog.h"
#include "isaasm.h"
#include "memorydumppane.h"
#include "memorysection.h"
#include "microcode.h"
#include "microcodepane.h"
#include "microcodeprogram.h"
#include "microobjectcodepane.h"
#include "updatechecker.h"
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), debugState(DebugState::DISABLED), updateChecker(new UpdateChecker()), codeFont(QFont(Pep::codeFont, Pep::codeFontSize)),
    memorySection(MemorySection::getInstance()), dataSection(CPUDataSection::getInstance()),
    controlSection(CPUControlSection::getInstance()), programManager(AsmProgramManager::getInstance()),
    statisticsLevelsGroup(new QActionGroup(this)), inDarkMode(false)
{
    // Initialize all global maps.
    Pep::initMicroEnumMnemonMaps();
    Pep::initEnumMnemonMaps();
    Pep::initMnemonicMaps();
    Pep::initAddrModesMap();
    Pep::initDecoderTables();

    // Perform any additional setup needed for UI objects.
    ui->setupUi(this);
    // Install this class as the global event filter.
    qApp->installEventFilter(this);
    ui->memoryWidget->init(memorySection, dataSection, controlSection);
    ui->cpuWidget->init(this);
    ui->memoryTracePane->init(memorySection);
    ui->AsmSourceCodeWidgetPane->init(memorySection, programManager);

    statisticsLevelsGroup->addAction(ui->actionStatistics_Level_All);
    statisticsLevelsGroup->addAction(ui->actionStatistics_Level_Minimal);
    statisticsLevelsGroup->addAction(ui->actionStatistics_Level_None);
    statisticsLevelsGroup->setExclusive(true);

    // Create & connect all dialogs.
    helpDialog = new HelpDialog(this);
    connect(helpDialog, &HelpDialog::copyToMicrocodeClicked, this, &MainWindow::helpCopyToMicrocodeButtonClicked);
    aboutPepDialog = new AboutPep(this);

    // Byte converter setup.
    byteConverterDec = new ByteConverterDec();
    ui->byteConverterToolBar->addWidget(byteConverterDec);
    byteConverterHex = new ByteConverterHex();
    ui->byteConverterToolBar->addWidget(byteConverterHex);
    byteConverterBin = new ByteConverterBin();
    ui->byteConverterToolBar->addWidget(byteConverterBin);
    byteConverterChar = new ByteConverterChar();
    ui->byteConverterToolBar->addWidget(byteConverterChar);
    byteConverterInstr = new ByteConverterInstr();
    ui->byteConverterToolBar->addWidget(byteConverterInstr);
    connect(byteConverterBin, &ByteConverterBin::textEdited, this, &MainWindow::slotByteConverterBinEdited);
    connect(byteConverterChar, &ByteConverterChar::textEdited, this, &MainWindow::slotByteConverterCharEdited);
    connect(byteConverterDec, &ByteConverterDec::textEdited, this, &MainWindow::slotByteConverterDecEdited);
    connect(byteConverterHex, &ByteConverterHex::textEdited, this, &MainWindow::slotByteConverterHexEdited);

    connect(qApp->instance(), SIGNAL(focusChanged(QWidget*, QWidget*)), this, SLOT(focusChanged(QWidget*, QWidget*)));

    // Connect IOWidget to memory
    ui->ioWidget->bindToMemorySection(memorySection);
    // Connect IO events
    connect(memorySection, &MemorySection::charRequestedFromInput, this, &MainWindow::onInputRequested);
    connect(memorySection, &MemorySection::receivedInput, this, &MainWindow::onInputReceived);

    // Connect Undo / Redo events
    connect(ui->microcodeWidget, &MicrocodePane::undoAvailable, this, &MainWindow::setUndoability);
    connect(ui->microcodeWidget, &MicrocodePane::redoAvailable, this, &MainWindow::setRedoability);
    connect(ui->AsmObjectCodeWidgetPane, &AsmObjectCodePane::undoAvailable, this, &MainWindow::setUndoability);
    connect(ui->AsmObjectCodeWidgetPane, &AsmObjectCodePane::redoAvailable, this, &MainWindow::setRedoability);
    connect(ui->AsmSourceCodeWidgetPane, &AsmSourceCodePane::undoAvailable, this, &MainWindow::setUndoability);
    connect(ui->AsmSourceCodeWidgetPane, &AsmSourceCodePane::redoAvailable, this, &MainWindow::setRedoability);
    connect(ui->ioWidget, &IOWidget::undoAvailable, this, &MainWindow::setUndoability);
    connect(ui->ioWidget, &IOWidget::redoAvailable, this, &MainWindow::setRedoability);

    // Connect simulation events.
    // Events that fire on simulationUpdate should be UniqueConnections, as they will be repeatedly connected and disconnected
    // via connectMicroDraw() and disconnectMicroDraw().
    connect(this, &MainWindow::simulationUpdate, ui->cpuWidget, &CpuPane::onSimulationUpdate, Qt::UniqueConnection);
    connect(this, &MainWindow::simulationUpdate, ui->memoryWidget, &MemoryDumpPane::updateMemory, Qt::UniqueConnection);
    connect(this, &MainWindow::simulationStarted, ui->memoryWidget, &MemoryDumpPane::onSimulationStarted);
    connect(controlSection, &CPUControlSection::simulationHitMicroBreakpoint, this, &MainWindow::onMicroBreakpointHit);
    connect(controlSection, &CPUControlSection::simulationHitASMBreakpoint, this, &MainWindow::onASMBreakpointHit);

    // Clear IOWidget every time a simulation is started.
    connect(this, &MainWindow::simulationStarted, ui->ioWidget, &IOWidget::onClear);
    connect(this, &MainWindow::simulationStarted, ui->microObjectCodePane, &MicroObjectCodePane::onSimulationStarted);
    connect(this, &MainWindow::simulationFinished, ui->microObjectCodePane, &MicroObjectCodePane::onSimulationFinished);
    connect(this, &MainWindow::simulationFinished, this->controlSection, &CPUControlSection::onSimulationFinished);
    connect(this, &MainWindow::simulationFinished, ui->cpuWidget, &CpuPane::onSimulationUpdate);
    connect(this, &MainWindow::simulationFinished, ui->memoryWidget, &MemoryDumpPane::onSimulationFinished);
    // Connect MainWindow so that it can propogate simulationFinished event and clean up when execution is finished.
    connect(controlSection, &CPUControlSection::simulationFinished, this, &MainWindow::onSimulationFinished);


    // Connect simulation events that are internal to the class.
    connect(this, &MainWindow::simulationUpdate, this, &MainWindow::handleDebugButtons, Qt::UniqueConnection);
    connect(this, &MainWindow::simulationUpdate, this, static_cast<void(MainWindow::*)()>(&MainWindow::highlightActiveLines), Qt::UniqueConnection);
    connect(this, &MainWindow::simulationStarted, this, &MainWindow::handleDebugButtons);

    // Connect font change events.
    connect(this, &MainWindow::fontChanged, ui->microcodeWidget, &MicrocodePane::onFontChanged);
    connect(this, &MainWindow::fontChanged, helpDialog, &HelpDialog::onFontChanged);
    connect(this, &MainWindow::fontChanged, ui->ioWidget, &IOWidget::onFontChanged);
    connect(this, &MainWindow::fontChanged, ui->AsmSourceCodeWidgetPane, &AsmSourceCodePane::onFontChanged);
    connect(this, &MainWindow::fontChanged, ui->AsmObjectCodeWidgetPane, &AsmObjectCodePane::onFontChanged);
    connect(this, &MainWindow::fontChanged, ui->AsmListingWidgetPane, &AsmListingPane::onFontChanged);
    connect(this, &MainWindow::fontChanged, ui->memoryWidget, &MemoryDumpPane::onFontChanged);

    // Connect dark mode events.
    connect(this, &MainWindow::darkModeChanged, ui->microcodeWidget, &MicrocodePane::onDarkModeChanged);
    connect(this, &MainWindow::darkModeChanged, helpDialog, &HelpDialog::onDarkModeChanged);
    connect(this, &MainWindow::darkModeChanged, ui->microObjectCodePane, &MicroObjectCodePane::onDarkModeChanged);
    connect(this, &MainWindow::darkModeChanged, ui->cpuWidget, &CpuPane::onDarkModeChanged);
    connect(this, &MainWindow::darkModeChanged, ui->microcodeWidget->getEditor(), &MicrocodeEditor::onDarkModeChanged);
    connect(this, &MainWindow::darkModeChanged, ui->memoryWidget, &MemoryDumpPane::onDarkModeChanged);
    connect(this, &MainWindow::darkModeChanged, ui->AsmSourceCodeWidgetPane, &AsmSourceCodePane::onDarkModeChanged);
    connect(this, &MainWindow::darkModeChanged, ui->AsmListingWidgetPane, &AsmListingPane::onDarkModeChanged);
    connect(this, &MainWindow::darkModeChanged, ui->asmListingTracePane, &AsmTracePane::onDarkModeChanged);

    connect(ui->cpuWidget, &CpuPane::appendMicrocodeLine, this, &MainWindow::appendMicrocodeLine);

    //Connect assembler pane widgets
    connect(ui->AsmSourceCodeWidgetPane, &AsmSourceCodePane::labelDoubleClicked, this, &MainWindow::doubleClickedCodeLabel);
    connect(ui->AsmObjectCodeWidgetPane, &AsmObjectCodePane::labelDoubleClicked, this, &MainWindow::doubleClickedCodeLabel);
    connect(ui->AsmListingWidgetPane, &AsmListingPane::labelDoubleClicked, this, &MainWindow::doubleClickedCodeLabel);

    // Events that notify view of changes in model.
    // These events are disconnected whenevr "run" or "continue" are called, because they have significant overhead,
    // but the provide no benefit when running.
    // They are reconnected at the end of execution, and the receiving widgets are manually notified that changes may have occured.
    connect(memorySection, &MemorySection::memoryChanged, ui->memoryWidget, &MemoryDumpPane::onMemoryChanged, Qt::ConnectionType::UniqueConnection);
    connect(ui->cpuWidget, &CpuPane::registerChanged, dataSection, &CPUDataSection::onSetRegisterByte, Qt::ConnectionType::UniqueConnection);
    connect(dataSection, &CPUDataSection::statusBitChanged, ui->cpuWidget, &CpuPane::onStatusBitChanged, Qt::ConnectionType::UniqueConnection);
    connect(dataSection, &CPUDataSection::registerChanged, ui->cpuWidget, &CpuPane::onRegisterChanged, Qt::ConnectionType::UniqueConnection);
    connect(dataSection, &CPUDataSection::memoryRegisterChanged, ui->cpuWidget, &CpuPane::onMemoryRegisterChanged, Qt::ConnectionType::UniqueConnection);

    // Connect events for breakpoints
    connect(ui->actionDebug_Remove_All_Microcode_Breakpoints, &QAction::triggered, ui->microcodeWidget, &MicrocodePane::onRemoveAllBreakpoints);
    connect(ui->actionDebug_Remove_All_Assembly_Breakpoints, &QAction::triggered, ui->AsmSourceCodeWidgetPane, &AsmSourceCodePane::onRemoveAllBreakpoints);
    connect(ui->actionDebug_Remove_All_Assembly_Breakpoints, &QAction::triggered, ui->asmListingTracePane, &AsmTracePane::onRemoveAllBreakpoints);
    connect(ui->actionDebug_Remove_All_Assembly_Breakpoints, &QAction::triggered, controlSection, &CPUControlSection::onRemoveAllPCBreakpoints);
    connect(ui->asmListingTracePane, &AsmTracePane::breakpointAdded, controlSection, &CPUControlSection::onAddPCBreakpoint);
    connect(ui->asmListingTracePane, &AsmTracePane::breakpointRemoved, controlSection, &CPUControlSection::onRemovePCBreakpoint);
    // Load dark mode style sheet.
    QFile f(":qdarkstyle/dark_style.qss");
    f.open(QFile::ReadOnly | QFile::Text);
    QTextStream ts(&f);
    darkStyle = ts.readAll();
    lightStyle = this->styleSheet();



    //Pre-render memory & fix maximum widget size.
    int maxSize = ui->memoryWidget->memoryDumpWidth();
    ui->memoryWidget->setMinimumWidth(maxSize);
    ui->memoryWidget->setMaximumWidth(maxSize);
    //ui->ioWidget->setMaximumWidth(maxSize);

    //Initialize state for ISA level simulation
    //Pep::memAddrssToAssemblerListing = &Pep::memAddrssToAssemblerListingProg;
    //Pep::listingRowChecked = &Pep::listingRowCheckedProg;

    //Initialize Microcode panes
    QFile file("://help/pep9micro.pepcpu");
    if(file.open(QFile::ReadOnly | QFile::Text)){
        QTextStream in(&file);
        ui->microcodeWidget->setMicrocode(in.readAll());
        ui->microcodeWidget->setModifiedFalse();
        ui->microcodeWidget->setCurrentFile(QFileInfo(file).filePath());
    }

    //Initialize debug menu
    handleDebugButtons();

    //Lastly, read in settings
    readSettings();

    // Resize docking widgets because QT does a poor job of it
    tabifyDockWidget(ui->memoryDockWdget, ui->memoryTraceDockWidget);
    ui->memoryDockWdget->raise();
    int scaleTotal = ui->memoryDockWdget->sizePolicy().verticalStretch()+ ui->ioDockWidget->sizePolicy().verticalStretch();
    double memory = geometry().height() * ui->memoryDockWdget->sizePolicy().verticalStretch() / static_cast<double>(scaleTotal);
    double io = geometry().height() * ui->ioDockWidget->sizePolicy().verticalStretch() / static_cast<double>(scaleTotal);
    resizeDocks({ui->memoryDockWdget, ui->ioDockWidget}, {static_cast<int>(memory),
                                                          static_cast<int>(io)}, Qt::Vertical);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete helpDialog;
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
            if (ui->cpuWidget->hasFocus()) {
                // single step or clock, depending
                if (debugState == DebugState::DEBUG_ISA || debugState == DebugState::DEBUG_MICRO) {
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
                     (ui->microcodeWidget->hasFocus() || ui->microObjectCodePane->hasFocus())) {
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
    connect(memorySection, &MemorySection::memoryChanged, ui->memoryWidget, &MemoryDumpPane::onMemoryChanged, Qt::ConnectionType::UniqueConnection);
    connect(ui->cpuWidget, &CpuPane::registerChanged, dataSection, &CPUDataSection::onSetRegisterByte, Qt::ConnectionType::UniqueConnection);
    connect(dataSection, &CPUDataSection::statusBitChanged, ui->cpuWidget, &CpuPane::onStatusBitChanged, Qt::ConnectionType::UniqueConnection);
    connect(dataSection, &CPUDataSection::registerChanged, ui->cpuWidget, &CpuPane::onRegisterChanged, Qt::ConnectionType::UniqueConnection);
    connect(dataSection, &CPUDataSection::memoryRegisterChanged, ui->cpuWidget, &CpuPane::onMemoryRegisterChanged, Qt::ConnectionType::UniqueConnection);
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
    disconnect(memorySection,&MemorySection::memoryChanged, ui->memoryWidget,&MemoryDumpPane::onMemoryChanged);
    disconnect(ui->cpuWidget, &CpuPane::registerChanged, dataSection, &CPUDataSection::onSetRegisterByte);
    disconnect(dataSection, &CPUDataSection::statusBitChanged, ui->cpuWidget, &CpuPane::onStatusBitChanged);
    disconnect(dataSection, &CPUDataSection::registerChanged, ui->cpuWidget, &CpuPane::onRegisterChanged);
    disconnect(dataSection, &CPUDataSection::memoryRegisterChanged, ui->cpuWidget, &CpuPane::onMemoryRegisterChanged);
    disconnect(this, &MainWindow::simulationUpdate, ui->memoryWidget, &MemoryDumpPane::updateMemory);
    disconnect(this, &MainWindow::simulationUpdate, ui->cpuWidget, &CpuPane::onSimulationUpdate);
    disconnect(this, &MainWindow::simulationUpdate, this, &MainWindow::handleDebugButtons);
    disconnect(this, &MainWindow::simulationUpdate, this, static_cast<void(MainWindow::*)()>(&MainWindow::highlightActiveLines));
    disconnect(this, &MainWindow::simulationStarted, this, static_cast<void(MainWindow::*)()>(&MainWindow::highlightActiveLines));
    dataSection->setEmitEvents(false);
}

void MainWindow::readSettings()
{
    QSettings settings("cslab.pepperdine","PEP9Micro");

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
    bool tempDarkMode = settings.value("inDarkMode", false).toBool();
    if(tempDarkMode != inDarkMode) {
        ui->actionDark_Mode->setChecked(tempDarkMode);
        on_actionDark_Mode_triggered();
    }
    quint16 debuggerLevel = settings.value("debugLevel", 1).toInt();
    if(debuggerLevel >= static_cast<quint16>(Enu::DebugLevels::END)) debuggerLevel = static_cast<int>(Enu::DebugLevels::DEFAULT);
    controlSection->setDebugLevel(static_cast<Enu::DebugLevels>(debuggerLevel));
    setCheckedFromDebugLevel(static_cast<Enu::DebugLevels>(debuggerLevel));

    // Restore last used split in assembly code pane
    val = settings.beginReadArray("codePaneSplit");
    QList<int> sizes;
    for(int it = 0; it < ui->codeSplitter->sizes().length(); it++) {
        settings.setArrayIndex(it);
        sizes.append(settings.value("size", 1).toInt());
    }
    ui->codeSplitter->setSizes(sizes);
    settings.endArray();


    settings.endGroup();

    //Handle reading for all children
    ui->microcodeWidget->readSettings(settings);
}

void MainWindow::writeSettings()
{
    QSettings settings("cslab.pepperdine","PEP9Micro");
    settings.beginGroup("MainWindow");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("font", codeFont);
    settings.setValue("filePath", curPath);
    settings.setValue("inDarkMode", inDarkMode);
    settings.setValue("debugLevel", (int)controlSection->getDebugLevel());
    settings.beginWriteArray("codePaneSplit", 3);
    QList<int> temp = ui->codeSplitter->sizes();
    for(int it = 0; it < 3; it++) {
        settings.setArrayIndex(it);
        settings.setValue("size", temp[it]);
    }
    settings.endArray();
    settings.endGroup();
    //Handle writing for all children
    ui->microcodeWidget->writeSettings(settings);
}

// Save methods
bool MainWindow::save(Enu::EPane which)
{
    bool retVal = true;
    switch(which) {
    /* For each pane, first check if there is already a file associated with the pane.
     * if there is not, then pass control to the save as function.
     * If there is a file, then attempt save to it, then remove any modified flags from the pane if it succeded.
     */
    case Enu::EPane::ESource:
        if(QFileInfo(ui->AsmSourceCodeWidgetPane->getCurrentFile()).absoluteFilePath().isEmpty())
        {
            retVal = saveAsFile(Enu::EPane::ESource);
        }
        else retVal = saveFile(ui->AsmSourceCodeWidgetPane->getCurrentFile().fileName(),Enu::EPane::ESource);
        if(retVal) ui->AsmSourceCodeWidgetPane->setModifiedFalse();
        break;
    case Enu::EPane::EObject:
        if(QFileInfo(ui->AsmObjectCodeWidgetPane->getCurrentFile()).absoluteFilePath().isEmpty())
        {
            retVal = saveAsFile(Enu::EPane::EObject);
        }
        if(retVal) ui->AsmObjectCodeWidgetPane->setModifiedFalse();
        break;
    case Enu::EPane::EListing:
        if(QFileInfo(ui->AsmListingWidgetPane->getCurrentFile()).absoluteFilePath().isEmpty())
        {
            retVal = saveAsFile(Enu::EPane::EListing);
        }
        break;
    case Enu::EPane::EMicrocode:
        if(QFileInfo(ui->microcodeWidget->getCurrentFile()).absoluteFilePath().isEmpty())
        {
            retVal = saveAsFile(Enu::EPane::EMicrocode);
        }
        else retVal = saveFile(ui->microcodeWidget->getCurrentFile().fileName(),Enu::EPane::EMicrocode);
        if(retVal) ui->microcodeWidget->setModifiedFalse();
        break;
    }
    return retVal;
}

bool MainWindow::maybeSave()
{
    static QMetaEnum metaenum = QMetaEnum::fromType<Enu::EPane>();
    bool retVal = true;
    // Iterate over all the panes, and attempt to save any modified panes before closing.
    for(int it = 0; it < metaenum.keyCount(); it++) {
        retVal = retVal && maybeSave(static_cast<Enu::EPane>(metaenum.value(it)));
    }
    return retVal;
}

bool MainWindow::maybeSave(Enu::EPane which)
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
        if(ui->AsmSourceCodeWidgetPane->isModified())
        {
            ret = QMessageBox::warning(this, dlgTitle,sourceText,buttons);
            if (ret == QMessageBox::Save)
                retVal = save(Enu::EPane::ESource);
            else if (ret == QMessageBox::Cancel)
                retVal = false;
        }
        break;
    case Enu::EPane::EObject:
        if(ui->AsmObjectCodeWidgetPane->isModified())
        {
            ret = QMessageBox::warning(this, dlgTitle,objectText,buttons);
            if (ret == QMessageBox::Save)
                retVal = save(Enu::EPane::EObject);
            else if (ret == QMessageBox::Cancel)
                retVal = false;
        }
        break;
    case Enu::EPane::EMicrocode:
        if(ui->microcodeWidget->isModified())
        {
            ret = QMessageBox::warning(this, dlgTitle,microcodeText,buttons);
            if (ret == QMessageBox::Save)
                retVal = save(Enu::EPane::EMicrocode);
            else if (ret == QMessageBox::Cancel)
                retVal = false;
        }
        break;
    default:
        break;
    }
    return retVal;
}

void MainWindow::loadFile(const QString &fileName, Enu::EPane which)
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
    switch(which) {
    /*
     * For each pane, switch to the tab which contains that pane.
     * Give that pane focus, notify the pane of the new file's name,
     * and set the pane's text to the contents of the file.
     * Ensure that the modified (*) flag is not set.
     */
    case Enu::EPane::ESource:
        ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->assemblerTab));
        ui->AsmSourceCodeWidgetPane->setFocus();
        ui->AsmSourceCodeWidgetPane->setCurrentFile(fileName);
        ui->AsmSourceCodeWidgetPane->setSourceCodePaneText(in.readAll());
        ui->AsmSourceCodeWidgetPane->setModifiedFalse();
        break;
    case Enu::EPane::EObject:
        ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->assemblerTab));
        ui->AsmObjectCodeWidgetPane->setFocus();
        ui->AsmObjectCodeWidgetPane->setCurrentFile(fileName);
        ui->AsmObjectCodeWidgetPane->setObjectCodePaneText(in.readAll());
        ui->AsmObjectCodeWidgetPane->setModifiedFalse();
        break;
    case Enu::EPane::EMicrocode:
        ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->debuggerTab));
        ui->debuggerTabWidget->setCurrentIndex(ui->debuggerTabWidget->indexOf(ui->microcodeDebuggerTab));
        ui->microcodeWidget->setFocus();
        ui->microcodeWidget->setCurrentFile(fileName);
        ui->microcodeWidget->setMicrocode(in.readAll());
        ui->microcodeWidget->setModifiedFalse();
        break;
    }
    curPath = QFileInfo(file).dir().absolutePath();
    statusBar()->showMessage(tr("File loaded"), 4000);
    QApplication::restoreOverrideCursor();
}

bool MainWindow::saveFile(Enu::EPane which)
{
    QString fileName;
    switch(which)
    {
    //Get the filename associated with the dialog
    case Enu::EPane::ESource:
        fileName = QFileInfo(ui->AsmSourceCodeWidgetPane->getCurrentFile()).absoluteFilePath();
        break;
    case Enu::EPane::EObject:
        fileName = QFileInfo(ui->AsmObjectCodeWidgetPane->getCurrentFile()).absoluteFilePath();
        break;
    case Enu::EPane::EListing:
        fileName = QFileInfo(ui->AsmListingWidgetPane->getCurrentFile()).absoluteFilePath();
        break;
    case Enu::EPane::EMicrocode:
        fileName = QFileInfo(ui->microcodeWidget->getCurrentFile()).absoluteFilePath();
        break;
    }
    return saveFile(fileName, which);
}

bool MainWindow::saveFile(const QString &fileName, Enu::EPane which)
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
        out << ui->AsmSourceCodeWidgetPane->toPlainText();
        msgOutput = &msgSource;
        break;
    case Enu::EPane::EObject:
        out << ui->AsmObjectCodeWidgetPane->toPlainText();
        msgOutput = &msgObject;
        break;
    case Enu::EPane::EListing:
        out << ui->AsmListingWidgetPane->toPlainText();
        msgOutput = &msgListing;
        break;
    case Enu::EPane::EMicrocode:
        out << ui->microcodeWidget->getMicrocode();
        msgOutput = &msgMicro;
        break;
    }
    QApplication::restoreOverrideCursor();
    curPath = QFileInfo(file).dir().absolutePath();
    statusBar()->showMessage(*msgOutput, 4000);
    return true;
}

bool MainWindow::saveAsFile(Enu::EPane which)
{
    // Default filenames for each pane.
    static const QString defSourceFile = "untitled.pep";
    static const QString defObjectFile = "untitled.pepo";
    static const QString defListingFile = "untitled.pepl";
    static const QString defMicroFile = "untitled.pepcpu";
    QString usingFile;

    // Titles for each pane.
    static const QString titleBase = "Save %1";
    static const QString sourceTitle = titleBase.arg("Assembler Source Code");
    static const QString objectTitle = titleBase.arg("Object Code");
    static const QString listingTitle = titleBase.arg("Assembler Listing");
    static const QString microTitle = titleBase.arg("Microcode");
    const QString *usingTitle;

    // Patterns for source code files.
    static const QString sourceTypes = "Pep9 Source (*.pep *.txt)";
    static const QString objectTypes = "Pep9 Object (*.pepo *.txt)";
    static const QString listingTypes = "Pep9 Listing (*.pepl)";
    static const QString microTypes = "Pep9 Microcode (*.pepcpu *.txt)";
    const QString *usingTypes;

    switch(which)
    {
    /*
     * For each pane, get the file associated with each pane, or the default if there is none.
     * Set the title and source code patterns for the pane.
     */
    case Enu::EPane::ESource:
        if(ui->AsmSourceCodeWidgetPane->getCurrentFile().fileName().isEmpty()) {
            usingFile = QDir(curPath).absoluteFilePath(defSourceFile);
        }
        else usingFile = ui->AsmSourceCodeWidgetPane->getCurrentFile().fileName();
        usingTitle = &sourceTitle;
        usingTypes = &sourceTypes;
        break;
    case Enu::EPane::EObject:
        if(ui->AsmObjectCodeWidgetPane->getCurrentFile().fileName().isEmpty()) {
            usingFile = QDir(curPath).absoluteFilePath(defObjectFile);
        }
        else usingFile = ui->AsmObjectCodeWidgetPane->getCurrentFile().fileName();
        usingTitle = &objectTitle;
        usingTypes = &objectTypes;
        break;
    case Enu::EPane::EListing:
        if(ui->AsmListingWidgetPane->getCurrentFile().fileName().isEmpty()) {
            usingFile = QDir(curPath).absoluteFilePath(defListingFile);
        }
        else usingFile = ui->AsmListingWidgetPane->getCurrentFile().fileName();
        usingTitle = &listingTitle;
        usingTypes = &listingTypes;
        break;
    case Enu::EPane::EMicrocode:
        if(ui->microcodeWidget->getCurrentFile().fileName().isEmpty()) {
            usingFile = QDir(curPath).absoluteFilePath(defMicroFile);
        }
        else usingFile = ui->microcodeWidget->getCurrentFile().fileName();
        usingTitle = &microTitle;
        usingTypes = &microTypes;
        break;
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
        switch(which)
        {
        case Enu::EPane::ESource:
            ui->AsmSourceCodeWidgetPane->setCurrentFile(fileName);
            break;
        case Enu::EPane::EObject:
            ui->AsmObjectCodeWidgetPane->setCurrentFile(fileName);
            break;
        case Enu::EPane::EListing:
            ui->AsmListingWidgetPane->setCurrentFile(fileName);
            break;
        case Enu::EPane::EMicrocode:
            ui->microcodeWidget->setCurrentFile(fileName);
            break;
        }
        return true;
    }
    else return false;
}

QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void MainWindow::print(Enu::EPane which)
{
    //Don't use a pointer for the text, because toPlainText() returns a temporary object
    QString text;
    const QString base = "Print %1";
    const QString source = base.arg("Assembler Source Code");
    const QString object = base.arg("Object Code");
    const QString listing = base.arg("Assembler Listing");
    const QString micro = base.arg("Microcode");
    const QString *title; //Pointer to the title of the dialog
    switch(which)
    {
    /*
     * Store text-to-print and dialog title based on which pane is to be printed
     */
    case Enu::EPane::ESource:
        title = &source;
        text = ui->AsmSourceCodeWidgetPane->toPlainText();
        break;
    case Enu::EPane::EObject:
        title = &object;
        text = ui->AsmObjectCodeWidgetPane->toPlainText();
        break;
    case Enu::EPane::EListing:
        title = &listing;
        text = ui->AsmListingWidgetPane->toPlainText();
        break;
    case Enu::EPane::EMicrocode:
        title = &micro;
        text = ui->microcodeWidget->getMicrocode();
        break;
    }
    QTextDocument document(text, this);
    document.setDefaultFont(QFont("Courier", 10, -1));

    QPrinter printer(QPrinter::HighResolution);

    QPrintDialog *dialog = new QPrintDialog(&printer, this);
    dialog->setWindowTitle(*title);
    if (dialog->exec() == QDialog::Accepted) {
        document.print(&printer);
    }
    dialog->deleteLater();
}

//Helper function that turns hexadecimal object code into a vector of unsigned characters, which is easier to copy into memory.
QVector<quint8> convertObjectCodeToIntArray(QString line)
{
    bool ok = false;
    quint8 temp;
    QVector<quint8> output;
    for(QString byte : line.split(" ")) {
        // toShort(...) should never throw any errors, so there should be no concerns if byte is not a hex constant.
        temp = byte.toShort(&ok, 16);
        // There could be a loss in precision if given text outside the range of an uchar but in range of a ushort.
        if(ok && byte.length()>0) output.append(temp);
    }
    return output;
}

void MainWindow::loadOperatingSystem()
{
    QVector<quint8> values;
    quint16 startAddress;
    QString  osFileString;
#pragma message ("TODO: Add ability to switch between operating systems")
    //In the future, have a switch between loading the aligned and unaligned code
    if(true) osFileString = (":/help/pep9os/alignedIO-OS.pep");
    else osFileString = ("failure_to_load");
    QFile osFile(osFileString);
    if(programManager->getOperatingSystem() == nullptr) {
        if(osFile.open(QFile::ReadOnly))
        {
            QStringList ls = QString(osFile.readAll()).split("\n");
            osFile.close();
            ui->AsmSourceCodeWidgetPane->assembleOS(ls);
        }
        else
        {
            qDebug() << "I shouldn't be here";
            qDebug() << osFile.errorString();
            return;
        }
    }
    values = programManager->getOperatingSystem()->getObjectCode();
    startAddress = 0xffff - values.length() + 1;
    memorySection->loadObjectCode(startAddress, values);
    memorySection->onIPortChanged(memorySection->getMemoryWord(0xFFF8, false));
    memorySection->onOPortChanged(memorySection->getMemoryWord(0xFFFA, false));
}

void MainWindow::loadObjectCodeProgram()
{
    // Get the object code, and convert it to an integer array.
    QString lines = ui->AsmObjectCodeWidgetPane->toPlainText();
    QVector<quint8> data;
    for(auto line : lines.split("\n", QString::SkipEmptyParts)) {
        data.append(convertObjectCodeToIntArray(line));
    }
    // Imitate the behavior of Pep/9.
    // Always copy bytes to memory starting at 0x0000, instead of invoking the loader.
    memorySection->loadObjectCode((quint16)0, data);
}

void MainWindow::set_Obj_Listing_filenames_from_Source()
{
    // If source code pane has a file, set the object code and listing to have the same file name.
    // Otherwise, set the filenames to empty.
    QFileInfo fileInfo(ui->AsmSourceCodeWidgetPane->getCurrentFile());
    QString object, listing;
    if(fileInfo.fileName().isEmpty()){
#pragma message("TODO: Determine title of object and listing panes")
        object = "";
        listing = "";
    }
    else {
        object = fileInfo.absoluteDir().absoluteFilePath(fileInfo.baseName()+".pepo");
        listing = fileInfo.absoluteDir().absoluteFilePath(fileInfo.baseName()+".pepl");
    }
    ui->AsmObjectCodeWidgetPane->setCurrentFile(object);
    ui->AsmListingWidgetPane->setCurrentFile(listing);
}

void MainWindow::doubleClickedCodeLabel(Enu::EPane which)
{
    QList<int> list, defaultList = {1,1,1};
    QList<int> old = ui->codeSplitter->sizes();
    auto max = std::minmax_element(old.begin(), old.end());
    static const int largeSize = 3000, smallSize = 1;
    bool sameSize = *max.second - *max.first <5;
    switch(which)
    {
    // Give the selected pane the majority of the screen space.
    case Enu::EPane::ESource:
        if(old[0] == *max.second && !sameSize) {
            list = defaultList;
        }
        else {
            list.append(largeSize);
            list.append(smallSize);
            list.append(smallSize);
        }
        ui->codeSplitter->setSizes(list);
        break;
    case Enu::EPane::EObject:
        if(old[1] == *max.second && !sameSize) {
            list = defaultList;
        }
        else {
            list.append(smallSize);
            list.append(largeSize);
            list.append(smallSize);
        }
        ui->codeSplitter->setSizes(list);
        break;
    case Enu::EPane::EListing:
        if(old[2] == *max.second && !sameSize) {
            list = defaultList;
        }
        else {
            list.append(smallSize);
            list.append(smallSize);
            list.append(largeSize);
        }
        ui->codeSplitter->setSizes(list);
        break;
    }
}

void MainWindow::debugButtonEnableHelper(const int which)
{
    // Crack the parameter using DebugButtons to properly enable and disable all buttons related to debugging and running.
    // Build Actions
    ui->ActionBuild_Assemble->setEnabled(which&DebugButtons::BUILD_ASM);
    ui->actionEdit_Remove_Error_Assembler->setEnabled(which&DebugButtons::BUILD_ASM);
    ui->actionEdit_Format_Assembler->setEnabled(which&DebugButtons::BUILD_ASM);
    ui->actionBuild_Load_Object->setEnabled(which&DebugButtons::BUILD_ASM);
    ui->actionBuild_Microcode->setEnabled(which&DebugButtons::BUILD_MICRO);
    ui->actionEdit_Remove_Error_Microcode->setEnabled(which&DebugButtons::BUILD_MICRO);
    ui->actionEdit_Format_Microcode->setEnabled(which&DebugButtons::BUILD_MICRO);

    // Debug & Run Actions
    ui->actionBuild_Run->setEnabled(which&DebugButtons::RUN);
    ui->actionBuild_Run_Object->setEnabled(which&DebugButtons::RUN_OBJECT);
    ui->actionDebug_Start_Debugging->setEnabled(which&DebugButtons::DEBUG);
    ui->actionDebug_Start_Debugging_Object->setEnabled(which&DebugButtons::DEBUG_OBJECT);
    ui->actionDebug_Start_Debugging_Loader->setEnabled(which&DebugButtons::DEBUG_LOADER);
    ui->actionDebug_Interupt_Execution->setEnabled(which&DebugButtons::INTERRUPT);
    ui->actionDebug_Continue->setEnabled(which&DebugButtons::CONTINUE);
    ui->actionDebug_Restart_Debugging->setEnabled(which&DebugButtons::RESTART);
    ui->actionDebug_Stop_Debugging->setEnabled(which&DebugButtons::STOP);
    ui->actionDebug_Single_Step_Assembler->setEnabled(which&DebugButtons::SINGLE_STEP_ASM);
    ui->actionDebug_Step_Over_Assembler->setEnabled(which&DebugButtons::STEP_OVER_ASM);
    ui->actionDebug_Step_Into_Assembler->setEnabled(which&DebugButtons::STEP_INTO_ASM);
    ui->actionDebug_Step_Out_Assembler->setEnabled(which&DebugButtons::STEP_OUT_ASM);
    ui->actionDebug_Single_Step_Microcode->setEnabled(which&DebugButtons::SINGLE_STEP_MICRO);

    // Statistics Actions
    ui->actionStatistics_Level_All->setEnabled(which&DebugButtons::STATS_LEVELS);
    ui->actionStatistics_Level_Minimal->setEnabled(which&DebugButtons::STATS_LEVELS);
    ui->actionStatistics_Level_None->setEnabled(which&DebugButtons::STATS_LEVELS);

    //File open & new actions
    ui->actionFile_New_Asm->setEnabled(which&DebugButtons::OPEN_NEW);
    ui->actionFile_New_Microcode->setEnabled(which&DebugButtons::OPEN_NEW);
    ui->actionFile_Open->setEnabled(which&DebugButtons::OPEN_NEW);
}

void MainWindow::highlightActiveLines(bool forceISA)
{
    //always highlight the current microinstruction
    ui->microcodeWidget->updateSimulationView();
    ui->microObjectCodePane->highlightCurrentInstruction();
    //If the µPC is 0, if a breakpoint has been reached, or if the microcode has a breakpoint, rehighlight the ASM views.
    if(controlSection->getLineNumber() == 0 || forceISA || controlSection->stoppedForBreakpoint()) {
        ui->memoryWidget->clearHighlight();
        ui->memoryWidget->highlight();
    }
}

void MainWindow::highlightActiveLines()
{
    return highlightActiveLines(false);
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
        ui->microObjectCodePane->setObjectCode(ui->microcodeWidget->getMicrocodeProgram(), nullptr);
        controlSection->setMicrocodeProgram(ui->microcodeWidget->getMicrocodeProgram());
    }
    else {
        ui->statusBar->showMessage("MicroAssembly failed", 4000);
        return false;
    }

    // Clear data models & application views
    controlSection->onClearCPU();
    controlSection->initCPU();
    ui->cpuWidget->clearCpu();


    // Don't allow the microcode pane to be edited while the program is running
    ui->microcodeWidget->setReadOnly(true);

    // If there is batch input, move input to  input buffer in the MemorySection
    ui->ioWidget->batchInputToBuffer();
    // No longer emits simulationStarted(), as this could trigger extra screen painting that is unwanted.
    return true;
}

void MainWindow::setCheckedFromDebugLevel(Enu::DebugLevels level)
{
    switch(level)
    {
    case Enu::DebugLevels::ALL:
        ui->actionStatistics_Level_All->setChecked(true);
        break;
    case Enu::DebugLevels::MINIMAL:
        ui->actionStatistics_Level_Minimal->setChecked(true);
        break;
    case Enu::DebugLevels::NONE:
        ui->actionStatistics_Level_None->setChecked(true);
        break;
    }
}

void MainWindow::onUpdateCheck(int val)
{
    val = (int)val; //Ugly way to get rid of unused paramter warning without actually modifying the parameter.
    //Dummy to handle update checking code
}

// File MainWindow triggers
void MainWindow::on_actionFile_New_Asm_triggered()
{
    //Try to save source code before clearing it, the object code pane, and the listing pane.
    if (maybeSave(Enu::EPane::ESource)) {
        ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->assemblerTab));
        ui->AsmSourceCodeWidgetPane->setFocus();
        ui->AsmSourceCodeWidgetPane->clearSourceCode();
        ui->AsmSourceCodeWidgetPane->setCurrentFile("");
        ui->AsmObjectCodeWidgetPane->clearObjectCode();
        ui->AsmObjectCodeWidgetPane->setCurrentFile("");
        ui->AsmListingWidgetPane->clearAssemblerListing();
        ui->AsmListingWidgetPane->setCurrentFile("");
    }
}

void MainWindow::on_actionFile_New_Microcode_triggered()
{
    //Try to save the microcode pane before clearing it & the micro-object-code pane.
    if (maybeSave(Enu::EPane::EMicrocode)) {
        ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->debuggerTab));
        ui->microcodeWidget->setFocus();
        ui->microcodeWidget->setMicrocode("");
        ui->microcodeWidget->setCurrentFile("");
        ui->microObjectCodePane->setObjectCode();
    }
}

void MainWindow::on_actionFile_Open_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(
                this,
                "Open text file",
                curPath,
                "Pep/9 files (*.pep *.pepo *.pepl *.pepcpu *.txt)");
    Enu::EPane which;
    // Depending on the file ending, change which pane will be loaded into.
    if (!fileName.isEmpty()) {
        if(fileName.endsWith("pep", Qt::CaseInsensitive)) which = Enu::EPane::ESource;
        else if(fileName.endsWith("pepo", Qt::CaseInsensitive)) which = Enu::EPane::EObject;
        else if(fileName.endsWith("pepl", Qt::CaseInsensitive)) which = Enu::EPane::EListing;
        else if(fileName.endsWith("pepcpu", Qt::CaseInsensitive)) which = Enu::EPane::EMicrocode;
        if(maybeSave(which)) {
            loadFile(fileName, which);
        }
        curPath = QFileInfo(fileName).absolutePath();
    }
}

bool MainWindow::on_actionFile_Save_Asm_triggered()
{
    return save(Enu::EPane::ESource);
}

bool MainWindow::on_actionFile_Save_Microcode_triggered()
{
    return save(Enu::EPane::EMicrocode);
}

bool MainWindow::on_actionFile_Save_Asm_Source_As_triggered()
{
    return saveAsFile(Enu::EPane::ESource);
}

bool MainWindow::on_actionFile_Save_Object_Code_As_triggered()
{
    return saveAsFile(Enu::EPane::EObject);
}

bool MainWindow::on_actionFile_Save_Assembler_Listing_As_triggered()
{
    return saveAsFile(Enu::EPane::EListing);
}

bool MainWindow::on_actionFile_Save_Microcode_As_triggered()
{
    return saveAsFile(Enu::EPane::EMicrocode);
}

void MainWindow::on_actionFile_Print_Assembler_Source_triggered()
{
    print(Enu::EPane::ESource);
}

void MainWindow::on_actionFile_Print_Object_Code_triggered()
{
    print(Enu::EPane::EObject);
}

void MainWindow::on_actionFile_Print_Assembler_Listing_triggered()
{
    print(Enu::EPane::EListing);
}

void MainWindow::on_actionFile_Print_Microcode_triggered()
{
    print(Enu::EPane::EMicrocode);
}

// Edit MainWindow triggers

void MainWindow::on_actionEdit_Undo_triggered()
{
    // Other Pep9Cpu panes do not support undo
    if (ui->microcodeWidget->hasFocus() && ui->actionDebug_Start_Debugging->isEnabled()) {
        ui->microcodeWidget->undo();
    }
    // However, the Pep9 panes do
    else if(ui->AsmSourceCodeWidgetPane->hasFocus()) {
        ui->AsmSourceCodeWidgetPane->undo();
    }
    else if(ui->AsmObjectCodeWidgetPane->hasFocus()) {
        ui->AsmObjectCodeWidgetPane->undo();
    }
    else if (ui->ioWidget->isAncestorOf(QApplication::focusWidget())) {
        ui->ioWidget->undo();
    }
}

void MainWindow::on_actionEdit_Redo_triggered()
{
    // Other Pep9Cpu panes do not support redo
    if (ui->microcodeWidget->hasFocus() && ui->actionDebug_Start_Debugging->isEnabled()) {
        ui->microcodeWidget->redo();
    }
    // However, the Pep9 panes do
    else if(ui->AsmSourceCodeWidgetPane->hasFocus()) {
        ui->AsmSourceCodeWidgetPane->redo();
    }
    else if(ui->AsmObjectCodeWidgetPane->hasFocus()) {
        ui->AsmObjectCodeWidgetPane->redo();
    }
    else if (ui->ioWidget->isAncestorOf(QApplication::focusWidget())) {
        ui->ioWidget->redo();
    }
}

void MainWindow::on_actionEdit_Cut_triggered()
{
    if (ui->microcodeWidget->hasFocus() && ui->actionDebug_Start_Debugging->isEnabled()) {
        ui->microcodeWidget->cut();
    }
    else if(ui->AsmSourceCodeWidgetPane->hasFocus()) {
        ui->AsmSourceCodeWidgetPane->cut();
    }
    else if(ui->AsmObjectCodeWidgetPane->hasFocus()) {
        ui->AsmObjectCodeWidgetPane->cut();
    }
    else if (ui->ioWidget->isAncestorOf(QApplication::focusWidget())) {
        ui->ioWidget->cut();
    }
}

void MainWindow::on_actionEdit_Copy_triggered()
{
    if (ui->microcodeWidget->hasFocus()) {
        ui->microcodeWidget->copy();
    }
    else if (helpDialog->hasFocus()) {
        helpDialog->copy();
    }
    else if(ui->AsmSourceCodeWidgetPane->hasFocus()) {
        ui->AsmSourceCodeWidgetPane->copy();
    }
    else if(ui->AsmObjectCodeWidgetPane->hasFocus()) {
        ui->AsmObjectCodeWidgetPane->copy();
    }
    else if (ui->ioWidget->isAncestorOf(QApplication::focusWidget())) {
        ui->ioWidget->copy();
    }
    // other panes should not be able to copy
}

void MainWindow::on_actionEdit_Paste_triggered()
{
    if (ui->microcodeWidget->hasFocus() && ui->actionDebug_Start_Debugging->isEnabled()) {
        ui->microcodeWidget->paste();
    }
    else if(ui->AsmSourceCodeWidgetPane->hasFocus()) {
        ui->AsmSourceCodeWidgetPane->paste();
    }
    else if(ui->AsmObjectCodeWidgetPane->hasFocus()) {
        ui->AsmObjectCodeWidgetPane->paste();
    }
    else if (ui->ioWidget->isAncestorOf(QApplication::focusWidget())) {
        ui->ioWidget->paste();
    }
    // other panes should not be able to paste
}

void MainWindow::on_actionEdit_UnComment_Line_triggered()
{
    if (!ui->actionDebug_Stop_Debugging->isEnabled()) { // we are not debugging
        ui->microcodeWidget->unCommentSelection();
    }
    else if(ui->AsmSourceCodeWidgetPane->hasFocus()) {
#pragma message("TODO: Add commenting features to AsmSourceCodeWidget")
        //ui->AsmSourceCodeWidgetPane->
    }
}

void MainWindow::on_actionEdit_Format_Assembler_triggered()
{
    QStringList assemblerListingList = ui->AsmSourceCodeWidgetPane->getAssemblerListingList();
    assemblerListingList.replaceInStrings(QRegExp("^............."), "");
    assemblerListingList.removeAll("");
    if (!assemblerListingList.isEmpty()) {
        ui->AsmSourceCodeWidgetPane->setSourceCodePaneText(assemblerListingList.join("\n"));
    }
}

void MainWindow::on_actionEdit_Format_Microcode_triggered()
{
    if (ui->microcodeWidget->microAssemble()) {
        ui->microcodeWidget->setMicrocode(ui->microcodeWidget->getMicrocodeProgram()->format());
    }
}

void MainWindow::on_actionEdit_Remove_Error_Assembler_triggered()
{
    ui->AsmSourceCodeWidgetPane->removeErrorMessages();
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

void MainWindow::on_actionBuild_Microcode_triggered()
{
    if(ui->microcodeWidget->microAssemble()) {
        ui->statusBar->showMessage("MicroAssembly succeeded", 4000);
        ui->microObjectCodePane->setObjectCode(ui->microcodeWidget->getMicrocodeProgram(), nullptr);
    }
    else {
        ui->statusBar->showMessage("MicroAssembly failed", 4000);
    }
}

//Build Events
bool MainWindow::on_ActionBuild_Assemble_triggered()
{
    loadOperatingSystem();
    if(ui->AsmSourceCodeWidgetPane->assemble()){
#pragma message ("TODO: cancel run on burn and reset prog state")
        ui->AsmObjectCodeWidgetPane->setObjectCode(ui->AsmSourceCodeWidgetPane->getObjectCode());
        ui->AsmListingWidgetPane->setAssemblerListing(ui->AsmSourceCodeWidgetPane->getAssemblerListingList(),
                                                      ui->AsmSourceCodeWidgetPane->getAsmProgram()->getSymbolTable());
        ui->asmListingTracePane->onRemoveAllBreakpoints();
        controlSection->onRemoveAllPCBreakpoints();
        ui->asmListingTracePane->setProgram(ui->AsmSourceCodeWidgetPane->getAsmProgram());
        //ui->memoryTracePane->setMemoryTrace();
        set_Obj_Listing_filenames_from_Source();
        loadObjectCodeProgram();
        ui->statusBar->showMessage("Assembly succeeded", 4000);
        return true;
    }
    else {
#pragma message ("TODO: fix current file title if problematic")
        ui->AsmObjectCodeWidgetPane->clearObjectCode();
        ui->AsmListingWidgetPane->clearAssemblerListing();
        ui->asmListingTracePane->clearSourceCode();
        ui->asmListingTracePane->onRemoveAllBreakpoints();
        // ui->pepCodeTraceTab->setCurrentIndex(0); // Make source code pane visible
        loadObjectCodeProgram();
        ui->statusBar->showMessage("Assembly failed", 4000);
        return false;
    }

}

void MainWindow::on_actionBuild_Load_Object_triggered()
{
    loadOperatingSystem();
    loadObjectCodeProgram();
    ui->memoryWidget->refreshMemory();
}

void MainWindow::on_actionBuild_Run_Object_triggered()
{
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
    emit simulationFinished();
}

void MainWindow::on_actionBuild_Run_triggered()
{
    if(!on_ActionBuild_Assemble_triggered()) return;
    debugState = DebugState::RUN;
    if (initializeSimulation()) {
        disconnectViewUpdate();
        emit simulationStarted();
        ui->memoryWidget->updateMemory();
        controlSection->onSimulationStarted();
        controlSection->onRun();
        connectViewUpdate();

    }
    else {
        debugState = DebugState::DISABLED;
    }
    onSimulationFinished();
}

// Debug slots

void MainWindow::handleDebugButtons()
{
    Enu::EMnemonic mnemon = Pep::decodeMnemonic[memorySection->getMemoryByte(dataSection->getRegisterBankWord(CPURegisters::PC), false)];
    bool enable_into = (mnemon == Enu::EMnemonic::CALL) || Pep::isTrapMap[mnemon];
    int enabledButtons=0;
    switch(debugState)
    {
    case DebugState::DISABLED:
        enabledButtons = DebugButtons::RUN | DebugButtons::RUN_OBJECT| DebugButtons::DEBUG | DebugButtons::DEBUG_OBJECT | DebugButtons::DEBUG_LOADER;
        enabledButtons |= DebugButtons::BUILD_ASM | DebugButtons::BUILD_MICRO | DebugButtons::STATS_LEVELS;
        enabledButtons |= DebugButtons::OPEN_NEW;
        break;
    case DebugState::RUN:
        enabledButtons = DebugButtons::STOP | DebugButtons::INTERRUPT;
        break;
    case DebugState::DEBUG_ISA:
        enabledButtons = DebugButtons::INTERRUPT | DebugButtons::STOP | DebugButtons::RESTART | DebugButtons::CONTINUE;
        enabledButtons |= DebugButtons::SINGLE_STEP_ASM | DebugButtons::STEP_OUT_ASM | DebugButtons::STEP_OVER_ASM | DebugButtons::SINGLE_STEP_MICRO;
        enabledButtons |= DebugButtons::STEP_INTO_ASM*(enable_into);
        break;
    case DebugState::DEBUG_MICRO:
        enabledButtons = DebugButtons::INTERRUPT | DebugButtons::STOP | DebugButtons::RESTART | DebugButtons::CONTINUE;
        enabledButtons |= DebugButtons::SINGLE_STEP_ASM | DebugButtons::SINGLE_STEP_MICRO;
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
    if(!on_ActionBuild_Assemble_triggered()) return false;
    loadObjectCodeProgram();
    loadOperatingSystem();
    return on_actionDebug_Start_Debugging_Object_triggered();

}

bool MainWindow::on_actionDebug_Start_Debugging_Object_triggered()
{
    connectViewUpdate();
    debugState = DebugState::DEBUG_ISA;
    if(initializeSimulation()) {
        emit simulationStarted();
        ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->debuggerTab));
        ui->debuggerTabWidget->setCurrentIndex(ui->debuggerTabWidget->indexOf(ui->assemblerDebuggerTab));
        controlSection->onDebuggingStarted();
        ui->cpuWidget->startDebugging();
        ui->memoryWidget->updateMemory();
        return true;
    }
    return false;
}

bool MainWindow::on_actionDebug_Start_Debugging_Loader_triggered()
{
#pragma message("TODO: Implement 'Start Debugging Loader' once the operating system is implemented")
    return false;
}

void MainWindow::on_actionDebug_Stop_Debugging_triggered()
{
    connectViewUpdate();
    debugState = DebugState::DISABLED;
    ui->microcodeWidget->clearSimulationView();

    ui->microObjectCodePane->clearSimulationView();
    ui->microcodeWidget->setReadOnly(false);

    ui->cpuWidget->stopDebugging();
    handleDebugButtons();
    highlightActiveLines(true);
    ui->memoryWidget->updateMemory();
    controlSection->onDebuggingFinished();
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
        QMessageBox::warning(0, "Pep/9", controlSection->getErrorMessage());
        onSimulationFinished();
        return; // we'll just return here instead of letting it fail and go to the bottom
    }
    connectViewUpdate();
    if(controlSection->stoppedForBreakpoint()) {
        emit simulationUpdate();
        QApplication::processEvents();
        highlightActiveLines(true);
    }
    else onSimulationFinished();
}

void MainWindow::on_actionDebug_Restart_Debugging_triggered()
{
    on_actionDebug_Stop_Debugging_triggered();
    on_actionDebug_Start_Debugging_triggered();
}

void MainWindow::on_actionDebug_Single_Step_Assembler_triggered()
{
    debugState = DebugState::DEBUG_ISA;
    Enu::EMnemonic mnemon = Pep::decodeMnemonic[memorySection->getMemoryByte(dataSection->getRegisterBankWord(CPURegisters::PC), false)];
    bool isTrap = Pep::isTrapMap[mnemon];
    ui->debuggerTabWidget->setCurrentIndex(ui->debuggerTabWidget->indexOf(ui->assemblerDebuggerTab));
    if(isTrap && ui->actionSystem_Trace_Traps->isChecked()){ //If it is a trap instruction & we are tracing traps, step into the trap.
        on_actionDebug_Step_Into_Assembler_triggered();
    }
    else if(isTrap) { //If we aren't tracing traps, step over the trap
        on_actionDebug_Step_Over_Assembler_triggered();
    }
    else{ //Otherwise, execute a single ISA step
        controlSection->onISAStep();
        if (controlSection->hadErrorOnStep()) {
            // simulation had issues.
            QMessageBox::warning(0, "Pep/9", controlSection->getErrorMessage());
            onSimulationFinished();
        }
    }
    emit simulationUpdate();
}

void MainWindow::on_actionDebug_Step_Over_Assembler_triggered()
{
    debugState = DebugState::DEBUG_ISA;
    int callDepth = controlSection->getCallDepth();
    ui->debuggerTabWidget->setCurrentIndex(ui->debuggerTabWidget->indexOf(ui->assemblerDebuggerTab));
    //Disconnect any drawing functions, since very many steps might execute, and it would be wasteful to update the UI
    disconnectViewUpdate();
    do{
        controlSection->onISAStep();
        if (controlSection->hadErrorOnStep()) {
            // simulation had issues.
            QMessageBox::warning(0, "Pep/9", controlSection->getErrorMessage());
            onSimulationFinished();
            break;
        }
    } while(callDepth < controlSection->getCallDepth()
            && !controlSection->getExecutionFinished()
            && !controlSection->stoppedForBreakpoint());
    connectViewUpdate();
    emit simulationUpdate();
}

void MainWindow::on_actionDebug_Step_Into_Assembler_triggered()
{
    debugState = DebugState::DEBUG_ISA;
    ui->debuggerTabWidget->setCurrentIndex(ui->debuggerTabWidget->indexOf(ui->assemblerDebuggerTab));
    controlSection->onISAStep();
    if (controlSection->hadErrorOnStep()) {
        // simulation had issues.
        QMessageBox::warning(0, "Pep/9", controlSection->getErrorMessage());
        onSimulationFinished();
    }
    else if(false) 1; //Stub to leave room for breakpoint handler.
    emit simulationUpdate();
}

void MainWindow::on_actionDebug_Step_Out_Assembler_triggered()
{
    debugState = DebugState::DEBUG_ISA;
    ui->debuggerTabWidget->setCurrentIndex(ui->debuggerTabWidget->indexOf(ui->assemblerDebuggerTab));
    int callDepth = controlSection->getCallDepth();
    //Disconnect any drawing functions, since very many steps might execute, and it would be wasteful to update the UI
    disconnectViewUpdate();
    do{
        controlSection->onISAStep();
        if (controlSection->hadErrorOnStep()) {
            // simulation had issues.
            QMessageBox::warning(0, "Pep/9", controlSection->getErrorMessage());
            onSimulationFinished();
            break;
        }
    } while(callDepth <= controlSection->getCallDepth()
            && !controlSection->getExecutionFinished()
            && !controlSection->stoppedForBreakpoint());
    connectViewUpdate();
    emit simulationUpdate();
}

void MainWindow::on_actionDebug_Single_Step_Microcode_triggered()
{
    debugState = DebugState::DEBUG_MICRO;
        ui->debuggerTabWidget->setCurrentIndex(ui->debuggerTabWidget->indexOf(ui->microcodeDebuggerTab));
    controlSection->onStep();
    if (controlSection->hadErrorOnStep()) {
        // simulation had issues.
        QMessageBox::warning(0, "Pep/9", controlSection->getErrorMessage());
        onSimulationFinished();
    }
    // If the simulation hit a microcode breakpoint, step again so that the next line wil be selected.
    else if (controlSection->stoppedForBreakpoint()) {
        // Process events until onMicroBreakpointHit() is called, and then step so the next line is selected.
        QApplication::processEvents();
        controlSection->onStep();
    }

    emit simulationUpdate();
}

void MainWindow::onMicroBreakpointHit()
{
    debugState = DebugState::DEBUG_MICRO;
    QApplication::processEvents();
    controlSection->onMicroBreakpointHandled();
}

void MainWindow::onASMBreakpointHit()
{
    qDebug() << "trapped on PC = " << dataSection->getRegisterBankWord(CPURegisters::PC);
    debugState = DebugState::DEBUG_ISA;
    QApplication::processEvents();
    controlSection->onASMBreakpointHandled();
}

// System MainWindow triggers
void MainWindow::on_actionSystem_Clear_CPU_triggered()
{
    controlSection->onClearCPU();
    ui->cpuWidget->clearCpu();
}

void MainWindow::on_actionSystem_Clear_Memory_triggered()
{
    controlSection->onClearMemory();
    ui->memoryWidget->refreshMemory();
}

void MainWindow::on_actionStatistics_Level_All_triggered()
{
    if(ui->actionStatistics_Level_All->isEnabled()) controlSection->setDebugLevel(Enu::DebugLevels::ALL);
}

void MainWindow::on_actionStatistics_Level_Minimal_triggered()
{
    if(ui->actionStatistics_Level_Minimal->isEnabled()) controlSection->setDebugLevel(Enu::DebugLevels::MINIMAL);
}

void MainWindow::on_actionStatistics_Level_None_triggered()
{
    if(ui->actionStatistics_Level_None->isEnabled()) controlSection->setDebugLevel(Enu::DebugLevels::NONE);
}

void MainWindow::onSimulationFinished()
{
    QString errorString;
    on_actionDebug_Stop_Debugging_triggered();

    QVector<MicroCodeBase*> prog = ui->microcodeWidget->getMicrocodeProgram()->getObjectCode();
    for (MicroCodeBase* x : prog) {
         if (x->hasUnitPost()&&!((UnitPostCode*)x)->testPostcondition(dataSection, errorString)) {
             ((UnitPostCode*)x)->testPostcondition(dataSection, errorString);
             ui->microcodeWidget->appendMessageInSourceCodePaneAt(-1, errorString);
             QMessageBox::warning(this, "Pep9CPU", "Failed unit test");
             return;
         }
    }
    ui->statusBar->showMessage("Execution Finished", 4000);
}

void MainWindow::on_actionDark_Mode_triggered()
{
    inDarkMode = ui->actionDark_Mode->isChecked();
    if(inDarkMode) {
        this->setStyleSheet(darkStyle);
    }
    else {
        this->setStyleSheet(lightStyle);
    }
    emit darkModeChanged(inDarkMode);
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

void MainWindow::slotByteConverterBinEdited(const QString &str)
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

void MainWindow::slotByteConverterCharEdited(const QString &str)
{
    if (str.length() > 0) {
        int data = (int)str[0].toLatin1();
        byteConverterBin->setValue(data);
        byteConverterDec->setValue(data);
        byteConverterHex->setValue(data);
        byteConverterInstr->setValue(data);
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
        byteConverterInstr->setValue(data);
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
void MainWindow::focusChanged(QWidget *oldFocus, QWidget * newFocus)
{
    if(oldFocus == ui->microcodeWidget || newFocus == ui->microcodeWidget)
        ui->microcodeWidget->highlightOnFocus();

    if(oldFocus == ui->microObjectCodePane || newFocus == ui->microObjectCodePane)
        ui->microObjectCodePane->highlightOnFocus();

    if(oldFocus == ui->memoryWidget || newFocus == ui->memoryWidget)
        ui->memoryWidget->highlightOnFocus();

    if(oldFocus == ui->cpuWidget || newFocus == ui->cpuWidget)
        ui->cpuWidget->highlightOnFocus();

    if(oldFocus == ui->AsmSourceCodeWidgetPane || newFocus == ui->AsmSourceCodeWidgetPane)
        ui->AsmSourceCodeWidgetPane->highlightOnFocus();

    if(oldFocus == ui->AsmObjectCodeWidgetPane || newFocus == ui->AsmObjectCodeWidgetPane)
        ui->AsmObjectCodeWidgetPane->highlightOnFocus();

    if(oldFocus == ui->AsmListingWidgetPane || newFocus == ui->AsmListingWidgetPane)
        ui->AsmListingWidgetPane->highlightOnFocus();

    int which = 0;
    if (ui->microcodeWidget->hasFocus()) {
        which = EditButtons::COPY | EditButtons::CUT | EditButtons::PASTE;
        which |= EditButtons::UNDO*ui->microcodeWidget->isUndoable() | EditButtons::REDO*ui->microcodeWidget->isRedoable();
    }
    else if (ui->memoryWidget->hasFocus()) {
        which = 0;
    }
    else if (ui->cpuWidget->hasFocus()) {
        which = 0;
    }
    else if (ui->microObjectCodePane->hasFocus()) {
        which = EditButtons::COPY;
    }
    else if (ui->AsmSourceCodeWidgetPane->hasFocus()) {
        which = EditButtons::COPY | EditButtons::CUT | EditButtons::PASTE;
        which |= EditButtons::UNDO * ui->AsmSourceCodeWidgetPane->isUndoable() | EditButtons::REDO * ui->AsmSourceCodeWidgetPane->isRedoable();
    }
    else if (ui->AsmObjectCodeWidgetPane->hasFocus()) {
        which = EditButtons::COPY | EditButtons::CUT | EditButtons::PASTE;
        which |= EditButtons::UNDO * ui->AsmObjectCodeWidgetPane->isUndoable() | EditButtons::REDO * ui->AsmObjectCodeWidgetPane->isRedoable();
    }
    else if (ui->AsmListingWidgetPane->hasFocus()) {
        which = EditButtons::COPY;
    }
    else if (ui->ioWidget->isAncestorOf(QApplication::focusWidget())) {
        which = ui->ioWidget->editActions();
    }

    ui->actionEdit_Undo->setEnabled(which & EditButtons::UNDO);
    ui->actionEdit_Redo->setEnabled(which & EditButtons::REDO);
    ui->actionEdit_Cut->setEnabled(which & EditButtons::CUT);
    ui->actionEdit_Copy->setEnabled(which & EditButtons::COPY);
    ui->actionEdit_Paste->setEnabled(which & EditButtons::PASTE);
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
    else if (ui->AsmSourceCodeWidgetPane->hasFocus()) {
        ui->actionEdit_Undo->setEnabled(b);
    }
    else if (ui->AsmObjectCodeWidgetPane->hasFocus()) {
        ui->actionEdit_Undo->setEnabled(b);
    }
    else if (ui->microObjectCodePane->hasFocus()) {
        ui->actionEdit_Redo->setDisabled(true);
    }
    else if (ui->ioWidget->isAncestorOf(QApplication::focusWidget())) {
        ui->actionEdit_Undo->setEnabled(b);
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
    else if (ui->AsmSourceCodeWidgetPane->hasFocus()) {
        ui->actionEdit_Redo->setEnabled(b);
    }
    else if (ui->AsmObjectCodeWidgetPane->hasFocus()) {
        ui->actionEdit_Redo->setEnabled(b);
    }
    else if (ui->microObjectCodePane->hasFocus()) {
        ui->actionEdit_Redo->setDisabled(true);
    }
    else if (ui->ioWidget->isAncestorOf(QApplication::focusWidget())) {
        ui->actionEdit_Redo->setEnabled(b);
    }
}

void MainWindow::appendMicrocodeLine(QString line)
{
    ui->microcodeWidget->appendMessageInSourceCodePaneAt(-2, line);
}

void MainWindow::helpCopyToMicrocodeButtonClicked()
{
    if (maybeSave()) {
        ui->microcodeWidget->setMicrocode(helpDialog->getExampleText());
        ui->microcodeWidget->microAssemble();
        ui->microObjectCodePane->setObjectCode(ui->microcodeWidget->getMicrocodeProgram(),nullptr);
        helpDialog->hide();
        statusBar()->showMessage("Copied to microcode", 4000);
    }
}

void MainWindow::onInputRequested()
{
    ui->microcodeWidget->setEnabled(false);
    ui->cpuWidget->setEnabled(false);
}

void MainWindow::onInputReceived()
{
    if(controlSection->getExecutionFinished()==false)
    {
    }
    ui->microcodeWidget->setEnabled(true);
    ui->cpuWidget->setEnabled(true);
}
