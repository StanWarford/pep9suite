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
#include <QSyntaxHighlighter>
#include <QFontDialog>
#include <QKeyEvent>
#include "asmsourcecodepane.h"
#include "ui_asmsourcecodepane.h"
#include "asmcode.h"
#include "pep.h"

// #include <QDebug>

AsmSourceCodePane::AsmSourceCodePane(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::SourceCodePane)
{
    ui->setupUi(this);

    connect(ui->textEdit->document(), SIGNAL(modificationChanged(bool)), this, SLOT(setLabelToModified(bool)));

    pepHighlighter = new PepASMHighlighter(ui->textEdit->document());

    connect(ui->textEdit, SIGNAL(undoAvailable(bool)), this, SIGNAL(undoAvailable(bool)));
    connect(ui->textEdit, SIGNAL(redoAvailable(bool)), this, SIGNAL(redoAvailable(bool)));

    ui->label->setFont(QFont(Pep::labelFont, Pep::labelFontSize));
    ui->textEdit->setFont(QFont(Pep::codeFont, Pep::codeFontSize));
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
    ui->textEdit->setText(string);
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
    ui->label->setText("Source Code - " + string);
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


