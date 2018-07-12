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
#include <QDebug>
#include <QFontDialog>
#include "colors.h"
#include "symbolentry.h"
#include "symbolvalue.h"
#include "symboltable.h"
MicrocodePane::MicrocodePane(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::MicrocodePane),symbolTable(nullptr),program(nullptr)
{
    ui->setupUi(this);

    editor = new MicrocodeEditor(this);

    editor->setFont(QFont(Pep::codeFont, Pep::codeFontSize));
    editor->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    editor->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(ui->label);
    layout->addWidget(editor);
    layout->setContentsMargins(0,0,0,0);
    layout->setVerticalSpacing(0);
    this->setLayout(layout);

    highlighter = NULL;
    initCPUModelState();

    connect(editor->document(), &QTextDocument::modificationChanged, this, &MicrocodePane::setLabelToModified);

    connect(editor->document(), &QTextDocument::undoAvailable, this, &MicrocodePane::undoAvailable);
    connect(editor->document(), &QTextDocument::redoAvailable, this, &MicrocodePane::redoAvailable);

    editor->setFocus();
}

MicrocodePane::~MicrocodePane()
{
    delete ui;
}

void MicrocodePane::initCPUModelState()
{
    if (highlighter != NULL) {
        delete highlighter;
    }
    highlighter = new PepMicroHighlighter(PepColors::lightMode,editor->document());

}

bool MicrocodePane::microAssemble()
{
    QVector<AsmCode*> codeList;
    QString sourceLine;
    QString errorString;
    QStringList sourceCodeList;
    AsmCode *code;
    int lineNum = 0;
    removeErrorMessages();
    QString sourceCode = editor->toPlainText().trimmed();
    sourceCodeList = sourceCode.split('\n');
    if(symbolTable)
    {
        symbolTable.clear();
    }
    symbolTable = QSharedPointer<SymbolTable>(new SymbolTable());
    while (lineNum < sourceCodeList.size()) {
        sourceLine = sourceCodeList[lineNum];
        if (!MicroAsm::processSourceLine(symbolTable.data(),sourceLine, code, errorString)) {
            appendMessageInSourceCodePaneAt(lineNum, errorString);
            return false;
        }
        codeList.append(code);
        lineNum++;
    }
    program = new MicrocodeProgram(codeList,symbolTable.data());
    for(auto sym : symbolTable->getSymbolEntries()){
            if(sym->isUndefined()){
                appendMessageInSourceCodePaneAt(-1,"// ERROR: Undefined symbol "+sym->getName());
                return false;
            }
            else if(sym->isMultiplyDefined()){
                appendMessageInSourceCodePaneAt(-1,"// ERROR: Multiply defined symbol "+sym->getName());
                return false;
            }
    }
    // we guarantee a \n at the end of our document for single step highlighting
    if (!sourceCode.endsWith("\n")) {
        //editor->appendPlainText("\n");
    }
    return true;
}

MicrocodeProgram* MicrocodePane::getMicrocodeProgram() {
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
        cursor.movePosition(QTextCursor::NextBlock);
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

    setLabelToModified(true);
}

QString MicrocodePane::getMicrocode()
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

void MicrocodePane::setFilename(QString fileName)
{
    if (fileName == "") {
        ui->label->setText("Microcode");
    }
    else {
        ui->label->setText(QString("Microcode - %1").arg(fileName));
    }
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
    settings.endGroup();
}

MicrocodeEditor *MicrocodePane::getEditor()
{
    return editor;
}

void MicrocodePane::onFontChanged(QFont font)
{
    editor->setFont(font);
}

void MicrocodePane::onDarkModeChanged(bool darkMode)
{
    if(darkMode)
    {
        highlighter->rebuildHighlightingRules(PepColors::darkMode);
    }
    else
    {
        highlighter->rebuildHighlightingRules(PepColors::lightMode);
    }
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

