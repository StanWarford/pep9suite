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

#include "asmsourcecodepane.h"
#include "ui_asmsourcecodepane.h"
#include "asmcode.h"
#include "memorysection.h"
#include "pep.h"
#include "colors.h"

// #include <QDebug>

AsmSourceCodePane::AsmSourceCodePane(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::SourceCodePane), currentFile()
{
    ui->setupUi(this);
    connect(ui->textEdit->document(), &QTextDocument::modificationChanged, this, &AsmSourceCodePane::setLabelToModified);

    pepHighlighter = new PepASMHighlighter(PepColors::lightMode, ui->textEdit->document());

    connect(ui->textEdit, &QPlainTextEdit::undoAvailable, this, &AsmSourceCodePane::undoAvailable);
    connect(ui->textEdit, &QPlainTextEdit::redoAvailable, this, &AsmSourceCodePane::redoAvailable);

    ui->label->setFont(QFont(Pep::labelFont, Pep::labelFontSize));
    ui->textEdit->setFont(QFont(Pep::codeFont, Pep::codeFontSize));

    connect(((AsmSourceTextEdit*)ui->textEdit),&AsmSourceTextEdit::breakpointAdded, this, &AsmSourceCodePane::onBreakpointAddedProp);
    connect(((AsmSourceTextEdit*)ui->textEdit),&AsmSourceTextEdit::breakpointRemoved, this, &AsmSourceCodePane::onBreakpointRemovedProp);
}

AsmSourceCodePane::~AsmSourceCodePane()
{
    delete ui;
}

bool AsmSourceCodePane::assemble()
{
    QString sourceLine;
    QString errorString;
    QStringList sourceCodeList;
    AsmCode *code;
    int lineNum = 0;
    bool dotEndDetected = false;

    removeErrorMessages();
    IsaAsm::listOfReferencedSymbols.clear();
    IsaAsm::listOfReferencedSymbolLineNums.clear();
    Pep::memAddrssToAssemblerListing->clear();
    Pep::symbolTable.clear();
    Pep::adjustSymbolValueForBurn.clear();
    Pep::symbolFormat.clear();
    Pep::symbolFormatMultiplier.clear();;
    Pep::symbolTraceList.clear(); // Does this clear the lists within the map?
    Pep::globalStructSymbols.clear();
    Pep::blockSymbols.clear();
    Pep::equateSymbols.clear();
    while (!codeList.isEmpty()) {
        delete codeList.takeFirst();
    }
    //Insert CharIn CharOut
#pragma message ("handle input and output when not using BURN at FFFF")
    Pep::symbolTable.insert("CharIn",MemorySection::getInstance()->getMemoryWord(0xFFF8,false));
    Pep::symbolTable.insert("CharOut",MemorySection::getInstance()->getMemoryWord(0xFFFA,false));
    QString sourceCode = ui->textEdit->toPlainText();
    sourceCodeList = sourceCode.split('\n');
    Pep::byteCount = 0;
    Pep::burnCount = 0;
    while (lineNum < sourceCodeList.size() && !dotEndDetected) {
        sourceLine = sourceCodeList[lineNum];
        if (!IsaAsm::processSourceLine(sourceLine, lineNum, code, errorString, dotEndDetected)) {
            appendMessageInSourceCodePaneAt(lineNum, errorString);
            return false;
        }
        codeList.append(code);
        lineNum++;
    }
    if (!dotEndDetected) {
        errorString = ";ERROR: Missing .END sentinel.";
        appendMessageInSourceCodePaneAt(0, errorString);
        return false;
    }
    if (Pep::byteCount > 65535) {
        errorString = ";ERROR: Object code size too large to fit into memory.";
        appendMessageInSourceCodePaneAt(0, errorString);
        return false;
    }
    for (int i = 0; i < IsaAsm::listOfReferencedSymbols.length(); i++) {
        if (!Pep::symbolTable.contains(IsaAsm::listOfReferencedSymbols[i])
                && !(IsaAsm::listOfReferencedSymbols[i] == "charIn")
                && !(IsaAsm::listOfReferencedSymbols[i] == "charOut")) {
            errorString = ";ERROR: Symbol " + IsaAsm::listOfReferencedSymbols[i] + " is used but not defined.";
            appendMessageInSourceCodePaneAt(IsaAsm::listOfReferencedSymbolLineNums[i], errorString);
            return false;
        }
    }
    Pep::traceTagWarning = false;
    for (int i = 0; i < codeList.size(); i++) {
        if (!codeList[i]->processFormatTraceTags(lineNum, errorString)) {
            appendMessageInSourceCodePaneAt(lineNum, errorString);
            Pep::traceTagWarning = true;
        }
    }
    if (!Pep::traceTagWarning && !(Pep::blockSymbols.isEmpty() && Pep::equateSymbols.isEmpty())) {
        for (int i = 0; i < codeList.size(); i++) {
            if (!codeList[i]->processSymbolTraceTags(lineNum, errorString)) {
                appendMessageInSourceCodePaneAt(lineNum, errorString);
                Pep::traceTagWarning = true;
            }
        }
    }
    return true;
}

