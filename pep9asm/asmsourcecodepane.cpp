// File: sourcecodepane.cpp
/*
    Pep9 is a virtual machine for writing machine language and assembly
    language programs.
    
    Copyright (C) 2009  J. Stanley Warford, Pepperdine University

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

#include <QList>
#include <QStringList>
#include <QTextCursor>
#include <QPalette>
#include <QPainter>
#include <QSyntaxHighlighter>
#include <QFontDialog>
#include <QKeyEvent>
#include <QPlainTextDocumentLayout>
#include <QScrollBar>
#include <QPaintEvent>
#include <QSharedPointer>

#include "asmsourcecodepane.h"
#include "ui_asmsourcecodepane.h"
#include "asmcode.h"
#include "pep.h"
#include "colors.h"
#include "asmprogram.h"
#include "asmsourcecodepane.h"
#include "asmprogrammanager.h"
#include "mainmemory.h"
#include "symbolentry.h"
#include "symboltable.h"
#include "symbolvalue.h"
#include "mainmemory.h"
AsmSourceCodePane::AsmSourceCodePane(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::SourceCodePane), inDarkMode(false), currentFile(), currentProgram(nullptr), addressToIndex(), memDevice(nullptr)
{
    ui->setupUi(this);
    connect(ui->textEdit->document(), &QTextDocument::modificationChanged, this, &AsmSourceCodePane::setLabelToModified);

    pepHighlighter = new PepASMHighlighter(PepColors::lightMode, ui->textEdit->document());

    connect(ui->textEdit, &QPlainTextEdit::undoAvailable, this, &AsmSourceCodePane::undoAvailable);
    connect(ui->textEdit, &QPlainTextEdit::redoAvailable, this, &AsmSourceCodePane::redoAvailable);

    ui->label->setFont(QFont(Pep::labelFont, Pep::labelFontSize));
    ui->textEdit->setFont(QFont(Pep::codeFont, Pep::codeFontSize));

    connect(static_cast<AsmSourceTextEdit*>(ui->textEdit), &AsmSourceTextEdit::breakpointAdded, this, &AsmSourceCodePane::onBreakpointAddedProp);
    connect(static_cast<AsmSourceTextEdit*>(ui->textEdit), &AsmSourceTextEdit::breakpointRemoved, this, &AsmSourceCodePane::onBreakpointRemovedProp);
}

void AsmSourceCodePane::init(QSharedPointer<MainMemory> memDevice, AsmProgramManager *manager)
{
    this->memDevice = memDevice;
    programManager = manager;
}

AsmSourceCodePane::~AsmSourceCodePane()
{
    delete ui;
}

bool AsmSourceCodePane::assemble()
{
    // Clean up any global state from previous compilation attempts
    removeErrorMessages();
    Pep::symbolFormat.clear();
    Pep::symbolFormatMultiplier.clear();
    addressToIndex.clear();

    QString sourceCode = ui->textEdit->toPlainText();
    IsaAsm myAsm(memDevice, *programManager);
    // List of errors and warnings and the lines on which they occured
    auto elist = QList<QPair<int, QString> >();
    bool success = myAsm.assembleUserProgram(sourceCode, currentProgram, elist);
    // Add all warnings and errors to source files
    appendMessagesInSourceCodePane(elist);
    // If assemble failed, don't perform any more work
    if(!success) {
        return false;
    }
    AsmProgramManager::getInstance()->setUserProgram(currentProgram);
    for (int i = 0; i < currentProgram->getProgram().size(); i++) {
        if(currentProgram->getProgram()[i]->getMemoryAddress() >=0) {
            addressToIndex[currentProgram->getProgram()[i]->getMemoryAddress()] = i;
        }
        if(ui->textEdit->lineHasBreakpoint(i)) {
            currentProgram->getProgram()[i]->setBreakpoint(true);
        }
    }
    return true;
}

QList<int> AsmSourceCodePane::getObjectCode()
{
    objectCode.clear();
    for (int i = 0; i < currentProgram->numberOfLines(); ++i) {
        currentProgram->getCodeAtIndex(i)->appendObjectCode(objectCode);
    }
    return objectCode;
}

QStringList AsmSourceCodePane::getAssemblerListingList()
{
    assemblerListingList.clear();
    for (int i = 0; i < currentProgram->numberOfLines(); i++) {
        currentProgram->getCodeAtIndex(i)->appendSourceLine(assemblerListingList);
    }
    return assemblerListingList;
}

bool AsmSourceCodePane::assembleDefaultOs()
{
    QString sourceCode = Pep::resToString(":/help/figures/pep9os.pep");
    return assembleOS(sourceCode);
}

bool AsmSourceCodePane::assembleOS(const QString& sourceCode)
{
    QSharedPointer<AsmProgram> prog;
    IsaAsm myAsm(memDevice, *programManager);
    // List of errors and warnings and the lines on which they occured
    auto elist = QList<QPair<int, QString>>();
    bool success = myAsm.assembleOperatingSystem(sourceCode, prog, elist);
    // Add all warnings and errors to source files
#pragma message("TODO: handle appending error messages in operating system")
    /*for (QPair<int,QString> pair : elist) {
        appendMessageInSourceCodePaneAt(pair.first, pair.second);
    }*/
    // If assemble failed, don't perform any more work
    if(!success) {
        return false;
    }
    for (int i = 0; i < prog->getProgram().size(); i++) {
        if(prog->getProgram()[i]->getMemoryAddress() >=0) addressToIndex[prog->getProgram()[i]->getMemoryAddress()] = i;
    }
    programManager->setOperatingSystem(prog);
    return true;
}

