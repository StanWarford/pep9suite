// File: micromainwindow.cpp
/*
    Pep9Micro is a complete CPU simulator for the Pep/9 instruction set,
    and is capable of assembling programs to object code, executing
    object code programs, and executing microcode fragments.

    Copyright (C) 2018  J. Stanley Warford & Matthew McRaven, Pepperdine University

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

#include "micromainwindow.h"
#include "ui_micromainwindow.h"
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

#include "about/aboutpep.h"
#include "assembler/asmargument.h"
#include "assembler/asmcode.h"
#include "assembler/asmprogram.h"
#include "assembler/asmprogrammanager.h"
#include "converters/byteconverterbin.h"
#include "converters/byteconverterchar.h"
#include "converters/byteconverterdec.h"
#include "converters/byteconverterhex.h"
#include "converters/byteconverterinstr.h"
#include "cpu/asmcpupane.h"
#include "cpu/registerfile.h"
#include "editor/asmprogramlistingpane.h"
#include "editor/asmsourcecodepane.h"
#include "editor/microcodepane.h"
#include "memory/amemorychip.h"
#include "memory/mainmemory.h"
#include "memory/memorychips.h"
#include "memory/memorydumppane.h"
#include "microassembler/microcode.h"
#include "microassembler/microcodeprogram.h"
#include "style/darkhelper.h"
#include "update/updatechecker.h"
#include "style/fonts.h"
#include "symbol/symboltable.h"

#include "cpudata.h"
#include "cpupane.h"
#include "decodertabledialog.h"
#include "fullmicrocodedcpu.h"
#include "microhelpdialog.h"
#include "microobjectcodepane.h"
#include "redefinemnemonicsdialog.h"



MicroMainWindow::MicroMainWindow(QWidget *parent) :
    QMainWindow(parent),ui(new Ui::MicroMainWindow), pep_version(new Pep9()),
    micro_assembler(Enu::CPUType::TwoByteDataBus, true),
    debugState(DebugState::DISABLED), codeFont(QFont(PepCore::codeFont, PepCore::codeFontSize)),
    updateChecker(new UpdateChecker()), isInDarkMode(false),
    memDevice(new MainMemory(nullptr)), controlSection(new FullMicrocodedCPU(AsmProgramManager::getInstance(), pep_version, memDevice)),
    dataSection(controlSection->getDataSection()), redefineMnemonicsDialog(new RedefineMnemonicsDialog(this)),
    decoderTableDialog(new DecoderTableDialog(nullptr)), programManager(AsmProgramManager::getInstance()), assembler(*programManager)

{
    // Initialize the memory subsystem
    QSharedPointer<RAMChip> ramChip(new RAMChip(1<<16, 0, memDevice.get()));
    memDevice->insertChip(ramChip, 0);
    // I/O chips will still need to be added later

    // Perform any additional setup needed for UI objects.
    ui->setupUi(this);
    // Install this class as the global event filter.
    qApp->installEventFilter(this);

    ui->memoryWidget->init(pep_version, memDevice, controlSection);
    ui->memoryWidget->showTitleLabel(false);
    ui->cpuWidget->init(controlSection, controlSection->getDataSection());
    ui->memoryTracePane->init(programManager, controlSection, memDevice, controlSection->getMemoryTrace());
    ui->assemblerPane->init(programManager);
    ui->asmProgramTracePane->init(pep_version, controlSection, programManager);
    ui->microcodeWidget->init(controlSection, true);
    ui->microObjectCodePane->init(controlSection, true);
    redefineMnemonicsDialog->init(false);
    // Pep/9's CPU model does not integrate with the cache, so the cache should not be shown.
    ui->executionStatisticsWidget->init(controlSection, true, false);

    // Create & connect all dialogs.
    helpDialog = new MicroHelpDialog(this);
    connect(helpDialog, &MicroHelpDialog::copyToSourceClicked, this, &MicroMainWindow::helpCopyToSourceClicked);
    // Load the about text and create the about dialog
    QFile aboutFile(":/help-micro/about.html");
    QString text = "";
    if(aboutFile.open(QFile::ReadOnly)) {
        text = QString(aboutFile.readAll());
    }
    QPixmap pixmap("://images/Pep9micro-icon.png");
    aboutPepDialog = new AboutPep(text, pixmap, this);

    connect(redefineMnemonicsDialog, &RedefineMnemonicsDialog::closed, this, &MicroMainWindow::redefine_Mnemonics_closed);
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
    connect(byteConverterBin, &ByteConverterBin::textEdited, this, &MicroMainWindow::slotByteConverterBinEdited);
    connect(byteConverterChar, &ByteConverterChar::textEdited, this, &MicroMainWindow::slotByteConverterCharEdited);
    connect(byteConverterDec, &ByteConverterDec::textEdited, this, &MicroMainWindow::slotByteConverterDecEdited);
    connect(byteConverterHex, &ByteConverterHex::textEdited, this, &MicroMainWindow::slotByteConverterHexEdited);

    connect(qApp, &QApplication::focusChanged, this, &MicroMainWindow::focusChanged);

    // Connect IOWidget to memory
    ui->ioWidget->bindToMemorySection(memDevice.get());
    // Connect IO events
    connect(memDevice.get(), &MainMemory::inputRequested, this, &MicroMainWindow::onInputRequested, Qt::QueuedConnection);
    connect(memDevice.get(), &MainMemory::outputWritten, this, &MicroMainWindow::onOutputReceived, Qt::QueuedConnection);

    // Connect Undo / Redo events
    connect(ui->microcodeWidget, &MicrocodePane::undoAvailable, this, &MicroMainWindow::setUndoability);
    connect(ui->microcodeWidget, &MicrocodePane::redoAvailable, this, &MicroMainWindow::setRedoability);
    connect(ui->assemblerPane, &AssemblerPane::undoAvailable, this, &MicroMainWindow::setUndoability);
    connect(ui->assemblerPane, &AssemblerPane::redoAvailable, this, &MicroMainWindow::setRedoability);
    connect(ui->ioWidget, &IOWidget::undoAvailable, this, &MicroMainWindow::setUndoability);
    connect(ui->ioWidget, &IOWidget::redoAvailable, this, &MicroMainWindow::setRedoability);

    // Connect simulation events.
    // Events that fire on simulationUpdate should be UniqueConnections, as they will be repeatedly connected and disconnected
    // via connectMicroDraw() and disconnectMicroDraw().
    connect(this, &MicroMainWindow::simulationUpdate, ui->cpuWidget, &CpuPane::onSimulationUpdate, Qt::UniqueConnection);
    connect(this, &MicroMainWindow::simulationUpdate, ui->memoryWidget, &MemoryDumpPane::updateMemory, Qt::UniqueConnection);
    connect(this, &MicroMainWindow::simulationUpdate, ui->memoryTracePane, &NewMemoryTracePane::updateTrace, Qt::UniqueConnection);
    connect(this, &MicroMainWindow::simulationStarted, ui->memoryWidget, &MemoryDumpPane::onSimulationStarted);
    connect(this, &MicroMainWindow::simulationStarted, ui->memoryTracePane, &NewMemoryTracePane::onSimulationStarted);
    connect(controlSection.get(), &FullMicrocodedCPU::hitBreakpoint, this, &MicroMainWindow::onBreakpointHit);

    // Clear IOWidget every time a simulation is started.
    connect(this, &MicroMainWindow::simulationStarted, ui->ioWidget, &IOWidget::onClear);
    connect(this, &MicroMainWindow::simulationStarted, ui->ioWidget, &IOWidget::onSimulationStart);

    connect(this, &MicroMainWindow::simulationStarted, ui->microObjectCodePane, &MicroObjectCodePane::onSimulationStarted);
    connect(this, &MicroMainWindow::simulationStarted, ui->executionStatisticsWidget, &ExecutionStatisticsWidget::onSimulationStarted);
    connect(ui->actionSystem_Clear_CPU, &QAction::triggered, ui->executionStatisticsWidget, &ExecutionStatisticsWidget::onClear);
    // Post finished events to the event queue so that they are processed after simulation updates.
    connect(this, &MicroMainWindow::simulationFinished, ui->microObjectCodePane, &MicroObjectCodePane::onSimulationFinished, Qt::QueuedConnection);
    connect(this, &MicroMainWindow::simulationFinished, controlSection.get(), &FullMicrocodedCPU::onSimulationFinished, Qt::QueuedConnection);
    connect(this, &MicroMainWindow::simulationFinished, ui->cpuWidget, &CpuPane::onSimulationFinished, Qt::QueuedConnection);
    connect(this, &MicroMainWindow::simulationFinished, ui->memoryWidget, &MemoryDumpPane::onSimulationFinished, Qt::QueuedConnection);
    connect(this, &MicroMainWindow::simulationFinished, ui->memoryTracePane, &NewMemoryTracePane::onSimulationFinished, Qt::QueuedConnection);
    connect(this, &MicroMainWindow::simulationFinished, ui->executionStatisticsWidget, &ExecutionStatisticsWidget::onSimulationFinished, Qt::QueuedConnection);

    // Connect MainWindow so that it can propogate simulationFinished event and clean up when execution is finished.
    connect(controlSection.get(), &FullMicrocodedCPU::simulationFinished, this, &MicroMainWindow::onSimulationFinished);


    // Connect simulation events that are internal to the class.
    connect(this, &MicroMainWindow::simulationUpdate, this, &MicroMainWindow::handleDebugButtons, Qt::UniqueConnection);
    connect(this, &MicroMainWindow::simulationUpdate, this, static_cast<void(MicroMainWindow::*)()>(&MicroMainWindow::highlightActiveLines), Qt::UniqueConnection);
    connect(this, &MicroMainWindow::simulationStarted, this, &MicroMainWindow::handleDebugButtons);

    // Connect font change events.
    connect(this, &MicroMainWindow::fontChanged, ui->microcodeWidget, &MicrocodePane::onFontChanged);
    connect(this, &MicroMainWindow::fontChanged, helpDialog, &MicroHelpDialog::onFontChanged);
    connect(this, &MicroMainWindow::fontChanged, ui->ioWidget, &IOWidget::onFontChanged);
    connect(this, &MicroMainWindow::fontChanged, ui->assemblerPane, &AssemblerPane::onFontChanged);
    connect(this, &MicroMainWindow::fontChanged, ui->memoryWidget, &MemoryDumpPane::onFontChanged);
    connect(this, &MicroMainWindow::fontChanged, ui->asmProgramTracePane, &AsmProgramTracePane::onFontChanged);

    // Connect dark mode events.
    connect(qApp, &QGuiApplication::paletteChanged, this, &MicroMainWindow::onPaletteChanged);
    connect(this, &MicroMainWindow::darkModeChanged, ui->microcodeWidget, &MicrocodePane::onDarkModeChanged);
    connect(this, &MicroMainWindow::darkModeChanged, helpDialog, &MicroHelpDialog::onDarkModeChanged);
    connect(this, &MicroMainWindow::darkModeChanged, ui->microObjectCodePane, &MicroObjectCodePane::onDarkModeChanged);
    connect(this, &MicroMainWindow::darkModeChanged, ui->cpuWidget, &CpuPane::onDarkModeChanged);
    connect(this, &MicroMainWindow::darkModeChanged, ui->microcodeWidget->getEditor(), &MicrocodeEditor::onDarkModeChanged);
    connect(this, &MicroMainWindow::darkModeChanged, ui->memoryWidget, &MemoryDumpPane::onDarkModeChanged);
    connect(this, &MicroMainWindow::darkModeChanged, ui->assemblerPane, &AssemblerPane::onDarkModeChanged);
    connect(this, &MicroMainWindow::darkModeChanged, ui->asmProgramTracePane, &AsmProgramTracePane::onDarkModeChanged);
    connect(this, &MicroMainWindow::darkModeChanged, ui->memoryTracePane, &NewMemoryTracePane::onDarkModeChanged);

    connect(ui->cpuWidget, &CpuPane::appendMicrocodeLine, this, &MicroMainWindow::appendMicrocodeLine);

    // Events that notify view of changes in model.
    // These events are disconnected whenevr "run" or "continue" are called, because they have significant overhead,
    // but the provide no benefit when running.
    // They are reconnected at the end of execution, and the receiving widgets are manually notified that changes may have occured.
    connect(memDevice.get(), &MainMemory::changed, ui->memoryWidget, &MemoryDumpPane::onMemoryChanged, Qt::ConnectionType::UniqueConnection);
    connect(ui->cpuWidget, &CpuPane::registerChanged, dataSection.get(), &CPUDataSection::onSetRegisterByte, Qt::ConnectionType::UniqueConnection);
    connect(dataSection.get(), &CPUDataSection::statusBitChanged, ui->cpuWidget, &CpuPane::onStatusBitChanged, Qt::ConnectionType::UniqueConnection);
    connect(dataSection.get(), &CPUDataSection::registerChanged, ui->cpuWidget, &CpuPane::onRegisterChanged, Qt::ConnectionType::UniqueConnection);
    connect(dataSection.get(), &CPUDataSection::memoryRegisterChanged, ui->cpuWidget, &CpuPane::onMemoryRegisterChanged, Qt::ConnectionType::UniqueConnection);

    // Connect events for breakpoints
    connect(ui->actionDebug_Remove_All_Microcode_Breakpoints, &QAction::triggered, ui->microcodeWidget, &MicrocodePane::onRemoveAllBreakpoints);
    connect(ui->actionDebug_Remove_All_Microcode_Breakpoints, &QAction::triggered,
            [&](){static_cast<InterfaceMCCPU*>(controlSection.get())->breakpointsRemoveAll();});
    connect(ui->microcodeWidget, &MicrocodePane::breakpointAdded,
            [&](quint16 address){static_cast<InterfaceMCCPU*>(controlSection.get())->breakpointAdded(address);});
    connect(ui->microcodeWidget, &MicrocodePane::breakpointRemoved,
            [&](quint16 address){static_cast<InterfaceMCCPU*>(controlSection.get())->breakpointRemoved(address);});


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
            [&](quint16 address){static_cast<InterfaceISACPU*>(controlSection.get())->breakpointAdded(address);});
    connect(programManager, &AsmProgramManager::breakpointRemoved,
            [&](quint16 address){static_cast<InterfaceISACPU*>(controlSection.get())->breakpointRemoved(address);});
    connect(programManager, &AsmProgramManager::setBreakpoints,
            [&](QSet<quint16> addresses){static_cast<InterfaceISACPU*>(controlSection.get())->breakpointsSet(addresses);});
    connect(programManager, &AsmProgramManager::removeAllBreakpoints,
            [&](){static_cast<InterfaceISACPU*>(controlSection.get())->breakpointsRemoveAll();});

    // Assemble default OS
    assembleDefaultOperatingSystem();

    // Initialize Microcode panes
    QFile file("://help-micro/pep9micro.pepmicro");
    if(file.open(QFile::ReadOnly | QFile::Text)){
        QTextStream in(&file);
        ui->microcodeWidget->setMicrocode(in.readAll());
        ui->microcodeWidget->setModifiedFalse();
        micro_assembler.setCPUType(dataSection->getCPUType());
        auto result = micro_assembler.assembleProgram(ui->microcodeWidget->getMicrocodeText());
        if(result.success) {
            ui->microObjectCodePane->setObjectCode(result.program, nullptr);
            controlSection->setMicrocodeProgram(result.program);
        }
        else {
            throw std::invalid_argument("Could not micro-assemble default microcode implementation.");
        }
    }

    // Initialize debug menu
    handleDebugButtons();

    // Read in settings
    readSettings();

    // Resize docking widgets because QT does a poor job of it.
    tabifyDockWidget(ui->memoryDockWdget, ui->memoryTraceDockWidget);
    ui->memoryDockWdget->raise();
    int scaleTotal = ui->memoryDockWdget->sizePolicy().verticalStretch()+ ui->ioDockWidget->sizePolicy().verticalStretch();
    double memory = geometry().height() * ui->memoryDockWdget->sizePolicy().verticalStretch() / static_cast<double>(scaleTotal);
    double io = geometry().height() * ui->ioDockWidget->sizePolicy().verticalStretch() / static_cast<double>(scaleTotal);
    resizeDocks({ui->memoryDockWdget, ui->ioDockWidget}, {static_cast<int>(memory),
                                                          static_cast<int>(io)}, Qt::Vertical);
    // Make the docks as large as the memory widget by default.
    resizeDocks({ui->memoryDockWdget, ui->ioDockWidget}, {static_cast<int>(ui->memoryWidget->maximumSize().width()),
                                                          static_cast<int>(ui->memoryWidget->maximumSize().width())}, Qt::Horizontal);

    // Create a new ASM file so that the dialog always has a file name in it.
    on_actionFile_New_Asm_triggered();

    // Make sure there is always an operating system loaded.
    // Otherwise, executing a object code program upon starting the application will
    // cause a crash.
    loadOperatingSystem();
}

MicroMainWindow::~MicroMainWindow()
{
    delete ui;
    delete helpDialog;
    delete aboutPepDialog;
}

void MicroMainWindow::changeEvent(QEvent *e)
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
void MicroMainWindow::closeEvent(QCloseEvent *event)
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

bool MicroMainWindow::eventFilter(QObject *, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if ((keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)) {
            if (ui->cpuWidget->hasFocus() && ui->actionDebug_Single_Step_Microcode->isEnabled()) {
                // single step or clock, depending of if currently debugging
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
            else if(ui->asmProgramTracePane->hasFocus() || ui->assemblerDebuggerTab->hasFocus()) {
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
            loadFile(fileEvent, PepCore::EPane::EListing);
            return true;
        }
        else if(fileEvent.endsWith("pepo", Qt::CaseInsensitive)) {
            loadFile(fileEvent, PepCore::EPane::EObject);
            return true;
        }
        else if(fileEvent.endsWith("pepcpu", Qt::CaseInsensitive)) {
            loadFile(fileEvent, PepCore::EPane::EMicrocode);
            return true;
        }
        else if(fileEvent.endsWith("pepmicro", Qt::CaseInsensitive)) {
            loadFile(fileEvent, PepCore::EPane::EMicrocode);
            return true;
        }
        else if(fileEvent.endsWith("pep", Qt::CaseInsensitive)) {
            loadFile(fileEvent, PepCore::EPane::ESource);
            return true;
        }
        return true;
    }
    // Touch events are giving CPU pane focus when it should be receiving it.
    // Therefore, accept all touch events to prevent them from getting to the CPU pane.
    else if(event->type() == QEvent::TouchBegin ||
            event->type() == QEvent::TouchEnd) {
        return true;
    }
    return false;
}

void MicroMainWindow::connectViewUpdate()
{
    connect(memDevice.get(), &MainMemory::changed, ui->memoryWidget, &MemoryDumpPane::onMemoryChanged, Qt::ConnectionType::UniqueConnection);
    connect(memDevice.get(), &MainMemory::changed, ui->memoryTracePane, &NewMemoryTracePane::onMemoryChanged, Qt::ConnectionType::UniqueConnection);
    connect(ui->cpuWidget, &CpuPane::registerChanged, dataSection.get(), &CPUDataSection::onSetRegisterByte, Qt::ConnectionType::UniqueConnection);
    connect(dataSection.get(), &CPUDataSection::statusBitChanged, ui->cpuWidget, &CpuPane::onStatusBitChanged, Qt::ConnectionType::UniqueConnection);
    connect(dataSection.get(), &CPUDataSection::registerChanged, ui->cpuWidget, &CpuPane::onRegisterChanged, Qt::ConnectionType::UniqueConnection);
    connect(dataSection.get(), &CPUDataSection::memoryRegisterChanged, ui->cpuWidget, &CpuPane::onMemoryRegisterChanged, Qt::ConnectionType::UniqueConnection);
    connect(this, &MicroMainWindow::simulationUpdate, ui->memoryWidget, &MemoryDumpPane::updateMemory, Qt::UniqueConnection);
    connect(this, &MicroMainWindow::simulationUpdate, ui->cpuWidget, &CpuPane::onSimulationUpdate, Qt::UniqueConnection);
    connect(this, &MicroMainWindow::simulationUpdate, this, &MicroMainWindow::handleDebugButtons, Qt::UniqueConnection);
    connect(this, &MicroMainWindow::simulationUpdate, this, static_cast<void(MicroMainWindow::*)()>(&MicroMainWindow::highlightActiveLines), Qt::UniqueConnection);
    // If application is running, active lines shouldn't be highlighted at the begin of the instruction, as this would be misleading.
    connect(this, &MicroMainWindow::simulationStarted, this, static_cast<void(MicroMainWindow::*)()>(&MicroMainWindow::highlightActiveLines), Qt::UniqueConnection);
    dataSection->setEmitEvents(true);
}

void MicroMainWindow::disconnectViewUpdate()
{
    // These signals are emitted for every memory address that is touched. I.e., a two-byte write would trigger this signal twice.
    //  Profiling indicates significant time is wasted updating the screen when executing instructions sequentially, e.g stepping over a DECI.
    disconnect(memDevice.get(), &MainMemory::changed, ui->memoryWidget,&MemoryDumpPane::onMemoryChanged);
    disconnect(memDevice.get(), &MainMemory::changed, ui->memoryTracePane, &NewMemoryTracePane::onMemoryChanged);

    disconnect(ui->cpuWidget, &CpuPane::registerChanged, dataSection.get(), &CPUDataSection::onSetRegisterByte);
    disconnect(dataSection.get(), &CPUDataSection::statusBitChanged, ui->cpuWidget, &CpuPane::onStatusBitChanged);
    disconnect(dataSection.get(), &CPUDataSection::registerChanged, ui->cpuWidget, &CpuPane::onRegisterChanged);
    disconnect(dataSection.get(), &CPUDataSection::memoryRegisterChanged, ui->cpuWidget, &CpuPane::onMemoryRegisterChanged);
    disconnect(this, &MicroMainWindow::simulationUpdate, ui->memoryWidget, &MemoryDumpPane::updateMemory);
    disconnect(this, &MicroMainWindow::simulationUpdate, ui->cpuWidget, &CpuPane::onSimulationUpdate);
    disconnect(this, &MicroMainWindow::simulationUpdate, this, &MicroMainWindow::handleDebugButtons);
    disconnect(this, &MicroMainWindow::simulationUpdate, this, static_cast<void(MicroMainWindow::*)()>(&MicroMainWindow::highlightActiveLines));
    disconnect(this, &MicroMainWindow::simulationStarted, this, static_cast<void(MicroMainWindow::*)()>(&MicroMainWindow::highlightActiveLines));
    dataSection->setEmitEvents(false);
}

void MicroMainWindow::readSettings()
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

    // Restore last used file path
    curPath = settings.value("filePath", QDir::homePath()).toString();
    // Restore dark mode state
    onDarkModeChanged();

    settings.endGroup();

    // Handle reading for all children
    ui->microcodeWidget->readSettings(settings);
    ui->assemblerPane->readSettings(settings);
}

void MicroMainWindow::writeSettings()
{
    QSettings settings;
    settings.beginGroup("MainWindow");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("font", codeFont);
    settings.setValue("filePath", curPath);
    settings.endGroup();
    //Handle writing for all children
    ui->microcodeWidget->writeSettings(settings);
    ui->assemblerPane->writeSettings(settings);
}

// Save methods
bool MicroMainWindow::save(PepCore::EPane which)
{
    bool retVal = true;
    /* For each pane, first check if there is already a file associated with the pane.
     * if there is not, then pass control to the save as function.
     * If there is a file, then attempt save to it, then remove any modified flags from the pane if it succeded.
     */
    switch(which) {
    case PepCore::EPane::ESource:
        if(ui->assemblerPane->getFileName(which).absoluteFilePath().isEmpty()) {
            retVal = saveAsFile(which);
        }
        else retVal = saveFile(ui->assemblerPane->getFileName(which).absoluteFilePath(), which);
        if(retVal) ui->assemblerPane->setModified(which, false);
        break;
    case PepCore::EPane::EObject:
        if(ui->assemblerPane->getFileName(which).absoluteFilePath().isEmpty()) {
            retVal = saveAsFile(which);
        }
        if(retVal) ui->assemblerPane->setModified(which, false);
        break;
    case PepCore::EPane::EListing:
        if(ui->assemblerPane->getFileName(PepCore::EPane::EListing).absoluteFilePath().isEmpty()) {
            retVal = saveAsFile(PepCore::EPane::EListing);
        }
        break;
    case PepCore::EPane::EMicrocode:
        if(QFileInfo(ui->microcodeWidget->getCurrentFile()).absoluteFilePath().isEmpty()) {
            retVal = saveAsFile(PepCore::EPane::EMicrocode);
        }
        else retVal = saveFile(ui->microcodeWidget->getCurrentFile().fileName(), PepCore::EPane::EMicrocode);
        if(retVal) {
            ui->microcodeWidget->setModifiedFalse();
        }
        break;
    default:
        // Provided a default - even though it should never occur -
        // to silence compiler warnings.
        break;
    }
    return retVal;
}