QList<int> AsmSourceCodePane::getObjectCode()
{
    objectCode.clear();
    for (int i = 0; i < codeList.size(); ++i) {
        codeList.at(i)->appendObjectCode(objectCode);
    }
    return objectCode;
}

QStringList AsmSourceCodePane::getAssemblerListingList()
{
    assemblerListingList.clear();
    listingTraceList.clear();
    hasCheckBox.clear();
    for (int i = 0; i < codeList.length(); i++) {
        codeList[i]->appendSourceLine(assemblerListingList, listingTraceList, hasCheckBox);
    }
    return assemblerListingList;
}

QStringList AsmSourceCodePane::getListingTraceList()
{
    return listingTraceList;
}

QList<bool> AsmSourceCodePane::getHasCheckBox()
{
    return hasCheckBox;
}

void AsmSourceCodePane::adjustCodeList(int addressDelta)
{
    for (int i = 0; i < codeList.length(); i++) {
        codeList[i]->adjustMemAddress(addressDelta);
    }
}

void AsmSourceCodePane::installOS()
{
#pragma message ("Install OS")
    /*
    for (int i = 0; i < 65536; i++) {
        Sim::Mem[i] = 0;
    }
    int j = Pep::romStartAddress;
    for (int i = 0; i < objectCode.size(); i++) {
        Sim::Mem[j++] = objectCode[i];
    }*/
}

bool AsmSourceCodePane::installDefaultOs()
{
    QString sourceLine;
    QString errorString;
    QStringList sourceCodeList;
    AsmCode *code;
    int lineNum = 0;
    bool dotEndDetected = false;

    IsaAsm::listOfReferencedSymbols.clear();
    Pep::memAddrssToAssemblerListing->clear();
    Pep::symbolTable.clear();
    Pep::adjustSymbolValueForBurn.clear();
    while (!codeList.isEmpty()) {
        delete codeList.takeFirst();
    }
    QString sourceCode = Pep::resToString(":/help/figures/pep9os.pep");
    sourceCodeList = sourceCode.split('\n');
    Pep::byteCount = 0;
    Pep::burnCount = 0;
    while (lineNum < sourceCodeList.size() && !dotEndDetected) {
        sourceLine = sourceCodeList[lineNum];
        if (!IsaAsm::processSourceLine(sourceLine, lineNum, code, errorString, dotEndDetected)) {
            return false;
        }
        codeList.append(code);
        lineNum++;
    }
    if (!dotEndDetected) {
        return false;
    }
    if (Pep::byteCount > 65535) {
        return false;
    }
    for (int i = 0; i < IsaAsm::listOfReferencedSymbols.length(); i++) {
        if (!Pep::symbolTable.contains(IsaAsm::listOfReferencedSymbols[i])) {
            return false;
        }
    }

    if (Pep::burnCount != 1) {
        return false;
    }

    // Adjust for .BURN
    int addressDelta = Pep::dotBurnArgument - Pep::byteCount + 1;
    QMutableMapIterator <QString, int> i(Pep::symbolTable);
    while (i.hasNext()) {
        i.next();
        if (Pep::adjustSymbolValueForBurn.value(i.key())) {
            i.setValue(i.value() + addressDelta);
        }
    }

    adjustCodeList(addressDelta);
    Pep::romStartAddress += addressDelta;
    getObjectCode();
    installOS();

    return true;
}

void AsmSourceCodePane::removeErrorMessages()
{
    QTextCursor cursor(ui->textEdit->document()->find(";ERROR:"));
    while (!cursor.isNull()) {
        cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
        cursor = ui->textEdit->document()->find(";ERROR:", cursor);
    }
    cursor = ui->textEdit->document()->find(";WARNING:");
    while (!cursor.isNull()) {
        cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
        cursor = ui->textEdit->document()->find(";WARNING:", cursor);
    }
}