void AsmSourceCodePane::removeErrorMessages()
{
    static const QString errorStr   = ";ERROR:";
    static const QString warnStr    = ";WARNING:";

    QTextCursor cursor(ui->textEdit->document());
    // Return if no useful changes can be made to the document
    if(cursor.isNull() ||
            ((ui->textEdit->document()->find(";ERROR:").position() == -1)
             && (ui->textEdit->document()->find(";WARNING:").position() == -1))) return;
    cursor.movePosition(QTextCursor::Start,QTextCursor::MoveAnchor);
    // Group the removing of all errors and warnings as a single edit action (can be undone with one undo).
    // This group must not be empty, nor can the cursor be changed to point to a differnt object,
    // otherwise the UI become an unusable mess
    cursor.beginEditBlock();
    int pos = 0;
    // Remove errors
    while (cursor.document()->find(errorStr, pos).position() != -1) {
        // Calculate the next position instead of using find(), since we don't want to change our cursor.
        pos = ui->textEdit->document()->find(errorStr).position();
        cursor.setPosition(pos-errorStr.length(), QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();

    }
    // Remove warnings
    pos = 0;
    while (cursor.document()->find(warnStr, pos).position() != -1) {
        // Calculate the next position instead of using find(), since we don't want to change our cursor.
        pos = ui->textEdit->document()->find(warnStr).position();
        cursor.setPosition(pos-warnStr.length(), QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();

    }

    cursor.endEditBlock();
}

void AsmSourceCodePane::appendMessagesInSourceCodePane(QList<QPair<int, QString> > errList)
{
    QTextCursor cursor(ui->textEdit->document());
    // If there are no errors to append, return early.
    // Otherwise an empty edit group would be created, which could cause issues
    // with the UI.
    if(errList.isEmpty()) return;
    else cursor.beginEditBlock();
    for (QPair<int,QString> pair : errList) {
        cursor.setPosition(0);
        for (int i = 0; i < pair.first; i++) {
            cursor.movePosition(QTextCursor::NextBlock);
        }
        cursor.movePosition(QTextCursor::EndOfLine);
        cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 1);
        if (cursor.selectedText() == " ") {
            cursor.setPosition(cursor.anchor());
        }
        else {
            cursor.setPosition(cursor.anchor());
            cursor.insertText(" ");
        }
        cursor.insertText(pair.second);
    }
    cursor.endEditBlock();
}

void AsmSourceCodePane::setSourceCodePaneText(QString string)
{
    ui->textEdit->setPlainText(string);
}

void AsmSourceCodePane::clearSourceCode()
{
    ui->textEdit->clear();
    currentProgram.clear(); //Should be safe, since we are using shared pointers
}

bool AsmSourceCodePane::isModified()
{
    return ui->textEdit->document()->isModified();
}

void AsmSourceCodePane::setModifiedFalse()
{
    ui->textEdit->document()->setModified(false);
}

QString AsmSourceCodePane::toPlainText()
{
    return ui->textEdit->toPlainText();
}

void AsmSourceCodePane::setCurrentFile(QString string)
{
    currentFile.setFileName(string);
    if(!currentFile.fileName().isEmpty()) {
        ui->label->setText("Source Code - " + QFileInfo(currentFile).fileName());
    }
    else {
        ui->label->setText("Source Code - untitled.pep");
    }
}

const QFile& AsmSourceCodePane::getCurrentFile() const
{
    return currentFile;
}

void AsmSourceCodePane::highlightOnFocus()
{
    if (ui->textEdit->hasFocus()) {
        ui->label->setAutoFillBackground(true);
    }
    else {
        ui->label->setAutoFillBackground(false);
    }
}

bool AsmSourceCodePane::hasFocus()
{
    return ui->textEdit->hasFocus();
}

void AsmSourceCodePane::undo()
{
    ui->textEdit->undo();
}

void AsmSourceCodePane::redo()
{
    ui->textEdit->redo();
}

bool AsmSourceCodePane::isUndoable()
{
    return ui->textEdit->document()->isUndoAvailable();
}

bool AsmSourceCodePane::isRedoable()
{
    return ui->textEdit->document()->isRedoAvailable();
}

void AsmSourceCodePane::cut()
{
    ui->textEdit->cut();
}

void AsmSourceCodePane::copy()
{
    ui->textEdit->copy();
}

void AsmSourceCodePane::paste()
{
    ui->textEdit->paste();
}

void AsmSourceCodePane::setReadOnly(bool b)
{
    ui->textEdit->setReadOnly(b);
}

void AsmSourceCodePane::tab()
{
    if (!ui->textEdit->isReadOnly()) {
        QTextCursor cursor = ui->textEdit->textCursor();
        cursor.movePosition(QTextCursor::StartOfLine);
        QString string;
        int curLinePos = ui->textEdit->textCursor().position() - cursor.position();
        int spaces;
        if (curLinePos < 9) {
            spaces = 9 - curLinePos;
        }
        else if (curLinePos < 17) {
            spaces = 17 - curLinePos;
        }
        else if (curLinePos < 29) {
            spaces = 29 - curLinePos;
        }
        else if (curLinePos == 29) {
            spaces = 5;
        }
        else {
            spaces = 4 - ((curLinePos - 30) % 4);
        }

        for (int i = 0; i < spaces; i++) {
            string.append(" ");
        }

        ui->textEdit->insertPlainText(string);
    }
}

QSharedPointer<AsmProgram> AsmSourceCodePane::getAsmProgram()
{
    return currentProgram;
}

const QSharedPointer<AsmProgram> AsmSourceCodePane::getAsmProgram() const
{
    return currentProgram;
}

void AsmSourceCodePane::writeSettings(QSettings &settings)
{
    settings.beginGroup("SourceCodePane");
    settings.endGroup();
}

void AsmSourceCodePane::readSettings(QSettings &settings)
{
    settings.beginGroup("SourceCodePane");
    settings.endGroup();
}

void AsmSourceCodePane::onFontChanged(QFont font)
{
    ui->textEdit->setFont(font);
}

void AsmSourceCodePane::onDarkModeChanged(bool darkMode)
{
    inDarkMode = darkMode;
    if(darkMode) pepHighlighter->rebuildHighlightingRules(PepColors::darkMode);
    else pepHighlighter->rebuildHighlightingRules(PepColors::lightMode);
    static_cast<AsmSourceTextEdit*>(ui->textEdit)->onDarkModeChanged(darkMode);
    pepHighlighter->rehighlight();
}

void AsmSourceCodePane::onRemoveAllBreakpoints()
{
    static_cast<AsmSourceTextEdit*>(ui->textEdit)->onRemoveAllBreakpoints();
}

void AsmSourceCodePane::onBreakpointAdded(quint16 address)
{
    if(addressToIndex.contains(address)) {
        static_cast<AsmSourceTextEdit*>(ui->textEdit)->onBreakpointAdded(addressToIndex[address]);
    }
}

void AsmSourceCodePane::onBreakpointRemoved(quint16 address)
{
    if(addressToIndex.contains(address)) {
        static_cast<AsmSourceTextEdit*>(ui->textEdit)->onBreakpointRemoved(addressToIndex[address]);
    }
}

void AsmSourceCodePane::mouseReleaseEvent(QMouseEvent *)
{
    ui->textEdit->setFocus();
}

void AsmSourceCodePane::mouseDoubleClickEvent(QMouseEvent *)
{
    emit labelDoubleClicked(Enu::EPane::ESource);
}

void AsmSourceCodePane::setLabelToModified(bool modified)
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

void AsmSourceCodePane::onBreakpointAddedProp(quint16 line)
{
    if(currentProgram.isNull()) return;
    else if(currentProgram->getProgram().length()<line) return;
    else emit breakpointAdded(currentProgram->getProgram().at(line)->getMemoryAddress());
}

void AsmSourceCodePane::onBreakpointRemovedProp(quint16 line)
{
    if(currentProgram.isNull()) return;
    else if(currentProgram->getProgram().length()<line) return;
    else emit breakpointRemoved(currentProgram->getProgram().at(line)->getMemoryAddress());
}

AsmSourceTextEdit::AsmSourceTextEdit(QWidget *parent): QPlainTextEdit(parent), colors(PepColors::lightMode)
{
    breakpointArea = new AsmSourceBreakpointArea(this);
    connect(this, &QPlainTextEdit::blockCountChanged, this, &AsmSourceTextEdit::updateBreakpointAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest, this, &AsmSourceTextEdit::updateBreakpointArea);
    connect(this, &AsmSourceTextEdit::textChanged, this, &AsmSourceTextEdit::onTextChanged);
}

AsmSourceTextEdit::~AsmSourceTextEdit()
{
    delete breakpointArea;
}

void AsmSourceTextEdit::breakpointAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(breakpointArea);
    painter.fillRect(event->rect(), colors.lineAreaBackground); // light grey
    QTextBlock block;
    int blockNumber, top, bottom;
    block = firstVisibleBlock();
    blockNumber = block.blockNumber();
    top = static_cast<int>( blockBoundingGeometry(block).translated(contentOffset()).top());
    bottom = top + static_cast<int>( blockBoundingRect(block).height());
    // Store painter's previous Antialiasing hint so it can be restored at the end
    bool antialias = painter.renderHints() & QPainter::Antialiasing;
    painter.setRenderHint(QPainter::Antialiasing, true);
    while (block.isValid() && top < event->rect().bottom()) {
        // If the current block is in the repaint zone
        if (block.isVisible() && bottom >= event->rect().top()) {
            // And if it has a breakpoint
            if(blockToIndex.contains(blockNumber) && breakpoints.contains(blockToIndex[blockNumber])) {
                painter.setPen(PepColors::transparent);
                painter.setBrush(colors.combCircuitRed);          
                painter.drawEllipse(QPoint(fontMetrics().height()/2, top+fontMetrics().height()/2),
                                    fontMetrics().height()/2 -1, fontMetrics().height()/2 -1);
            }
        }
        block = block.next();
        top = bottom;
        bottom = top + static_cast<int>(blockBoundingRect(block).height());
        ++blockNumber;
    }
    painter.setRenderHint(QPainter::Antialiasing, antialias);
}