bool MicroMainWindow::maybeSave()
{
    static QMetaEnum metaenum = QMetaEnum::fromType<PepCore::EPane>();
    bool retVal = true;
    // Iterate over all the panes, and attempt to save any modified panes before closing.
    for(int it = 0; it < metaenum.keyCount(); it++) {
        retVal = retVal && maybeSave(static_cast<PepCore::EPane>(metaenum.value(it)));
    }
    return retVal;
}

bool MicroMainWindow::maybeSave(PepCore::EPane which)
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
    case PepCore::EPane::ESource:
        if(ui->assemblerPane->isModified(which)) {
            ret = QMessageBox::warning(this, dlgTitle, sourceText, buttons);
            if (ret == QMessageBox::Save)
                retVal = save(PepCore::EPane::ESource);
            else if (ret == QMessageBox::Cancel)
                retVal = false;
        }
        break;
    case PepCore::EPane::EObject:
        if(ui->assemblerPane->isModified(which)) {
            ret = QMessageBox::warning(this, dlgTitle, objectText, buttons);
            if (ret == QMessageBox::Save)
                retVal = save(PepCore::EPane::EObject);
            else if (ret == QMessageBox::Cancel)
                retVal = false;
        }
        break;
    case PepCore::EPane::EMicrocode:
        if(ui->microcodeWidget->isModified()) {
            ret = QMessageBox::warning(this, dlgTitle,microcodeText,buttons);
            if (ret == QMessageBox::Save)
                retVal = save(PepCore::EPane::EMicrocode);
            else if (ret == QMessageBox::Cancel)
                retVal = false;
        }
        break;
    default:
        break;
    }
    return retVal;
}

