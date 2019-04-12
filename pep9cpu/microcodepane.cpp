// File: microcodepane.cpp
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

#include "microcodepane.h"
#include "ui_microcodepane.h"
#include "microcode.h"
#include "pep.h"
#include "microcodeprogram.h"
#include "symboltable.h"
#include <QGridLayout>
#include <QFontDialog>
#include "colors.h"
#include "symbolentry.h"
#include "symbolvalue.h"
#include "symboltable.h"
#include "newcpudata.h"
MicrocodePane::MicrocodePane(QWidget *parent) :
        QWidget(parent), dataSection(nullptr),
        ui(new Ui::MicrocodePane), inDarkMode(false), symbolTable(nullptr), program(nullptr), currentFile(), microASM(nullptr)
{
    ui->setupUi(this);

    editor = new MicrocodeEditor(true,false, this);

    editor->setFont(QFont(Pep::codeFont, Pep::codeFontSize));
    editor->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    editor->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(ui->label);
    layout->addWidget(editor);
    layout->setContentsMargins(0,0,0,0);
    layout->setVerticalSpacing(0);
    this->setLayout(layout);

    highlighter = nullptr;

    connect(editor->document(), &QTextDocument::modificationChanged, this, &MicrocodePane::setLabelToModified);

    connect(editor->document(), &QTextDocument::undoAvailable, this, &MicrocodePane::undoAvailable);
    connect(editor->document(), &QTextDocument::redoAvailable, this, &MicrocodePane::redoAvailable);
    connect(editor, &MicrocodeEditor::breakpointAdded, this, &MicrocodePane::onBreakpointAdded);
    connect(editor, &MicrocodeEditor::breakpointRemoved, this, &MicrocodePane::onBreakpointRemoved);
    editor->setFocus();
}

MicrocodePane::~MicrocodePane()
{
    delete ui;
}

void MicrocodePane::init(QSharedPointer<InterfaceMCCPU> cpu, QSharedPointer<NewCPUDataSection> newData, QSharedPointer<AMemoryDevice> memDevice, bool fullCtrlSection)
{
    if(!dataSection.isNull()) {
        disconnect(dataSection.get(), &NewCPUDataSection::CPUTypeChanged, this, &MicrocodePane::onCPUTypeChanged);
    }
    if(microASM != nullptr) delete microASM;
    microASM = new MicroAsm(memDevice, newData->getCPUType(), fullCtrlSection);
    dataSection = newData;
    connect(dataSection.get(), &NewCPUDataSection::CPUTypeChanged, this, &MicrocodePane::onCPUTypeChanged);
    editor->init(cpu);
    // Calls initCPUModelState() to refresh the highlighters
    useFullCtrlSection(fullCtrlSection);
    // initCPUModelState();
}

void MicrocodePane::initCPUModelState()
{
    if (highlighter != nullptr) {
        delete highlighter;
    }
    highlighter = new PepMicroHighlighter(dataSection->getCPUType(), fullCtrlSection, PepColors::lightMode,editor->document());

}

