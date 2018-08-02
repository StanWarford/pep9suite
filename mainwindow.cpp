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
#include <QPrinter>
#include <QPrintDialog>

#include "aboutpep.h"
#include "asmargument.h"
#include "asmcode.h"
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
    ui(new Ui::MainWindow), debugState(DebugState::DISABLED), updateChecker(new UpdateChecker()),
    memorySection(MemorySection::getInstance()), dataSection(CPUDataSection::getInstance()),
    controlSection(CPUControlSection::getInstance())
{
    Pep::initMicroEnumMnemonMaps();
    Pep::initEnumMnemonMaps();
    Pep::initMnemonicMaps();
    Pep::initAddrModesMap();
    Pep::initDecoderTables();
    ui->setupUi(this);
    ui->memoryWidget->init(memorySection,dataSection);
    ui->cpuWidget->init(this);


    helpDialog = new HelpDialog(this);
    aboutPepDialog = new AboutPep(this);

    // Byte converter setup
    // Byte converter setup
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
    connect(helpDialog, &HelpDialog::copyToMicrocodeClicked, this, &MainWindow::helpCopyToMicrocodeButtonClicked);

    connect(qApp->instance(), SIGNAL(focusChanged(QWidget*, QWidget*)), this, SLOT(focusChanged(QWidget*, QWidget*)));

    //Connect Undo / Redo events
    connect(ui->microcodeWidget, &MicrocodePane::undoAvailable, this, &MainWindow::setUndoability);
    connect(ui->microcodeWidget, &MicrocodePane::redoAvailable, this, &MainWindow::setRedoability);
    connect(ui->AsmObjectCodeWidgetPane, &AsmObjectCodePane::undoAvailable, this, &MainWindow::setUndoability);
    connect(ui->AsmObjectCodeWidgetPane, &AsmObjectCodePane::redoAvailable, this, &MainWindow::setRedoability);
    connect(ui->AsmSourceCodeWidgetPane, &AsmSourceCodePane::undoAvailable, this, &MainWindow::setUndoability);
    connect(ui->AsmSourceCodeWidgetPane, &AsmSourceCodePane::redoAvailable, this, &MainWindow::setRedoability);
    //connect(ui->microcodeWidget, &MicrocodePane::undoAvailable, this, &MainWindow::setUndoability);
    //connect(ui->microcodeWidget, &MicrocodePane::redoAvailable, this, &MainWindow::setRedoability);
    //Connect CPU widget
    connect(ui->cpuWidget, &CpuPane::writeByte, this, &MainWindow::updateMemAddress);

    //Connect Simulation events
#pragma message ("TODO: fix micro object code pane")
    //connect(this, &MainWindow::beginSimulation,this->objectCodePane,&ObjectCodePane::onBeginSimulation);
    //connect(this, &MainWindow::endSimulation,this->objectCodePane,&ObjectCodePane::onEndSimulation);
    connect(this, &MainWindow::simulationFinished, this->controlSection,&CPUControlSection::onSimulationFinished);
    connect(this, &MainWindow::updateSimulation, ui->cpuWidget, &CpuPane::onSimulationUpdate, Qt::UniqueConnection);
    connect(this, &MainWindow::simulationFinished, ui->cpuWidget, &CpuPane::onSimulationUpdate, Qt::UniqueConnection);
    //Connect font change events
    connect(this, &MainWindow::fontChanged, ui->microcodeWidget, &MicrocodePane::onFontChanged);
    connect(this, &MainWindow::fontChanged, helpDialog, &HelpDialog::onFontChanged);
    connect(this, &MainWindow::fontChanged, ui->ioWidget, &IOWidget::onFontChanged);
    connect(this, &MainWindow::fontChanged, ui->AsmSourceCodeWidgetPane, &AsmSourceCodePane::onFontChanged);
    connect(this, &MainWindow::fontChanged, ui->AsmObjectCodeWidgetPane, &AsmObjectCodePane::onFontChanged);
    connect(this, &MainWindow::fontChanged, ui->AssemblerListingWidgetPane, &AssemblerListingPane::onFontChanged);

    //Connect dark mode events
    connect(this, &MainWindow::darkModeChanged, ui->microcodeWidget, &MicrocodePane::onDarkModeChanged);
    connect(this, &MainWindow::darkModeChanged, helpDialog, &HelpDialog::onDarkModeChanged);
    //connect(this, &MainWindow::darkModeChanged, objectCodePane, &ObjectCodePane::onDarkModeChanged);
    connect(this, &MainWindow::darkModeChanged, ui->cpuWidget, &CpuPane::onDarkModeChanged);
    connect(this, &MainWindow::darkModeChanged, ui->microcodeWidget->getEditor(), &MicrocodeEditor::onDarkModeChanged);
    connect(this, &MainWindow::darkModeChanged, ui->memoryWidget, &MemoryDumpPane::onDarkModeChanged);

    qApp->installEventFilter(this);

    //Load Style sheets
    QFile f(":qdarkstyle/dark_style.qss");
    f.open(QFile::ReadOnly | QFile::Text);
    QTextStream ts(&f);
    darkStyle = ts.readAll();
    lightStyle = this->styleSheet();
    connect(ui->cpuWidget, &CpuPane::appendMicrocodeLine, this, &MainWindow::appendMicrocodeLine);

    //Connect events for IO tab
    ui->ioWidget->bindToMemorySection(memorySection);
    connect(this,&MainWindow::beginSimulation,ui->ioWidget,&IOWidget::onClear);

    //Events necessary for asynchronous run
    connect(controlSection,&CPUControlSection::simulationFinished,this,&MainWindow::onSimulationFinished);
    connect(memorySection,&MemorySection::charRequestedFromInput,this,&MainWindow::onInputRequested);
    connect(memorySection, &MemorySection::receivedInput,this,&MainWindow::onInputReceived);
    connect(memorySection, &MemorySection::memoryChanged, ui->memoryWidget, &MemoryDumpPane::onMemoryChanged, Qt::ConnectionType::UniqueConnection);
    connect(ui->cpuWidget, &CpuPane::registerChanged, dataSection, &CPUDataSection::onSetRegisterByte, Qt::ConnectionType::UniqueConnection);
    connect(dataSection, &CPUDataSection::statusBitChanged, ui->cpuWidget, &CpuPane::onStatusBitChanged, Qt::ConnectionType::UniqueConnection);
    connect(dataSection, &CPUDataSection::registerChanged, ui->cpuWidget, &CpuPane::onRegisterChanged, Qt::ConnectionType::UniqueConnection);
    connect(dataSection, &CPUDataSection::memoryRegisterChanged, ui->cpuWidget, &CpuPane::onMemoryRegisterChanged, Qt::ConnectionType::UniqueConnection);

    //Pre-render memory
    ui->memoryWidget->refreshMemory();
    quint32 maxSize = ui->memoryWidget->memoryDumpWidth();
    ui->memoryWidget->setMinimumWidth(maxSize);
    ui->memoryWidget->setMaximumWidth(maxSize);
    ui->ioWidget->setMaximumWidth(maxSize);

    //Initialize state for ISA level simulation
    Pep::memAddrssToAssemblerListing = &Pep::memAddrssToAssemblerListingProg;
    Pep::listingRowChecked = &Pep::listingRowCheckedProg;

    //Initialize Microcode panes
    QFile file("://help/pep9micro.pepcpu");
    if(file.open(QFile::ReadOnly | QFile::Text)){
        QTextStream in(&file);
        ui->microcodeWidget->setMicrocode(in.readAll());
        ui->microcodeWidget->setModifiedFalse();
    }
    //Connect assembler pane widgets

    //Initialize debug menu
    handleDebugButtons();
    connect(ui->AsmSourceCodeWidgetPane, &AsmSourceCodePane::labelDoubleClicked, this, &MainWindow::doubleClickedCodeLabel);
    connect(ui->AsmObjectCodeWidgetPane, &AsmObjectCodePane::labelDoubleClicked, this, &MainWindow::doubleClickedCodeLabel);
    connect(ui->AssemblerListingWidgetPane, &AssemblerListingPane::labelDoubleClicked, this, &MainWindow::doubleClickedCodeLabel);
    //Lastly, read in settings
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
                     (ui->microcodeWidget->hasFocus() /*|| objectCodePane->hasFocus()*/)) {
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

void MainWindow::connectMicroDraw()
{
    connect(memorySection, &MemorySection::memoryChanged, ui->memoryWidget, &MemoryDumpPane::onMemoryChanged, Qt::ConnectionType::UniqueConnection);
    connect(ui->cpuWidget, &CpuPane::registerChanged, dataSection, &CPUDataSection::onSetRegisterByte, Qt::ConnectionType::UniqueConnection);
    connect(dataSection, &CPUDataSection::statusBitChanged, ui->cpuWidget, &CpuPane::onStatusBitChanged, Qt::ConnectionType::UniqueConnection);
    connect(dataSection, &CPUDataSection::registerChanged, ui->cpuWidget, &CpuPane::onRegisterChanged, Qt::ConnectionType::UniqueConnection);
    connect(dataSection, &CPUDataSection::memoryRegisterChanged, ui->cpuWidget, &CpuPane::onMemoryRegisterChanged, Qt::ConnectionType::UniqueConnection);
}

void MainWindow::disconnectMicroDraw()
{
    disconnect(memorySection,&MemorySection::memoryChanged, ui->memoryWidget,&MemoryDumpPane::onMemoryChanged);
    disconnect(ui->cpuWidget, &CpuPane::registerChanged, dataSection, &CPUDataSection::onSetRegisterByte);
    disconnect(dataSection, &CPUDataSection::statusBitChanged, ui->cpuWidget, &CpuPane::onStatusBitChanged);
    disconnect(dataSection, &CPUDataSection::registerChanged, ui->cpuWidget, &CpuPane::onRegisterChanged);
    disconnect(dataSection, &CPUDataSection::memoryRegisterChanged, ui->cpuWidget, &CpuPane::onMemoryRegisterChanged);
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
    ui->microcodeWidget->readSettings(settings);
}

void MainWindow::writeSettings()
{
    QSettings settings("cslab.pepperdine","PEP9CPU");
    settings.beginGroup("MainWindow");
    settings.setValue("pos", pos());
    settings.setValue("size", size());
    settings.setValue("font",codeFont);
    settings.setValue("filePath", curPath);
    settings.endGroup();
    //Handle writing for all children
    ui->microcodeWidget->writeSettings(settings);
}

// Save methods
bool MainWindow::save(Enu::EPane which)
{
    bool retVal = true;
    switch(which)
    {
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
        if(QFileInfo(ui->AssemblerListingWidgetPane->getCurrentFile()).absoluteFilePath().isEmpty())
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
    //Iterate over all the panes, and attempt to save any modified panes before closing.
    for(int it = 0; it < metaenum.keyCount(); it++)
    {
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

void MainWindow::loadFile(const QString &fileName, Enu::EPane which) //Todo
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
    switch(which)
    {
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
    case Enu::EPane::ESource:
        fileName = QFileInfo(ui->AsmSourceCodeWidgetPane->getCurrentFile()).absoluteFilePath();
        break;
    case Enu::EPane::EObject:
        fileName = QFileInfo(ui->AsmObjectCodeWidgetPane->getCurrentFile()).absoluteFilePath();
        break;
    case Enu::EPane::EListing:
        fileName = QFileInfo(ui->AssemblerListingWidgetPane->getCurrentFile()).absoluteFilePath();
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
    const QString msgSource = "Source saved";
    const QString msgObject = "Object code saved";
    const QString msgListing = "Listing saved";
    const QString msgMicro = "Microcode saved";
    const QString *msgOutput; //Mutable pointer to const data
    switch(which)
    {
    case Enu::EPane::ESource:
        out << ui->AsmSourceCodeWidgetPane->toPlainText();
        msgOutput = &msgSource;
        break;
    case Enu::EPane::EObject:
        out << ui->AsmObjectCodeWidgetPane->toPlainText();
        msgOutput = &msgObject;
        break;
    case Enu::EPane::EListing:
        out << ui->AssemblerListingWidgetPane->toPlainText();
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
    const QString defSourceFile = "untitled.pep";
    const QString defObjectFile = "untitled.pepo";
    const QString defListingFile = "untitled.pepl";
    const QString defMicroFile = "untitled.pepcpu";
    QString usingFile;
    const QString titleBase = "Save %1";
    const QString sourceTitle = titleBase.arg("Assembler Source Code");
    const QString objectTitle = titleBase.arg("Object Code");
    const QString listingTitle = titleBase.arg("Assembler Listing");
    const QString microTitle = titleBase.arg("Microcode");
    const QString *usingTitle;
    const QString sourceTypes = "Pep9 Source (*.pep *.txt)";
    const QString objectTypes = "Pep9 Object (*.pepo *.txt)";
    const QString listingTypes = "Pep9 Listing (*.pepl)";
    const QString microTypes = "Pep9 Microcode (*.pepcpu *.txt)";
    const QString *usingTypes;
    switch(which)
    {
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
        if(ui->AssemblerListingWidgetPane->getCurrentFile().fileName().isEmpty()) {
            usingFile = QDir(curPath).absoluteFilePath(defListingFile);
        }
        else usingFile = ui->AssemblerListingWidgetPane->getCurrentFile().fileName();
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
        switch(which)
        {
        case Enu::EPane::ESource:
            ui->AsmSourceCodeWidgetPane->setCurrentFile(fileName);
            break;
        case Enu::EPane::EObject:
            ui->AsmObjectCodeWidgetPane->setCurrentFile(fileName);
            break;
        case Enu::EPane::EListing:
            ui->AssemblerListingWidgetPane->setCurrentFile(fileName);
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
    const QString *text; //Pointer to the text to be printed
    const QString base = "Print %1";
    const QString source = base.arg("Assembler Source Code");
    const QString object = base.arg("Object Code");
    const QString listing = base.arg("Assembler Listing");
    const QString micro = base.arg("Microcode");
    const QString *title; //Pointer to the title of the dialog
    switch(which)
    {
    /*
     * Set the pointer to the text-to-print and dialog title based on which pane is to be printed
     */
    case Enu::EPane::ESource:
        title = &source;
        text = &ui->AsmSourceCodeWidgetPane->toPlainText();
        break;
    case Enu::EPane::EObject:
        title = &object;
        text = &ui->AsmObjectCodeWidgetPane->toPlainText();
        break;
    case Enu::EPane::EListing:
        title = &listing;
        text = &ui->AssemblerListingWidgetPane->toPlainText();
        break;
    case Enu::EPane::EMicrocode:
        title = &micro;
        text = &ui->microcodeWidget->getMicrocode();
        break;
    }
    QTextDocument document(*text, this);
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
    for(QString byte : line.split(" "))
    {
        temp = byte.toShort(&ok,16);
        if(ok && byte.length()>0) output.append(temp);
    }
    return output;
}

void MainWindow::loadOperatingSystem()
{
    QVector<quint8> values;
    quint16 startAddress = 0xfc17;
    QString  osFileString;
#pragma message ("TODO: Add ability to switch between operating systems")
    //In the future, have a switch between loading the aligned and unaligned code
    if(true) osFileString = (":/help/pep9os.txt");
    else osFileString = ("failure_to_load");
    QFile osFile(osFileString);
    if(osFile.open(QFile::ReadOnly))
    {
        QTextStream file(&osFile);
        while(!file.atEnd())
        {
            QString text = file.readLine();
            values.append(convertObjectCodeToIntArray(text));
        }
        osFile.close();
        startAddress = 0xffff - values.length() + 1;
        memorySection->loadObjectCode(startAddress,values);
        memorySection->onIPortChanged(memorySection->getMemoryWord(0xFFF8, false));
        memorySection->onOPortChanged(memorySection->getMemoryWord(0xFFFA, false));
    }
    else
    {
        qDebug()<<osFile.errorString();
    }
}

void MainWindow::loadObjectCodeProgram()
{
    QString lines = ui->AsmObjectCodeWidgetPane->toPlainText();
    QVector<quint8> data;
    for(auto line : lines.split("\n",QString::SkipEmptyParts))
    {
        data.append(convertObjectCodeToIntArray(line));
    }
    memorySection->loadObjectCode((quint16)0, data);
}

void MainWindow::set_Obj_Listing_filenames_from_Source()
{
    QFileInfo fileInfo(ui->AsmSourceCodeWidgetPane->getCurrentFile());
    QString object, listing;
    if(fileInfo.fileName().isEmpty()){
        object = "";
        listing = "";
    }
    else {
        object = fileInfo.absoluteDir().absoluteFilePath(fileInfo.baseName()+".pepo");
        listing = fileInfo.absoluteDir().absoluteFilePath(fileInfo.baseName()+".pepl");
    }
    ui->AsmObjectCodeWidgetPane->setCurrentFile(object);
    ui->AssemblerListingWidgetPane->setCurrentFile(listing);
}

void MainWindow::doubleClickedCodeLabel(Enu::EPane which)
{
    QList<int> list;
    switch(which)
    {
    case Enu::EPane::ESource:
        list.append(3000);
        list.append(1);
        list.append(1);
        ui->codeSplitter->setSizes(list);
        break;
    case Enu::EPane::EObject:
        list.append(1);
        list.append(3000);
        list.append(1);
        ui->codeSplitter->setSizes(list);
        break;
    case Enu::EPane::EListing:
        list.append(1);
        list.append(1);
        list.append(3000);
        ui->codeSplitter->setSizes(list);
        break;
    }
}

void MainWindow::buttonEnableHelper(const int which)
{
    // Crack the parameter using DebugButtons to properly enable and disable all buttons related to debugging and running.
    ui->actionBuild_Run->setEnabled(which&DebugButtons::RUN);
    ui->actionBuild_Run_Object->setEnabled(which&DebugButtons::RUN_OBJECT);
    ui->actionDebug_Start_Debugging->setEnabled(which&DebugButtons::DEBUG);
    ui->actionDebug_Start_Debugging_Object->setEnabled(which&DebugButtons::DEBUG_OBJECT);
    ui->actionDebug_Start_Debugging_Loader->setEnabled(which&DebugButtons::DEBUG_LOADER);
    ui->actionDebug_Interupt_Execution->setEnabled(which&DebugButtons::INTERRUPT);
    ui->actionDebug_Continue->setEnabled(which&DebugButtons::CONTINUE);
    ui->actionDebug_Restart_Debugging->setEnabled(which&DebugButtons::RESTART);
    ui->actionDebug_Stop_Debugging->setEnabled(which&DebugButtons::STOP);
    ui->actionDebug_Step_Over_Assembler->setEnabled(which&DebugButtons::STEP_OVER_ASM);
    ui->actionDebug_Step_Into_Assembler->setEnabled(which&DebugButtons::STEP_INTO_ASM);
    ui->actionDebug_Step_Out_Assembler->setEnabled(which&DebugButtons::STEP_OUT_ASM);
    ui->actionDebug_Single_Step_Microcode->setEnabled(which&DebugButtons::SINGLE_STEP_MICRO);
}

void MainWindow::highlightActiveLines()
{
    if(true) //If in microcode, highlight the active line
    {
        ui->microcodeWidget->updateSimulationView();
        //objectCodePane->highlightCurrentInstruction();
    }
    if(controlSection->getLineNumber() == 0) //If the µPC is 0, hihglight the instruction at the current PC in the listing & memory
    {
        ui->memoryWidget->clearHighlight();
        ui->memoryWidget->highlightPC_SP();
        ui->memoryWidget->highlightLastWritten();
    }
}

bool MainWindow::initializeSimulation()
{
    emit beginSimulation();
    // Load microprogram into the micro control store
    if (ui->microcodeWidget->microAssemble()) {
        ui->statusBar->showMessage("MicroAssembly succeeded", 4000);
        if(ui->microcodeWidget->getMicrocodeProgram()->hasMicrocode() == false)
        {
            ui->statusBar->showMessage("No microcode program to build", 4000);
            return false;
        }
#pragma message("TODO: Reimplement object code pane")
        //objectCodePane->setObjectCode(microcodePane->getMicrocodeProgram(),nullptr);
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
    handleDebugButtons();
    return true;
}

void MainWindow::onUpdateCheck(int val)
{
    val = (int)val; //Ugly way to get rid of unused paramter warning without actually modifying the parameter.
    //Dummy to handle update checking code
}

// File MainWindow triggers
void MainWindow::on_actionFile_New_Asm_triggered()
{
    if (maybeSave(Enu::EPane::ESource)) {
        ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->assemblerTab));
        ui->AsmSourceCodeWidgetPane->setFocus();
        ui->AsmSourceCodeWidgetPane->clearSourceCode();
        ui->AsmSourceCodeWidgetPane->setCurrentFile("");
        ui->AsmObjectCodeWidgetPane->clearObjectCode();
        ui->AsmObjectCodeWidgetPane->setCurrentFile("");
        ui->AssemblerListingWidgetPane->clearAssemblerListing();
        ui->AssemblerListingWidgetPane->setCurrentFile("");
    }
}

void MainWindow::on_actionFile_New_Microcode_triggered()
{
    if (maybeSave(Enu::EPane::EMicrocode)) {
        ui->tabWidget->setCurrentIndex(ui->tabWidget->indexOf(ui->debuggerTab));
        ui->microcodeWidget->setFocus();
        ui->microcodeWidget->setMicrocode("");
        ui->microcodeWidget->setCurrentFile("");
#pragma message("TODO: fix object code pane")
        //objectCodePane->setObjectCode();
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
    if (!fileName.isEmpty()) {
        if(fileName.endsWith("pep",Qt::CaseInsensitive)) which = Enu::EPane::ESource;
        else if(fileName.endsWith("pepo",Qt::CaseInsensitive)) which = Enu::EPane::EObject;
        else if(fileName.endsWith("pepl",Qt::CaseInsensitive)) which = Enu::EPane::EListing;
        else if(fileName.endsWith("pepcpu",Qt::CaseInsensitive)) which = Enu::EPane::EMicrocode;
        if(maybeSave(which)){
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
    codeFont = QFont(Pep::codeFont, Pep::codeFontSize);
    emit fontChanged(codeFont);
}

//Build Events
void MainWindow::on_ActionBuild_Assemble_triggered()
{
    loadOperatingSystem();
    Pep::burnCount = 0;
    if(ui->AsmSourceCodeWidgetPane->assemble()){
#pragma message ("TODO: cancel run on burn and reset prog state")
        if(Pep::burnCount > 0) (void)0;
        ui->AsmObjectCodeWidgetPane->setObjectCode(ui->AsmSourceCodeWidgetPane->getObjectCode());
        ui->AssemblerListingWidgetPane->setAssemblerListing(ui->AsmSourceCodeWidgetPane->getAssemblerListingList());
        //listingTracePane->setListingTrace(sourceCodePane->getAssemblerListingList(), sourceCodePane->getHasCheckBox());
        //memoryTracePane->setMemoryTrace();
        //listingTracePane->showAssemblerListing();
        set_Obj_Listing_filenames_from_Source();
        loadObjectCodeProgram();
    }
    else {
#pragma message ("TODO: fix current file title if problematic")
        ui->AsmObjectCodeWidgetPane->clearObjectCode();
        ui->AssemblerListingWidgetPane->clearAssemblerListing();
        // listingTracePane->clearListingTrace();
        // ui->pepCodeTraceTab->setCurrentIndex(0); // Make source code pane visible
        loadObjectCodeProgram();
    }

}

void MainWindow::on_actionBuild_Load_Object_triggered()
{
    loadOperatingSystem();
    loadObjectCodeProgram();
    ui->memoryWidget->updateMemory();
}

void MainWindow::on_actionBuild_Run_Object_triggered()
{
    debugState = DebugState::RUN;
    if (initializeSimulation()) {
        disconnectMicroDraw();
        ui->memoryWidget->updateMemory();
        controlSection->onRun();
    }
}

void MainWindow::on_actionBuild_Run_triggered()
{
    debugState = DebugState::RUN;
    if (initializeSimulation()) {
        disconnectMicroDraw();
        loadOperatingSystem();
        loadObjectCodeProgram();
        ui->memoryWidget->updateMemory();
        controlSection->onRun();
    }
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
        enabledButtons = DebugButtons::RUN | DebugButtons::DEBUG | DebugButtons::DEBUG_OBJECT | DebugButtons::DEBUG_LOADER;
        break;
    case DebugState::RUN:
        enabledButtons = DebugButtons::STOP | DebugButtons::INTERRUPT;
        break;
    case DebugState::DEBUG_ISA:
        enabledButtons = DebugButtons::INTERRUPT | DebugButtons::STOP | DebugButtons::RESTART | DebugButtons::CONTINUE;
        enabledButtons |= DebugButtons::STEP_OUT_ASM | DebugButtons::STEP_OVER_ASM | DebugButtons::SINGLE_STEP_MICRO;
        enabledButtons |= DebugButtons::STEP_INTO_ASM*(enable_into);
        break;
    case DebugState::DEBUG_MICRO:
        enabledButtons = DebugButtons::INTERRUPT | DebugButtons::STOP | DebugButtons::RESTART | DebugButtons::CONTINUE;
        enabledButtons |= DebugButtons::STEP_OVER_ASM | DebugButtons::SINGLE_STEP_MICRO;
        break;
    case DebugState::DEBUG_RESUMED:
        enabledButtons = DebugButtons::INTERRUPT | DebugButtons::STOP | DebugButtons::RESTART;
        break;
    default:
        break;
    }
    buttonEnableHelper(enabledButtons);
}

bool MainWindow::on_actionDebug_Start_Debugging_triggered()
{
    connectMicroDraw();
    debugState = DebugState::DEBUG_ISA;
    if(initializeSimulation()) {
        controlSection->onDebuggingStarted();
        ui->cpuWidget->startDebugging();
        ui->memoryWidget->updateMemory();
        highlightActiveLines();

        QApplication::processEvents();
        return true;
    }
    return false;
}

void MainWindow::on_actionDebug_Stop_Debugging_triggered()
{
    connectMicroDraw();
    debugState = DebugState::DISABLED;
    ui->microcodeWidget->clearSimulationView();
    //objectCodePane->clearSimulationView();
    // disable the actions available while we're debugging
    ui->actionDebug_Stop_Debugging->setEnabled(false);

    // enable actions related to editing/starting debugging
    ui->actionBuild_Run->setEnabled(true);
    ui->actionDebug_Start_Debugging->setEnabled(true);
    ui->microcodeWidget->setReadOnly(false);

    ui->cpuWidget->stopDebugging();
    handleDebugButtons();
    highlightActiveLines();
    ui->memoryWidget->updateMemory();
    emit simulationFinished();
}

void MainWindow::on_actionDebug_Continue_triggered()
{
    debugState = DebugState::DEBUG_RESUMED;
    handleDebugButtons();
    highlightActiveLines();
    disconnectMicroDraw();
    controlSection->onRun();
    if(controlSection->hadErrorOnStep())
    {
        QMessageBox::warning(0, "Pep/9", controlSection->getErrorMessage());
        onSimulationFinished();
        return; // we'll just return here instead of letting it fail and go to the bottom
    }
    onSimulationFinished();

}

void MainWindow::on_actionDebug_Step_Over_Assembler_triggered()
{
    debugState = DebugState::DEBUG_ISA;
    int callDepth = controlSection->getCallDepth();
    do{
        controlSection->onISAStep();
        if (controlSection->hadErrorOnStep()) {
            // simulation had issues.
            QMessageBox::warning(0, "Pep/9", controlSection->getErrorMessage());
            onSimulationFinished();
        }
    } while(callDepth < controlSection->getCallDepth());

    handleDebugButtons();
    highlightActiveLines();
    emit updateSimulation();
}

void MainWindow::on_actionDebug_Step_Into_Assembler_triggered()
{
    debugState = DebugState::DEBUG_ISA;
    controlSection->onISAStep();
    if (controlSection->hadErrorOnStep()) {
        // simulation had issues.
        QMessageBox::warning(0, "Pep/9", controlSection->getErrorMessage());
        onSimulationFinished();
    }
    handleDebugButtons();
    highlightActiveLines();
    emit updateSimulation();
}

void MainWindow::on_actionDebug_Step_Out_Assembler_triggered()
{
    debugState = DebugState::DEBUG_ISA;
    int callDepth = controlSection->getCallDepth();
    do{
        controlSection->onISAStep();
        if (controlSection->hadErrorOnStep()) {
            // simulation had issues.
            QMessageBox::warning(0, "Pep/9", controlSection->getErrorMessage());
            onSimulationFinished();
        }
    } while(callDepth <= controlSection->getCallDepth());

    handleDebugButtons();
    highlightActiveLines();
    emit updateSimulation();
}

void MainWindow::on_actionDebug_Single_Step_Microcode_triggered()
{
    debugState = DebugState::DEBUG_MICRO;
    controlSection->onStep();
    if (controlSection->hadErrorOnStep()) {
        // simulation had issues.
        QMessageBox::warning(0, "Pep/9", controlSection->getErrorMessage());
        onSimulationFinished();
    }

    handleDebugButtons();
    highlightActiveLines();
    emit updateSimulation();
}

// System MainWindow triggers
void MainWindow::on_actionSystem_Clear_CPU_triggered()
{
    controlSection->onClearCPU();
    ui->cpuWidget->clearCpu();
}

void MainWindow::on_actionSystem_Clear_Memory_triggered()
{
    ui->memoryWidget->refreshMemory();
    controlSection->onClearMemory();
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
void MainWindow::focusChanged(QWidget *, QWidget *)
{
    ui->microcodeWidget->highlightOnFocus();
    ui->memoryWidget->highlightOnFocus();
    //objectCodePane->highlightOnFocus();
    ui->cpuWidget->highlightOnFocus();

    if (ui->microcodeWidget->hasFocus()) {
        ui->actionEdit_Undo->setDisabled(!ui->microcodeWidget->isUndoable());
        ui->actionEdit_Redo->setDisabled(!ui->microcodeWidget->isRedoable());
        ui->actionEdit_Cut->setDisabled(false);
        ui->actionEdit_Copy->setDisabled(false);
        ui->actionEdit_Paste->setDisabled(false);
    }
    else if (ui->memoryWidget->hasFocus()) {
        ui->actionEdit_Undo->setDisabled(true);
        ui->actionEdit_Redo->setDisabled(true);
        ui->actionEdit_Cut->setDisabled(true);
        ui->actionEdit_Copy->setDisabled(true);
        ui->actionEdit_Paste->setDisabled(true);
    }
    else if (ui->cpuWidget->hasFocus()) {
        ui->actionEdit_Undo->setDisabled(true);
        ui->actionEdit_Redo->setDisabled(true);
        ui->actionEdit_Cut->setDisabled(true);
        ui->actionEdit_Copy->setDisabled(true);
        ui->actionEdit_Paste->setDisabled(true);
    }
    /*else if (objectCodePane->hasFocus()) {
        ui->actionEdit_Undo->setDisabled(true);
        ui->actionEdit_Redo->setDisabled(true);
        ui->actionEdit_Cut->setDisabled(true);
        ui->actionEdit_Copy->setDisabled(false);
        ui->actionEdit_Paste->setDisabled(true);
    }*/
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
#pragma message("TODO: Set undoability for IO Panes / µObjectPane")
    /*else if (objectCodePane->hasFocus()) {
        ui->actionEdit_Redo->setDisabled(true);
    }*/
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
    #pragma message("TODO: Set redoability for IO Panes / µObjectPane")
    /*else if (inputPane->hasFocus()) {
        ui->actionEdit_Redo->setDisabled(!b);
    }
    else if (terminalPane->hasFocus()) {
        ui->actionEdit_Redo->setDisabled(!b);
    }
    else if (objectCodePane->hasFocus()) {
        ui->actionEdit_Redo->setDisabled(true);
    }*/
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
        //objectCodePane->setObjectCode(microcodePane->getMicrocodeProgram(),nullptr);
        helpDialog->hide();
        statusBar()->showMessage("Copied to microcode", 4000);
    }
}

void MainWindow::updateMemAddress(int address)
{
#pragma message("TODO: Connect Memory Dump Pane to event loop")
    //mainMemory->setMemAddress(address, memorySection->getMemoryByte(address, false));
    //mainMemory->showMemEdited(address);
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






