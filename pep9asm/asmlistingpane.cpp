// File: assemblerlistingpane.cpp
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

#include <QScrollBar>
#include <QFontDialog>
#include "asmlistingpane.h"
#include "ui_asmlistingpane.h"
#include "pep.h"
#include "pepasmhighlighter.h"
#include <QMouseEvent>
#include "asmprogram.h"
#include "colors.h"
#include "symbolentry.h"
#include "symboltable.h"
#include "symbolvalue.h"
AsmListingPane::AsmListingPane(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::AsmListingPane), inDarkMode(false), currentFile()
{
    ui->setupUi(this);

    pepHighlighter = new PepASMHighlighter(PepColors::lightMode, ui->plainTextEdit->document());

    ui->label->setFont(QFont(Pep::labelFont, Pep::labelFontSize));
    ui->plainTextEdit->setFont(QFont(Pep::codeFont, Pep::codeFontSize));
    ui->plainTextEdit->setReadOnly(true);
}

AsmListingPane::~AsmListingPane()
{
    delete ui;
}

void AsmListingPane::setAssemblerListing(QSharedPointer<AsmProgram> program, QSharedPointer<SymbolTable> symTable) {
    clearAssemblerListing();
    ui->plainTextEdit->appendPlainText(program->getProgramListing());
    if(!symTable->getSymbolMap().isEmpty()) {
        ui->plainTextEdit->appendPlainText(symTable->getSymbolTableListing());
    }
    ui->plainTextEdit->verticalScrollBar()->setValue(ui->plainTextEdit->verticalScrollBar()->minimum());
}

void AsmListingPane::clearAssemblerListing()
{
    ui->plainTextEdit->clear();
}

bool AsmListingPane::isModified()
{
    return ui->plainTextEdit->document()->isModified();
}

QString AsmListingPane::toPlainText()
{
    return ui->plainTextEdit->toPlainText();
}

void AsmListingPane::setCurrentFile(QString string)
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

const QFile &AsmListingPane::getCurrentFile() const
{
    return currentFile;
}

void AsmListingPane::highlightOnFocus()
{
    if (ui->plainTextEdit->hasFocus()) {
        ui->label->setAutoFillBackground(true);
    }
    else {
        ui->label->setAutoFillBackground(false);
    }
}

bool AsmListingPane::hasFocus()
{
    return ui->plainTextEdit->hasFocus();
}

void AsmListingPane::copy()
{
    ui->plainTextEdit->copy();
}

void AsmListingPane::setFocus()
{
    ui->plainTextEdit->setFocus();
}

bool AsmListingPane::isEmpty()
{
    return ui->plainTextEdit->toPlainText() == "";
}

void AsmListingPane::rebuildHighlightingRules()
{
    if(inDarkMode) pepHighlighter->rebuildHighlightingRules(PepColors::darkMode);
    else pepHighlighter->rebuildHighlightingRules(PepColors::lightMode);
    pepHighlighter->rehighlight();
}

void AsmListingPane::onFontChanged(QFont font)
{
    ui->plainTextEdit->setFont(font);
}

void AsmListingPane::onDarkModeChanged(bool darkMode)
{
    inDarkMode = darkMode;
    if(darkMode) pepHighlighter->rebuildHighlightingRules(PepColors::darkMode);
    else pepHighlighter->rebuildHighlightingRules(PepColors::lightMode);
    pepHighlighter->rehighlight();
}

void AsmListingPane::mouseReleaseEvent(QMouseEvent *)
{
    ui->plainTextEdit->setFocus();
}

void AsmListingPane::mouseDoubleClickEvent(QMouseEvent *)
{
    emit labelDoubleClicked(Enu::EPane::EListing);
}