int AsmSourceTextEdit::breakpointAreaWidth()
{
    return 5 + fontMetrics().height();
}

void AsmSourceTextEdit::breakpointAreaMousePress(QMouseEvent *event)
{
    QTextBlock block;
    int blockNumber, top, bottom, lineNumber;
    block = firstVisibleBlock();
    blockNumber = block.blockNumber();
    top = static_cast<int>(blockBoundingGeometry(block).translated(contentOffset()).top());
    bottom = top + static_cast<int>(blockBoundingRect(block).height());
    // For each visible block (usually a line), iterate until the current block contains the location of the mouse event.
    while (block.isValid() && top <= event->pos().y()) {
        if (event->pos().y()>=top && event->pos().y()<=bottom) {
            // Check if the clicked line is a code line
            if(blockToIndex.contains(blockNumber)) {
                lineNumber = blockToIndex[blockNumber];
                // If the clicked code line has a breakpoint, remove it.
                if(breakpoints.contains(lineNumber)) {
                    breakpoints.remove(lineNumber);
                    emit breakpointRemoved(lineNumber);
                }
                // Otherwise add a breakpoint.
                else {
                    breakpoints.insert(lineNumber);
                    emit breakpointAdded(lineNumber);
                }
            }
            break;
        }

        block = block.next();
        top = bottom;
        bottom = top + static_cast<int>(blockBoundingRect(block).height());
        ++blockNumber;
    }
    update();
}

