// File: assemblerlistingpane.cpp
/*
    Pep9 is a virtual machine for writing machine language and assembly
    language programs.

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

#include "asmprogramlistingpane.h"
#include "ui_asmprogramlistingpane.h"

#include <QFontDialog>
#include <QMouseEvent>
#include <QScrollBar>

#include "assembler/asmprogram.h"
#include "highlight/pepasmhighlighter.h"
#include "style/colors.h"
#include "style/fonts.h"
#include "symbol/symbolentry.h"
#include "symbol/symboltable.h"
#include "symbol/symbolvalue.h"

AsmProgramListingPane::AsmProgramListingPane(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::AsmProgramListingPane), currentFile(), inDarkMode(false)
{
    ui->setupUi(this);

    pepHighlighter = new PepASMHighlighter(PepColors::lightMode, ui->plainTextEdit->document());

    ui->label->setFont(QFont(PepCore::labelFont, PepCore::labelFontSize));
    ui->plainTextEdit->setFont(QFont(PepCore::codeFont, PepCore::codeFontSize));
    ui->plainTextEdit->setReadOnly(true);
}

AsmProgramListingPane::~AsmProgramListingPane()
{
    delete ui;
}

void AsmProgramListingPane::setAssemblerListing(QSharedPointer<AsmProgram> program, QSharedPointer<SymbolTable> symTable) {
    clearAssemblerListing();
    ui->plainTextEdit->appendPlainText(program->getProgramListing());
    if(!symTable->getSymbolMap().isEmpty()) {
        ui->plainTextEdit->appendPlainText(symTable->getSymbolTableListing());
    }
    ui->plainTextEdit->verticalScrollBar()->setValue(ui->plainTextEdit->verticalScrollBar()->minimum());
}

void AsmProgramListingPane::clearAssemblerListing()
{
    ui->plainTextEdit->clear();
}

bool AsmProgramListingPane::isModified()
{
    return ui->plainTextEdit->document()->isModified();
}

QString AsmProgramListingPane::toPlainText()
{
    return ui->plainTextEdit->toPlainText();
}

void AsmProgramListingPane::setCurrentFile(QString string)
{
    if (!string.isEmpty()) {
        currentFile.setFileName(string);
        ui->label->setText("Assembler Listing - " + QFileInfo(currentFile).fileName());
    }
    else {
        currentFile.setFileName("");
        ui->label->setText("Assembler Listing - untitled.pepl");
    }
}

const QFile &AsmProgramListingPane::getCurrentFile() const
{
    return currentFile;
}

void AsmProgramListingPane::highlightOnFocus()
{
    if (ui->plainTextEdit->hasFocus()) {
        ui->label->setAutoFillBackground(true);
    }
    else {
        ui->label->setAutoFillBackground(false);
    }
}

bool AsmProgramListingPane::hasFocus()
{
    return ui->plainTextEdit->hasFocus();
}

void AsmProgramListingPane::copy()
{
    ui->plainTextEdit->copy();
}

void AsmProgramListingPane::setFocus()
{
    ui->plainTextEdit->setFocus();
}

bool AsmProgramListingPane::isEmpty()
{
    return ui->plainTextEdit->toPlainText() == "";
}

void AsmProgramListingPane::rebuildHighlightingRules()
{
    if(inDarkMode) pepHighlighter->rebuildHighlightingRules(PepColors::darkMode);
    else pepHighlighter->rebuildHighlightingRules(PepColors::lightMode);
    pepHighlighter->rehighlight();
}

void AsmProgramListingPane::onFontChanged(QFont font)
{
    ui->plainTextEdit->setFont(font);
}

void AsmProgramListingPane::onDarkModeChanged(bool darkMode)
{
    inDarkMode = darkMode;
    if(darkMode) pepHighlighter->rebuildHighlightingRules(PepColors::darkMode);
    else pepHighlighter->rebuildHighlightingRules(PepColors::lightMode);
    pepHighlighter->rehighlight();
}

void AsmProgramListingPane::mouseReleaseEvent(QMouseEvent *)
{
    ui->plainTextEdit->setFocus();
}

void AsmProgramListingPane::mouseDoubleClickEvent(QMouseEvent *)
{
    emit labelDoubleClicked(PepCore::EPane::EListing);
}