bool MicrocodePane::microAssemble()
{
    QVector<AMicroCode*> codeList;
    QString sourceLine;
    QString errorString;
    QStringList sourceCodeList;
    AMicroCode *code;
    int lineNum = 0;

    if(isModified() == false && program != nullptr) {
        return true;
    }

    removeErrorMessages();
    QString sourceCode = editor->toPlainText().trimmed();
    sourceCodeList = sourceCode.split('\n');

    if(symbolTable) {
        symbolTable.clear();
    }
    symbolTable = QSharedPointer<SymbolTable>(new SymbolTable());

    microASM->setCPUType(dataSection->getCPUType());
    while (lineNum < sourceCodeList.size()) {
        sourceLine = sourceCodeList[lineNum];
        if (!microASM->processSourceLine(symbolTable.data(),sourceLine, code, errorString)) {
            appendMessageInSourceCodePaneAt(lineNum, errorString);
            // Create a dummy program that will delete all asm code entries
            QSharedPointer<MicrocodeProgram>::create(codeList, symbolTable);
            return false;
        }
        if(code->isMicrocode() && static_cast<MicroCode*>(code)->hasControlSignal(Enu::EControlSignals::MemRead) &&
                static_cast<MicroCode*>(code)->hasControlSignal(Enu::EControlSignals::MemWrite)) {
            appendMessageInSourceCodePaneAt(lineNum, "\\ ERROR: Can't have memread and memwrite");
            // Create a dummy program that will delete all asm code entries
            QSharedPointer<MicrocodeProgram>::create(codeList, symbolTable);
            return false;
        }
        codeList.append(code);
        lineNum++;
    }

    program =  QSharedPointer<MicrocodeProgram>::create(codeList, symbolTable);
    for(auto sym : symbolTable->getSymbolEntries()) {
            if(sym->isUndefined()){
                appendMessageInSourceCodePaneAt(-1,"// ERROR: Undefined symbol "+sym->getName());
                return false;
            }
            else if(sym->isMultiplyDefined()) {
                appendMessageInSourceCodePaneAt(-1,"// ERROR: Multiply defined symbol "+sym->getName());
                return false;
            }
    }

    // Use line - 1, since internally code lines are 0 indexed, but display as 1 indexed.
    for(auto line : editor->getBreakpoints()) {
        program->getCodeLine(line - 1)->setBreakpoint(true);
    }
    return true;
}

void MicrocodePane::clearProgram()
{
    // If an invalid program is saved, it might be incorrectly "run"
    // due to caching in microcode pane. Clearing the cached microprogram
    // will work around this bug. Also clear symbol table as it is associated
    // with a microcode program.
    program.clear();
    symbolTable.clear();
}

QSharedPointer<MicrocodeProgram> MicrocodePane::getMicrocodeProgram() {
    return program;
}

void MicrocodePane::removeErrorMessages()
{
    QTextCursor cursor(editor->document()->find("// ERROR:"));
    while (!cursor.isNull()) {
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
        cursor = editor->document()->find("// ERROR:", cursor);
    }
}

void MicrocodePane::appendMessageInSourceCodePaneAt(int lineNumber, QString message)
{
    QTextCursor cursor(editor->document());
    if (lineNumber == -2) {
        cursor.setPosition(editor->textCursor().position());
        cursor.movePosition(QTextCursor::EndOfLine);
        cursor.clearSelection();
        editor->setTextCursor(cursor);
        editor->ensureCursorVisible();
        message.append("\n");
    }
    else if (lineNumber == -1) {
        cursor.setPosition(0);
        cursor.movePosition(QTextCursor::End);
        if (cursor.block().text() == "") {
            cursor.insertText(message);
        }
        else{
            editor->appendPlainText(message);
        }
        return;
    }
    else {
        cursor.setPosition(0);
        for (int i = 0; i < lineNumber; i++) {
            cursor.movePosition(QTextCursor::NextBlock);
        }
        cursor.movePosition(QTextCursor::EndOfBlock);
        cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 1);
        if (cursor.selectedText() == " ") {
            cursor.setPosition(cursor.anchor());
        }
        else {
            cursor.setPosition(cursor.anchor());
            cursor.insertText(" ");
        }
    }
    cursor.insertText(message);
}

void MicrocodePane::setMicrocode(QString microcode)
{
    QStringList sourceCodeList;
    sourceCodeList = microcode.split('\n');
    for (int i = 0; i < sourceCodeList.size(); i++) {
        sourceCodeList[i].remove(QRegExp("^[0-9]+\\.?\\s*"));
    }
    microcode = sourceCodeList.join("\n");
    editor->setPlainText(microcode);
    program = nullptr;
    setLabelToModified(true);
}

QString MicrocodePane::getMicrocodeText()
{
    return editor->toPlainText();
}

void MicrocodePane::highlightOnFocus()
{
    if (editor->hasFocus()) {
        ui->label->setAutoFillBackground(true);
    }
    else {
        ui->label->setAutoFillBackground(false);
    }
}

void MicrocodePane::setCurrentFile(QString fileName)
{
    if (fileName.isEmpty()) {
        currentFile.setFileName("");
        if(fullCtrlSection) {
            ui->label->setText("Microcode - untitled.pepmicro");
        }
        else{
            ui->label->setText("Microcode - untitled.pepcpu");
        }
    }
    else {
        currentFile.setFileName(fileName);
        ui->label->setText(QString("Microcode - %1").arg(QFileInfo(currentFile).fileName()));
    }
}