void AsmSourceCodePane::appendMessageInSourceCodePaneAt(int lineNumber, QString message)
{
    QTextCursor cursor(ui->textEdit->document());
    cursor.setPosition(0);
    for (int i = 0; i < lineNumber; i++) {
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
    cursor.insertText(message);
}

void AsmSourceCodePane::setSourceCodePaneText(QString string)
{
    ui->textEdit->setPlainText(string);
}

void AsmSourceCodePane::clearSourceCode()
{
    ui->textEdit->clear();
    codeList.clear(); // This may cause issues with "format from listing" - but this needs to be cleared regardless.
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
    if(darkMode) pepHighlighter->rebuildHighlightingRules(PepColors::darkMode);
    else pepHighlighter->rebuildHighlightingRules(PepColors::lightMode);
    ((AsmSourceTextEdit*)ui->textEdit)->onDarkModeChanged(darkMode);
    pepHighlighter->rehighlight();
}

void AsmSourceCodePane::onRemoveAllBreakpoints()
{
    ((AsmSourceTextEdit*)ui->textEdit)->onRemoveAllBreakpoints();
}

void AsmSourceCodePane::onBreakpointAdded(quint16 line)
{
    ((AsmSourceTextEdit*)ui->textEdit)->onBreakpointAdded(line);
}

void AsmSourceCodePane::onBreakpointRemoved(quint16 line)
{
    ((AsmSourceTextEdit*)ui->textEdit)->onBreakpointRemoved(line);
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
    emit breakpointAdded(line);
}

void AsmSourceCodePane::onBreakpointRemovedProp(quint16 line)
{
    emit breakpointRemoved(line);
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
    top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    bottom = top + (int) blockBoundingRect(block).height();
    // Store painter's previous Antialiasing hint so it can be restored at the end
    bool antialias = painter.renderHints() & QPainter::Antialiasing;
    painter.setRenderHint(QPainter::Antialiasing, true);
    while (block.isValid() && top < event->rect().bottom()) {
        // If the current block is in the repaint zone
        if (block.isVisible() && bottom >= event->rect().top()) {
            // And if it has a breakpoint
            if(blockToInstr.contains(blockNumber) && breakpoints.contains(blockToInstr[blockNumber])) {
                painter.setPen(PepColors::transparent);
                painter.setBrush(colors.combCircuitRed);          
                painter.drawEllipse(QPoint(fontMetrics().height()/2, top+fontMetrics().height()/2),
                                    fontMetrics().height()/2 -1, fontMetrics().height()/2 -1);
            }
        }
        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
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
    top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    bottom = top + (int) blockBoundingRect(block).height();
    // For each visible block (usually a line), iterate until the current block contains the location of the mouse event.
    while (block.isValid() && top <= event->pos().y()) {
        if (event->pos().y()>=top && event->pos().y()<=bottom) {
            // Check if the clicked line is a code line
            if(blockToInstr.contains(blockNumber)) {
                lineNumber = blockToInstr[blockNumber];
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
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
    update();
}

const QSet<quint16> AsmSourceTextEdit::getBreakpoints() const
{
    return breakpoints;
}

void AsmSourceTextEdit::onRemoveAllBreakpoints()
{
    breakpoints.clear();
    update();
}

void AsmSourceTextEdit::onBreakpointAdded(quint16 line)
{
#pragma message ("TODO: Handle breakpoints being added externally.")
    breakpoints.insert(blockToInstr[line]);
}

void AsmSourceTextEdit::onBreakpointRemoved(quint16 line)
{
#pragma message ("TODO: Handle breakpoints being removed externally.")
    breakpoints.remove(blockToInstr[line]);
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
    QMap<quint16, quint16> oldToNew, old = blockToInstr;
    blockToInstr.clear();
    int cycleNumber = 1;
    QStringList sourceCodeList = toPlainText().split('\n');

    for (int i = 0; i < sourceCodeList.size(); i++) {
        if (QRegExp("^;s*|^\\s*$", Qt::CaseInsensitive).indexIn(sourceCodeList.at(i)) == 0) {
        }
        else {
            blockToInstr.insert(i, cycleNumber++);
        }
    }
    QSet<quint16> toRemove;
    for(auto x : breakpoints) {
        if (blockToInstr.key(x, -1) == -1) toRemove.insert(-1);
    }
    breakpoints.subtract(toRemove);
    update();
}

void AsmSourceTextEdit::resizeEvent(QResizeEvent *evt){

    QPlainTextEdit::resizeEvent(evt);

    QRect cr = contentsRect();
    breakpointArea->setGeometry(QRect(cr.left(), cr.top(), breakpointAreaWidth(), cr.height()));
}