void MicroMainWindow::loadFile(const QString &fileName, PepCore::EPane which)
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
    case PepCore::EPane::ESource:
        ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->assemblerTab));
        ui->assemblerPane->loadSourceFile(QFileInfo(file).absoluteFilePath(), in.readAll());
        emit ui->actionDebug_Remove_All_Assembly_Breakpoints->trigger();
        break;
    case PepCore::EPane::EObject:
        ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->assemblerTab));
        ui->assemblerPane->loadObjectFile(QFileInfo(file).absoluteFilePath(), in.readAll());
        break;
    case PepCore::EPane::EMicrocode:
        ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->debuggerTab));
        ui->debuggerTabWidget->setCurrentIndex(ui->debuggerTabWidget->indexOf(ui->microcodeDebuggerTab));
        ui->microcodeWidget->setFocus();
        ui->microcodeWidget->setCurrentFile(fileName);
        ui->microcodeWidget->setMicrocode(Pep::removeCycleNumbers(in.readAll()));
        ui->microcodeWidget->setModifiedFalse();
        emit ui->actionDebug_Remove_All_Microcode_Breakpoints->trigger();
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

bool MicroMainWindow::saveFile(PepCore::EPane which)
{
    QString fileName;
    switch(which)
    {
    //Get the filename associated with the dialog
    case PepCore::EPane::ESource:
        fileName = QFileInfo(ui->assemblerPane->getFileName(which)).absoluteFilePath();
        break;
    case PepCore::EPane::EObject:
        fileName = QFileInfo(ui->assemblerPane->getFileName(which)).absoluteFilePath();
        break;
    case PepCore::EPane::EListing:
        fileName = QFileInfo(ui->assemblerPane->getFileName(which)).absoluteFilePath();
        break;
    case PepCore::EPane::EMicrocode:
        fileName = QFileInfo(ui->microcodeWidget->getCurrentFile()).absoluteFilePath();
        break;
    default:
        // Provided a default - even though it should never occur -
        // to silence compiler warnings.
        break;
    }
    return saveFile(fileName, which);
}

bool MicroMainWindow::saveFile(const QString &fileName, PepCore::EPane which)
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
    case PepCore::EPane::ESource:
        out << ui->assemblerPane->getPaneContents(which);
        msgOutput = &msgSource;
        break;
    case PepCore::EPane::EObject:
        out << ui->assemblerPane->getPaneContents(which);
        msgOutput = &msgObject;
        break;
    case PepCore::EPane::EListing:
        out << ui->assemblerPane->getPaneContents(which);
        msgOutput = &msgListing;
        break;
    case PepCore::EPane::EMicrocode:
        out << Pep::addCycleNumbers(ui->microcodeWidget->getMicrocodeText());
        msgOutput = &msgMicro;
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