const QFile &MicrocodePane::getCurrentFile() const
{
    return currentFile;
}

bool MicrocodePane::hasFocus()
{
    return editor->hasFocus();
}

void MicrocodePane::cut()
{
    editor->cut();
}

void MicrocodePane::copy()
{
    editor->copy();
}

void MicrocodePane::paste()
{
    editor->paste();
}

void MicrocodePane::undo()
{
    editor->undo();
}

void MicrocodePane::redo()
{
    editor->redo();
}

bool MicrocodePane::isUndoable()
{
    return editor->document()->isUndoAvailable();
}

bool MicrocodePane::isRedoable()
{
    return editor->document()->isRedoAvailable();
}

void MicrocodePane::setReadOnly(bool ro)
{
    editor->setReadOnly(ro);
}

bool MicrocodePane::isModified()
{
    return editor->document()->isModified();
}

void MicrocodePane::setModifiedFalse()
{
    editor->document()->setModified(false);
    setLabelToModified(false);
}

void MicrocodePane::updateSimulationView()
{
    editor->highlightSimulatedLine();
}

void MicrocodePane::clearSimulationView()
{
    editor->clearSimulationView();
}

void MicrocodePane::unCommentSelection()
{
    editor->unCommentSelection();
}

void MicrocodePane::readSettings(QSettings &settings)
{
    settings.beginGroup("MicrocodePane");
    editor->readSettings(settings);
    settings.endGroup();
}

void MicrocodePane::writeSettings(QSettings &settings)
{
    settings.beginGroup("MicrocodePane");
    editor->writeSettings(settings);
    settings.setValue("currentFile", QFileInfo(currentFile).filePath());
    settings.endGroup();
}

MicrocodeEditor *MicrocodePane::getEditor()
{
    return editor;
}

QString MicrocodePane::toPlainText()
{
    return editor->document()->toPlainText();
}

void MicrocodePane::useFullCtrlSection(bool fullCtrlSection)
{
    microASM->useExtendedAssembler(fullCtrlSection);
    this->fullCtrlSection = fullCtrlSection;
    initCPUModelState();
}

void MicrocodePane::onFontChanged(QFont font)
{
    editor->setFont(font);
}

void MicrocodePane::onDarkModeChanged(bool darkMode)
{
    inDarkMode = darkMode;
    if(darkMode) highlighter->rebuildHighlightingRules(PepColors::darkMode);
    else highlighter->rebuildHighlightingRules(PepColors::lightMode);
    highlighter->rehighlight();

}

void MicrocodePane::onRemoveAllBreakpoints()
{
    editor->onRemoveAllBreakpoints();
    if(program == nullptr) return;
    else {
        for(quint16 it = 0; it < program->codeLength(); it++) {
            program->getCodeLine(it)->setBreakpoint(false);
        }
    }
}

void MicrocodePane::onCPUTypeChanged(Enu::CPUType type)
{
    program.clear();
    highlighter->setCPUType(type);
    highlighter->rehighlight();
}

void MicrocodePane::setLabelToModified(bool modified)
{
    QString temp = ui->label->text();
    if (modified) {
        ui->label->setText(temp.append(temp.endsWith(QChar('*')) ? "" : "*"));
    }
    else if (temp.endsWith(QChar('*'))) {
        temp.chop(1);
        ui->label->setText(temp);
    }
}

void MicrocodePane::onBreakpointAdded(quint16 line)
{
    // Use line-1, since internally code lines are 0 indexed, but display as 1 indexed.
    if(program == nullptr) return;
    else program->getCodeLine(line - 1)->setBreakpoint(true);
}

void MicrocodePane::onBreakpointRemoved(quint16 line)
{
    // Use line-1, since internally code lines are 0 indexed, but display as 1 indexed.
    if(program == nullptr) return;
    else program->getCodeLine(line - 1)->setBreakpoint(false);
}

void MicrocodePane::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