const QSet<quint16> AsmSourceTextEdit::getBreakpoints() const
{
    return breakpoints;
}

bool AsmSourceTextEdit::lineHasBreakpoint(int line) const
{
    return this->blockToIndex.contains(line) && breakpoints.contains(blockToIndex[line]);
}

void AsmSourceTextEdit::onRemoveAllBreakpoints()
{
    breakpoints.clear();
    update();
}

void AsmSourceTextEdit::onBreakpointAdded(quint16 line)
{
    breakpoints.insert(blockToIndex[line]);
}

void AsmSourceTextEdit::onBreakpointRemoved(quint16 line)
{
    breakpoints.remove(blockToIndex[line]);
}

void AsmSourceTextEdit::onDarkModeChanged(bool darkMode)
{
    if(darkMode) colors = PepColors::darkMode;
    else colors = PepColors::lightMode;
}

void AsmSourceTextEdit::updateBreakpointAreaWidth(int)
{
    setViewportMargins(breakpointAreaWidth(), 0, 0, 0);
}

void AsmSourceTextEdit::updateBreakpointArea(const QRect &rect, int dy)
{
    if (dy)
        breakpointArea->scroll(0, dy);
    else
        breakpointArea->update(0, rect.y(), breakpointArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateBreakpointAreaWidth(0);
}

void AsmSourceTextEdit::onTextChanged()
{
    QMap<quint16, quint16> oldToNew, old = blockToIndex;
    blockToIndex.clear();
    int cycleNumber = 1;
    QStringList sourceCodeList = toPlainText().split('\n');

    for (int i = 0; i < sourceCodeList.size(); i++) {
        // If the line is spaces followed by a comment, is just spaces, or is spaces followed by an optional symbol followed by the start
        // of a dot command, then the line contains no executable code. Therefore, don't allow it to be selected for a breakpoint.
        if (QRegExp("\\s*;s*|^\\s*$|\\s*(([a-zAZ0-9]+:\\s*))?\\.", Qt::CaseInsensitive).indexIn(sourceCodeList.at(i)) == 0) {
        }
        else {
            blockToIndex.insert(i, cycleNumber++);
        }
    }
    QSet<quint16> toRemove;
    for(auto x : breakpoints) {
        if (blockToIndex.key(x, -1) == -1) toRemove.insert(-1);
    }
    breakpoints.subtract(toRemove);
    update();
}

void AsmSourceTextEdit::resizeEvent(QResizeEvent *evt)
{
    QPlainTextEdit::resizeEvent(evt);

    QRect cr = contentsRect();
    breakpointArea->setGeometry(QRect(cr.left(), cr.top(), breakpointAreaWidth(), cr.height()));
}

AsmSourceBreakpointArea::~AsmSourceBreakpointArea()
{
    // Need out-of-line destructor to prevent vtable from
    // being emitted in every including translation unit.
}