bool MicroMainWindow::saveAsFile(PepCore::EPane which)
{
    // Default filenames for each pane.
    static const QString defSourceFile = "untitled.pep";
    static const QString defObjectFile = "untitled.pepo";
    static const QString defListingFile = "untitled.pepl";
    static const QString defMicroFile = "untitled.pepmicro";
    QString usingFile;

    // Titles for each pane.
    static const QString titleBase = "Save %1";
    static const QString sourceTitle = titleBase.arg("Assembler Source Code");
    static const QString objectTitle = titleBase.arg("Object Code");
    static const QString listingTitle = titleBase.arg("Assembler Listing");
    static const QString microTitle = titleBase.arg("Microcode");
    const QString *usingTitle;

    // Patterns for source code files.
    static const QString sourceTypes = "Pep/9 Source (*.pep *.txt)";
    static const QString objectTypes = "Pep/9 Object (*.pepo *.txt)";
    static const QString listingTypes = "Pep/9 Listing (*.pepl)";
    static const QString microTypes = "Pep/9 Microcode (*.pepmicro *.pepcpu  *.txt)";
    const QString *usingTypes;

    QFileInfo file;
    /*
     * For each pane, get the file associated with each pane, or the default if there is none.
     * Set the title and source code patterns for the pane.
     */
    switch(which)
    {
    case PepCore::EPane::ESource:
        file = ui->assemblerPane->getFileName(which);
        if(file.fileName().isEmpty()) {
            usingFile = QDir(curPath).absoluteFilePath(defSourceFile);
        }
        else usingFile = file.fileName();
        usingTitle = &sourceTitle;
        usingTypes = &sourceTypes;
        break;
    case PepCore::EPane::EObject:
        file = ui->assemblerPane->getFileName(which);
        if(file.fileName().isEmpty()) {
            usingFile = QDir(curPath).absoluteFilePath(defObjectFile);
        }
        else usingFile = file.fileName();
        usingTitle = &objectTitle;
        usingTypes = &objectTypes;
        break;
    case PepCore::EPane::EListing:
        file = ui->assemblerPane->getFileName(which);
        if(file.fileName().isEmpty()) {
            usingFile = QDir(curPath).absoluteFilePath(defListingFile);
        }
        else usingFile = file.fileName();
        usingTitle = &listingTitle;
        usingTypes = &listingTypes;
        break;
    case PepCore::EPane::EMicrocode:
        if(ui->microcodeWidget->getCurrentFile().fileName().isEmpty()) {
            usingFile = QDir(curPath).absoluteFilePath(defMicroFile);
        }
        else usingFile = ui->microcodeWidget->getCurrentFile().fileName();
        usingTitle = &microTitle;
        usingTypes = &microTypes;
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
        case PepCore::EPane::ESource:
            ui->assemblerPane->setFileName(which, fileInfo);
            break;
        case PepCore::EPane::EObject:
            ui->assemblerPane->setFileName(which, fileInfo);
            break;
        case PepCore::EPane::EListing:
            ui->assemblerPane->setFileName(which, fileInfo);
            break;
        case PepCore::EPane::EMicrocode:
            ui->microcodeWidget->setCurrentFile(fileName);
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

QString MicroMainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void MicroMainWindow::print(PepCore::EPane which)
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
    PepMicroHighlighter* mcHi;
    switch(which)
    {
    case PepCore::EPane::ESource:
        title = &source;
        document.setPlainText(ui->assemblerPane->getPaneContents(which));
        asHi = new PepASMHighlighter(PepColors::lightMode, &document);
        hi = asHi;
        hi->rehighlight();
        break;
    case PepCore::EPane::EObject:
        title = &object;
        document.setPlainText(ui->assemblerPane->getPaneContents(which));
        break;
    case PepCore::EPane::EListing:
        title = &listing;
        document.setPlainText(ui->assemblerPane->getPaneContents(which));
        asHi = new PepASMHighlighter(PepColors::lightMode, &document);
        hi = asHi;
        hi->rehighlight();
        break;
    case PepCore::EPane::EMicrocode:
        title = &micro;
        document.setPlainText(ui->microcodeWidget->toPlainText());
        mcHi = new PepMicroHighlighter(Enu::CPUType::TwoByteDataBus,
                true, PepColors::lightMode, &document);
        mcHi->forceAllFeatures(true);
        hi = mcHi;
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

void MicroMainWindow::assembleDefaultOperatingSystem()
{
    // Need to assemble operating system.
    QString defaultOSText = Pep::resToString(":/help-micro/alignedIO-OS.pep", false);
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
QVector<quint8> convertObjectCodeToIntArray(QString line, bool& success)
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

void MicroMainWindow::loadOperatingSystem()
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
    memDevice->loadValues(programManager->getOperatingSystem()->getBurnAddress(), values);
}

bool MicroMainWindow::loadObjectCodeProgram()
{
    // Get the object code, and convert it to an integer array.
    QString lines = ui->assemblerPane->getPaneContents(PepCore::EPane::EObject);
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

void MicroMainWindow::set_Obj_Listing_filenames_from_Source()
{
    ui->assemblerPane->setFilesFromSource();
}

void MicroMainWindow::debugButtonEnableHelper(const int which)
{
    // Crack the parameter using DebugButtons to properly enable
    // and disable all buttons related to debugging and running.
    // Build Actions
    ui->actionBuild_Assemble->setEnabled(which & DebugButtons::BUILD_ASM);
    ui->actionEdit_Remove_Error_Assembler->setEnabled(which & DebugButtons::BUILD_ASM);
    ui->actionEdit_Format_Assembler->setEnabled((which & DebugButtons::BUILD_ASM));
    ui->actionBuild_Load_Object->setEnabled(which & DebugButtons::BUILD_ASM);
    ui->actionBuild_Microcode->setEnabled(which & DebugButtons::BUILD_MICRO);
    ui->actionEdit_Remove_Error_Microcode->setEnabled(which & DebugButtons::BUILD_MICRO);
    ui->actionEdit_Format_Microcode->setEnabled((which & DebugButtons::BUILD_MICRO));

    // Debug & Run Actions
    ui->actionBuild_Run->setEnabled(which & DebugButtons::RUN);
    ui->actionBuild_Execute->setEnabled(which);
    ui->actionBuild_Run_Object->setEnabled(which & DebugButtons::RUN_OBJECT);
    ui->actionDebug_Start_Debugging->setEnabled(which & DebugButtons::DEBUG);
    ui->actionDebug_Start_Debugging_Object->setEnabled(which & DebugButtons::DEBUG_OBJECT);
    ui->actionDebug_Start_Debugging_Loader->setEnabled(which & DebugButtons::DEBUG_LOADER);
    ui->actionDebug_Start_Debugging_Microcode->setEnabled(which & DebugButtons::DEBUG_MICRO);
    ui->actionDebug_Interupt_Execution->setEnabled(which & DebugButtons::INTERRUPT);
    ui->actionDebug_Continue->setEnabled(which & DebugButtons::CONTINUE);
    ui->actionDebug_Stop_Debugging->setEnabled(which & DebugButtons::STOP);
    ui->actionDebug_Single_Step_Assembler->setEnabled(which & DebugButtons::STEP_OVER_ASM);
    ui->actionDebug_Step_Over_Assembler->setEnabled(which & DebugButtons::STEP_OVER_ASM);
    ui->actionDebug_Step_Into_Assembler->setEnabled(which & DebugButtons::STEP_INTO_ASM);
    ui->actionDebug_Step_Out_Assembler->setEnabled(which & DebugButtons::STEP_OUT_ASM);
    ui->actionDebug_Single_Step_Microcode->setEnabled(which & DebugButtons::SINGLE_STEP_MICRO);

    // File open & new actions
    ui->actionFile_New_Asm->setEnabled(which & DebugButtons::OPEN_NEW);
    ui->actionFile_New_Microcode->setEnabled(which & DebugButtons::OPEN_NEW);
    ui->actionFile_Open->setEnabled(which & DebugButtons::OPEN_NEW);

    // Operating Assembly / Mnemonic Redefinition actions
    ui->actionSystem_Redefine_Mnemonics->setEnabled(which & DebugButtons::INSTALL_OS);
    ui->actionSystem_Reinstall_Default_OS->setEnabled(which & DebugButtons::INSTALL_OS);
    ui->actionSystem_Assemble_Install_New_OS->setEnabled(which & DebugButtons::INSTALL_OS);
    ui->actionSystem_Code_Fragment->setEnabled(which & DebugButtons::INSTALL_OS);
    ui->actionSystem_Complete_Microcode->setEnabled(which & DebugButtons::INSTALL_OS);
    ui->actionSystem_Redefine_Decoder_Tables->setEnabled(which & DebugButtons::INSTALL_OS);

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
    // If the table to redefine decoder entries is visible, and one is unable to build microcode
    // (i.e. the simulation is running), then make sure the dialog is hidden.
    // It would be dangerous to allow microcode menmonics to be modified as a simulation is ongoing.
    if(!(which & DebugButtons::INSTALL_OS) && decoderTableDialog->isVisible()) {
        decoderTableDialog->hide();
    }
}

void MicroMainWindow::highlightActiveLines()
{
    //always highlight the current microinstruction
    ui->microcodeWidget->updateSimulationView();
    ui->microObjectCodePane->highlightCurrentInstruction();
    //If the µPC is 0, if a breakpoint has been reached, or if the microcode has a breakpoint, rehighlight the ASM views.
    ui->memoryWidget->clearHighlight();
    ui->memoryWidget->highlight();
    if(controlSection->atMicroprogramStart() || controlSection->stoppedForBreakpoint()) {
        ui->asmProgramTracePane->updateSimulationView();
    }
}

bool MicroMainWindow::initializeSimulation()
{
    // Load microprogram into the micro control store
    micro_assembler.setCPUType(dataSection->getCPUType());
    auto result = micro_assembler.assembleProgram(ui->microcodeWidget->getMicrocodeText());
    for(auto& [line, message] : result.errorMessages) {
        ui->microcodeWidget->appendMessageInSourceCodePaneAt(line, message);
    }
    if (result.success) {
        ui->statusBar->showMessage("MicroAssembly succeeded", 4000);
        if(result.program->hasMicrocode() == false) {
            ui->statusBar->showMessage("No microcode program to build", 4000);
            return false;
        }
        ui->microObjectCodePane->setObjectCode(result.program, nullptr);
        controlSection->setMicrocodeProgram(result.program);
        static_cast<InterfaceMCCPU*>(controlSection.get())->breakpointsSet(ui->microcodeWidget->getBreakpoints());
    }
    else {
        ui->statusBar->showMessage("MicroAssembly failed", 4000);
        ui->microObjectCodePane->setObjectCode();
        controlSection->setMicrocodeProgram(nullptr);
        return false;
    }

    // Always clear CPU state, as maintaining state makes no sense in an
    // ISA level simulation. This differs from Pep9CPU where the application
    // only clears the CPU when preconditions are hit.
    controlSection->onResetCPU();
    controlSection->initCPU();
    ui->cpuWidget->clearCpu();

    // Don't allow the microcode pane to be edited while the program is running
    ui->microcodeWidget->setReadOnly(true);

    CPUDataSection* data = this->dataSection.get();
    AMemoryDevice* memory = this->memDevice.get();
    // If there are preconditions then apply them.
    if(controlSection->getProgram()->hasUnitPre()) {
        // Unlike Pep9CPU, do not clear all of memory. With the assembly level features,
        // we need to make sure that operating system / user program does not get removed from memory,
        // which would break the simulator whenever microprograms contained preconditions.
        for(auto line : controlSection->getProgram()->getObjectCode()) {
            if(line->hasUnitPre()) {
                static_cast<UnitPreCode*>(line)->setUnitPre(data, memory);
            }
        }
    }

    // No longer emits simulationStarted(), as this could trigger extra screen painting that is unwanted.
    return true;
}

void MicroMainWindow::onUpdateCheck(int /*val*/)
{
    // Dummy to handle update checking code
}

// File MainWindow triggers
void MicroMainWindow::on_actionFile_New_Asm_triggered()
{
    // Try to save source code before clearing it, the object code pane, and the listing pane.
    if (maybeSave(PepCore::EPane::ESource)) {
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

void MicroMainWindow::on_actionFile_New_Microcode_triggered()
{
    //Try to save the microcode pane before clearing it & the micro-object-code pane.
    if (maybeSave(PepCore::EPane::EMicrocode)) {
        ui->actionDebug_Remove_All_Microcode_Breakpoints->trigger();
        ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->debuggerTab));
        ui->microcodeWidget->setFocus();
        ui->microcodeWidget->setMicrocode("");
        ui->microcodeWidget->setCurrentFile("");
        ui->microObjectCodePane->setObjectCode();
        emit ui->actionDebug_Remove_All_Microcode_Breakpoints->trigger();
    }
}

void MicroMainWindow::on_actionFile_Open_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(
                this,
                "Open text file",
                curPath,
                "Pep/9 files (*.pep *.pepo *.pepl *.pepmicro *.pepcpu *.txt)");
    // If we don't recognize an extension, assume it is an assembler source document
    PepCore::EPane which = PepCore::EPane::ESource;
    // Depending on the file ending, change which pane will be loaded into.
    if (!fileName.isEmpty()) {
        if(fileName.endsWith("pep", Qt::CaseInsensitive)) which = PepCore::EPane::ESource;
        else if(fileName.endsWith("pepo", Qt::CaseInsensitive)) which = PepCore::EPane::EObject;
        else if(fileName.endsWith("pepl", Qt::CaseInsensitive)) which = PepCore::EPane::EListing;
        else if(fileName.endsWith("pepmicro", Qt::CaseInsensitive)) which = PepCore::EPane::EMicrocode;
        else if(fileName.endsWith("pepcpu", Qt::CaseInsensitive)) which = PepCore::EPane::EMicrocode;
        if(maybeSave(which)) {
            loadFile(fileName, which);
        }
        curPath = QFileInfo(fileName).absolutePath();
    }
}

bool MicroMainWindow::on_actionFile_Save_Asm_triggered()
{
    return save(PepCore::EPane::ESource);
}

bool MicroMainWindow::on_actionFile_Save_Microcode_triggered()
{
    return save(PepCore::EPane::EMicrocode);
}

bool MicroMainWindow::on_actionFile_Save_Asm_Source_As_triggered()
{
    return saveAsFile(PepCore::EPane::ESource);
}

bool MicroMainWindow::on_actionFile_Save_Object_Code_As_triggered()
{
    return saveAsFile(PepCore::EPane::EObject);
}

bool MicroMainWindow::on_actionFile_Save_Assembler_Listing_As_triggered()
{
    return saveAsFile(PepCore::EPane::EListing);
}

bool MicroMainWindow::on_actionFile_Save_Microcode_As_triggered()
{
    return saveAsFile(PepCore::EPane::EMicrocode);
}

void MicroMainWindow::on_actionFile_Print_Assembler_Source_triggered()
{
    print(PepCore::EPane::ESource);
}

void MicroMainWindow::on_actionFile_Print_Object_Code_triggered()
{
    print(PepCore::EPane::EObject);
}

void MicroMainWindow::on_actionFile_Print_Assembler_Listing_triggered()
{
    print(PepCore::EPane::EListing);
}

void MicroMainWindow::on_actionFile_Print_Microcode_triggered()
{
    print(PepCore::EPane::EMicrocode);
}

// Edit MainWindow triggers

void MicroMainWindow::on_actionEdit_Undo_triggered()
{
    // Other Pep9Cpu panes do not support undo
    if (ui->microcodeWidget->hasFocus() && ui->actionDebug_Start_Debugging->isEnabled()) {
        ui->microcodeWidget->undo();
    }
    // However, the Pep9 panes do
    else if(ui->assemblerPane->isAncestorOf(focusWidget())) {
        ui->assemblerPane->undo();
    }
    else if (ui->ioWidget->isAncestorOf(QApplication::focusWidget())) {
        ui->ioWidget->undo();
    }
}

void MicroMainWindow::on_actionEdit_Redo_triggered()
{
    // Other Pep9Cpu panes do not support redo
    if (ui->microcodeWidget->hasFocus() && ui->actionDebug_Start_Debugging->isEnabled()) {
        ui->microcodeWidget->redo();
    }
    // However, the Pep9 panes do
    else if(ui->assemblerPane->isAncestorOf(focusWidget())) {
        ui->assemblerPane->redo();
    }
    else if (ui->ioWidget->isAncestorOf(QApplication::focusWidget())) {
        ui->ioWidget->redo();
    }
}

void MicroMainWindow::on_actionEdit_Cut_triggered()
{
    if (ui->microcodeWidget->hasFocus() && ui->actionDebug_Start_Debugging->isEnabled()) {
        ui->microcodeWidget->cut();
    }
    else if(ui->assemblerPane->isAncestorOf(focusWidget())) {
        ui->assemblerPane->cut();
    }
    else if (ui->ioWidget->isAncestorOf(QApplication::focusWidget())) {
        ui->ioWidget->cut();
    }
}

void MicroMainWindow::on_actionEdit_Copy_triggered()
{
    if (ui->microcodeWidget->hasFocus()) {
        ui->microcodeWidget->copy();
    }
    else if (helpDialog->hasFocus()) {
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

void MicroMainWindow::on_actionEdit_Paste_triggered()
{
    if (ui->microcodeWidget->hasFocus() && ui->actionDebug_Start_Debugging->isEnabled()) {
        ui->microcodeWidget->paste();
    }
    else if(ui->assemblerPane->isAncestorOf(focusWidget())) {
        ui->assemblerPane->paste();
    }
    else if (ui->ioWidget->isAncestorOf(QApplication::focusWidget())) {
        ui->ioWidget->paste();
    }
    // other panes should not be able to paste
}

void MicroMainWindow::on_actionEdit_UnComment_Line_triggered()
{
    if (!ui->actionDebug_Stop_Debugging->isEnabled()) { // we are not debugging
        ui->microcodeWidget->unCommentSelection();
    }
}

void MicroMainWindow::on_actionEdit_Format_Assembler_triggered()
{
    ui->assemblerPane->removeErrorMessages();
    auto out = AsmOutput();
    out.success = assembler.assembleUserProgram(ui->assemblerPane->getPaneContents(PepCore::EPane::ESource), out.prog, out.errors);

    if(out.errors.size() != 0) {
        ui->assemblerPane->addErrorsToSource(out.errors);
    }
    if(out.success){
        ui->assemblerPane->setPanesFromProgram(out);
        programManager->setUserProgram(out.prog);
        ui->asmProgramTracePane->onRemoveAllBreakpoints();
        static_cast<InterfaceISACPU*>(controlSection.get())->breakpointsRemoveAll();
        ui->asmProgramTracePane->setProgram(out.prog);
        set_Obj_Listing_filenames_from_Source();
        ui->statusBar->showMessage("Assembly succeeded", 4000);
        handleDebugButtons();
    }
    else {
        ui->assemblerPane->clearPane(PepCore::EPane::EObject);
        ui->assemblerPane->clearPane(PepCore::EPane::EListing);
        ui->asmProgramTracePane->clearSourceCode();
        ui->asmProgramTracePane->onRemoveAllBreakpoints();
        loadObjectCodeProgram();
        ui->statusBar->showMessage("Assembly failed", 4000);
    }

    QString code = out.prog->getFormattedSourceCode();
    ui->assemblerPane->setPaneContents(PepCore::EPane::ESource, code);
}

void MicroMainWindow::on_actionEdit_Format_Microcode_triggered()
{
    micro_assembler.setCPUType(dataSection->getCPUType());
    auto result = micro_assembler.assembleProgram(ui->microcodeWidget->getMicrocodeText());
    if(result.success) {
        ui->microcodeWidget->setMicrocode(result.program->format());
    }
    for(auto& [line, message] : result.errorMessages) {
        ui->microcodeWidget->appendMessageInSourceCodePaneAt(line, message);
    }
}

void MicroMainWindow::on_actionEdit_Remove_Error_Assembler_triggered()
{
    ui->assemblerPane->removeErrorMessages();
}

void MicroMainWindow::on_actionEdit_Remove_Error_Microcode_triggered()
{
    ui->microcodeWidget->removeErrorMessages();
}

void MicroMainWindow::on_actionEdit_Font_triggered()
{
    bool ok = false;
    QFont font  = QFontDialog::getFont(&ok, codeFont, this, "Set Source Code Font");
    if(ok) {
        codeFont = font;
        emit fontChanged(codeFont);
    }
}

void MicroMainWindow::on_actionEdit_Reset_font_to_Default_triggered()
{
    codeFont = QFont(PepCore::codeFont, PepCore::codeFontSize);
    emit fontChanged(codeFont);
}

void MicroMainWindow::on_actionBuild_Microcode_triggered()
{
    micro_assembler.setCPUType(dataSection->getCPUType());
    auto result = micro_assembler.assembleProgram(ui->microcodeWidget->getMicrocodeText());
    for(auto& [line, message] : result.errorMessages) {
        ui->microcodeWidget->appendMessageInSourceCodePaneAt(line, message);
    }
    if(result.success) {
        ui->statusBar->showMessage("MicroAssembly succeeded", 4000);
        ui->microObjectCodePane->setObjectCode(result.program, nullptr);
        controlSection->setMicrocodeProgram(result.program);
    }
    else {
        ui->statusBar->showMessage("MicroAssembly failed", 4000);
        ui->microObjectCodePane->setObjectCode();
        controlSection->setMicrocodeProgram(nullptr);
    }
}

//Build Events
bool MicroMainWindow::on_actionBuild_Assemble_triggered()
{
    ui->assemblerPane->removeErrorMessages();
    auto out = AsmOutput();
    out.success = assembler.assembleUserProgram(ui->assemblerPane->getPaneContents(PepCore::EPane::ESource), out.prog, out.errors);

    if(out.errors.size() != 0) {
        ui->assemblerPane->addErrorsToSource(out.errors);
    }
    if(out.success){
        ui->assemblerPane->setPanesFromProgram(out);
        programManager->setUserProgram(out.prog);
        ui->asmProgramTracePane->onRemoveAllBreakpoints();
        static_cast<InterfaceISACPU*>(controlSection.get())->breakpointsRemoveAll();
        ui->asmProgramTracePane->setProgram(out.prog);
        set_Obj_Listing_filenames_from_Source();
        ui->statusBar->showMessage("Assembly succeeded", 4000);
        handleDebugButtons();
        return true;
    }
    else {
        ui->assemblerPane->clearPane(PepCore::EPane::EObject);
        ui->assemblerPane->clearPane(PepCore::EPane::EListing);
        ui->asmProgramTracePane->clearSourceCode();
        ui->asmProgramTracePane->onRemoveAllBreakpoints();
        loadObjectCodeProgram();
        ui->statusBar->showMessage("Assembly failed", 4000);
        return false;
    }

}

void MicroMainWindow::on_actionBuild_Load_Object_triggered()
{
    loadOperatingSystem();
    loadObjectCodeProgram();
    ui->memoryWidget->refreshMemory();
    ui->memoryWidget->clearHighlight();
}

void MicroMainWindow::on_actionBuild_Execute_triggered()
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

void MicroMainWindow::on_actionBuild_Run_triggered()
{
    if(!on_actionBuild_Assemble_triggered()) return;
    loadOperatingSystem();
    loadObjectCodeProgram();
    debugState = DebugState::RUN;
    if (initializeSimulation()) {
        ui->asmProgramTracePane->startSimulationView();
        disconnectViewUpdate();
        memDevice->clearAllByteCaches();
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

void MicroMainWindow::on_actionBuild_Run_Object_triggered()
{
    // Must load operating system, as this will not be done by
    // on_actionBuild_Execute_triggered().
    loadOperatingSystem();
    if(loadObjectCodeProgram()) {
        on_actionBuild_Execute_triggered();
    }
}

// Debug slots

void MicroMainWindow::handleDebugButtons()
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
        enabledButtons |= DebugButtons::BUILD_ASM | DebugButtons::BUILD_MICRO;
        enabledButtons |= DebugButtons::OPEN_NEW | DebugButtons::INSTALL_OS | DebugButtons::DEBUG_MICRO;
        enabledButtons |= DebugButtons::CLEAR;
        break;
    case DebugState::RUN:
        enabledButtons = DebugButtons::STOP | DebugButtons::INTERRUPT;
        break;
    case DebugState::DEBUG_ISA:
        enabledButtons = DebugButtons::INTERRUPT | DebugButtons::STOP | DebugButtons::CONTINUE*(!waiting_io);
        enabledButtons |= DebugButtons::STEP_OUT_ASM*(!waiting_io);
        enabledButtons |= DebugButtons::STEP_OVER_ASM*(!waiting_io) | DebugButtons::SINGLE_STEP_MICRO*(!waiting_io);
        enabledButtons |= DebugButtons::STEP_INTO_ASM*(enable_into * !waiting_io);
        break;
    case DebugState::DEBUG_MICRO:
        enabledButtons = DebugButtons::INTERRUPT | DebugButtons::STOP | DebugButtons::CONTINUE*(!waiting_io);
        enabledButtons |= /*DebugButtons::SINGLE_STEP_ASM*(!waiting_io * 0) |*/ DebugButtons::STEP_OUT_ASM*(!waiting_io);
        enabledButtons |= DebugButtons::STEP_OVER_ASM*(!waiting_io) | DebugButtons::SINGLE_STEP_MICRO*(!waiting_io);
        enabledButtons |= DebugButtons::STEP_INTO_ASM*(enable_into * !waiting_io);
        break;
    case DebugState::DEBUG_RESUMED:
        enabledButtons = DebugButtons::INTERRUPT | DebugButtons::STOP;
        break;
    case DebugState::DEBUG_ONLY_MICRO:
        enabledButtons =  DebugButtons::STOP | DebugButtons::CONTINUE;
        enabledButtons |= DebugButtons::SINGLE_STEP_MICRO;
        break;
    default:
        break;
    }
    debugButtonEnableHelper(enabledButtons);
}

bool MicroMainWindow::on_actionDebug_Start_Debugging_triggered()
{  
    if(!on_actionBuild_Assemble_triggered()) return false;
    // Unlike Pep9asm, on_actionDebug_Start_Debugging_Object_triggered switched to the
    // debugger tab. This is because in the event of useless object code there is still microcode to debug.
    return on_actionDebug_Start_Debugging_Object_triggered();

}

bool MicroMainWindow::on_actionDebug_Start_Debugging_Object_triggered()
{
    connectViewUpdate();
    loadOperatingSystem();
    loadObjectCodeProgram();
    debugState = DebugState::DEBUG_ISA;
    ui->asmProgramTracePane->startSimulationView();
    if(initializeSimulation()) {
        emit simulationStarted();
        controlSection->onSimulationStarted();
        controlSection->enableDebugging();
        static_cast<InterfaceISACPU*>(controlSection.get())->breakpointsSet(programManager->getBreakpoints());
        ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->debuggerTab));
        ui->debuggerTabWidget->setCurrentIndex(ui->debuggerTabWidget->indexOf(ui->assemblerDebuggerTab));
        memDevice->clearAllByteCaches();
        ui->cpuWidget->startDebugging();
        // Erase and re-render memory rather then update in place via updateMemory.
        // For small changes, updateMemory is faster, but for large changes it is much slower.
        // When beggining debugging, all addresses in the computer are 0'ed out and then
        // the object code program / OS is loaded. This generate 64k update entries, hence very slow.
        ui->memoryWidget->refreshMemory();
        // Force re-highlighting on memory to prevent last run's data
        // from remaining highlighted. Otherwise, if the last program
        // was "run", then every byte that it modified will be highlighted
        // upon starting the simulation.
        highlightActiveLines();
        ui->memoryTracePane->updateTrace();
        ui->asmProgramTracePane->setFocus();
        return true;
    }
    return false;
}

bool MicroMainWindow::on_actionDebug_Start_Debugging_Loader_triggered()
{
    if(!on_actionBuild_Assemble_triggered()) return false;
    memDevice->clearMemory();
    loadOperatingSystem();
    // Copy object code to batch input pane and make it the active input pane
    QString objcode = ui->assemblerPane->getPaneContents(PepCore::EPane::EObject);
    // Replace all new line characters with spaces
    objcode = objcode.replace('\n', ' ');
    ui->ioWidget->setBatchInput(objcode);
    ui->ioWidget->setActivePane(PepCore::EPane::EBatchIO);
    if(!on_actionDebug_Start_Debugging_Object_triggered()) return false;
    // Skip over any initialization code in the microprogram.
    controlSection->setMicroPCToStart();
    quint16 sp, pc;
    memDevice->readWord(programManager->getOperatingSystem()->getBurnValue() - 9, sp);
    memDevice->readWord(programManager->getOperatingSystem()->getBurnValue() - 3, pc);
    // Write SP, PC to simulator
    auto pc_reg = to_uint8_t(Pep9::CPURegisters::PC);
    auto sp_reg = to_uint8_t(Pep9::CPURegisters::SP);
    controlSection->getDataSection()->getRegisterBank().overwriteRegisterWordStart(pc_reg, pc);
    controlSection->getDataSection()->getRegisterBank().writeRegisterWord(pc_reg, pc);
    controlSection->getDataSection()->getRegisterBank().writeRegisterWord(sp_reg, sp);
    // Memory has been cleared, but will not display as such unless explicitly refreshed.
    ui->memoryWidget->refreshMemory();
    emit simulationUpdate();
    return true;
}

bool MicroMainWindow::on_actionDebug_Start_Debugging_Microcode_triggered()
{
    connectViewUpdate();
    // Load microprogram into the micro control store
    micro_assembler.setCPUType(dataSection->getCPUType());
    auto result = micro_assembler.assembleProgram(ui->microcodeWidget->getMicrocodeText());
    for(auto& [line, message] : result.errorMessages) {
        ui->microcodeWidget->appendMessageInSourceCodePaneAt(line, message);
    }
    if (result.success) {
        ui->statusBar->showMessage("MicroAssembly succeeded", 4000);
        if(result.program->hasMicrocode() == false) {
            ui->statusBar->showMessage("No microcode program to build", 4000);
            return false;
        }
        ui->microObjectCodePane->setObjectCode(result.program, nullptr);
        controlSection->setMicrocodeProgram(result.program);
        static_cast<InterfaceMCCPU*>(controlSection.get())->breakpointsSet(ui->microcodeWidget->getBreakpoints());
    }
    else {
        ui->statusBar->showMessage("MicroAssembly failed", 4000);
        ui->microObjectCodePane->setObjectCode();
        controlSection->setMicrocodeProgram(nullptr);
        return false;
    }

    debugState = DebugState::DEBUG_ONLY_MICRO;

    // Unlike at the ISA level, we only need to clear CPU state in the microcode-only
    // level when there are preconditions (like in Pep9CPU). However, in microcode
    // mode, both the CPU and memory must be cleared.

    if(controlSection->getProgram()->hasUnitPre()) {
        controlSection->onResetCPU();
        controlSection->initCPU();
        ui->cpuWidget->clearCpu();
        memDevice->clearMemory();
        // Unlike Pep9CPU, do not clear all of memory. With the assembly level features,
        // we need to make sure that operating system / user program does not get removed from memory,
        // which would break the simulator whenever microprograms contained preconditions.

        CPUDataSection* data = this->dataSection.get();
        AMemoryDevice* memory = this->memDevice.get();
        for(auto line : controlSection->getProgram()->getObjectCode()) {
            if(line->hasUnitPre()) {
                static_cast<UnitPreCode*>(line)->setUnitPre(data, memory);
            }
        }
    }
    // Don't allow the microcode pane to be edited while the program is running
    ui->microcodeWidget->setReadOnly(true);

    emit simulationStarted();
    controlSection->onSimulationStarted();
    controlSection->enableDebugging();
    static_cast<InterfaceISACPU*>(controlSection.get())->breakpointsSet(programManager->getBreakpoints());
    ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->debuggerTab));
    ui->debuggerTabWidget->setCurrentIndex(ui->debuggerTabWidget->indexOf(ui->microcodeDebuggerTab));
    memDevice->clearAllByteCaches();
    ui->cpuWidget->startDebugging();
    ui->memoryWidget->refreshMemory();
    ui->asmProgramTracePane->clearSourceCode();
    // Force re-highlighting on memory to prevent last run's data
    // from remaining highlighted. Otherwise, if the last program
    // was "run", then every byte that it modified will be highlighted
    // upon starting the simulation.
    highlightActiveLines();
    ui->memoryTracePane->hide();
    ui->microcodeWidget->setFocus();
    return true;
}

void MicroMainWindow::on_actionDebug_Stop_Debugging_triggered()
{
    connectViewUpdate();
    highlightActiveLines();
    debugState = DebugState::DISABLED;
    ui->microcodeWidget->setReadOnly(false);
    // Handle case of execution being canceled during IO
    memDevice->clearIO();
    reenableUIAfterInput();
    ui->ioWidget->cancelWaiting();
    ui->cpuWidget->stopDebugging();
    ui->asmProgramTracePane->clearSimulationView();
    // Don't clear selection in microcode widget, otherwise it is difficult
    // to debug a fault microprogram.
    //ui->microcodeWidget->clearSimulationView();
    //ui->microObjectCodePane->clearSimulationView();
    //QTextCursor curs = ui->microcodeWidget->getEditor()->textCursor();
    //ui->microcodeWidget->getEditor()->centerCursor();
    //curs.clearSelection();
    //ui->microcodeWidget->getEditor()->setTextCursor(curs);
    handleDebugButtons();
    controlSection->onSimulationFinished();
    emit simulationFinished();
}

void MicroMainWindow::on_actionDebug_Single_Step_Assembler_triggered()
{
    quint8 is;
    quint16 addr = controlSection->getCPURegWordStart(Pep9::CPURegisters::PC);
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

void MicroMainWindow::on_actionDebug_Interupt_Execution_triggered()
{
    // Enable debugging in CPU and then temporarily pause execution.
    controlSection->enableDebugging();
    controlSection->forceBreakpoint(PepCore::BreakpointTypes::ASSEMBLER);
    connectViewUpdate();
    debugState = DebugState::DEBUG_ISA;
    highlightActiveLines();
    handleDebugButtons();
    // Interupt should activate the assembler debugger tab, as this is the level where it makes the most sense.
    ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->debuggerTab));
    ui->debuggerTabWidget->setCurrentIndex(ui->debuggerTabWidget->indexOf(ui->assemblerDebuggerTab));
}

