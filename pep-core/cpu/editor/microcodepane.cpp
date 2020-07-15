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

#include <QFontDialog>
#include <QGridLayout>

#include "cpu/interfacemccpu.h"
#include "microassembler/microcode.h"
#include "microassembler/microcodeprogram.h"
#include "style/colors.h"
#include "style/fonts.h"
#include "symbol/symbolentry.h"
#include "symbol/symbolvalue.h"
#include "symbol/symboltable.h"

MicrocodePane::MicrocodePane(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::MicrocodePane), inDarkMode(false),  currentFile()
{
    ui->setupUi(this);

    editor = new MicrocodeEditor(true,false, this);

    editor->setFont(QFont(PepCore::codeFont, PepCore::codeFontSize));
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

void MicrocodePane::init(QSharedPointer<InterfaceMCCPU> cpu, bool fullCtrlSection)
{

    editor->init(cpu);

    highlighter = new PepMicroHighlighter(cpu->getCPUType(), fullCtrlSection, PepColors::lightMode,editor->document());
    // Use helper function to set correct default names of microcode files on load.
    setCurrentFile("");
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
    this->fullCtrlSection = fullCtrlSection;
    highlighter->setFullControlSection(fullCtrlSection);
    highlighter->rehighlight();
}

QSet<quint16> MicrocodePane::getBreakpoints() const
{
    QSet<quint16> points;
    // Use line-1, since internally code lines are 0 indexed, but display as 1 indexed.
    for(auto point : editor->getBreakpoints()) {
        points.insert(point-1);
    }
    return points;
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
}

void MicrocodePane::onCPUTypeChanged(Enu::CPUType type)
{
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
    emit breakpointAdded(line - 1);
}

void MicrocodePane::onBreakpointRemoved(quint16 line)
{
    // Use line-1, since internally code lines are 0 indexed, but display as 1 indexed.
    emit breakpointRemoved(line - 1);

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