void MicroMainWindow::on_actionDebug_Continue_triggered()
{
    // Don't allow a transition from microcode-only to ISA level.
    if(debugState == DebugState::DEBUG_ONLY_MICRO) {
        debugState = DebugState::DEBUG_RESUMED_ONLY_MICRO;
    }
    else {
        debugState = DebugState::DEBUG_RESUMED;
    }

    handleDebugButtons();
    disconnectViewUpdate();
    controlSection->onRun();
    connectViewUpdate();
    emit simulationUpdate();
    QApplication::processEvents();
    if(controlSection->hadErrorOnStep()) {
        return; // we'll just return here instead of letting it fail and go to the bottom
    }
    else if(controlSection->stoppedForBreakpoint()) {
        highlightActiveLines();
    }
}

void MicroMainWindow::on_actionDebug_Step_Over_Assembler_triggered()
{
    debugState = DebugState::DEBUG_ISA;
    ui->debuggerTabWidget->setCurrentIndex(ui->debuggerTabWidget->indexOf(ui->assemblerDebuggerTab));
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

void MicroMainWindow::on_actionDebug_Step_Into_Assembler_triggered()
{
    debugState = DebugState::DEBUG_ISA;
    ui->debuggerTabWidget->setCurrentIndex(ui->debuggerTabWidget->indexOf(ui->assemblerDebuggerTab));
    controlSection->stepInto();
    emit simulationUpdate();
}

void MicroMainWindow::on_actionDebug_Step_Out_Assembler_triggered()
{
    debugState = DebugState::DEBUG_ISA;
    ui->debuggerTabWidget->setCurrentIndex(ui->debuggerTabWidget->indexOf(ui->assemblerDebuggerTab));
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

void MicroMainWindow::on_actionDebug_Single_Step_Microcode_triggered()
{
    // Don't allow transition from microcode-only simulation to ISA level simulation.
    if(debugState != DebugState::DEBUG_ONLY_MICRO) {
        debugState = DebugState::DEBUG_MICRO;
    }


    ui->debuggerTabWidget->setCurrentIndex(ui->debuggerTabWidget->indexOf(ui->microcodeDebuggerTab));
    if(controlSection->atMicroprogramStart()) {
        memDevice->clearAllByteCaches();
    }
    controlSection->onMCStep();
    emit simulationUpdate();

}

void MicroMainWindow::onMicroBreakpointHit()
{
    // Don't allow a transition from microcode-only to ISA level.
    if(debugState == DebugState::DEBUG_RESUMED_ONLY_MICRO) {
        debugState = DebugState::DEBUG_ONLY_MICRO;
    }
    else {
        debugState = DebugState::DEBUG_MICRO;
    }
    ui->debuggerTabWidget->setCurrentIndex(ui->debuggerTabWidget->indexOf(ui->microcodeDebuggerTab));
}

void MicroMainWindow::onASMBreakpointHit()
{
    debugState = DebugState::DEBUG_ISA;
    ui->debuggerTabWidget->setCurrentIndex(ui->debuggerTabWidget->indexOf(ui->assemblerDebuggerTab));
}

void MicroMainWindow::onPaletteChanged(const QPalette &)
{
    onDarkModeChanged();
}

// System MainWindow triggers
void MicroMainWindow::on_actionSystem_Clear_CPU_triggered()
{
    controlSection->onResetCPU();
    ui->cpuWidget->clearCpu();
}

void MicroMainWindow::on_actionSystem_Clear_Memory_triggered()
{
    memDevice->clearMemory();
    ui->memoryWidget->refreshMemory();
    ui->memoryWidget->clearHighlight();
}

void MicroMainWindow::on_actionSystem_Assemble_Install_New_OS_triggered()
{
    ui->assemblerPane->removeErrorMessages();
    auto out = AsmOutput();
    out.success = assembler.assembleUserProgram(ui->assemblerPane->getPaneContents(PepCore::EPane::ESource), out.prog, out.errors);
    if(out.errors.size() != 0) {
        ui->assemblerPane->addErrorsToSource(out.errors);
    }
    if(out.success) {
        ui->assemblerPane->setPanesFromProgram(out);
        programManager->setOperatingSystem(out.prog);
        ui->asmProgramTracePane->onRemoveAllBreakpoints();
        static_cast<InterfaceISACPU*>(controlSection.get())->breakpointsRemoveAll();
        set_Obj_Listing_filenames_from_Source();
        ui->statusBar->showMessage("Assembly succeeded, OS installed", 4000);
    }
    else {
        ui->assemblerPane->clearPane(PepCore::EPane::EObject);
        ui->assemblerPane->clearPane(PepCore::EPane::EListing);
        ui->asmProgramTracePane->clearSourceCode();
        ui->asmProgramTracePane->onRemoveAllBreakpoints();
        ui->statusBar->showMessage("Assembly failed, previous OS remains", 4000);
    }
    loadOperatingSystem();
    ui->memoryWidget->refreshMemory();
}

void MicroMainWindow::on_actionSystem_Reinstall_Default_OS_triggered()
{
    qDebug() << "Reinstalled default OS";
    assembleDefaultOperatingSystem();
    loadOperatingSystem();
    ui->memoryWidget->refreshMemory();
}

void MicroMainWindow::on_actionSystem_Redefine_Mnemonics_triggered()
{
    redefineMnemonicsDialog->show();
}

void MicroMainWindow::on_actionSystem_Redefine_Decoder_Tables_triggered()
{
    decoderTableDialog->show();
}

void MicroMainWindow::redefine_Mnemonics_closed()
{
    // Propogate ASM-level instruction definition changes across the application.
    ui->assemblerPane->rebuildHighlightingRules();
    ui->asmProgramTracePane->rebuildHighlightingRules();
}

void MicroMainWindow::onSimulationFinished()
{
    QString errorString;
    on_actionDebug_Stop_Debugging_triggered();

    QVector<AMicroCode*> prog = controlSection->getProgram()->getObjectCode();
    bool hadPostTest = false;

    CPUDataSection* data = this->dataSection.get();
    AMemoryDevice* memory = this->memDevice.get();
    if(controlSection->hadErrorOnStep()) {
        QMessageBox::critical(
          this,
          tr("Pep/9 Micro"),
          controlSection->getErrorMessage());
        ui->statusBar->showMessage("Execution failed", 4000);
    }
    else {
        for (AMicroCode* x : prog) {
            if(x->hasUnitPost()) {
                hadPostTest = true;
                UnitPostCode* code = dynamic_cast<UnitPostCode*>(x);
                if(!code->testPostcondition(data, memory, errorString)) {
                    ui->microcodeWidget->appendMessageInSourceCodePaneAt(-1, errorString);
                    QMessageBox::warning(this, "Pep/9 Micro", "Failed unit test");
                    ui->microcodeWidget->getEditor()->setFocus();
                    ui->statusBar->showMessage("Failed unit test", 4000);
                    return;
                }
            }
        }
    }
    if(hadPostTest) ui->statusBar->showMessage("Passed unit test", 4000);
    else ui->statusBar->showMessage("Execution finished", 4000);

}

void MicroMainWindow::onDarkModeChanged()
{
    isInDarkMode = inDarkMode();
    emit darkModeChanged(isInDarkMode, styleSheet());
}

void MicroMainWindow::on_actionView_Assembler_Tab_triggered()
{
    ui->tabWidget->setCurrentWidget(ui->assemblerTab);
}

void MicroMainWindow::on_actionView_Debugger_Tab_triggered()
{
    ui->tabWidget->setCurrentWidget(ui->debuggerTab);
}

void MicroMainWindow::on_actionView_Statistics_Tab_triggered()
{
    ui->tabWidget->setCurrentWidget(ui->statsTab);
}

// help:
void MicroMainWindow::on_actionHelp_triggered()
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

void MicroMainWindow::on_actionHelp_Writing_Assembly_Programs_triggered()
{
    helpDialog->show();
    helpDialog->selectItem("Writing Assembly Language Programs");
}

void MicroMainWindow::on_actionHelp_Machine_Language_triggered()
{
    helpDialog->show();
    helpDialog->selectItem("Writing Machine Language");
}

void MicroMainWindow::on_actionHelp_Assembly_Language_triggered()
{
    helpDialog->show();
    helpDialog->selectItem("Writing Assembly Language");
}

void MicroMainWindow::on_actionHelp_Debugging_Assembly_triggered()
{
    helpDialog->show();
    helpDialog->selectItem("Debugging Assembly Language");
}

void MicroMainWindow::on_actionHelp_Writing_Trap_Handlers_triggered()
{
    helpDialog->show();
    helpDialog->selectItem("Writing Trap Handlers");
}

void MicroMainWindow::on_actionHelp_Using_Pep_9_CPU_triggered()
{
    helpDialog->show();
    helpDialog->selectItem("Using Pep/9 Micro");
}

void MicroMainWindow::on_actionHelp_Interactive_Use_triggered()
{
    helpDialog->show();
    helpDialog->selectItem("Interactive Use");
}

void MicroMainWindow::on_actionHelp_Microcode_Use_triggered()
{
    helpDialog->show();
    helpDialog->selectItem("Microcode Use");
}

void MicroMainWindow::on_actionHelp_Debugging_Use_triggered()
{
    helpDialog->show();
    helpDialog->selectItem("Debugging Use");
}

// help:

void MicroMainWindow::on_actionHelp_Pep9Reference_triggered()
{
    helpDialog->show();
    helpDialog->selectItem("Pep/9 Reference");
}

void MicroMainWindow::on_actionHelp_Examples_triggered()
{
    helpDialog->show();
    helpDialog->selectItem("Examples");
}

void MicroMainWindow::on_actionHelp_Pep9_Operating_System_triggered()
{
    helpDialog->show();
    helpDialog->selectItem("Pep/9 Operating System");
}

void MicroMainWindow::on_actionHelp_Pep9_Microcode_Implementation_triggered()
{
    helpDialog->show();
    helpDialog->selectItem("Pep/9 Microcode Implementation");
}

void MicroMainWindow::on_actionHelp_About_Pep9Micro_triggered()
{
    aboutPepDialog->show();
}

void MicroMainWindow::on_actionHelp_About_Qt_triggered()
{
    QDesktopServices::openUrl(QUrl("http://www.qt.io/"));
}

// Byte Converter slots

void MicroMainWindow::slotByteConverterBinEdited(const QString &str)
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

void MicroMainWindow::slotByteConverterCharEdited(const QString &str)
{
    if (str.length() > 0) {
        int data = static_cast<int>(str[0].toLatin1());
        byteConverterBin->setValue(data);
        byteConverterDec->setValue(data);
        byteConverterHex->setValue(data);
        byteConverterInstr->setValue(data);
    }
}

void MicroMainWindow::slotByteConverterDecEdited(const QString &str)
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

void MicroMainWindow::slotByteConverterHexEdited(const QString &str)
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
void MicroMainWindow::focusChanged(QWidget *oldFocus, QWidget *)
{
    // Unhighlight the old widget.
    if(ui->microcodeWidget->isAncestorOf(oldFocus)) {
        ui->microcodeWidget->highlightOnFocus();
    }
    else if(ui->microObjectCodePane->isAncestorOf(oldFocus)) {
        ui->microObjectCodePane->highlightOnFocus();
    }
    else if(ui->memoryWidget->isAncestorOf(oldFocus)) {
        ui->memoryWidget->highlightOnFocus();
    }
    else if(ui->cpuWidget->isAncestorOf(oldFocus)) {
        ui->cpuWidget->highlightOnFocus();
    }
    else if(ui->assemblerPane->isAncestorOf(oldFocus)) {
        ui->assemblerPane->highlightOnFocus();
    }
    else if(ui->asmProgramTracePane->isAncestorOf(oldFocus)) {
        ui->asmProgramTracePane->highlightOnFocus();
    }
    else if (ui->ioWidget->isAncestorOf(oldFocus)) {
        ui->ioWidget->highlightOnFocus();
    }

    // Highlight the newly focused widget.
    int which = 0;
    if (ui->microcodeWidget->hasFocus()) {
        which = PepCore::EditButton::COPY | PepCore::EditButton::CUT | PepCore::EditButton::PASTE;
        which |= PepCore::EditButton::UNDO*ui->microcodeWidget->isUndoable() | PepCore::EditButton::REDO*ui->microcodeWidget->isRedoable();
        ui->microcodeWidget->highlightOnFocus();
    }
    else if (ui->memoryWidget->hasFocus()) {
        which = PepCore::EditButton::COPY;
        ui->memoryWidget->highlightOnFocus();
    }
    else if (ui->cpuWidget->hasFocus()) {
        which = 0;
        ui->cpuWidget->highlightOnFocus();
    }
    else if (ui->microObjectCodePane->hasFocus()) {
        which = PepCore::EditButton::COPY;
        ui->microObjectCodePane->highlightOnFocus();
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

    ui->actionEdit_Undo->setEnabled(which & PepCore::EditButton::UNDO);
    ui->actionEdit_Redo->setEnabled(which & PepCore::EditButton::REDO);
    ui->actionEdit_Cut->setEnabled(which & PepCore::EditButton::CUT);
    ui->actionEdit_Copy->setEnabled(which & PepCore::EditButton::COPY);
    ui->actionEdit_Paste->setEnabled(which & PepCore::EditButton::PASTE);
}

void MicroMainWindow::setUndoability(bool b)
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
    else if (ui->assemblerPane->isAncestorOf(focusWidget())) {
        ui->actionEdit_Undo->setEnabled(ui->assemblerPane->isUndoable() && b);
    }
    else if (ui->microObjectCodePane->hasFocus()) {
        ui->actionEdit_Redo->setDisabled(true);
    }
    else if (ui->ioWidget->isAncestorOf(QApplication::focusWidget())) {
        ui->actionEdit_Undo->setEnabled(b);
    }
    else if (ui->memoryTracePane->hasFocus()) {
        ui->actionEdit_Undo->setEnabled(false);
    }
}

void MicroMainWindow::setRedoability(bool b)
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
    else if (ui->assemblerPane->isAncestorOf(focusWidget())) {
        ui->actionEdit_Redo->setEnabled(ui->assemblerPane->isRedoable() && b);
    }
    else if (ui->microObjectCodePane->hasFocus()) {
        ui->actionEdit_Redo->setDisabled(true);
    }
    else if (ui->ioWidget->isAncestorOf(QApplication::focusWidget())) {
        ui->actionEdit_Redo->setEnabled(b);
    }
    else if (ui->memoryTracePane->hasFocus()) {
        ui->actionEdit_Redo->setEnabled(false);
    }
}

void MicroMainWindow::appendMicrocodeLine(QString line)
{
    ui->microcodeWidget->appendMessageInSourceCodePaneAt(-2, line);
}

void MicroMainWindow::helpCopyToSourceClicked()
{
    helpDialog->hide();
    ui->ioWidget->onClear();
    PepCore::EPane destPane, inputPane;
    QString input;
    QString code = helpDialog->getCode(destPane, inputPane, input);
    if(code.isEmpty()) {
        return;
    }
    else {
        switch(destPane)
        {
        case PepCore::EPane::ESource:
            if(maybeSave(PepCore::EPane::ESource)) {
                ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->assemblerTab));
                ui->assemblerPane->loadSourceFile("", code);
                emit ui->actionDebug_Remove_All_Assembly_Breakpoints->trigger();
                statusBar()->showMessage("Copied to assembler source code", 4000);
            }
            break;
        case PepCore::EPane::EObject:
            if(maybeSave(PepCore::EPane::EObject)) {
                ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->assemblerTab));
                ui->assemblerPane->loadObjectFile("", code);
                statusBar()->showMessage("Copied to assembler object code", 4000);
            }
            break;
        case PepCore::EPane::EMicrocode:
            if(maybeSave(PepCore::EPane::EMicrocode)) {
                ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->debuggerTab));
                ui->debuggerTabWidget->setCurrentIndex(ui->debuggerTabWidget->indexOf(ui->microcodeDebuggerTab));
                ui->microcodeWidget->setFocus();
                ui->microcodeWidget->setCurrentFile("");
                ui->microcodeWidget->setMicrocode(code);
                ui->microcodeWidget->setModifiedFalse();
                on_actionBuild_Microcode_triggered();

                emit ui->actionDebug_Remove_All_Microcode_Breakpoints->trigger();
                statusBar()->showMessage("Copied to microcode", 4000);
            }
            break;
        default:
            // No other panes allow copying help into them.
            return;

    }
}

    switch(inputPane)
    {
    case PepCore::EPane::ETerminal:
        // It doesn't make sense to put input text into a terminal, so ignore input text.
        ui->ioWidget->setActivePane(PepCore::EPane::ETerminal);
        break;
    case PepCore::EPane::EBatchIO:
        ui->ioWidget->setBatchInput(input);
        ui->ioWidget->setActivePane(PepCore::EPane::EBatchIO);
        break;
    default:
        break;
    }
}

void MicroMainWindow::onOutputReceived(quint16 address, quint8 value)
{
    ui->ioWidget->onOutputReceived(address, value);
}

void MicroMainWindow::onInputRequested(quint16 address)
{
    handleDebugButtons();
    connectViewUpdate();
    // If we are debugging the application, switch to the debugger
    // if it is not already active to show which line is requesting the
    // input.
    // ONLY_MICRO modes cannot trigger memory-mapped IO, and as such
    // do not need to be considered.
    if(ui->ioWidget->inInteractiveMode() ||
            this->debugState == DebugState::DEBUG_ISA ||
            this->debugState == DebugState::DEBUG_RESUMED ||
            this->debugState == DebugState::DEBUG_MICRO) {
        ui->tabWidget->setCurrentWidget(ui->debuggerTab);
    }
    // Must highlight lines with both the assembler and microcode debugger visible.
    // This is because of a bug in centerCursor() which doesn't center the cursor
    // iff the widget has never been visible.
    auto cw = ui->debuggerTabWidget->currentWidget();
    ui->debuggerTabWidget->setCurrentWidget(ui->assemblerDebuggerTab);
    highlightActiveLines();
    ui->debuggerTabWidget->setCurrentWidget(ui->microcodeDebuggerTab);
    highlightActiveLines();
    ui->debuggerTabWidget->setCurrentWidget(cw);
    // End fix for centerCursor()

    emit simulationUpdate();
    ui->microcodeWidget->setEnabled(false);
    ui->cpuWidget->setEnabled(false);
    statusBar()->showMessage("Input requested", 4000);
    ui->ioWidget->onDataRequested(address);
    handleDebugButtons();
    reenableUIAfterInput();
    disconnectViewUpdate();
}

void MicroMainWindow::onBreakpointHit(PepCore::BreakpointTypes type)
{
    ui->memoryWidget->refreshMemory();
    ui->memoryTracePane->updateTrace();
    switch(type) {
    case PepCore::BreakpointTypes::ASSEMBLER:
        onASMBreakpointHit();
        break;
    case PepCore::BreakpointTypes::MICROCODE:
        onMicroBreakpointHit();
        break;
    }
}

void MicroMainWindow::reenableUIAfterInput()
{
    ui->microcodeWidget->setEnabled(true);
    ui->cpuWidget->setEnabled(true);
}
